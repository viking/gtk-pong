CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)
LDFLAGS += -lsqlite3

gtk-pong: main.o database.o
	gcc $(LDFLAGS) -o $@ $^

main.o: main.c
	gcc $(CFLAGS) -c -o $@ $<

database.o: database.c database.h
	gcc $(CFLAGS) -c -o $@ $<
