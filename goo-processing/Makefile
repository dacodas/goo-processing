# all: build-trie simplify-entries process-trie
all: process-trie

process-trie: src/process-trie.cpp prepare
	g++ $(CFLAGS) -ggdb src/process-trie.cpp -lpcrecpp -o bin/process-trie 

# build-trie: src/build-trie.cpp
# 	g++ -ggdb src/build-trie.cpp -lpcrecpp -o bin/build-trie

# simplify-entries: src/simplify-entries.cpp
# 	g++ -ggdb src/simplify-entries.cpp -lmyhtml -lpcre2-8 -o bin/simplify-entries

.PHONY: clean-everything prepare

prepare:
	mkdir -p bin

clean-everything:
	goo-process clear_all_results
	rm bin/*
