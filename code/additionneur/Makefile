# Makefile SystemC
# by Jerome Cornet

# Only one variable needed: SYSTEMCROOT, pointing
# to the systemc install and source tree
SYSTEMC = $(SYSTEMCROOT)
TLM = $(TLMROOT)

# guess target os name used by systemc's configure
ARCH = $(shell uname -m | sed -e 's/x86_64.*/linux64/' -e 's/i.86.*/linux/')

INCLUDES = -I. -I$(SYSTEMC)/include

CC = g++
CPPFLAGS = $(INCLUDES)
CXXFLAGS = -Wno-deprecated -O0

LD = $(CC)
LDFLAGS = 
LDLIBS = -L. -L$(SYSTEMC)/lib-$(ARCH) -Xlinker -Bstatic -lsystemc -Xlinker -Bdynamic -lm -pthread

SRCS = $(wildcard *.cpp)
DEPS = $(SRCS:%.cpp=%.d)
OBJS = $(SRCS:%.cpp=%.o)

PROGRAM = run.x

all: $(PROGRAM)

clean: FORCE
	-rm -f *.d *.o *.out core $(PROGRAM) trace.vcd

$(PROGRAM):	$(DEPS) $(OBJS)
		$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(PROGRAM) 2>&1 | c++filt

%.d: %.cpp
	@ echo Making dependencies for $<
	@ $(CC) -MM $(INCLUDES) $< | sed 's#: # $*.d : #1' > $@

%.o: %.cpp %.d
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Include dependency files
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif

FORCE: ;
