CC=gcc -std=c11
INCLUDE=-I. -I../../
CFLAGS=$(INCLUDE) -g

main.o:  ../main.c
	$(CC) -c ../main.c -o main.o $(CFLAGS) 

.PHONY: clean realclean
clean:
	$(RM) main.o

realclean: clean
	-$(RM) *~
