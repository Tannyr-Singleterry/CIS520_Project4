CC = gcc
CFLAGS = -O2 -Wall -Wextra -Werror -pthread -std=gnu11
TARGET = hw4-pthread

all: $(TARGET)

$(TARGET): hw4-pthread.c
	$(CC) $(CFLAGS) -o $(TARGET) hw4-pthread.c

clean:
	rm -f $(TARGET)