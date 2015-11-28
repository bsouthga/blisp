all: interpreter

interpreter:
	cd ./interpreter && make

install:
	sudo apt-get install libedit-dev
	mkdir ./interpreter/bin

clean:
	cd ./interpreter && make clean

check:
	./interpreter/bin/blisp ./test/index.blisp

.PHONY: interpreter clean check install
