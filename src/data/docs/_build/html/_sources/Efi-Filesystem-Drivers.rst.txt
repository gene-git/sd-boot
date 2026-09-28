Efi Filesystem Drivers
----------------------

While the *EFI* is always Fat-32, a */boot* partition may use other filesystems.
In this case the boot loader requires efi filesystem drivers that provided by the *efifs* package.

When this package is installed the driver files are located in the directory */usr/lib/efifs-x86/*,
sd-boot detects this, via an alpm hook, and installs them into::

    <EFI>/EFI/systemd/drivers/

where systemd-boot expects to find them.

These can also be manually installed (or removed) from the command line using::

   /usr/lib/sd-boot/sd-boot-efifs-update add

or::

   /usr/lib/sd-boot/sd-boot-efifs-update remove



