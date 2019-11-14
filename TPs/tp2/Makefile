.PHONY: all
all: native-wrapper iss

.PHONY: hardware
hardware:
	@cd hardware && $(MAKE)

.PHONY: check-native-wrapper
check-native-wrapper: native-wrapper
	native-wrapper/run.x

.PHONY: native-wrapper
native-wrapper:
	@cd native-wrapper && $(MAKE)

.PHONY: iss
iss:
	@cd iss && $(MAKE)

.PHONY: clean-subdirs
clean-subdirs:
	@cd hardware && $(MAKE) clean
	@cd native-wrapper && $(MAKE) clean
	@cd iss && $(MAKE) clean
	@cd software/native/ && $(MAKE) clean
	@cd software/cross/ && $(MAKE) clean
	@cd elf-loader/ && $(MAKE) clean
clean: clean-subdirs

ROOT=../
WITH_SDL=yesPlease
include $(ROOT)/Makefile.common

OWNFILES=iss/sc_main_iss.cpp iss/mb_wrapper.cpp iss/mb_wrapper.h \
	${wildcard hardware/*.h hardware/*.cpp} \
	${wildcard native/*.h native/*.cpp}

clang-format:
	$(CLANG_FORMAT) -i $(OWNFILES)
