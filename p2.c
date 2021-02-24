/*Brian Duong
John Carroll
CS570
10/9/20

My program 4 is a command line interpreter that builds off my previous program 2. However, this time around, there are a
few additional features that I added including: multiple pipes (up to 10), tilda metacharacter, $ metacharacter, environ,
>> metacharacter, and other needed additions. The multiple pipelines are still following vertical piping, and I didn't need
to change anything in my getword.c.

My program 2 is a simple command line interpreter that uses a slightly modified version of my getword.c from program 1.
I only have one declared function which is my parse() that utilizes getword.c to store each word into the charArray
while setting flags for each metacharacter. My main() handles all of the error checking as well as most of the functionality
of the command line interpreter. The only modification to my getword.c is the addition of a global background flag that I can
access in my p2.c. Every other flag is handled by my parse() function.
*/

#include "p2.h"
#include <stdlib.h>

//Global Variable Declarations
char charArray[MAXSTORAGE];     // character array passed into getword()
char *newargv[MAXITEM];         // stores each of the individual words
char *infile;                   // name of the infile
char *outfile;                  // name of the outfile
int numChar = 0;                // stores the value of getword
int pipeIndex;                  // index of where the pipe char is found
char *delimName;                // delim name for hereis flag
char *root;                     // variable to store root in cd
char *nextRoot;                 // variable to store the value after the root in cd
int pipePos[11];

//Flag Declarations
int inFlag = 0;                 // input flag
int outFlag = 0;                // output flag
int ampersandFlag = 0;          // ampersand flag
int pipeFlag = 0;               // pipe flag
int bgFlag = 0;                 // background flag
int printFlag = 0;              // print flag
int tildaFlag = 0;              // tilda flag
int hereisFlag = 0;             // hereis flag
int cdFlag = 0;                 // cd flag
int cdFlag2 = 0;                // cd special flag
int errorFlag = 0;              // error flag

