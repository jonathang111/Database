Compile: main.cpp
	g++ -std=c++17 -g main.cpp CacheReadWrite/CacheRW.cpp CacheReadWrite/FileStorage.cpp DBConstruction/FileRead.cpp KeyAndSort/KeySort.cpp -o main

clean:
	rm -f main
	rm -f dbcachetest.bin