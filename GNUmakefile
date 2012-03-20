#!gmake
.PHONY: debug release clean clobber package install debuginst releaseinst tests


ifeq ($(wildcard Makefile), Makefile)
all:
	$(MAKE) -f Makefile $(MAKECMDGOALS)

install:
	$(MAKE) -f Makefile $(MAKECMDGOALS)

clean:
	$(MAKE) -f Makefile $(MAKECMDGOALS)

.DEFAULT:
	$(MAKE) -f Makefile $(MAKECMDGOALS)

else

all: install
install: Debug/Makefile Release/Makefile
	@$(MAKE) -C Debug install
	@$(MAKE) -C Release install

clean:
	@-$(MAKE) -C Debug clean
	@-$(MAKE) -C Release clean

package: Release/Makefile
	@$(MAKE) -C Release clean
	@$(MAKE) -C Release package

tests: Debug/Makefile
	@$(MAKE) -C Debug tests

.DEFAULT: Debug/Makefile
	@$(MAKE) -C Debug $(MAKECMDGOALS)
endif

clobber:
	rm -rf Debug Release

debug: Debug/Makefile
	@$(MAKE) -C Debug

Debug/Makefile:
	@mkdir -p Debug
	@cd Debug; cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX:PATH=install

release: Release/Makefile
	@$(MAKE) -C Release

Release/Makefile:
	@mkdir -p Release
	@cd Release; cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=install

