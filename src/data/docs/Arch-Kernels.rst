.. _arch_kernels:

Installing Arch Kernels With sd-boot
------------------------------------

By default the Archlinux kernel package, *linux*, is not in the list of kernels managed by sd-boot.

In order for sd-boot to manage it, there are three steps that must be done.

* Add **linux** to the list of sd-boot managed kernels:

    echo linux >> /etc/sd-boot/kernel.packages

* Prevent mkinitcpio from installing a duplicate of the same kernel:

    * Option 1

      Remove mkinicpio package

    * Option 2.
      
      Mask the mkinitcpio hooks.   

      .. code-block:: bash

        sudo mkdir -p /etc/pacman.d/hooks
        sudo ln -s /dev/null /etc/pacman.d/hooks/60-mkinitcpio-remove.hook
        sudo ln -s /dev/null /etc/pacman.d/hooks/90-mkinitcpio-install.hook

* Mask 2 dracut pacman hooks
  
  As of now, the archlinux dracut package incluides two ALPM hooks which can
  cause an arch kernel and initrd files to be instaleld in /boot. This is bad,
  especially since */boot* is hardcoded in the scripts invoked by the hooks.

  Mask these so they do not fire.

  .. code-block:: bash

    sudo mkdir -p /etc/pacman.d/hooks
    sudo ln -sf /dev/null /etc/pacman.d/hooks/90-dracut-install.hook
    sudo ln -sf /dev/null /etc/pacman.d/hooks/60-dracut-remove.hook


In addition please be aware that, as of time of writing,  the two microcode packages

* intel-ucode
* amd-ucode

both intall an image into */boot*. This happends independently of where the ESP is mounted.
It is also legacy and unnecessary, since it is no longer used by anything.
