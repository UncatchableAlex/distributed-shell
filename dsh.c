/*
 * dsh.c
 *
 *  Created on: 2024-02-18
 *      Author: alex
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>
#include<math.h>

#define MIN(x, y) (x < y ? x : y)


int spawnProcess(char *path, char **args, int foreground) {
    int childPid = fork();
    if (!childPid) {
       execv(path, args);
    } else if (foreground) {
        wait(NULL);
        return 0;
    }
    return 0;
}



char *getFullPath(char *shortPath) {
    // copy the path into a dedicated buffer:
    char *pathOrig = getenv("PATH");
    char path[strlen(pathOrig) + 1];
    strcpy(path, pathOrig);
    char **dirs = split(path, ":");
    int i = 0;
    int shortPathLen = strlen(shortPath);
    char *fullPath = NULL;
    while (dirs[i]) {
        char *fullPathMaybe = (char*)malloc((shortPathLen + strlen(dirs[i]) + 2) * sizeof(char));
        sprintf(fullPathMaybe, "%s/%s", dirs[i], shortPath);
     //   printf("trying |%s|", fullPathMaybe);
        if (!fullPath && access(fullPathMaybe, F_OK | X_OK) == 0) {
      //      printf(" success");
            deepFree2d((void**)dirs);
            return fullPathMaybe;
        } else {
            free(fullPathMaybe);
        }
    //    printf("\n");
        i++;
    }
    deepFree2d((void**)dirs);
    return NULL;
}


char *trim(char *str) {
    int strLen = strlen(str);
    int startString = 0;
    int endString = strLen - 1;
    while (str[startString++] == ' ');
//    printf("will enter loop: %d\n", str[endString]);
    while (str[endString--] == ' ') {
    }
 //   printf("charAt %d: %c\n", endString, str[endString]);
    startString--;
    endString += 2;
 //   printf("start: %d\nend: %d\n", startString, endString);
    char *newStr = (char*) malloc((1 + endString - startString) * sizeof(char));
    str[endString] = '\0';
    strcpy(newStr, str + (startString * sizeof(char)));
    free(str);
    return newStr;
}


void deepFree2d(void **arr) {
    int i = 0;
    while (arr[i]) {
        free(arr[i]);
        i++;
    }
    free(arr);
}

/**
 * Tokenize a string into an array. the string is destroyed
*/
char **split(char *str, char *delimiter) {
    int len = strlen(str);
    // count the number of items that will be in our returned array
    int itemCount = 1; // we start item count at 2 because we will be adding DELIMITERS not actual items
    int delimLen = strlen(delimiter);
    for (int i = 0; i <= len - delimLen; i++) {
        // chop the string into a substring from i to i + delimLen
        char breakChar = str[i + delimLen];
        str[i + delimLen] = '\0';
        // if the substring str(i, i+delimLen) is our delimiter, increase the itemCount
    //    printf("comparing |%s| to |%s| ", str + (i*sizeof(char)), delimiter);
        if (!strcmp(str + (i*sizeof(char)), delimiter)) {
    //        printf(" success!");
            itemCount++;
        }
    //    printf("\n");
        // put that character back (we need it! It's part of the string that we are digesting):
        str[i + delimLen] = breakChar;
    }
   // printf("item count: %d\n", itemCount);
   // printf("str: %s\n", str);
    char *startOfDelim = strstr(str, delimiter);
    char **arr = (char**) malloc(sizeof(char*) * (itemCount + 1)); // itemCount+1 because we need to make room for the NULL terminator
    arr[itemCount] = NULL; // set the last element to null before we even start
    int i = 0;
    while (str[0] != '\0') {
        int strLen = strlen(str);
      //  printf("str: %s\n", str);
      //  printf("startOfDelim: |%s|\n", startOfDelim);
        startOfDelim = startOfDelim ? startOfDelim : str + strLen;
       // printf("startOfDelimPost: |%s|\n", startOfDelim);
        int pos = startOfDelim - str;
      //  printf("pos: %d\n", pos);
        char* nextEntry = (char*) malloc((pos + 1) * sizeof(char)); // dont forget to allocate space for the null terminator
        str[pos] = '\0';
        strcpy(nextEntry, str);
        arr[i] = nextEntry; // point the ith entry of our array at the next entry
        str = (char*) MIN(str + strLen, startOfDelim +  delimLen); //make sure that we don't jump the null terminator of str
        startOfDelim = strstr(str, delimiter);
        i++;
    }
    return arr;
} 


