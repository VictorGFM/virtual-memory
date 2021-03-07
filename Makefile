ALL = tp2virtual
SRC = $(wildcard *.c */*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

CC = gcc

CFLAGS = -g -Wall -O3

LIBS += -lm

all: $(ALL)

$(ALL): $(OBJ)
	$(CC) $(CFLAGS) -c $(SRC) 
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LIBS) 

clean:
	rm $(ALL) *.o

