CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)

gtk-pong: main.o
	gcc $(LDFLAGS) -o $@ $^

main.o: main.c
	gcc $(CFLAGS) -c -o $@ $<
