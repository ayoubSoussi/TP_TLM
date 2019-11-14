MODULE = iss

ESOFT_BIN = ../software/cross/a.out

SRCS = sc_main_iss.cpp rv32_wrapper.cpp rv32.cpp

TARGET = run.x

all: $(TARGET) $(ESOFT_BIN)

ROOT=../..
WITH_SDL=yesPlease
include $(ROOT)/Makefile.common

EXTRALDLIBS += ../hardware/libhardware.a ../elf-loader/libloader.a -lm
CXXEXTRAFLAGS = -g -I../hardware
CEXTRAFLAGS = -I.

$(TARGET): $(OBJS) $(EXTRALDLIBS) $(ENSITLM_LIB)
	$(LD) $(ESOFT_OBJS) $(OBJS) -o $@ $(LDFLAGS) $(EXTRALDLIBS) $(LDLIBS)

.PHONY: $(ESOFT_BIN)
$(ESOFT_BIN):
	cd ../software/cross && $(MAKE)

.PHONY: ../hardware/libhardware.a
../hardware/libhardware.a:
	@cd .. && $(MAKE) hardware

.PHONY: ../elf-loader/libloader.a
../elf-loader/libloader.a:
	@cd ../elf-loader/ && $(MAKE)

# Include dependency files
-include $(ESOFT_DEPS)
