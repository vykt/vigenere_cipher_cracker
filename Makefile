CC=gcc
CFLAGS=-ggdb -O3
CLEAN_TARGETS=crack crack.o

crack: crack.o
	${CC} ${CFLAGS} -o crack crack.o

crack.o: crack.c
	${CC} ${CFLAGS} -c crack.c

clean:
	rm ${CLEAN_TARGETS}
