TARGET = dtmf

CC = gcc
CFLAGS = -Wall -Wextra -g -lm

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
INCLUDE = $(wildcard include/*.c)


.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

obj/%.o: src/%.c $(INCLUDE)
	mkdir -p obj
	$(CC) $(CFLAGS) -I include -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
	rm -rf obj/
