/*
 * main.c
 *
 *  	Date: 2024-02-18
 *      Author: Alex M
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
	// loop until the user types exit or kills the process:
	while(TRUE) {
		char *cmdline = (char*) malloc(MAXBUF); // stores user input from commmand line
		// display the prompt:
		printf("dsh> ");
		if(!fgets(cmdline, MAXBUF, stdin)) {
			exit(0);
		}
		cmdline[strlen(cmdline) - 1] = '\0'; // get rid of the newline
		// trim the command line and free the old string
		char *ptr = cmdline;
		cmdline = trim(cmdline);
		free(ptr);
		// if the user didn't enter anything, then restart the loop
		if(!strlen(cmdline)) {
			free(cmdline);
			continue;
		}

		// determine if we should run in the foreground or background
		int foreground = TRUE;
		if (cmdline[strlen(cmdline) - 1] == '&') {
			foreground = FALSE;
			cmdline[strlen(cmdline) - 1] = '\0'; // get rid of the ampersand
		}
		// trim the cmdline again to remove any whitespace:
		ptr = cmdline;
		cmdline = trim(cmdline);
		free(ptr);
		// tokenize the input
		char **cmdArr = split(cmdline, " ");
		// if the user entered pwd and wants to see the current working directory:
		if (!strcmp(cmdArr[0], "pwd")) {
			getcwd(cwdBuf, MAXBUF);
			printf("%s\n", cwdBuf);
		}
		// if the user wants to exit
		else if (!strcmp(cmdArr[0], "exit")) {
			free(cmdline);
			deepFree2d((void**)cmdArr);
			exit(0);
		}
		// if the user entered cd
		else if (!strcmp(cmdArr[0], "cd")) {
			// if the user didn't specify which directory to change into, change into the home directory:
			if (!cmdArr[1]) {
				chdir(getenv("HOME"));
			}
			// if the user did specify the directory, resolve the path that we need to change into
			else {
				getcwd(cwdBuf, MAXBUF);
				char nextDir[(strlen(cwdBuf) + strlen(cmdArr[1]) + 2)];
				sprintf(nextDir, "%s/%s", cwdBuf, cmdArr[1]);
				// if we weren't able to change there, tell the user:
				int cdFail = chdir(nextDir);
				printf(cdFail ? "No such file or directory\n" : "");
			}
		}
		// otherwise, we assume that we are running a program
		else {
			char *fullPath;
			// If the first character is a slash, then we are being given an absolute path and no resolving is needed:
			if (cmdArr[0][0] == '/' && !access(cmdArr[0], F_OK | X_OK)) {
				fullPath = cmdArr[0];
			} 
			// otherwise resolve the full path of the the program to run
			else if (cmdArr[0][0] != '/') {
				fullPath = getFullPath(cmdArr[0]);
				free(cmdArr[0]);
				cmdArr[0] = fullPath;
			}
			// if we couldn't find the path to run, complain:
			if (!fullPath) {
				printf("Error: %s not found\n", cmdArr[0]);
			}
			// if we were able to find the path, run it
			else {
				spawnProcess(fullPath, cmdArr, foreground);
			}
		}
		free(cmdline);
		deepFree2d((void**)cmdArr);
	}
}
