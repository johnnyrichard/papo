CC=gcc

.PHONY: all
all: hash_table_test string_view_test

.PHONY: hash_table_test
hash_table_test: ../hash_table.o hash_table_test.c
	$(CC) -o hash_table_test ../hash_table.o hash_table_test.c
	./hash_table_test

.PHONY: string_view_test
string_view_test: ../string_view.o string_view_test.c
	$(CC) -o string_view_test ../string_view.o string_view_test.c
	./string_view_test

.PHONY: clean
clean:
	@$(RM) hash_table_test
