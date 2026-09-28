.. _arch_kernels:

Installing Arch Kernels With sd-boot
------------------------------------

By default the Archlinux kernel package, *linux*, is not in the list of kernels managed by sd-boot.

In order for sd-boot to manage it, there are two steps that must be done.

* Add **linux** to the list of sd-boot managed kernels::

    echo linux >> /etc/sd-boot/kernel.packages

* Prevent mkinitcpio from installing a duplicate of the same kernel::

    sudo mkdir -p /etc/pacman.d/hooks
    sudo ln -s /dev/null /etc/pacman.d/hooks/60-mkinitcpio-remove.hook
    sudo ln -s /dev/null /etc/pacman.d/hooks/90-mkinitcpio-install.hook


