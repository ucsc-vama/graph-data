CC = g++
CFLAGS = -wall -g

all: hash median sort_imbalance
hash	:	hash.o
		$(CC) -o hash hash.o
hash.o	:	hash.cc
		$(CC) -c hash.cc
median	:	median.o
		$(CC) -o median median.o
median.o	:	median.cc
		$(CC) -c median.cc
sort_imbalance	:	sort_imbalance.o
		$(CC) -o sort_imbalance sort_imbalance.o
sort_imbalance.o	:	sort_imbalance.cc
		$(CC) -c sort_imbalance.cc
jch_median	:	median.o
		$(CC) -o median median.o
jch_median.o	:	median.cc
		$(CC) -c median.cc
jch_vertex	:	jch_vertex.o
		$(CC) -o jch_vertex jch_vertex.o
jch_vertex.o	:	jch_vertex.cc
		$(CC) -c jch_vertex.cc
sort_vertex	:	sort_vertex.o
		$(CC) -o sort_vertex sort_vertex.o
sort_vertex.o	:	sort_vertex.cc
		$(CC) -c sort_vertex.cc
clean	:
		rm -f hash hash.o median median.o sort_imbalance sort_imbalance.o jch_median jch_median.o jch_vertex jch_vertex.o sort_vertex sort_vertex.o
infer	:
		make clean; infer-capture -- make;