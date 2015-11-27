all: interpreter

clean:
	cd ./interpreter && make clean

interpreter:
	cd ./interpreter && make

test:
	./interpreter/bin/blisp ./test/index.blisp

.PHONY: interpreter clean test
