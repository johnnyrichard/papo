all: main.c server.h
	$(CC) -g -o papo main.c server.c
