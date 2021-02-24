ALL = tp2virtual
SRC = $(wildcard *.c */*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

CC = gcc

CFLAGS = -g -Wall -O3

all: $(ALL)

$(ALL): $(OBJ)
	$(CC) $(CFLAGS) -c $(SRC)
	$(CC) $(CFLAGS) $(OBJ) -o $@ 

clean:
	rm $(ALL) *.o

