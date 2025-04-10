GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install
GSL_LIBDIR = $(shell [ -e $(GSL_PATH)/lib ] && \
        echo $(GSL_PATH)/lib || \
        echo $(GSL_PATH)/lib64)
CFLAGS = -std=c99 -Wall -Wextra -fPIC -g3 -I$(GSL_PATH)/include -Isrc
LDFLAGS = -lm -lgsl -lgslcblas -ldl \
	-L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 \
	-Wl,--rpath=${GSL_PATH}/lib


all: build

build: server client

%.o: src/%.c 
	$(CC) $< $(CFLAGS) -c

libPlayer1.so: player1.o move.o board.o graph.o
	gcc -shared -fPIC $^ -o $@

libPlayer2.so: player2.o move.o board.o graph.o
	gcc -shared -fPIC $^ -o $@

server: server.o graph.o board.o
	gcc $^ $(LDFLAGS) -o $@

client: libPlayer1.so libPlayer2.so	

alltests: graph.o move.o
	$(CC) --coverage $(CFLAGS) -c test/graph_test.c -o graph_test.o
	$(CC) --coverage $(CFLAGS) -c test/move_test.c -o move_test.o
	$(CC) --coverage $(CFLAGS) -c test/alltests.c -o alltests.o
	$(CC) -ftest-coverage $(CFLAGS) graph.o move.o graph_test.o move_test.o alltests.o $(LDFLAGS) -lgcov -o $@


test: alltests

install: server client test
	cp server libPlayer1.so libPlayer2.so alltests install

clean:
	@rm -f *~ src/*~ test/*~ server alltests

.PHONY: client install test clean
