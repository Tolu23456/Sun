CC      = gcc
CFLAGS  = -Iinclude -Wall -Wextra -std=c11 -O2
SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:.c=.o)
TARGET  = sun

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
