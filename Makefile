CFLAGS=-DLOG_USE_COLOR

all: main.c server.c log.c
	$(CC) $(CFLAGS) -g -o papo main.c server.c log.c
