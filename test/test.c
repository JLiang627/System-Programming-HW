#include <stdlib.h>  // getenv() 和 NULL 需要此標頭檔
#include <stdio.h>   // printf() 需要此標頭檔

int main() {
    char *value;

    // 1. 嘗試檢索 "HOME" 環境變數的值
    value = getenv("HOME");

    // 2. 檢查返回值：
    //    a. value == NULL: 變數未定義或不存在
    if (value == NULL) {
        printf("HOME is not defined\n");
    } 
    //    b. value == '\0': 變數已定義，但值是空字串 (這是原始範例的額外檢查)
    else if (*value == '\0') {
        printf("HOME defined but has no value\n");
    } 
    //    c. 其他情況: 變數已定義且有值
    else {
        printf("HOME %s\n", value);
    }
    return 0;
}