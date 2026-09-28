========
Appendix
========

Potential Todo Items
====================

* When switching layout from bls to uki, previous kernel is not removed
  since it was installed using bls layout while *kernel-install remove*
  is now using uki layout. This leaves the previous bls kernel install instead of
  removing it. Same would be true switching layout from uki back to bls.

  This means, at least for now, that manual intervention is necessary after changing layout
  to avoid leaving un-needed files. While it is benign they do take disk space.

  For example after changing to uki layout, check */boot/loader/entries* and
  */boot/<machine-id>/* and remove the older kernel(s) that are no longer needed.

  In uki mode there are no loader entry files at all and kernels are installed
  in */boot/EFI/Linux* not in */boot/<machine-id>*.

  Here */boot* means either */boot* or */efi* as appropriate.

* So, it might be good (nice to have) for the code be more helpful with this.

