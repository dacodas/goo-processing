build:
	# https://httpd.apache.org/docs/2.4/programs/apxs.html
	x86_64-pc-linux-gnu-gcc -march=native -O2 -pipe -DLINUX -D_REENTRANT -D_GNU_SOURCE -pthread -c GooTrieClient.cpp -fPIC -DPIC -o GooTrieClient.o
	x86_64-pc-linux-gnu-gcc -march=native -O2 -pipe -DLINUX -D_REENTRANT -D_GNU_SOURCE -pthread -c GooDictionaryClient.cpp -fPIC -DPIC -o GooDictionaryClient.o
	x86_64-pc-linux-gnu-gcc -march=native  -I/usr/include/libxml2 -O2 -pipe -DLINUX -D_REENTRANT -D_GNU_SOURCE -pthread -c GrabEntryDefinition.cpp -fPIC -DPIC -o GrabEntryDefinition.o
	x86_64-pc-linux-gnu-gcc -march=native -O2 -pipe -DLINUX -D_REENTRANT -D_GNU_SOURCE -pthread -I/usr/include/apache2 -I/usr/include/apr-1 -I/usr/include/apr-1 -I/usr/include/db5.3 -c mod_goo.cpp -fPIC -DPIC -o mod_goo.o

	x86_64-pc-linux-gnu-gcc -shared  -fPIC -DPIC  mod_goo.o GrabEntryDefinition.o GooTrieClient.o GooDictionaryClient.o  -Wl,--as-needed -Wl,--no-as-needed  -Wl,-O1   -Wl,-soname -Wl,mod_goo.so -o mod_goo.so
