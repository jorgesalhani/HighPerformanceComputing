CFLAGS = -Wall -g
LPTHREADFLAGS = -D_REENTRANT -lpthread
TARGET = program
SRC = fire_seq.c

all: ${TARGET}

${TARGET}: ${SRC}
	gcc ${CFLAGS} ${SRC} -o ${TARGET} ${LPTHREADFLAGS}

run:
	./${TARGET}.exe

clean:
	rm -f ${TARGET}