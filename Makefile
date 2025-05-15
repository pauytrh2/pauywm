CC = gcc
CFLAGS = -Wall -Wextra -std=c17
LDFLAGS = -lX11
TARGET = main
SRC = main.c

.PHONY: all build clear

all: clear build

build:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clear:
	rm -f $(TARGET)