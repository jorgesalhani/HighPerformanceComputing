CFLAGS = -Wall -g
LPTHREADFLAGS = -D_REENTRANT -lpthread
TARGET = fire_seq
SRC = fire_seq.c
INPUT_FILE = entrada_carga_pequena.txt

all: ${TARGET}

${TARGET}: ${SRC}
	gcc ${CFLAGS} ${SRC} -o ${TARGET} ${LPTHREADFLAGS}

run:
	./${TARGET} ${INPUT_FILE}

valgrind: all
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./${TARGET} ${INPUT_FILE}

clean:
	rm -f ${TARGET}