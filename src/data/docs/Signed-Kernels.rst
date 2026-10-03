=============================
Signing Kernels & Secure Boot
=============================

Kernels will be signed if keys are available thanks to kernel-install and sbctl.
Many thanks to Foxboron for writing sbctl which simplifies things enormously.

This is a very brief overview.
Please see Arch wiki and man pages for details on secure boot and kernel signing.

Overview
========

This note is purposefully brief and incomplete but provides
some background around this somewhat large and complex topic.
For simplicity and because it is the current desirable approach I will 
focus on UKI and not BLS layout here.

Signing a kernel provides a way to be sure that it has not changed after the
signature was created. Signatures are stored to the TPM hardware from where 
they can be read during the boot process. There are several slots available
and specific slots are designated for specific purposes.

To skip to the section on configuring systemd-tpm2-setup, see: :ref:`systemd_tpm2`.

Secure Boot On
--------------

PCR slot 7 holds the UEFI secure boot state along with the valid certificate
database, *db*. It also holds an ever growing list of invalid certificates, *dbx*.
Some machines may even no longer have enough space to hold *dbx*. 

PCR 7 is required by secure boot. Only a UKI that can be validated by the certificates
in slot 7 are permitted to boot by the UEFI firmware.

That is all that is required for secure boot.

Systemd-TPM2-Setup
------------------

Systemd provides additional useful checks on the operating system 
via *systemd-tpm2-setup-early* and *systemd-tpm2-setup* services. 
This is most useful when secure boot is disabled. It's pretty 
benign and redundant when using secure boot.

To take advantage of these, *ukify* is used to provide keys and sign the hashes
of the initrd, the kernel image, the kernel cmdline, any embedded OS-release metadata, 
splash screen image and, importantly, the systemd-stub. 

At boot time systemd-stub, computes the hash of the same set of inidividual hashes 
and pushes it down to PCR-11.  A little later in the boot cycle, systemd extracts the 
public key from the UKI along with the signed hash. It then uses the public key to verify 
the signature is authentic and that the hash inside the signature matches the value in PCR-11. 
If they match then all is well and the systemd service exits with success. If not, it exits with a fail.

So that's pretty useful. If anything changed, systemd advertizes the failure during boot
and dmesg.

Hardware
--------

There are presently two versions of hardware that can be used to store
signatures on the motherboard. TPM version 1.2 and TPM version 2.0. The major
difference between these is that 1.2 chips only support SHA-1, while 2.0 chips support SHA-256.
Since (some) signatures now use SHA-256, these are not compatible with TPM 1.2.

To check which is available. For TPM2::

    bootctl | head -7 | tail -3

    TPM2 Support: yes
    Measured UKI: yes
    Measured OS: yes

whereas for version 1.2 they result shows three *no*'s.


What To Do
==========

Armed with the background on kernel signatures, here are some things
that may be helpful. 

There are two:

* distinct validation / signatures 
* sets of tools to create the keys / signatures
* target systems that validate them.

.. tabularcolumns:: |p{0.2\linewidth}|p{0.3\linewidth}|p{0.3\linewidth}|

.. raw:: latex

   \begingroup\footnotesize  % <-- Options: \small, \footnotesize, \scriptsize, \

.. list-table:: Kernel Signing
   :header-rows: 1

   * - **What** 
     - **Secure Boot (sbctl --create-keys)**
     - **Systemd OS (ukify genkeys)**
   * - does the Validation
     - UEFI Firmware
     - Systemd / Kernel (OS)
   * - is Validated
     - Validates the UKI
     - Validates the state of the OS

.. raw:: latex

   \endgroup

Secure Boot and sbctl
---------------------

The sbctl package must be installed.

To have the kernels signed by local keys, first create the keys:

.. code-block:: bash

   sbctl create-keys

You may be prompted to *migrate* an older install, if so then do it:

.. code-block:: bash

   sbctl setup --migrate

With the keys in place sbctl will sign the kernel UKI image. Note that if the
layout is *bls* then the initrd is separate and unsigned.

