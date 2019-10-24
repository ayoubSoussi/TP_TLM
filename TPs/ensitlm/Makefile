MODULE = ensitlm

TARGET = ensitlm.h.gch libensitlm.a
SRCS = bus.cpp

ROOT=..
include $(ROOT)/Makefile.common

FILES=${wildcard *.h *.cpp}

clang-format:
	$(CLANG_FORMAT) -i $(FILES)
