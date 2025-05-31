.PHONY: all

ifeq ($(NO_DEL),1)
RM =
else
RM := rm $(TEST) && find . -name "*.o" -exec rm {} +
endif

rb_tree.o: ../kernel/scheduler/rb_tree.c
	@clang -o rb_tree.o -c ../kernel/scheduler/rb_tree.c -isystem ../kernel/include/ -isystem ../libc/include

rb_benchmark: rb_tree_benchmark.c rb_tree.o Makefile
	clang -o $@ rb_tree_benchmark.c rb_tree.o -isystem ../kernel/include
