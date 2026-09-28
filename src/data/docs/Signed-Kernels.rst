Signing Kernels & Secure Boot
=============================

Kernels will be signed if keys are available thanks to kernel-install and sbctl.
Many thanks to Foxboron for writing sbctl which simplifies things enormously.

This is a very brief overview.
Please see Arch wiki and man pages for details on secure boot and kernel signing.

Signing Kernel
--------------

The sbctl package must be installed.

To have the kernels signed by local keys, first create the keys:

.. code-block:: bash

   sbctl create-keys

You may be prompted to *migrate* an older install, if so then do it:


.. code-block:: bash

   sbctl setup --migrate

Thats all that's required. This will sign the kernel image. Note that if the
layout is *bls* then the initrd is separate and unsigned.

You may want to change to UKI where the initrd and the kernel image are packaged into
a single file which gets signed. To change the layout simply edit
*/etc/kernel/install.conf* and change the layout to uki:


.. code-block:: bash

   layout=uki
   initrd_generator=dracut
   uki_generator=ukify

That's all that's required to have a unified kernel image signed by the local keys.

Signing BLS vs UKI
------------------

When signing a kernel, the preferred way is to use uki layout as the initrd is part of
the uki image that gets signed. In bls layout, the kernel is signed but the initrd
is not.

When using bls layout, the kernel signing is handled by the sbct plugin, */usr/lib/91-sbctl.install*.
When using uku layout, signing of the uki image is handled by ukify.

Best I can tell, the sbctl plugin does not to attempt to sign the image a second time. However,
being cautious, sd-boot will de-activate the sbctl plugin in this case to eliminate any possibility
of a second signing happening.


Secure Boot
-----------

**Big caveat**. Per the wiki, using local keys can lead to significant problems and may even break
the machine.

To use secure boot, the keys must be enrolled and secure boot activated in the UEFI Bios.

To enroll the signing keys the bios needs to have secure boot set to *setup* mode and then
boot up the machine and use *sbctl* to enroll the keys.

.. code-block:: bash

   sbctl enroll-keys --microsoft

At this point the machine is in secure boot mode and will only boot signed kernels.


