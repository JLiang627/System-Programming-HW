CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu99

SRCS = router.c node.c main.c

OBJS = $(SRCS:.c=.o)

TARGET = a.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)