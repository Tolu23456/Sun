CC      = gcc
CFLAGS  = -Iinclude -Wall -Wextra -std=c11 -O2
SRC     = src/archive.c src/bcgen.c src/codegen.c src/codegen_native.c src/lexer.c src/main.c src/parser.c src/server.c src/sun.c src/vm.c src/vfs.c
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
