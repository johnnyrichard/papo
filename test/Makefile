CC=gcc

.PHONY: all
all: hash_table_test

.PHONY: hash_table_test
hash_table_test: ../hash_table.o hash_table_test.c
	$(CC) -o hash_table_test ../hash_table.o hash_table_test.c
	./hash_table_test

.PHONY: clean
clean:
	@$(RM) hash_table_test
