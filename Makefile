all: interpreter

interpreter:
	cd ./interpreter && make

clean:
	cd ./interpreter && make clean

check:
	./interpreter/bin/blisp ./test/index.blisp

.PHONY: interpreter clean check
