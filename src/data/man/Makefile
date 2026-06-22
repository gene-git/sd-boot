#
# Man pages.
# rst2man provided by python-docutils
#

MAN_PAGES = man8/sd-boot-efifs-update.8.gz 
MAN_PAGES += man8/sd-boot-find-boot-mounts.8.gz 
MAN_PAGES += man8/sd-boot-efi-tool-update.8.gz 
MAN_PAGES += man8/sd-boot-kernel-update.8.gz

VERSION = $(shell cat ../../version.txt)
DATE = $(shell date +'%B %Y')

default: $(MAN_PAGES)

man8/%.8.gz: %.8.rst
	@echo "$<"
	@/usr/bin/sed -e "s/@VERSION@/$(VERSION)/g" -e "s/@DATE@/$(DATE)/g" $< \
        | rst2man $(OPTS) | /usr/bin/gzip > $@

clean:
	rm -f $(MAN_PAGES)

.PHONY: default clean
