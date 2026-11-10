// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Helper routines for run_cmd_output - read child process stdout / stderr.
 *
 * read_child_outputs(int out_fd, int err_fd, int pid_fd, int timeout_ms,
 *                    char **output_p, char **error_p)
 *      - Read child stdout and stderr concurrently until EOF on both,
 *        or until the child exits (if pid_fd >= 0).
 *      - A stream is read if its fd >= 0 and its char ** is non-null.
 *      - pid_fd: optional pidfd for the child (-1 if none). When the child
 *        exits, whatever is in the pipes is drained and reading stops, even
 *        if a grandchild (e.g. a daemon it started) still holds the pipe
 *        open - without this we would wait forever for EOF.
 *      - timeout_ms > 0: give up after this long and return RUN_CMD_TIMEOUT.
 *        Output read so far is still returned (useful for logging what hung).
 *        Caller is responsible for killing the child.
 *        timeout_ms <= 0: no timeout.
 *      - Read fds are set O_NONBLOCK so a read() can never block.
 *      - Each requested *xxx_p points to allocated output
 *        (empty string "" if there was none). Caller frees.
 *      - fds are not closed - caller owns them.
 *
 * read_child_output(int fdes, char **output_p)
 *      - Single stream version, no timeout.
 */
#include "sd-boot-cmd.h"
#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

enum ReadConstants {
    CHUNK = 4096,
    NUM_STREAMS = 2,                /* stdout, stderr */
    MAX_POLL_FDS = NUM_STREAMS + 1, /* + pidfd */
    MS_PER_SEC = 1000,
    NS_PER_MS = 1000000,
};

/*
 * Result of one read_chunk()
 * - READ_MORE: got data (or EINTR) - more may be available now
 * - READ_EMPTY: pipe currently empty (EAGAIN) - wait for poll()
 * - READ_DONE: EOF or unrecoverable error
 */
typedef enum {
    READ_MORE,
    READ_EMPTY,
    READ_DONE,
} ReadStatus;

/*
 * One stream being read
 * - done:   EOF or unrecoverable error - stop polling
 * - failed: some error occurred (read or memory)
 */
typedef struct {
    int fdes;
    bool done;
    bool failed;
    Dynamic_str str;
} Stream;

/*
 * State for one read_child_outputs() call
 * - deadline: monotonic ms, 0 = no timeout
 * - polled[i] == nullptr marks the pidfd slot
 */
typedef struct {
    Stream streams[NUM_STREAMS];
    char **dest[NUM_STREAMS];
    int pid_fd;
    int64_t deadline;
    bool timed_out;
    bool child_exited;
} Reader;

/*
 * Make sure there's room for a full CHUNK-sized read.
 * Grow geometrically (double capacity) instead of resizing
 * to exactly what's needed on every iteration - this keeps
 * the total number of realloc()/copy operations to O(log n)
 * rather than O(n / CHUNK).
 */
