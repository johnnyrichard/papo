CFLAGS=-DLOG_USE_COLOR
CC=gcc

.PHONY: all
all: main.c server.c log.c
	$(CC) $(CFLAGS) -g -o papo main.c server.c log.c

.PHONY: test
test: libpapo.so
	@python test.py

libpapo.so: server.o log.o
	$(CC) $(CFLAGS) -shared -o libpapo.so server.o log.o

log.o: log.c
	$(CC) $(CFLAGS) -fPIC -c log.c

.PHONY: clean
clean:
	rm *.o *.so
