CC     := gcc
CFLAGS := -Wall -Werror 

SRCS   := client.c \
	server.c \
	mkfs.c

OBJS   := ${SRCS:c=o}
PROGS  := ${SRCS:.c=}

.PHONY: all
all: ${PROGS}

${PROGS} : % : %.o Makefile
	${CC} $< -o $@ udp.c

clean:
	rm -f ${PROGS} ${OBJS}

server-sm: server-sm.c udp.c
	gcc server-sm.c -o server-sm udp.c

client-app: client-app.c udp.c
	gcc client-app.c udp.c -o client-app

libmfs.so: mfs.c udp.c 
	gcc -c -Wall -Werror -fpic mfs.c 
	gcc -c -Wall -Werror -fpic udp.c 
	gcc -shared -o libmfs.so mfs.o udp.o

%.o: %.c Makefile
	${CC} ${CFLAGS} -c $<
