# Makefile SystemC
# by Jerome Cornet


INCLUDES = -I./dpp/source/include/ \
	-I./elf_file_loader/src/include/ \
	-I./elf_file_loader/libelfpp/include \
	-I./loader/include/ \
	-I./include

CC = g++
CPPFLAGS = $(INCLUDES)
CXXFLAGS = -Wno-deprecated -O0 -g -Wall $(CXXEXTRAFLAGS)

LD = $(CC)
LDFLAGS =

SRCS = ./elf_file_loader/libelfpp/src/elfpp_access.cc  \
       ./elf_file_loader/libelfpp/src/elfpp_symbol.cc  \
       ./elf_file_loader/libelfpp/src/elfpp_object.cc  \
       ./elf_file_loader/libelfpp/src/elfpp_reloc.cc   \
       ./elf_file_loader/libelfpp/src/elfpp_section.cc \
       ./loader/src/binary_file_symbol.cpp	       \
       ./loader/src/loader.cpp			       \
       ./loader/src/binary_file_section.cpp	       \
       ./elf_file_loader/src/elf_file_loader.cpp
BASE := $(SRCS)
BASE := $(BASE:%.cpp=%)
BASE := $(BASE:%.cc=%)
DEPS = $(BASE:%=%.d)
OBJS = $(BASE:%=%.o)

#BASIC_SRCS = $(wildcard $(BASIC)/*.cpp)
#BASIC_OBJS = $(BASIC_SRCS:%.cpp=%.o)

LIB=libloader.a

all: $(LIB)

clean: FORCE
	-rm -f $(DEPS) $(OBJS) *.out core $(PROGRAM) $(LIB)

debug:
	@echo "LIB=$(LIB)"
	@echo "SRCS=$(SRCS)"
	@echo "BASE=$(BASE)"
	@echo "OBJS=$(OBJS)"
	@echo "DEPS=$(DEPS)"

$(LIB):	$(DEPS) $(OBJS)
	ar crs $(LIB) $(OBJS)

%.d: %.cpp
	@ echo Making dependencies for $<
	@ $(CC) -MM $(INCLUDES) $< | sed 's#: # $*.d : #1' > $@

%.d: %.cc
	@ echo Making dependencies for $<
	@ $(CC) -MM $(INCLUDES) $< | sed 's#: # $*.d : #1' > $@

%.o: %.cpp %.d
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.cc %.d
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Include dependency files
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif

FORCE: ;
