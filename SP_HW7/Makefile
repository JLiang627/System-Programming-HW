# 1. 編譯器與旗標
CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu99

# 2. 依賴的標頭檔
#    (只有 mymain.o 和 popen_pclose.o 需要 mypipe.h)
DEPS = mypipe.h

# 3. 目標執行檔 (我們總共想建立 2 個)
TARGETS = mymain toUpp

# 4. 預設規則 (all)
#    (輸入 make 時，預設會執行此規則，它會去建立所有 TARGETS)
all: $(TARGETS)

# 5. 連結規則 (Linking rule) for 'mymain'
#    'mymain' 是由 mymain.o 和 popen_pclose.o 連結而成
mymain: mymain.o popen_pclose.o
	$(CC) $(CFLAGS) -o $@ $^

# 6. 連結規則 (Linking rule) for 'toUpp'
#    'toUpp' 是由 toUpp.o 連結而成
toUpp: toUpp.o
	$(CC) $(CFLAGS) -o $@ $^

# 7. 編譯規則 (Compilation rule) - 針對需要 'mypipe.h' 的檔案
#    告訴 make, mymain.o 和 popen_pclose.o 依賴 mypipe.h
mymain.o popen_pclose.o: %.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

# 8. 通用編譯規則 - 針對其他 .c 檔案 (例如 toUpp.c)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 9. 清理規則
#    (會清除所有 TARGETS 和所有 .o 檔案)
clean:
	rm -f $(TARGETS) mymain.o popen_pclose.o toUpp.o

