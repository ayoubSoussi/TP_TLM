TARGET=run.x
all: $(TARGET)

SRCS=${wildcard *.cpp}

ROOT=../../TPs/
include $(ROOT)/Makefile.common

$(TARGET): $(OBJS) $(ENSITLM_LIB)
	$(LD) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)


