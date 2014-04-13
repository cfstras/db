gen: gen.cpp Makefile
	g++ -g -O2 -Werror gen.cpp -o gen

silent: gen.cpp Makefile
	g++ -DSILENT -g -O2 -Werror gen.cpp -o gen

clean:
	rm -rf gen
