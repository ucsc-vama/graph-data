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
clean	:
		rm -f hash hash.o median median.o sort_imbalance sort_imbalance.o
infer	:
		make clean; infer-capture -- make;