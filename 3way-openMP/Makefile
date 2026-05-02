CC = gcc
CFLAGS = -O2 -Wall -Wextra -Werror -std=gnu11 -fopenmp
TARGET = hw4-openmp

all: $(TARGET)

$(TARGET): hw4-openmp.c
	$(CC) $(CFLAGS) -o $(TARGET) hw4-openmp.c

clean:
	rm -f $(TARGET)