int main(){
        int wordCount;          // number of words from parse
        pid_t pid, kidPid, cPid, gcPid, ggcPid;         // parent, kid, child, and grandchild pids

        int inFd = NULL;        // input file descriptor
        int outFd = NULL;       // output file descriptor
        int offset = 0;         // offset for piping
        int g = 0;              // variables for looping in pipe
        int w = 0;

        int flags = O_CREAT | O_RDWR | O_EXCL;          // flags for opening an output file
        int modes = S_IRUSR | S_IWUSR;                  // modes for opening an output file

        int fd[20];     // declare file descriptor array for piping

        size_t buf = 0;
        FILE *delimFile;
        char *delimLine;

        setpgid(0,0);
        (void) signal(SIGTERM, sighandler);

        for(;;){
                if(cdFlag == 1){
                        printf("%s", basename(get_current_dir_name())); // current directory name
                }
                else if(cdFlag2 == 1){
                        printf("/");    // shows root directory
                }

                printf(":570: ");
                wordCount = parse();

                /* Exits the for loop when EOF is encountered */
                if(numChar == -255){
                        break;
                }

                /* Reprompts if the line is empty */
                if(wordCount == 0){
                        if(inFlag != 0 || outFlag != 0){
                                fprintf(stderr, "Error: Redirection without a command.\n");
                                continue;
                        }
                        continue;
                }

                /* Case for the the $ or ~ error occurs */
                if(errorFlag > 0){
                        continue;
                }

                /* Case for when the hereis metacharacter is detected */
                if(hereisFlag != 0){
                        if(delimName == NULL){
                                fprintf(stderr, "Error: Missing hereis terminator.\n");
                                continue;
                        }
                        if(inFlag == 2){
                                fprintf(stderr, "Error: Ambiguous input redirect.\n");
                                continue;
                        }
                        strcat(delimName, "\n");        // saved to be used later in main
                        delimFile = fopen(DELIM_FILE, "w+");
                        while(getline(&delimLine, &buf, stdin) >= 0){   // loop to find the delimiter
                                if(strcmp(delimLine, delimName) == 0){
                                        break;          // break out once the delimiter is found
                                }
                                fprintf(delimFile, "%s", delimLine);    //print delim
                        }
                        free(delimLine);        // free previously allocated space
                        delimLine = NULL;
                        fclose(delimFile);      // close file

                        infile = DELIM_FILE;    // set infile to delimfile
                        inFlag = 2;
                        hereisFlag = 0;         // reset hereis flag
                }

                /* Case for when the first argument is "cd". */
                if(strcmp(newargv[0], "cd") == 0){
                        if(wordCount == 1){
                                if(chdir(getenv("HOME")) != 0){
                                        fprintf(stderr, "Error: No such HOME directory.\n");
                                }
                                else if(strcmp(getenv("HOME"), "/") == 0){      // find root directory
                                        cdFlag2 = 1;
                                }
                                else{
                                        cdFlag = 1;     // flag for current directory
                                }
                        }
                        else if(wordCount == 2){
                                if(chdir(newargv[1]) == -1){
                                        fprintf(stderr, "Error: No such file or directory.\n");
                                }
                                else{
                                        cdFlag2 = 0;    // reset cdFlag2
                                        cdFlag = 1;
                                }
                        }
                        else{
                                fprintf(stderr, "Error: Too many arguments for cd.\n");
                        }
                        continue;
                }

                /* Case for when the first argument is "environ". */
                if(strcmp(newargv[0], "environ") == 0){

                        char *environ;

                        // Case for "environ" with 1 argument
                        if(wordCount == 2){
                                if((environ = getenv(newargv[1])) != NULL){
                                        printf("%s\n", environ);
                                }
                                else{
                                        fprintf(stderr, "Error: No such file or directory: %s.\n", newargv[1]);
                                }
                        }

                        // Case for "environ" with 2 arguments
                        else if(wordCount == 3){
                                if(setenv(newargv[1], newargv[2], 1) == -1){
                                        fprintf(stderr, "Error: setenv has failed.\n");
                                }
                                if((environ = getenv(newargv[1])) == NULL){
                                        fprintf(stderr, "Error: No such file or directory.\n");
                                }
                        }

                        else{
                                fprintf(stderr, "Error: Too many arguments for environ.\n");
                        }
                        continue;
                }

                /* Case for when the input metacharacter is detected. */
                if(inFlag != 0){
                        if(inFlag > 2){
                                fprintf(stderr, "Error: More than 1 input redirection.\n");
                                continue;
                        }

                        if(infile == NULL){
                                fprintf(stderr, "Error: Missing name for redirection.\n");
                                continue;
                        }

                        else if((inFd = open(infile, O_RDONLY)) == -1){
                                fprintf(stderr, "Error: Failed to open inputfile %s.\n", infile);
                                continue;
                        }
                }

                /* Case for when the output metacharacter is detected. */
                if(outFlag != 0){
                        if(outFlag > 2){
                                fprintf(stderr, "Error: More than 1 output redirection.\n");
                                continue;
                        }
                        if(outfile == NULL){
                                fprintf(stderr, "Error: Missing name for redirection.\n");
                                continue;
                        }
                        else if((outFd = open(outfile, flags, modes)) == -1){
                                fprintf(stderr, "Error: Failed to open output file %s.\n", outfile);
                                continue;
                        }
                }

                /* Case for when the pipe metacharacter is detected. */
                if (pipeFlag > 0){
                        fflush(stdout);         // clears the output buffer before forking
                        fflush(stderr);         // clears the stderr buffer before forking

                        /* Case for successful child fork. The child handles the second half of vertical piping
 			which includes reading in the string into the input side of the pipe. It then calls execvp
                        on the second command. */
                        if((cPid = fork()) == 0){
                                CHK(pipe(fd));          // pipe the file descriptor array for reading and writing
                                fflush(stdout);         // clears the output buffer before forking
                                fflush(stderr);         // clears the output buffer before forking

                                /* Case for successful grandchild fork. The grandchild handles the first half of vertical
                                piping which includes sending the string into the output side of the pipe. It then calls
                                execvp on the first command. */
                                if((gcPid = fork()) == 0){
                                        for(g = 1; g <= pipeFlag; g++){
                                                CHK(pipe(fd+(2*g)));    // pipe the file descriptor at the point in the loop
                                                if(g < pipeFlag){
                                                        /* Case for successful greatgrandchild fork. The greatgrandchild
                                                        handles all the pipes except for the first one through the
                                                        for loop */
                                                        if ((ggcPid = fork()) == 0){
                                                                continue; // continue the loop by forking;

                                                        }

                                                        else{
                                                                CHK(dup2(fd[(2*g)-1], STDOUT_FILENO));  // copies STDOUT in write
                                                                CHK(dup2(fd[2*g], STDIN_FILENO));       // copies STDIN in read
                                                                for (w = 0; w < (2*g)+2; w++) {
                                                                        CHK(close(fd[w]));      // close the pipe
                                                                }
                                                                CHK(offset = (pipeFlag - 1) - g);
                                                                /* Runs all the commands after the first and second */
                                                                CHK(execvp(newargv[pipePos[offset]], newargv+pipePos[offset]));
                                                                exit(0);
                                                        }
                                                }

                                                else{
                                                        /* Case for input redirection in the pipe */
                                                        if (inFlag == 2){
                                                                CHK(dup2(inFd, STDIN_FILENO));
                                                                CHK(close(inFd));
                                                        }
                                                        CHK(dup2(fd[2*g-1], STDOUT_FILENO));
                                                        for (w = 0; w < (2*g+2); w++) {
                                                                CHK(close(fd[w]));
                                                        }
                                                        /* Runs the first command */
                                                        CHK(execvp(newargv[0], newargv));
                                                }
                                        }
                                }

                                else{
                                        /* Case for output redirection in the pipe */
                                        if (outFlag == 2){
                                                CHK(dup2(outFd, STDOUT_FILENO));
                                                CHK(close(outFd));
                                        }
                                        CHK(dup2(fd[0], STDIN_FILENO)); // copies STDIN in write
                                        CHK(close(fd[0]));              // close the read side of the pipe
                                        CHK(close(fd[1]));              // close the write side of the pipe
                                        offset = pipeFlag - 1;

                                        /* Runs the second command */
                                        CHK(execvp(newargv[pipePos[offset]], newargv + pipePos[offset]));
                                        exit(0);
                                }
                        }

                        /* Reaps the backgrounded zombie processes created by the childPid fork. */
                        for (;;) {
                                pid = wait(NULL);
                                if (pid == cPid) {
                                        break;
                                }
                        }
                }

                fflush(stdout);
                fflush(stderr);

                if((kidPid = fork()) == -1){
                        perror("Error: Cannot fork\n");
                        exit(1);
                }

                /* Case for successful kid fork */
                else if(kidPid == 0){

                        /* If it already encountered a pipe, it exits to avoid running an extra execvp. */
                        if(pipeFlag != 0){
                                exit(0);
                        }

                        /* Case for background metacharacter. If it encounters an "&" at the end of a line,
                        it will put the child process in the background and continue through the for loop. */
                        if(inFlag == 0 && ampersandFlag == 1){
                                /* If there is no input redirection, we change the child process to point
                                to NULL so it can finish the process without taking any more input. */
                                int nullFd;
                                if((nullFd = open("/dev/null", O_RDONLY)) == -1){
                                        fprintf(stderr, "Failed to open file.\n");
                                        exit(3);
                                }
                                dup2(nullFd, STDIN_FILENO);     // copies STDIN into the null file descriptor
                                close(nullFd);                  // closes the null file descriptor
                        }

                        /* Case for input redirection */
                        if(inFlag != 0){
                                dup2(inFd, STDIN_FILENO);       // copies STDIN into the input file descriptor
                                if(close(inFd) == -1){          // closes the input file descriptor
                                        fprintf(stderr, "Error! Unable to close file descriptor.\n");
                                        exit(4);
                                }
                        }
                        /* Case for output redirection */
                        if(outFlag != 0){
                                dup2(outFd, STDOUT_FILENO);     // copies STDOUT into the output file descriptor
                                if(close(outFd) == -1){         // closes the output file descriptor
                                        fprintf(stderr, "Error! Unable to close file descriptor.\n");
                                        exit(4);
                                }
                        }

                        /* Runs the command */
                        if(execvp(newargv[0], newargv) == -1){
                                fprintf(stderr, "Error! Cannot execute %s.\n", newargv[0]);
                                exit(2);
                        }
                }

                /* Case for background flag. Returns the command and PID if it encounters an "&" at the
                end of a line. */
                else if(ampersandFlag != 0){
                        printf("%s [%d]\n", newargv[0], kidPid);
                        continue;
                }

                /* Reaps the backgrounded zombie processes created by the kidPid fork. */
                else{
                        for(;;){
                                pid = wait(NULL);
                                if(pid == kidPid){
                                        break;
                                }
                        }
                }
        }
        killpg(getpgrp(), SIGTERM);     // kills the entire process group
        printf("p2 terminated. \n");
        exit(0);
}

