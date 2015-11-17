all: interpreter

clean:
	cd ./interpreter && make clean

interpreter:
	cd ./interpreter && make

.PHONY: interpreter clean
