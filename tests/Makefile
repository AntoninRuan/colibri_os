.PHONY: all

TEST := rb_tree_test

ifeq ($(NO_DEL),1)
RM =
else
RM := rm $(TEST) && find . -name "*.o" -exec rm {} +
endif

all: $(TEST)
	@./run_test.py $(TEST)
	@$(RM)

rb_tree.o: ../kernel/scheduler/rb_tree.c
	@clang -o rb_tree.o -c $< -isystem ../kernel/include/ -isystem ../libc/include

rb_tree_test: rb_tree_test.c rb_tree.o Makefile
	@clang -o $@ rb_tree_test.c rb_tree.o -isystem ../kernel/include -DVALID_TEST

ext2.o: ../kernel/driver/fs/ext2.c Makefile
	clang -o $@ -c $< -isystem ../kernel/include -O0 -ggdb

ext2_driver: ext2_driver.c ext2.o Makefile
	clang -o $@ ext2_driver.c ext2.o -isystem ../kernel/include -ggdb
