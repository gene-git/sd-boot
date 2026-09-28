
===========
Development
===========

C-Code Language Choice
======================

While I was tempted to do this in python, partly because systemd chose to use it for
*/usr/lib/kernel/install.d/60-ukify.install*, I decided to use C.

It's important to be able to do initial testing and validation as non-root user
and of course without touching any important files in any actual root directory.

While the the C-code version is a bit more work and a little more complicated to create,
the benefit, in my view, is that the c-code is much easier to test and debug
and keep organized than bash version.

C also has the advantage that the compiled code has almost no dependencies and
is highly performant. Way faster than bash or python. For a system utility this
is a pretty nice benefit.

Testing C-Code
==============

Testing and development are done using a *Testing* directory that is writable by non-root user.
This is where kernels will be installed, loader entries updated and so on. The tools
are installed into this tree and are statically linked.

Otherwise the environment is kept clean other than the *PATH* variable since dracut requires it.

This allows all programs to be tested by leveraging kernel-install's ability to work
completely in the testing root tree.

The environment variable *SDB_DEV_TEST* activates the test usage, allowing
testing and debugging to be done completely within the non-root test tree.

There are two test sets provided under *src/c-code/scripts*. Both test sets
should be run from the *src* directory.

* ./tests/scripts/run-test-suite

  This runs the the tools with *root* set to *Testing/__root__*.
  This is run as part of the build check process.
  It also runs the tools under valgrind.

* ./tests/scripts/static-analysis:

  It runs static code analysis using cppcheck and clang-tidy.
  Note that kernel-install always runs *chown* and since all tests are run as ordinary
  (non-root) user this leads to some warning messages landing in the testing logs.
  These are benign as kernel-install ignores them. They dont happen in production
  where kernel-install is running as root and chown is permitted.

  For each of these tests, the logs save stdout, stderr and the exit status of the tool
  along with the output from valgrind.

The image and initrd files will be installed in::

    Testing/__root__/boot/<machine-id>/<kernel-version>/


while the loader entry files, for efi tools and kernels in *bls* layout will be in::

    Testing/__root__/boot/loader/entries/<machine-id>-<kernel-version>.conf

You may notice that the loader entries have a longer path to the kernel image and initrd.

This is normal.

When run in a test tree, kernel-install identifies the *mount* point and removes it from
the front of the path. In test mode where the test directory is not actually a mount point
(such as /boot) the pathname written to the loader entry will include more elements.
This is fine and when run in producion the image file will simply be *linux* instead of
*/long/path/to/linux*. The same is true for the initrd file as well.

Development and Debugging
=========================

Debuggable executables can be installed into the test root directory.
From the *src* directory:

.. code-block:: bash

   ./do-dev-build
   cd tests/Testing
   export SDB_DEV_TEST=true

Then executables can now be debugged using, for example:

.. code-block:: bash

   gdb ./__root__/usr/bin/sd-boot-kernel-update

And for this executable for example, within gdb use::

    run add linux

Plugins are called by kernel-install but may be run manually as well:

* ./plugin-tests/run-loaderentry-efi (for bls plugin-tests/run-loaderentry-kernel)

  Standalone tests of the plugin that modify the raw loader entry files.
  The scripts run the tests under valgrind, but there is an option
  to run in the debugger as well.



