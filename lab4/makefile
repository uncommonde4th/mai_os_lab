CC = gcc
CFLAGS = -Wall -Wextra

LIB_DIR = /usr/local/lib

main: impl0 impl1
	$(CC) $(CFLAGS) -o main.out main.c -ldl -lm

regular_main_impl0: lib0
	$(CC) $(CFLAGS) -o link_main0.out link_main.c impl0.o -ldl -lm

regular_main_impl1: lib1
	$(CC) $(CFLAGS) -o link_main1.out link_main.c impl1.o -ldl -lm

impl0: lib0
	$(CC) $(CFLAGS) -shared -o libimpl0.so impl0.o -lm

impl1: lib1
	$(CC) $(CFLAGS) -shared -o libimpl1.so impl1.o -lm

lib0:
	$(CC) $(CFLAGS) -c impl0.c -fPIC

lib1:
	$(CC) $(CFLAGS) -c impl1.c -fPIC

clean:
	rm -f *.out *.so *.o

all: main regular_main_impl0 regular_main_impl1
