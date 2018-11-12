all: build-trie simplify-entries

build-trie: src/build-trie.cpp
	g++ -ggdb src/build-trie.cpp -lpcrecpp -o bin/build-trie

simplify-entries: src/simplify-entries.cpp
	g++ -ggdb src/simplify-entries.cpp -lmyhtml -lpcre2-8 -o bin/simplify-entries

.PHONY: clean-everything
clean-everything:
	goo-process clear_all_results
	rm bin/*