static int mem_grow_if_needed(Dynamic_str *str) {
    int ret = 0;

    if (str->num_alloc - str->num_used < CHUNK) {
        size_t new_size = str->num_alloc * 2;
        size_t needed = str->num_used + CHUNK;
        if (new_size < needed) {
            new_size = needed;
        }
        ret = dynamic_str_alloc(new_size, str);
        if (ret != 0 && new_size > needed) {
            /*
             * Doubling may have overshot the allocator's size cap
             * even though the actually-needed amount is still
             * within it - fall back to requesting just that.
             */
            ret = dynamic_str_alloc(needed, str);
        }
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

/*
 * Do one read() on a stream.
 * fd is O_NONBLOCK so this never blocks.
 *
 * If memory can't grow (alloc failure or MEM_MAX cap) we keep reading
 * but discard the data: the child must never block on a full pipe,
 * or it never exits and we'd hang in waitpid().
 */
static ReadStatus read_chunk(Stream *stream) {
    char scratch[CHUNK];
    char *ptr = scratch;

    if (!stream->failed) {
        if (mem_grow_if_needed(&stream->str) == 0) {
            ptr = stream->str.bytes + stream->str.num_used;
        } else {
            stream->failed = true;
        }
    }

    ssize_t bytes_read = read(stream->fdes, ptr, CHUNK);

    if (bytes_read < 0) {
        int err = errno;
        if (err == EINTR) {
            return READ_MORE;
        }
        if (err == EAGAIN || err == EWOULDBLOCK) {
            return READ_EMPTY;
        }
        perror("Error reading child pipe");
        stream->failed = true;
        stream->done = true;
        return READ_DONE;
    }

    if (bytes_read == 0) {
        /*
         * EOF
         */
        stream->done = true;
        return READ_DONE;
    }

    if (ptr != scratch) {
        stream->str.num_used += (size_t)bytes_read;
    }
    return READ_MORE;
}

/*
 * Child has exited: read whatever is already in the pipe, then stop.
 * Anything written later (by a grandchild still holding the pipe) is ignored.
 */
static void drain_stream(Stream *stream) {
    while (!stream->done && read_chunk(stream) == READ_MORE) {
    }
    stream->done = true;
}

/*
 * Resize down, null terminate, and hand buffer to caller.
 * If there was no output, *output_p is an allocated empty string "".
 */
static int stream_finish(Stream *stream, char **output_p) {
    Dynamic_str *str = &stream->str;

    if (dynamic_str_alloc(str->num_used + 1, str) != 0) {
        return -1;
    }
    str->bytes[str->num_used] = '\0';

    *output_p = str->bytes;
    str->bytes = nullptr;
    str->num_alloc = 0;
    str->num_used = 0;
    return 0;
}

/*
 * Monotonic clock in milliseconds - unaffected by wall clock changes.
 */
static int64_t now_ms() {
    struct timespec now = {};
    (void)clock_gettime(CLOCK_MONOTONIC, &now);
    return ((int64_t)now.tv_sec * MS_PER_SEC) + (now.tv_nsec / NS_PER_MS);
}

/*
 * Milliseconds left until deadline, suitable for poll().
 * - deadline 0: no timeout -> -1 (poll waits forever)
 * - expired: 0
 */
static int poll_wait_ms(int64_t deadline) {
    if (deadline == 0) {
        return -1;
    }
    int64_t left = deadline - now_ms();
    if (left <= 0) {
        return 0;
    }
    return left > INT_MAX ? INT_MAX : (int)left;
}

static int set_nonblock(int fdes) {
    int flags = fcntl(fdes, F_GETFL);
    if (flags < 0) {
        perror("fcntl F_GETFL");
        return -1;
    }
    unsigned int new_flags = (unsigned int)flags | (unsigned int)O_NONBLOCK;
    if (fcntl(fdes, F_SETFL, (int)new_flags) < 0) {
        perror("fcntl O_NONBLOCK");
        return -1;
    }
    return 0;
}

static bool revents_has(short revents, short flag) {
    return ((unsigned int)revents & (unsigned int)flag) != 0U;
}

/*
 * Set up streams to read. Caller's output pointers are nulled first so
 * cleanup never frees something we didn't allocate.
 */
static int reader_init(Reader *reader, const int fds[NUM_STREAMS], int pid_fd, int timeout_ms) {
    bool any = false;

    for (size_t i = 0; i < NUM_STREAMS; i++) {
        reader->streams[i] = (Stream){.fdes = -1, .done = true};
        if (reader->dest[i]) {
            *reader->dest[i] = nullptr;
        }
    }
    reader->pid_fd = pid_fd;
    reader->deadline = timeout_ms > 0 ? now_ms() + timeout_ms : 0;

    for (size_t i = 0; i < NUM_STREAMS; i++) {
        if (!reader->dest[i] || fds[i] < 0) {
            continue;
        }
        if (set_nonblock(fds[i]) != 0) {
            return -1;
        }
        reader->streams[i].fdes = fds[i];
        reader->streams[i].done = false;
        any = true;
    }

    if (!any) {
        msg(MSG_ERR, "  ! read_child: nothing to read\n");
        return -1;
    }
    return 0;
}

static bool reader_active(const Reader *reader) {
    for (size_t i = 0; i < NUM_STREAMS; i++) {
        if (!reader->streams[i].done) {
            return true;
        }
    }
    return false;
}

/*
 * Fill pollfd array: active streams, then pidfd (polled[] slot nullptr).
 */
static nfds_t reader_pollfds(Reader *reader, struct pollfd pfds[MAX_POLL_FDS], Stream *polled[MAX_POLL_FDS]) {
    nfds_t nfds = 0;

    for (size_t i = 0; i < NUM_STREAMS; i++) {
        if (!reader->streams[i].done) {
            pfds[nfds] = (struct pollfd){.fd = reader->streams[i].fdes, .events = POLLIN};
            polled[nfds] = &reader->streams[i];
            nfds++;
        }
    }
    if (reader->pid_fd >= 0) {
        pfds[nfds] = (struct pollfd){.fd = reader->pid_fd, .events = POLLIN};
        polled[nfds] = nullptr;
        nfds++;
    }
    return nfds;
}

/*
 * Service fds that poll() reported ready.
 * POLLIN, POLLHUP or POLLERR on a pipe: read() tells us which.
 * POLLHUP may still have buffered data - read until EOF.
 */
static void reader_handle_ready(Reader *reader, const struct pollfd *pfds, Stream *const *polled,
        nfds_t nfds) {
    for (nfds_t i = 0; i < nfds; i++) {
        if (pfds[i].revents == 0) {
            continue;
        }
        if (!polled[i]) {
            reader->child_exited = true;
            continue;
        }
        if (revents_has(pfds[i].revents, POLLNVAL)) {
            polled[i]->failed = true;
            polled[i]->done = true;
            continue;
        }
        (void)read_chunk(polled[i]);
    }

    /*
     * Child exited (or pidfd error) - collect what's left and stop.
     */
    if (reader->child_exited) {
        for (size_t i = 0; i < NUM_STREAMS; i++) {
            drain_stream(&reader->streams[i]);
        }
    }
}

/*
 * One poll() round. Returns 0 to keep going, -1 on error.
 */
static int reader_poll_once(Reader *reader) {
    struct pollfd pfds[MAX_POLL_FDS] = {};
    Stream *polled[MAX_POLL_FDS] = {};
    nfds_t nfds = reader_pollfds(reader, pfds, polled);

    int wait_ms = poll_wait_ms(reader->deadline);
    if (wait_ms == 0) {
        reader->timed_out = true;
        return 0;
    }

    int num_ready = poll(pfds, nfds, wait_ms);
    if (num_ready < 0) {
        if (errno == EINTR) {
            return 0;
        }
        perror("Error polling child pipes");
        return -1;
    }
    if (num_ready > 0) {
        reader_handle_ready(reader, pfds, polled, nfds);
    }
    return 0;
}

/*
 * Hand results to caller - all or nothing.
 * On timeout, partial output is still returned.
 */
static int reader_results(Reader *reader) {
    for (size_t i = 0; i < NUM_STREAMS; i++) {
        if (reader->streams[i].fdes >= 0 && reader->streams[i].failed) {
            return -1;
        }
    }
    for (size_t i = 0; i < NUM_STREAMS; i++) {
        if (reader->dest[i] && reader->streams[i].fdes >= 0 &&
                stream_finish(&reader->streams[i], reader->dest[i]) != 0) {
            return -1;
        }
    }
    return reader->timed_out ? RUN_CMD_TIMEOUT : 0;
}

/*
 * On error, free anything already handed to caller.
 */
static void reader_cleanup(Reader *reader, int ret) {
    for (size_t i = 0; i < NUM_STREAMS; i++) {
        dynamic_str_free(&reader->streams[i].str);
    }

    if (ret == 0 || ret == RUN_CMD_TIMEOUT) {
        return;
    }
    for (size_t i = 0; i < NUM_STREAMS; i++) {
        if (reader->dest[i] && *reader->dest[i]) {
            free(*reader->dest[i]);
            *reader->dest[i] = nullptr;
        }
    }
}

int read_child_outputs(int out_fd, int err_fd, int pid_fd, int timeout_ms, char **output_p, char **error_p) {
    /*
     * Read stdout and stderr concurrently via poll().
     * Reading one to EOF before the other can deadlock: if the child
     * fills the other pipe (~64 KiB) it blocks and never closes the first.
     */
    const int fds[NUM_STREAMS] = {out_fd, err_fd};
    Reader reader = {.dest = {output_p, error_p}};

    int ret = reader_init(&reader, fds, pid_fd, timeout_ms);
    while (ret == 0 && reader_active(&reader) && !reader.timed_out) {
        ret = reader_poll_once(&reader);
    }
    if (ret == 0) {
        ret = reader_results(&reader);
    }
    reader_cleanup(&reader, ret);
    return ret;
}

int read_child_output(int fdes, char **output_p) {
    if (!output_p) {
        msg(MSG_ERR, "  ! read_child: bad output ptr\n");
        return -1;
    }
    return read_child_outputs(fdes, -1, -1, 0, output_p, nullptr);
}