/* My parse function utilizes getword() to store every individual word into the newargv array. It also sets flags
based on the metacharacters it detects for the functionality in main. */
int parse(){
        int wordCount = 0;
        int increment = 0;      // saves the size of the word to add to newargv
        int k = 0;              // counter to increment through newargv
        int posNumChar = 0;     // the absolute value of numChar

        /* Variables for $ metacharacter case */
        char *realPath = NULL;
        char *environment = NULL;
        char *temp = NULL;
        root = NULL;
        nextRoot = NULL;

        /* Reset the flags at the start of parse */
        inFlag = 0, outFlag = 0, pipeFlag = 0, ampersandFlag = 0, hereisFlag = 0, errorFlag = 0;
        infile = NULL, outfile = NULL;

        /* Variables for ~ metacharacter case */
        FILE *passFile = fopen("/etc/passwd", "r");
        size_t lineBufSize = 0;
        ssize_t lineSize;
        char *buffer = NULL;
        char *token = NULL;
        char *tokenResult = NULL;


        for(;;){

                numChar = getword(charArray + increment);       // initializes numChar using getword()

                /* Case for no characters or EOF */
                if(numChar == 0 || numChar == -255){
                        break;
                }

                /* Case for if the word count exceeds 100 */
                if(wordCount > MAXITEM){
                        fprintf(stderr, "Error! Too many arguments entered.\n");
                        break;
                }

                /* Case for detecting the dollar sign metacharacter */
                if(numChar < 0){
                        temp = charArray + increment;
                        environment = getenv(temp);     // get environment the variable after the $
                        if(environment == NULL){
                                fprintf(stderr, "Error: No such file or directory: %s\n", temp);
                                errorFlag++;
                        }
                        else{
                                if(outFlag == 1){
                                        outfile = environment;
                                }
                                else if(inFlag == 1){
                                        infile = environment;
                                }
                        }
                }

                /* Case for detecting the tilda metacharacter */
                if(tildaFlag == 1){
                        int p = 0;
                        temp = charArray + increment;

                        /* Case for when the /etc/passwd can't be opened */
                        if(!passFile){
                                fprintf(stderr, "Error: Cannot open /etc/passwd");
                                errorFlag++;
                        }
                        else{
                                root = strtok(temp, "/");       // set root to "/"
                                nextRoot = strtok(NULL, " ");   // set the char after the root
                                lineSize = getline(&buffer, &lineBufSize, passFile);

                                /* Loop through passFile and find the line with the correct token */
                                while(lineSize >= 0){
                                        token = strtok(buffer, ":");
                                        if(strcmp(token, temp) == 0){

                                                /* Go to the fifth token and store that value as the result */
                                                for(p = 0; p < 5; p++){
                                                        token = strtok(NULL, ":");
                                                }
                                                tokenResult = token;
                                                break;
                                        }
                                        lineSize = getline(&buffer, &lineBufSize, passFile);
                                }
                                free(buffer);
                                buffer = NULL;
                                fclose(passFile);
                                if(tokenResult == NULL){
                                        fprintf(stderr, "Error: Illegal name for absolute path %s.\n", tokenResult);
                                        errorFlag++;
                                }
                                else{
                                        if(nextRoot != NULL){
                                                strcat(tokenResult, "/");
                                                strcat(tokenResult, nextRoot);
                                        }
                                }
                        }
                }

                /* Case for detecting the background metacharacter */
                if(bgFlag == 1){
                        ampersandFlag = 1;
                        continue;
                }

                /* Case for detecting the hereis metacharacter */
                else if(((*(charArray + increment) == '<') && (*(charArray + increment + 1) == '<')) || hereisFlag == 1){
                        hereisFlag++;
                        if(hereisFlag == 2){
                                delimName = charArray + increment;      // find the delim argument
                        }
                        if(hereisFlag == 1){
                                delimName = NULL;       // set delim name to something
                        }
                }

                /* Case for detecting the input redirection metacharacter */
                else if(*(charArray + increment) == '<' || inFlag == 1){
                        inFlag++;
                        if(inFlag == 2){
                                infile = charArray + increment;         // sets infile to the name of the infile
                        }
                        else if(inFlag == 1){
                                infile = NULL;          // makes sure infile has a valid name
                        }
                }

                /* Case for detecting the output redirection metacharacter */
                else if(*(charArray + increment) == '>' || outFlag == 1){

                        /* Case for detecting the ">>" metacharacter */
                        if(*(charArray + increment + 1) == '>'){
                                newargv[k] = charArray + increment;
                                newargv[k+1] = charArray + increment + 1;
                                k++;
                        }
                        else{
                                outFlag++;
                                if(outFlag == 2){
                                        if(numChar < 0){        // skip the else, bc outfile is changed in $ case
                                        }
                                        else{
                                                outfile = charArray + increment;        // sets outfile to the name of the outfile
                                        }
                                }
                                else if(outFlag == 1){
                                        outfile = NULL;         // makes sure outfile has a valid name
                                }
                        }
                }

                /* Case for detecting the pipe metacharacter */
                else if(*(charArray + increment) == '|'){
                        newargv[k] = NULL;      // replaces the pipe with a null for future test
                        pipeIndex = k+1;        // set the pipe index for the pipe child execvp
                        pipePos[pipeFlag] = pipeIndex;
                        pipeFlag++;
                        k++;
                }

                else{
                        /* Case for $ metacharacter */
                        if(numChar < 0){
                                newargv[k] = environment;
                        }

                        /* Case for ~ metacharacter */
                        else if(tildaFlag == 1){
                                newargv[k] = tokenResult;
                                tildaFlag = 0;
                        }

                        /* Case for regular thing that happens regularly */
                        else{
                                newargv[k] = charArray + increment;     // sets each word to each index of newargv
                                        }
                                }
                                else if(outFlag == 1){
                                        outfile = NULL;         // makes sure outfile has a valid name
                                }
                        }
                }

                /* Case for detecting the pipe metacharacter */
                else if(*(charArray + increment) == '|'){
                        newargv[k] = NULL;      // replaces the pipe with a null for future test
                        pipeIndex = k+1;        // set the pipe index for the pipe child execvp
                        pipePos[pipeFlag] = pipeIndex;
                        pipeFlag++;
                        k++;
                }

                else{
                        /* Case for $ metacharacter */
                        if(numChar < 0){
                                newargv[k] = environment;
                        }

                        /* Case for ~ metacharacter */
                        else if(tildaFlag == 1){
                                newargv[k] = tokenResult;
                                tildaFlag = 0;
                        }

                        /* Case for regular thing that happens regularly */
                        else{
                                newargv[k] = charArray + increment;     // sets each word to each index of newargv
                        }

                //      printf("newargv[%d] = %s\n", k, newargv[k]);
                        wordCount++;                            // increment word count
                        k++;                                    // increment newargv counter
                }

                posNumChar = abs(numChar);              // disregard the $ metacharacter from getword
                increment += posNumChar + 1;            // increment based on the size of each word
        }
        newargv[k] = NULL;      // allows for execvp to know when to stop reading arguments
        return wordCount;
}

void sighandler(){
}
