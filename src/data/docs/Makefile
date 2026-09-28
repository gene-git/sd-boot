#
# wg_tool:
# src/data/docs/Makefile
#
PKG = sd-boot

SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = .
BUILDDIR      = _build

SOURCES = conf.py ../../version.txt index.rst Readme.rst \
		  Appendix.rst \
		  Arch-Kernels.rst \
		  Configuration.rst \
		  Development.rst \
		  Dracut-Options.rst \
		  Efi-Filesystem-Drivers.rst \
		  Efi-Tools.rst \
		  Layout.rst \
		  Loader-Entries.rst \
		  Signed-Kernels.rst \
		  Skipping-Kernel-Plugins.rst

SOURCES_MAN = ../man/sd-boot-efifs-update.8.rst \
			  ../man/sd-boot-efi-tool-update.8.rst \
			  ../man/sd-boot-find-boot-mounts.8.rst \
			  ../man/sd-boot-kernel-update.8.rst

MAN_READY = ./_build/man-ready/sd-boot-efifs-update.8.rst \
			./_build/man-ready/sd-boot-efi-tool-update.8.rst \
			./_build/man-ready/sd-boot-find-boot-mounts.8.rst \
			./_build/man-ready/sd-boot-kernel-update.8.rst

.PHONY: help Makefile clean html latexpdf

HTML_EXCL   = --exclude _sources/ --exclude .buildinfo  --exclude .doctrees/


html: $(BUILDDIR)/html/index.html
latexpdf: $(PKG).pdf
# man_pages: $(MAN_READY) 

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

$(BUILDDIR)/html/index.html: $(SOURCES) $(MAN_READY)
	@$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS) $(O)
	@$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS) $(O)
	@rsync -a $(HTML_EXCL) _build/html .
	@echo "HTML doc build complete. Path: $@"

$(PKG).pdf: $(SOURCES) $(MAN_READY)
	@$(SPHINXBUILD) -b latex "$(SOURCEDIR)" "$(BUILDDIR)/latex" $(SPHINXOPTS) $(O)
	@echo "Running xelatex ..."
	$(MAKE) -C "$(BUILDDIR)/latex" all-pdf LATEXOPTS="-interaction=nonstopmode" PDFLATEX=xelatex
	$(MAKE) -C "$(BUILDDIR)/latex" all-pdf LATEXOPTS="-interaction=nonstopmode" PDFLATEX=xelatex
	@rsync -av _build/latex/$(PKG).pdf .
	@echo "PDF doc build complete. Path: $@"

_build/man-ready/%.8.rst: ../man/%.8.rst
	@/usr/bin/mkdir -p ./_build/man-ready
	@echo "$<"
	@/usr/bin/grep -Ev '^:Author|^:Manual|^:Date|^:Version' $< > $@

clean:
	@rm -rf $(BUILDDIR)/*
	@rm -rf $(PKG).pdf
	@rm -rf ./html
