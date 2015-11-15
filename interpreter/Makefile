all: blisp

blisp:
	cc \
		-std=c99 \
		-Wall \
		./src/blisp.c ./src/mpc.c \
		-ledit \
		-lm \
		-o ./bin/blisp

clean:
	rm ./bin/*
