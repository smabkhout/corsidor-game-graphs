.DEFAULT_GOAL := all

GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install
GSL_LIBDIR = $(shell [ -e $(GSL_PATH)/lib ] && \
        echo $(GSL_PATH)/lib || \
        echo $(GSL_PATH)/lib64)
CFLAGS = --coverage -std=c99 -Wall -Wextra -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -fPIC -g3 -O0 -I$(GSL_PATH)/include -Isrc
LDFLAGS = --coverage -lm -lgsl -lgslcblas -ldl \
	-L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 \
	-Wl,--rpath=${GSL_PATH}/lib
	
.PHONY: rapport

rapport:
	pdflatex rapport/rapport.tex
	evince rapport.pdf &
	rm -f rapport.toc rapport.aux rapport.log rapport.out rapport/rapoprt.aux rapport/rapport.log rapport/rapport.out

all: build

build: server client 

build_tests: alltests

%.o: src/%.c 
	$(CC) $< $(CFLAGS) -c

libPlayer1.so: player1.o strategie3.o board.o graph.o move2.o
	gcc -shared -fPIC $(CFLAGS) $^ -o $@ -lgcov

libPlayer4.so: player2.o move2.o strategies.o board.o graph.o
	gcc -shared -fPIC $(CFLAGS) $^ -o $@ -lgcov

libPlayer3.so: player3.o move2.o strategie3.o board.o graph.o
	gcc -shared -fPIC $^ -o $@ -lgcov

libPlayer2.so: neg_player.o move2.o strategies.o board.o graph.o
	gcc -shared -fPIC $^ -o $@ -lgcov

server: server.o graph.o move2.o board.o
	gcc $^ $(LDFLAGS) -o $@

client: libPlayer1.so libPlayer2.so	libPlayer3.so libPlayer4.so 

alltests: graph.o strategie3.o strategies.o move2.o board.o
	$(CC) --coverage $(CFLAGS) -c test/graph_test.c -o graph_test.o
	$(CC) --coverage $(CFLAGS) -c test/strategie3_test.c -o strategie3_test.o
	$(CC) --coverage $(CFLAGS) -c test/move2_test.c -o move2_test.o
	$(CC) --coverage $(CFLAGS) -c test/test_player.c -o test_player.o
	$(CC) --coverage $(CFLAGS) -c test/alltests.c -o alltests.o
	$(CC) -ftest-coverage $(CFLAGS) board.o graph.o move2.o strategies.o strategie3.o test_player.o graph_test.o strategie3_test.o move2_test.o alltests.o $(LDFLAGS) -lgcov -o $@


test: alltests
	./alltests

install: build build_tests
	cp server libPlayer1.so libPlayer2.so libPlayer3.so libPlayer4.so alltests install/

clean:
	@rm -f *~ src/*~ test/*~ server alltests *.o *.gcno *.gcda install/*.so install/server