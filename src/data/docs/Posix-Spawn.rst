
Posix Spawn
===========

We chose to use posix_spawn() to run executables as a child process.
The alternative is the bastion fork/exec approach.

Why sd-boot uses it over fork/exec
----------------------------------

**Speed and memory**

glibc runs posix_spawn with clone(CLONE_VM | CLONE_VFORK), so the parent’s page tables
are never copied. 

fork() does copy the page tables, and in a large process under strict memory accounting it 
could fail with ENOMEM even though the child is about to exec anyway. Nothing in sd-boot
is especially memory intensive, still it is the better option.

**Exec errors come back to the caller**

In more recent glibc (2.24+), if the exec fails (missing binary, no permission, etc), posix_spawn returns the 
error to the parent. For example, nonexistent: No such file or directory can show up without any additional
code.

With fork/exec, the child has to send errno back over a close-on-exec pipe. This means more code to get right.

**Safe when using threads**

sd-boot does not currently use threads, but that option is open wit posix_spawn().

After a fork() in a threaded program, the child may only call async-signal-safe functions until it calls exec. 
A malloc or printf there could deadlock on a lock another thread held at fork time for example.

posix_spawn avoids that all of that and any bugs that come along as a result.

**Fewer footguns**

There’s no forked copy of the parent process that could flush stdio buffers twice, run atexit 
handlers, or return into parent code by mistake. It is inherently safer.

**It covers everything sd-boot uses**

sd-boot does not need anything offered by fork/exec that is not currently availably via
posix_spawn().

From redirecting stdin/stdout/stderr, a new process group, child process’s signal mask, and resetting 
signals to their defaults. And, those steps are rin in the right order between the clone and the exec, 
without hand-writing anything.

What fork/exec does that posix_spawn does not
---------------------------------------------

If ever need to run a command inside a new namespace, then would need
fork/exec. As of today the items below are not available with posix_spawn():

* setrlimit
* umask
* prctl(PR_SET_PDEATHSIG)
* unshare()/namespaces
* chroot
* dropping privileges

If in the future there's a need to drop privileges, then an alternative to fork/exec 
would be to spawn setpriv using something like:

.. code-block:: c

   char *argv[] = {
       "/usr/bin/setpriv",
       "--reuid=nobody", "--regid=nobody", "--clear-groups",
       "--inh-caps=-all", "--bounding-set=-all", "--no-new-privs",
       "--", "/usr/bin/real-command", "arg", nullptr,
   };

setpriv is available in the core util-linux package.
