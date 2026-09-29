Dracut options
--------------

sd-boot provides a default dracut config file:

.. code-block:: text

   /usr/lib/dracut/dracut.conf.d/010-sd-boot-dracut.conf

If changes are needed, then overrides can be provided as usual using a drop in file::

    /etc/dracut.conf.d/xxx.conf

since entries found in */etc*/ take precedence over */usr/lib/* as usual.
Files within each directory are processed in alphanumeric order. For example a file called 020-dracut.conf
will override any settings 010-dracut.conf in the same directory.
The last file read by dracut sets the options used.

This treatment of configuration file precedence is standard on linux, and kernel-install follows the
same rules for it's configuration files.


