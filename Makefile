GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install
GSL_LIBDIR = $(shell [ -e $(GSL_PATH)/lib ] && \
        echo $(GSL_PATH)/lib || \
        echo $(GSL_PATH)/lib64)
CFLAGS = -std=c99 -Wall -Wextra -fPIC -g3 -I$(GSL_PATH)/include
LDFLAGS = -lm -lgsl -lgslcblas -ldl \
	-L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 \
	-Wl,--rpath=${GSL_PATH}/lib

all: build

build: server client

%.o: src/%.c
	$(CC) $< $(CFLAGS) -c

libPlayer1.so: player1.o 
	gcc -shared -fPIC $^ -o $@

libPlayer2.so: player2.o 
	gcc -shared -fPIC $^ -o $@

server:

client:

alltests:

test: alltests

install: server client test

clean:
	@rm -f *~ src/*~ test/*~

.PHONY: client install test clean
