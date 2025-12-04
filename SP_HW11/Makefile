CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 \
		 -Wno-unused-parameter \
		 -Wno-unused-variable \
		 -Wno-unused-value
LDFLAGS = -lrt

TARGET = program
SOURCE = program.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(SOURCE) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	@echo "Cleaned up $(TARGET) and object files."

.PHONY: all clean