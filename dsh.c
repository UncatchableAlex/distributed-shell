/*
 * dsh.c
 *
 *  Created on: 2024-02-18
 *      Author: Alex M
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

// Spawn a child process and run the program at the given path if it is runnable.
// set foreground to true if the parent should wait for the child.
void spawnProcess(char *path, char **args, int foreground) {
    int childPid = fork();
    // if it's the child:
    if (!childPid) {
       execv(path, args);
    }
    // if it's the parent, wait for the child if instructed
     else if (foreground) {
        wait(NULL);
        return;
    }
    return;
}

/**
* Given a short path, resolve the full path and return a pointer. If the shortpath refers to a file
* in the current working directory, then that file will be used. Otherwise, we will look for a 
* file on the PATH and return the full path to that instead.
*/
char *getFullPath(char *shortPath) {
    // allocate space on the stack to store the shortPath concatenated with the working directory
    char cwdBuf[MAXBUF];
    int shortPathLen = strlen(shortPath);
    getcwd(cwdBuf, MAXBUF);
    char *fullPathMaybe = (char*)malloc((shortPathLen + strlen(cwdBuf) + 2) * sizeof(char));
    sprintf(fullPathMaybe, "%s/%s", cwdBuf, shortPath);
    // check if the file is in the working directory:
    if(access(fullPathMaybe, F_OK | X_OK) == 0) {
        return fullPathMaybe;
    }
    // copy the path into a dedicated buffer:
    char *pathOrig = getenv("PATH");
    char path[strlen(pathOrig) + 1];
    strcpy(path, pathOrig);
    // get each sub-path
    char **dirs = split(path, ":");
    int i = 0;
    char *fullPath = NULL;
    free(fullPathMaybe);
    // check every subpath for the file
    while (dirs[i]) {
        fullPathMaybe = (char*)malloc((shortPathLen + strlen(dirs[i]) + 2) * sizeof(char));
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

/**
* Returns a version of the inputted string without trailing and leading whitespace
*/
char *trim(char *str) {
    if (!str) {
        return NULL;
    }
    int strLen = strlen(str);
    int startString = 0;
    int endString = strLen - 1;
    // get a pointer to first real character in the string:
    while (str[startString++] == ' ');
    // get a pointer to the last real character in the string:
    while ((endString > startString) && str[endString--] == ' ');
    // adjust our pointers because we overshot:
    startString--;
    endString += 2;
    // allocate space for the string that we are going to return:
    char *newStr = (char*) malloc((1 + endString - startString) * sizeof(char));
    str[endString] = '\0';
    strcpy(newStr, str + (startString * sizeof(char)));
    return newStr;
}

// Utility function to free everything in an array of pointers
void deepFree2d(void **arr) {
    int i = 0;
    while (arr[i]) {
        free(arr[i]);
        i++;
    }
    free(arr);
}

/**
 * Tokenize a string into an array by a delimiter. The string is destroyed
*/
char **split(char *str, char *delimiter) {
    if (!str || !delimiter || !strlen(str)) {
        printf("returning null\n");
        return NULL;
    }
    int len = strlen(str);
    // count the number of items that will be in our returned array
    int itemCount = 1; // we start item count at 2 because we will be adding DELIMITERS not actual items
    int delimLen = strlen(delimiter);
    for (int i = 0; i <= len - delimLen; i++) {
        // chop the string into a substring from i to i + delimLen
        char breakChar = str[i + delimLen];
        str[i + delimLen] = '\0';
        // if the substring str(i, i+delimLen) is our delimiter, increase the itemCount
        if (!strcmp(str + (i*sizeof(char)), delimiter)) {
            itemCount++;
        }
        // put that character back (we need it! It's part of the string that we are digesting):
        str[i + delimLen] = breakChar;
    }
    char *startOfDelim = strstr(str, delimiter);
    char **arr = (char**) malloc(sizeof(char*) * (itemCount + 1)); // itemCount+1 because we need to make room for the NULL terminator
    arr[itemCount] = NULL; // set the last element to null before we even start
    int i = 0;
    while (str[0] != '\0') {
        int strLen = strlen(str);
        startOfDelim = startOfDelim ? startOfDelim : str + strLen;
        int pos = startOfDelim - str;
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


