#include <unistd.h>     
#include <sys/types.h>  
#include <pwd.h>        
#include <stdlib.h>     
#include <errno.h>      
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>

void errExit(const char *msg){
    int savedErrno = errno;
    fprintf(stderr, "%s: %s\n", msg, strerror(savedErrno));

    exit(EXIT_FAILURE); 
}

uid_t userIdFromName(const char *name){
    struct passwd *pwd;
    uid_t u;
    char *endptr;

    if (name == NULL || *name == '\0') {  
        return getuid();
    }

    u = strtoul(name, &endptr, 10);  // 將輸入字串直接轉換為數字 UID（如果用戶輸入的是數字）

    if (*endptr == '\0') return u;  // 若整個字串都是數字，將其視為 UID 數字並return
    
    errno = 0; // 準備檢查 getpwnam 錯誤
    pwd = getpwnam(name);

    if (pwd == NULL) {
        if (errno == 0)
            fprintf(stderr, "User %s not found\n", name);  // getpwnam 成功返回 NULL 且 errno=0，表示找不到使用者名稱
        else 
            errExit("getpwnam");  // getpwnam 返回 NULL 且 errno != 0，表示發生系統錯誤
        exit(EXIT_FAILURE);
    }

    return pwd->pw_uid;
}


int main(int argc, char *argv[]) {
    struct dirent *dp;
    int temp_UID;
    char statusPath[256];
    uid_t userID = userIdFromName(argv[0]);
    DIR *dir = opendir("/proc");
    if (dir == 0)errExit("opendir /proc");
    
    while ((dp = readdir(dir)) != NULL) {
        if (strspn(dp->d_name, "0123456789") == strlen(dp->d_name)) {
        
            snprintf(statusPath, 256, "/proc/%s/status", dp->d_name); // 構建路徑 
            int fd = open(statusPath, O_RDONLY); // 嘗試開啟檔案 (競賽條件處理)
            if (fd == -1) {
                if (errno == ENOENT) {
                    continue; // 程序消失，跳過
                } else {
                    errExit("open /proc/PID/status"); // 系統錯誤
                }
        }
        
        // --- 進入這裡表示檔案已成功開啟 ---

        // 4. 讀取並解析檔案
        // 4a. 讀取整個檔案到緩衝區（省略讀取迴圈和錯誤檢查）
        // 4b. 解析出 Name: 和 Uid: 欄位
        
        // 5. 執行 UID 比對
        // if (parsed_uid == target_userID) {
        
        // 6. 列印結果 (這裡就是您要 print 的地方)
        // print_result(dp->d_name, command_name);
        
        // }
        
        close(fd); // 必須關閉檔案
    }


    }

    

    return 0;
}