You may want to change to UKI where the initrd and the kernel image are packaged into
a single file which gets signed. 
To change the layout please see the :ref:`layout` section of the manual

That's all that's required to have a unified kernel image signed by the local keys.

Signing BLS vs UKI
^^^^^^^^^^^^^^^^^^

When it comes to kernel signing, the stronly preferred way is with uki layout. The initrd is part of
the uki image that gets signed. In bls layout, the ony kernel is signed while neither the initrd
nor the kernel command line are signed.

With BLS layout, the kernel signing is handled by the sbctl plugin, */usr/lib/91-sbctl.install*.
With UKI layout, ukify handles signing the UKI image.

Best I can tell, the sbctl plugin does not to attempt to double sign the image. However,
being cautious, sd-boot de-activates the sbctl plugin in this case to eliminate any possibility
of a second signature.

Activating Secure Boot
^^^^^^^^^^^^^^^^^^^^^^

**Big caveat**. Per the wiki, using local keys can lead to significant problems and may even break
the machine.

To use secure boot, the keys must be enrolled and secure boot activated in the UEFI Bios.

To enroll the signing keys the bios needs to have secure boot set to *setup* mode and then
boot up the machine and use *sbctl* to enroll the keys.

.. code-block:: bash

   sbctl enroll-keys --microsoft

At this point the machine is in secure boot mode and will only boot signed kernels.


.. _systemd_tpm2:

Systemd-TPM2-Setup
------------------

When booting a UKI image with TPM2 hardware where systemd-tpm2-setup has not been configured, 
a warning is logged::

    [FAILED] Failed to start Early TPM SRK Setup

There are three ways to approach this:

* Ignore it since its benign
* Turn off the systemd service 
* Configure the service 

Turn Off the Service
^^^^^^^^^^^^^^^^^^^^

Simply mask the two services:

.. code-block:: bash

   systemctl mask systemd-tpm2-setup-early.service systemd-tpm2-setup.service

Configure the Service
^^^^^^^^^^^^^^^^^^^^^

All that's needed to configure this is to generate a key pair
and configure ukify to use them to sign the hashed OS components and rebuild the UKI image(s).

This is the correct way to do this:

Configure ukify::

    cp /usr/lib/kernel/uki.conf /etc/kernel/uki.conf

then edit */etc/kernel/uki.conf* and change [PCRSignature:NAME] to [PCRSignature:all] and 
uncomment the three key lines::

    [PCRSignature:all]
    PCRPrivateKey=/etc/systemd/tpm2-pcr-private-key.pem
    PCRPublicKey=/etc/systemd/tpm2-pcr-public-key.pem

Next, generate the key pair::

    ukify genkey --config=/etc/kernel/uki.conf


Lastly, rebuild the UKI image for each kernel::

   sd-boot-kernel-update add linux-customr
   sd-boot-kernel-update add linux-stable
   ...

You'll see in the output something similar to::

    /usr/lib/systemd/systemd-measure sign ...
      --pcrpkey=/etc/systemd/tpm2-pcr-public-key.pem  ...
      --private-key=/etc/systemd/tpm2-pcr-private-key.pem
      --public-key=/etc/systemd/tpm2-pcr-public-key.pem ...

    Signing kernel /boot/EFI/Linux/<machine-id>-<kernel-version>.efi
    sd-boot: Completed kernel update linux-custom

You can verify this worked on the new UKI image path shown using::

    # uki='/boot/EFI/Linux/<machine-id>-<kernel-version>.efi'
    # ukify inspect "$uki" | grep -E -A2 '\.(pcrpkey|pcrsig)'
    .pcrpkey:
      size: 451 bytes
      sha256: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    --
    .pcrsig:
      size: 8398 bytes
      sha256: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

Of course use the real path of the UKI.

Next time this kernel is booted the the fail log will be replaced with::

    Starting TPM SRK Setup...


Please note that while boot times are essentially unchanged, it does
take a little longer to build the UKI due to the additional time needed 
to compute the various hashes.

