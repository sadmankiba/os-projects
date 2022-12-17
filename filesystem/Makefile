CC     := gcc
CFLAGS := -DDEBUG

SRCS   := mkfs.c

OBJS   := ${SRCS:c=o}
PROGS  := ${SRCS:.c=}

.PHONY: all
all: ${PROGS}

${PROGS} : % : %.o Makefile
	${CC} $< -o $@ udp.c

clean:
	rm -f ${PROGS} ${OBJS}

server: server.c udp.c message.h mfs.h Makefile
	gcc $(CFLAGS) server.c -o server udp.c

server-dbg: server.c udp.c message.h mfs.h Makefile
	gcc $(CFLAGS) server.c -o server udp.c

client-app: client-app.c mfs.c udp.c 
	gcc client-app.c udp.c -o client-app

client-app-dbg: client-app.c mfs.c udp.c 
	gcc -DDEBUG client-app.c udp.c -o client-app

libmfs.so: mfs.c udp.c 
	gcc -c -Wall -Werror -fpic mfs.c 
	gcc -c -Wall -Werror -fpic udp.c 
	gcc -shared -o libmfs.so mfs.o udp.o

%.o: %.c Makefile
	${CC} ${CFLAGS} -c $<
