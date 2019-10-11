all: handout.ps

PROG=main_refcount.x

$(PROG): main_refcount.cpp refcount_ptr.h
	g++ -g $^ -o $@

output.txt: $(PROG)
	(echo "bash> ./$(PROG)" ; ./$(PROG)) > output.txt 2>&1

handout.ps: refcount_ptr.h main_refcount.cpp output.txt Makefile
	a2ps -o "$@" $^ --file-align=fill

.PHONY: clean
clean:
	$(RM) output.txt $(PROG) handout.ps
