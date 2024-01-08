CFLAGS = -Wall -Werror -pthread

ifdef DEBUG
	CFLAGS += -g
else
	CFLAGS += -O3
endif

main: main.c
	gcc $(CFLAGS) main.c mergesort.c -o main

test: test.c mergesort.c
	gcc $(CFLAGS) test.c mergesort.c -o test

clean:
	rm -f main test
