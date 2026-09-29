========
Appendix
========

.. _build_options:

Compile and Loader Options
==========================

Compilers provide options that can enhance the safety of the compiled code.
These are the options currently used to build sd-boot.

**Compile Options**:

.. tabularcolumns:: |>{\centering\arraybackslash}\X{35}{100}|>{\raggedright\arraybackslash}\X{65}{100}|

.. list-table::
   :widths: 35 65
   :stub-columns: 1

   * - -pipe
     - Speeds up compilation
   * - -fno-plt Optimization             
     - for shared library calls
   * - -Wmissing-prototypes              
     - Warn for global function missing prototype decl
   * - -fvisibility=hidden               
     - Only exported symbols in shared lib sym table
   * - -fstack-protector-strong          
     - Protect function calls with stack canaries
   * - -fstack-clash-protection          
     - Prevents stack-clash exploits
   * - -ftrivial-auto-var-init=zero      
     - Eliminate uninitialized stack memory leaks
   * - -fzero-call-used-regs=used-gpr    
     - Wipe general registers before returning
   * - -mshstk                           
     - Intel Shadow Stack protection
   * - -fcf-protection                   
     - Control-flow enforcement

**Link Options**:

.. tabularcolumns:: |>{\centering\arraybackslash}\X{35}{100}|>{\raggedright\arraybackslash}\X{65}{100}|

.. list-table::
   :widths: 35 65
   :stub-columns: 1

   * - -Wl,--as-needed                   
     - Limit shared to 'as needed'
   * - -Wl,-z,relro                      
     - Mark relocation tables read-only
   * - -Wl,-z,now                        
     - Force immediate binding (Full RELRO)
   * - -Wl,-z,noexecstack                
     - Strictly enforce non-executable stack
   * - -Wl,-z,pack-relative-relocs       
     - Optimize layout with DT_RELR packing
   * - -Wl,-z,defs                       
     - Catch unresolved symbols at link time, not at runtime

The test suite is also run using code compiled with *-fanalyzer* as 
well as *-fsanitize=address,undefined* and no warnings or errors are found.

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

