#
# Man pages.
# rst2man provided by python-docutils
#

MAN_PAGES = sd-boot-efifs-update.8.gz 
MAN_PAGES += sd-boot-find-boot-mounts.8.gz 
MAN_PAGES += sd-boot-efi-tool-update.8.gz 
MAN_PAGES += sd-boot-kernel-update.8.gz

VERSION = $(shell cat ../../version.txt)
DATE = $(shell date +'%B %Y')

default: $(MAN_PAGES)

OPTS = --strict --no-raw --no-generator

%.8.gz: %.rst
	@echo "$<"
	@/usr/bin/sed -e "s/@VERSION@/$(VERSION)/g" -e "s/@DATE@/$(DATE)/g" $< \
		| rst2man $(OPTS) | /usr/bin/gzip > man8/$@

clean:
	rm -f $(MAN_PAGES)

.PHONY: default clean
