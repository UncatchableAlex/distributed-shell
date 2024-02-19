#define MAXBUF 256  // max number of characteres allowed on command line

// TODO: Any global variables go below


int dsh();
char **split(char *str, char *delimiter);
char *trim(char* str);
char *getFullPath(char *shortPath);
void deepFree2d(void **arr);
int spawnProcess(char *path, char **args, int foreground);
