# 1. 編譯器與旗標
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99

# 2. 目標執行檔名稱
TARGET = toUpp

# 3. 預設規則 (all)
all: $(TARGET)

# 4. 編譯規則
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

# 5. 清理規則
clean:
	rm -f $(TARGET)