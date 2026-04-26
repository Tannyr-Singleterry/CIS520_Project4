CC = mpicc
CFLAGS = -O2 -Wall -Wextra -Werror -std=gnu11
TARGET = 3way-mpi

all: $(TARGET)

$(TARGET): hw4-mpi.c
	$(CC) $(CFLAGS) -o $(TARGET) hw4-mpi.c

clean:
	rm -f $(TARGET)