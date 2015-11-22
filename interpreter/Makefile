all: blisp

blisp:
	cc \
		-std=c99 \
		-Wall \
		./src/blisp.c ./lib/mpc.c \
		-ledit \
		-lm \
		-g \
		-o ./bin/blisp

clean:
	rm -rf ./bin
	mkdir bin
