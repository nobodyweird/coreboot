##
## This file is part of the coreboot project.
##
## Copyright (C) 2015 Google Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##

# 2016-1 tag
STABLE_COMMIT_ID=fa85e826c16b9ce1ad302a57e9c4b24db0d8b930

TAG-$(CONFIG_UBOOT_MASTER)=origin/master
TAG-$(CONFIG_UBOOT_STABLE)=$(STABLE_COMMIT_ID)

project_name=U-Boot
project_dir=u-boot
project_git_repo=http://git.denx.de/u-boot.git
project_config_file=$(project_dir)/.config

unexport KCONFIG_AUTOHEADER
unexport KCONFIG_AUTOCONFIG
unexport KCONFIG_DEPENDENCIES
unexport KCONFIG_SPLITCONFIG
unexport KCONFIG_TRISTATE
unexport KCONFIG_NEGATIVES

all: build

$(project_dir):
	echo "    Cloning $(project_name) from Git"
	git clone $(project_git_repo) $(project_dir)

fetch: $(project_dir)
ifeq ($(CONFIG_UBOOT_MASTER),y)
	echo "    Fetching new commits from the $(project_name) git repo"
	git fetch

	#master doesn't get a file, so it's continuously updated
	rm -f $(project_dir)/$(STABLE_COMMIT_ID)
else
	cd $(project_dir); git show $(TAG-y) >/dev/null 2>&1 ;	if [ $$? -ne 0 ]; \
	then echo "    Fetching new commits from the $(project_name) git repo"; git fetch; fi
	touch $(project_dir)/$(STABLE_COMMIT_ID)
endif

$(project_dir)/tag-$(TAG-y): fetch
	echo "    Checking out $(project_name) revision $(TAG-y)"
	cd $(project_dir); git checkout master; git branch -D coreboot 2>/dev/null; git checkout -b coreboot $(TAG-y)

config: $(project_dir)/$(TAG-y)
	rm -f $(project_config_file)
ifneq ($(CONFIG_PAYLOAD_CONFIGFILE),)
ifneq ("$(wildcard $(CONFIG_PAYLOAD_CONFIGFILE))","")
	cat $(CONFIG_PAYLOAD_CONFIGFILE)" > tag-$(project_config_file)
else
	echo "Error: File $(CONFIG_PAYLOAD_CONFIGFILE) does not exist"
	false
endif
else
	cat $(project_dir)/configs/coreboot-x86_defconfig >> $(project_config_file)
endif

	$(MAKE) -C $(project_dir) olddefconfig

build: config
	echo "    MAKE       $(project_name) $(TAG-y)"
	$(MAKE) -C $(project_dir)

clean:
	test -d $(project_dir) && $(MAKE) -C $(project_dir) clean || exit 0

distclean:
	rm -rf $(project_dir)

.PHONY: config build clean distclean fetch
