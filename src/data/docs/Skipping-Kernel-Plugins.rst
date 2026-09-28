Skipping Kernel Plugins
=======================

kernel-install, by default, calls every plugin.
There may be times when there's a need to de-activate a plugin. This is accomplished using
a setting in sd-boot config file::

    /etc/sd-boot/config.yaml

Set *skip_kernel_plugins* to  the list of plugins to be skippedi.
Each must be a full pathname::

    skip_kernel_plugins:
        - /usr/lib/kernel/install.d/91-xxx.install
        - /usr/lib/kernel/install.d/91-yyy.install

sd-boot effects this by passing the plugin list to be used to kernel-install using the environment variable, 
KERNEL_INSTALL_PLUGINS.

This environment variable contains the list of all plugins to be invoked by kernel-install.
The list excludes any of those configured as above to be skipped.

