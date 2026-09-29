Layout: BLS vs UKI
==================

First off, we prefer the *uki* layout. It is simpler, does not require separate loader entry
files, and both kernel and initrd are signed since they are in one file. The *uki* file
itself is an *efi* file and can therefore be directly booted without the need of a boot manager
should that ever be needed.

For these reasons the default *install.conf* file provided by *sd-boot* uses *uki* layout.

With UKI layout kernel image is written to:

.. code-block:: text

    $BOOT/EFI/Linux/<machine-id>-<kernel-version>.efi

Of particular note is that there are no loader entry files with UKI layout.

BLS layout uses three separate files:

.. code-block:: text

    $BOOT/<machine-id>/<kernel-version>/linux
    $BOOT/<machine-id>/<kernel-version>/initrd
    $BOOT/loader/entries/<<machine-id>-<kernel-version>.conf

kernel-install uses settings taken from */usr/lib/kernel* that may be overridden
with files in */etc/kernel*.

sd-boot provides::

    /usr/lib/kernel/install.conf.d/010-sd-boot-install.conf

which contains::

    layout=uki
    initrd_generator=dracut
    uki_generator=ukify

To switch back to BLS from UKI layout create or edit the file::

    /etc/kernel/install.conf

and set *layout=bls*.
Then install the kernel package again.

BLS layout uses *type #1* loader entries. If layout is changed from *bls* back to *uki*
neither the old BLS loader entry files nor the BLS kernel and initrd are automatically removed.
These need to be manually removed.

.. code-block:: text

   rm $BOOT/loader/entries/<<machine-id>-<kernel-version>.conf
   rm -rf $BOOT/<machine-id>/<kernel-version>

While dracut can generate the UKI file without using ukify, this has some limitations.
As of now, I recommend using ukify.

We would like to provide an additional option *OSRelease=* to ukify with a
modified version of os-release having *PRETTY_NAME* and *BUILD_ID*  leading to
a more user friendly boot menu title. This option is available in the ukify
command line tool, but kernel-install does not call this, instead it imports
the ukify python module and calls the functions directly. The kernel-install
code does not provide for the *OSRelease=* option.

At this time, however, there is no clean way to do this that I could find.
Hopefully kernel-install will allow this in the future. In the meantime
the boot menu items in *uki* mode are precise but quite long.

