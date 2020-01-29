CC=gcc -std=c11
INCLUDE=-I. -I../../
CFLAGS=$(INCLUDE) -g

main.o:  ../main.c
	$(CC) $(INCLUDE) -c $< -o $@ 

.PHONY: clean realclean
clean:
	$(RM) main.o

realclean: clean
	-$(RM) *~
