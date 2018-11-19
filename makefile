test : ts_ll.c node.c
	clang -std=c99 -Wall -Werror -lpthread $^ -o $@

clean:
	rm test