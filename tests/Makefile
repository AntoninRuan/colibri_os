.PHONY: all

TEST := rb_tree

all: $(TEST)
	@./run_test.py $(TEST)
	@rm $(TEST)
	@find . -name "*.o" -exec rm {} +

rb_tree:
	@clang -o rb_tree.o -c ../kernel/scheduler/rb_tree.c -isystem ../kernel/include/ -isystem ../libc/include
	@clang -o $@ rb_tree_test.c rb_tree.o -isystem ../kernel/include
