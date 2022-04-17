CC=gcc

.PHONY: all
all: main.c server.c log.o hash_table.o string_view.o
	$(CC) -ggdb -o papo main.c server.c log.o hash_table.o string_view.o

.PHONY: test
test: all
	@make -C test

.PHONY: ftest
ftest: libpapo.so
	@python ftest.py -v --locals

libpapo.so: server.o log.o
	$(CC) -shared -o libpapo.so server.o log.o

log.o: log.c
	$(CC) -DLOG_USE_COLOR -fPIC -c log.c

.PHONY: clean
clean:
	@$(RM) *.o 
	@$(RM) *.so
	@$(RM) papo
