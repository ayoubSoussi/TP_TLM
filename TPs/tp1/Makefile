TARGET=run.x
all: $(TARGET)

SRCS=${wildcard *.cpp}
ROOT=../..
WITH_X11=YesPlease
include $(ROOT)/Makefile.common

$(TARGET): $(OBJS) $(ENSITLM_LIB)
	$(LD) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

FILES=${wildcard *.h *.cpp}

clang-format:
	$(CLANG_FORMAT) -i $(FILES)
