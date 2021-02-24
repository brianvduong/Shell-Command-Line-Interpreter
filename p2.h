#include <stdio.h>
#include "getword.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include "CHK.h"

#define MAXITEM 100 //max number of words per line
#define MAXSTORAGE 25500 //100 words can have 255 characters long
#define DELIM_FILE "delimitertestfile.bogusextention"
int parse();
void sighandler();
