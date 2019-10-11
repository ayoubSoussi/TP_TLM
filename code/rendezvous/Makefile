TARGET=run.x
all: $(TARGET)

SRCS=${wildcard *.cpp}

ROOT=../../TPs
include $(ROOT)/Makefile.common

$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)


