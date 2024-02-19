/*
 * main.c
 *
 *  	Date: 2024-02-18
 *      Author: alex
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include "dsh.h"

#define MAX_PROC 250

int main(int argc, char *argv[]) {

	// DO NOT REMOVE THE BLOCK BELOW (FORK BOMB PREVENTION) //
	struct rlimit limit;
	limit.rlim_cur = MAX_PROC;
	limit.rlim_max = MAX_PROC;
	setrlimit(RLIMIT_NPROC, &limit);
	// DO NOT REMOVE THE BLOCK ABOVE THIS LINE //
	char cwdBuf[MAXBUF];
	for(int i = 0; i < 5; i++) {
		char *cmdline = (char*) malloc(MAXBUF); // stores user input from commmand line
		printf("dsh> ");
		if(!fgets(cmdline, MAXBUF, stdin)) {
			exit(0);
		}
		cmdline[strlen(cmdline) - 1] = '\0'; // get rid of the newline
		cmdline = trim(cmdline);

		// determine if we should run in the foreground or background
		int foreground = 1;
		if (cmdline[strlen(cmdline) - 1] == '&') {
			foreground = 0;
			cmdline[strlen(cmdline) - 1] = '\0'; // get rid of the ampersand
		}
		char **cmdArr = split(cmdline, " ");
		if (!strcmp(cmdArr[0], "pwd")) {
			getcwd(cwdBuf, MAXBUF);
			printf("%s\n", cwdBuf);
		}
		else if (!strcmp(cmdArr[0], "exit")) {
			free(cmdline);
			deepFree2d((void**)cmdArr);
			exit(0);
		}
		else if (!strcmp(cmdArr[0], "cd")) {
			if (!cmdArr[1]) {
				chdir("/");
			}
			else {
				getcwd(cwdBuf, MAXBUF);
				char nextDir[(strlen(cwdBuf) + strlen(cmdArr[1]) + 2)];
				sprintf(nextDir, "%s/%s", cwdBuf, cmdArr[1]);
				int cdFail = chdir(nextDir);
				printf(cdFail ? "No such file or directory\n" : "");
			}
		}
		else {
			char *fullPath;
			// resolve the full path of the command that we were given:
			if (cmdline[0] == '/') {
				fullPath = cmdArr[0];
			} 
			else {
				fullPath = getFullPath(cmdArr[0]);
				free(cmdArr[0]);
				cmdArr[0] = fullPath;
			}
			printf("received path: |%s|\n", fullPath);
			if (!fullPath) {
				printf("Error: %s not found\n", cmdline);
			}
			else {
				spawnProcess(fullPath, cmdArr, foreground);
			}
		}
		free(cmdline);
		deepFree2d((void**)cmdArr);
	}


	
	// test my split function:
	/*
	char myStr[60];
	strcpy(myStr, "hello world hello mom world world world");
	char **splitArr = split(myStr, " world");
	int i = 0;
	while (splitArr[i]) {
		printf("|%s|\n", splitArr[i]);
		i++;
	}
	printf("lastindex: %d", splitArr[i]);

	return 0;
	*/
}
