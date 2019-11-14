MODULE = native-wrapper

ESOFT_OBJS = ../software/native/main.o

SRCS = sc_main_native.cpp native_wrapper.cpp

TARGET = run.x

ROOT=../..
WITH_SDL=yesPlease
include $(ROOT)/Makefile.common

EXTRALDLIBS += ../hardware/libhardware.a ../elf-loader/libloader.a
CXXEXTRAFLAGS = -I../hardware
CEXTRAFLAGS = -I. 

$(TARGET): $(ESOFT_OBJS) $(OBJS) $(EXTRALDLIBS) $(ENSITLM_LIB)
	$(LD) $(ESOFT_OBJS) $(OBJS) -o $@ $(LDFLAGS) $(EXTRALDLIBS) $(LDLIBS)

.PHONY: ../hardware/libhardware.a
../hardware/libhardware.a:
	@cd .. && $(MAKE) hardware

.PHONY: $(ESOFT_OBJS)
$(ESOFT_OBJS):
	cd ../software/native/ && $(MAKE)

FORCE:

# Include dependency files
-include $(ESOFT_DEPS)
