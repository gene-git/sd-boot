Boot Loader Entries
-------------------

There are two kinds of systemd boot loader entries that show up in the boot menu.

- **type #1** 
  
  - Uses a separate *loader entry file* 
  
  - Used when there are separate kernel and initrd images or for efi tools.

- **type #2**

  - Used with UKI images only.

    UKI images have the kernel and initrd combined into a single file where 
    the UKI file itself is sufficient to provide the boot loader menu without
    any additional loader entry file.

Since sd-boot uses *kernel-install* to do the real work, type #1 boot loader entries are
automatically created with *bls* layout. It creates one new entry for each kernel version.

After using sd-boot the very first time (only), you should remove any stale (fixed) loader 
entries from */boot/loader/entries* or */efi/loader/entries* (if that's where they are) for 
any kernel package now managed by sd-boot.

sd-boot also provides a kernel-install plugin that modifies the raw type#1 loader entries.
These are generated for efi tools and for kernels using *bls* layout.

The primary difference is the *Title* which is modified to be the package name.
The title is shown in the boot menu.

By default the title is *Arch Linux* which comes from */etc/os-release* for every
kernel and efi tool.

efi-tools additionally remove the kernel boot command line options from the entry
(since they are irrelevant) and change::

   linux <tool>.efi

to::

   efi <tool>.efi


It also means that the systemd-boot *loader.conf* file located in the *EFI*
needs to be updated as well when the default kernel to boot is one of those managed by sd-boot.

Lets show how we can use this so that a *linux-custom* kernel is booted by default.

The loader entries are located in::

   <EFI>/loader/entries/<machine-id>-<kernel-version>.conf
    
for example::

   <EFI>/loader/entries/<machine-id>-7.2.0-custom-1.conf

We would match this entry in *loader.conf* to be the default kernel using::

    # loader.conf
    default *-custom-*
    timeout 5
    editor  yes

The wildcards are standard shell file globbing used to match the loader entry filename.
AFter making this change, its helpful to verify everything is correct by running:

.. code-block:: bash

   bootctl
   bootctl list

For *uki* layout there are no loader entry files but the comments on *loader.conf* apply
similarly. The *loader.conf* example should work fine for BLS and UKI layouts.

