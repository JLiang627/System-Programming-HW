# 編譯器設定
CC = gcc
CFLAGS = -Wall -g
# 連結選項：rt (Realtime Extensions, for shm), pthread (POSIX Threads/Semaphores)
LDFLAGS = -lrt -pthread

# 目標執行檔名稱
TARGET = main

# 來源檔案
SRCS = main.c
# 標頭檔 (依賴關係用)
HEADERS = stack.h

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)