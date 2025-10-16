#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>

typedef struct ProcessInfo {
    int pid;          // Process ID
    int ppid;         // Parent Process ID
    char name[256];   
    struct ProcessInfo *children; 
    struct ProcessInfo *sibling;  
} ProcessInfo;


#define MAX_PID 65536
ProcessInfo *process_list[MAX_PID];

int get_process_info(int pid, ProcessInfo *info) {
    char path[512];
    char line[512];
    FILE *fp;

    // 構建 status 檔案的路徑
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    // 如果 open 失敗，則返回錯誤。
    fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Warning: Process %d disappeared or permission denied.\n", pid);
        return -1;
    }

    // 初始化資訊結構
    info->pid = pid;
    info->ppid = 0; // 預設為 0
    info->name[0] = '\0';

    // 逐行讀取文件，直到找到 Name: 和 PPid:
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "Name:", 5) == 0) {
            // 解析 Name: 欄位
            char *start = line + 5;
            // 跳過開頭的空白
            while (*start && isspace(*start)) {
                start++;
            }
            // 複製名稱，並移除尾隨的換行符和空白
            size_t len = strcspn(start, "\n\r");
            len = (len >= sizeof(info->name) - 1) ? sizeof(info->name) - 1 : len;
            strncpy(info->name, start, len);
            info->name[len] = '\0';
        } else if (strncmp(line, "PPid:", 5) == 0) {
            // 解析 PPid: 欄位
            info->ppid = atoi(line + 5);
        }
        
        // 如果同時找到了 Name 和 PPid，就可以提前退出
        if (info->name[0] != '\0' && info->ppid != 0) {
            break;
        }
    }

    fclose(fp);

    // 檢查是否成功讀取 Name 和 PPid
    if (info->name[0] == '\0') {
        // Name 欄位可能因為格式或其他問題未讀取到，將命令名稱設為未知
        snprintf(info->name, sizeof(info->name), "[Unknown Cmd for %d]", pid);
    }

    return 0;
}


void print_tree(ProcessInfo *proc, const char *prefix, int is_last) {
    if (proc == NULL) {
        return;
    }

    // 列印當前節點
    printf("%s%s %d (%s)\n", prefix, (is_last ? "`- " : "|- "), proc->pid, proc->name);

    // 準備下一層的縮排字串
    char next_prefix[512];
    // 如果不是最後一個，下一層的縮排應該是 prefix + "|"
    // 如果是最後一個，下一層的縮排應該是 prefix + " "
    snprintf(next_prefix, sizeof(next_prefix), "%s%s", prefix, (is_last ? "   " : "|  "));

    // 處理子程序
    ProcessInfo *child = proc->children;

    // 首先計算子程序數量，以確定誰是最後一個子程序
    int child_count = 0;
    ProcessInfo *temp = child;
    while (temp != NULL) {
        child_count++;
        temp = temp->sibling;
    }

    // 遍歷並遞迴地列印子程序
    int i = 0;
    while (child != NULL) {
        i++;
        int is_child_last = (i == child_count);
        print_tree(child, next_prefix, is_child_last);
        child = child->sibling;
    }
}


int main(int argc, char *argv[]) {
    DIR *dirp;
    struct dirent *dp;
    
    // 初始化程序列表
    memset(process_list, 0, sizeof(process_list));

    // 開啟 /proc 目錄
    dirp = opendir("/proc");
    if (dirp == NULL) {
        perror("Failed to open /proc directory");
        return EXIT_FAILURE;
    }

    // 遍歷 /proc 目錄中的所有項目
    while ((dp = readdir(dirp)) != NULL) {
        // 只處理名字是數字的目錄，這些是程序目錄
        if (dp->d_type == DT_DIR) {
            int pid = atoi(dp->d_name);
            if (pid > 0 && pid < MAX_PID) {
                ProcessInfo *proc = malloc(sizeof(ProcessInfo));
                if (proc == NULL) {
                    perror("malloc failed");
                    closedir(dirp);
                    return EXIT_FAILURE;
                }
                
                // 獲取程序資訊
                if (get_process_info(pid, proc) == 0) {
                    // 成功獲取資訊，將程序加入到全域列表中
                    proc->children = NULL;
                    proc->sibling = NULL;
                    process_list[pid] = proc;
                } else {
                    // 程序已消失，釋放記憶體
                    free(proc);
                }
            }
        }
    }

    closedir(dirp);

    // 建立父子關係
    for (int i = 1; i < MAX_PID; i++) {
        ProcessInfo *proc = process_list[i];
        if (proc != NULL) {
            int ppid = proc->ppid;
            
            // 檢查 PPID 是否有效且在我們的列表中
            if (ppid > 0 && ppid < MAX_PID && process_list[ppid] != NULL) {
                ProcessInfo *parent = process_list[ppid];
                
                // 將當前程序作為其父程序的第一個子程序
                // 使用單向鏈表來儲存子程序和兄弟程序
                proc->sibling = parent->children;
                parent->children = proc;
            } 
        }
    }

    // 找到根程序 (PID 1) 並列印樹
    ProcessInfo *root = process_list[1];
    if (root != NULL) {
        printf("Process Hierarchy Tree (PID: Command):\n");
        // 從 init 程序 (PID 1) 開始遞迴列印
        print_tree(root, "", 1); // 根節點始終為 "最後一個" 元素，使用 `'-`
    } else {
        fprintf(stderr, "Error: Could not find the init process (PID 1).\n");
        return EXIT_FAILURE;
    }

    // 釋放記憶體 (在實際生產程式中很重要，雖然在這裡不是必須的，因為程式即將退出)
    for (int i = 1; i < MAX_PID; i++) {
        if (process_list[i] != NULL) {
            free(process_list[i]);
        }
    }
    
    return EXIT_SUCCESS;
}
