build:
	# https://httpd.apache.org/docs/2.4/programs/apxs.html
	x86_64-pc-linux-gnu-g++ -std=c++17 -ggdb -march=native -O2 -pipe -DLINUX -D_REENTRANT -D_GNU_SOURCE -pthread -c GooTrieClient.cpp -fPIC -DPIC -static -lstdc++ -o GooTrieClient.o
	x86_64-pc-linux-gnu-g++ -std=c++17 -ggdb -march=native -O2 -pipe -DLINUX -D_REENTRANT -D_GNU_SOURCE -pthread -c GooDictionaryClient.cpp -fPIC -DPIC -static  -lstdc++ -o GooDictionaryClient.o
	x86_64-pc-linux-gnu-g++ -std=c++17 -ggdb -march=native -O2 -pipe -DLINUX -D_REENTRANT -D_GNU_SOURCE -pthread -I/usr/include/libxml2 -c GrabEntryDefinition.cpp -fPIC -DPIC -static -lstdc++ -o GrabEntryDefinition.o
	x86_64-pc-linux-gnu-g++ -std=c++17 -ggdb -march=native -O2 -pipe -DLINUX -D_REENTRANT -D_GNU_SOURCE -pthread -I/usr/include/apache2 -I/usr/include/apr-1 -I/usr/include/apr-1 -I/usr/include/db5.3 -c mod_goo.cpp -fPIC -DPIC -static -o mod_goo.o

	x86_64-pc-linux-gnu-g++ -std=c++17 -ggdb -shared  -fPIC -DPIC  mod_goo.o GrabEntryDefinition.o GooTrieClient.o GooDictionaryClient.o -lcurl -lxml2 -lstdc++ -Wl,--as-needed -Wl,--no-as-needed  -Wl,-O1   -Wl,-soname -Wl,mod_goo.so -o mod_goo.so

grabDictionaryEntries: 

	LANG=en_US.utf8 g++ -std=c++17 -ggdb -I/usr/include/libxml2 -lxml2 src/GrabEntryDefinition.cpp src/ThreadPool.cpp src/ProcessEntries.cpp -o getEntriesContents -lstdc++fs -lpthread
