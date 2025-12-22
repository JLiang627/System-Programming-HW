CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lrt -pthread

TARGET = main
SRCS = main.c
HEADERS = stack.h

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)