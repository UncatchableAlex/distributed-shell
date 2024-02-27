#define MAXBUF 256  // max number of characteres allowed on command line
#define TRUE 1
#define FALSE 0

int dsh();
char **split(char *str, char *delimiter);
char *trim(char* str);
char *getFullPath(char *shortPath);
void deepFree2d(void **arr);
void spawnProcess(char *path, char **args, int foreground);
