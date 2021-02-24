/*Brian Duong
John Carroll
CS570
9/1/20

My program 1 uses the same code from my program 0 as a base with additional features required for the assignment.
The only difference is that I removed the if statement for duplicate characters as that is no longer needed in
prog1. Additionally, I added cases for all the known meta characters as well as some other special cases like a
preceding '/'. I have the same logic structure as prog0, where all the if statements in front serve as special
cases, whereas all the if statements in the overarching while loop add to the char array.
*/

#include <stdlib.h>
#include <stdio.h>
#include "getword.h"

int getword(char *w){

        /* Preface: The way I check the next character after iochar is using tempchar in combination with ungetc().
        Since I use this structure a lot for checking future characters, I want to explain it here in the beginning.
        I set tempchar = getchar() to save the next character for comparison with iochar; then I ungetc(tempchar, stdin)
        at the end of the comparison to put tempchar back on the stack negating the first getchar(). This allows me
        compare the 2 values without affecting the values in stdin */

        int iochar = getchar();         //sets iochar to the first character in stdin
        int count = 0;                  //variable for size of word
        int isNeg = 1;                  //boolean for detecting '$'
        int isEOF = 0;                  //boolean for checking EOF
        int tempchar;                   //temporary character for checking the next character after iochar
        int i;                          //variable for my for loop
        char * c;                       //character pointer for '~' case
        extern int bgFlag;
        bgFlag = 0;
        extern int tildaFlag;
        tildaFlag = 0;

        /* I use getenv("HOME") to retreive the pointer to the value of where "HOME" is and I set that
        equal to the 'c' variable I created above. This allows me to iterate through the name of the
        home directory using a for loop, and ultimately adding the directory to my character array. */
        c = getenv("HOME");

        /* This while loop iterates through the stdin using getchar whenever it encounters a space or
        tab. This allows getword to skip all leading tabs and spaces. */
        while((iochar == ' ') || (iochar == '\t')){
                iochar = getchar();
        }

        /* Checks for '\n' when the count is still zero and returns an empty array. */
        if(iochar == '\n'){
                *w = '\0';
                return 0;
        }

        /* This is my case for encountering a '~' at the beginning of a word. It uses a for loop to add the
        directory to the character array and allows for the rest of the word to be appended to the end. */
        if(iochar == '~'){

                tempchar = getchar();

                if(tempchar == EOF){    //if next char is EOF, terminate array and return
                        *w++ = '\0';
                        return count;
                }

                if((tempchar != ' ') && (tempchar != '\t') && (tempchar != '\n')){
                        tildaFlag = 1;
                }

                else{
                        for(i = 0; i < strlen(c); i++){         //loops through 'c' until it hits the end (size of c)
                                *w++ = c[i];                    //adds each char to the array
                                count++;                        //increments count
                        }
                }

                ungetc(tempchar, stdin);
                iochar = getchar();
        }

        /* Checks for a '$' at the beginning of a word. It sets the isNeg boolean to -1 allowing me to return
        a negative count. */
        if(iochar == '$'){
                if((tempchar = getchar()) == EOF){      //if next char is EOF, terminate array and return
                        *w++ = '\0';
                        return 0;
                }
                ungetc(tempchar, stdin);
                isNeg = -1;                             //sets boolean to negative
                iochar = getchar();                     //iterates stdin to avoid appending the '$' to the array
        }

        /* Checks for a metacharacter at the beginning of a word. It then adds the metacharacter to the array,
        terminates the array, and returns the count. The nested-if checks for the metacharacter '<<' */
        if(iochar == '<' || iochar == '>' || iochar == '|' || iochar == '&'){
                tempchar = getchar();
                if(iochar == '<' && tempchar == '<'){   //checks for '<<'
                        *w++ = (char) tempchar;         //adds the first '<' to the array
                        count++;                        //increments count
                        tempchar = getchar();           //iterates stdin to point to the second '<'
                }

                if((iochar == '&') && ((tempchar == '\n') || (tempchar == EOF) || (tempchar == ' '))) {
                        bgFlag = 1;
                }

                ungetc(tempchar, stdin);
                *w++ = (char) iochar;                   //adds the metacharacter to the array, terminates, and returns
                *w = '\0';
                count++;
                return count;
        }

        /* This is the main while loop that adds to the character array. All of the if statements are for different
        cases that getword encounters as it iterates through a word. The last three lines do the following:
        1.) adds iochar to the character array
        2.) increments count
        3.) iterates stdin to the next character
        The while loop ends when it hits a certain case, or EOF. */
        while(iochar != EOF){

                /* Checks if the word is larger than STORAGE-1. Terminates the array when it reaches the max size.
                The ungetc is needed because stdin is iterated 1 character too far when this if-statement is hit. */
                if(count == STORAGE - 1){
                        *w = '\0';
                        ungetc(iochar, stdin);          //ungetc puts the last character back on into stdin
                        break;
                }

                /* Checks for a space or a tab. Terminates the array and breaks out of the while loop. */
                if((iochar == ' ') || (iochar == '\t')){
                        *w = '\0';
                        break;
                }

                /* Checks for a metacharacter. Terminates the array and uses ungetc() to put the metacharacter
                back on the stack. This is required for the metacharacter if-statement before the while loop to run. */
                if(iochar == '<' || iochar == '>' || iochar == '|' || iochar == '&'){
                        *w = '\0';
                        ungetc(iochar, stdin);          //ungetc puts the metacharacter back into stdin
                        break;
                }

                /* Checks for a backslash. It then uses tempchar to determine the character after the backslash.
                If tempchar is a new line, it terminates and breaks. If tempchar is a metacharacter, it treats the
                metacharacter as a regular non-meta character. */
                if(iochar == '\\'){
                        tempchar = getchar();
                        if(tempchar == '\n'){           //checks for '\n'; terminates and breaks
                                *w = '\0';
                                break;
                        }
                        else if(tempchar == '<' || tempchar == '>' || tempchar == '|' || tempchar == '~'
                        || tempchar == '$' || tempchar == ' ' || tempchar == '\\' || tempchar == '&'){
                                ungetc(tempchar, stdin);        //negates the getchar() in the beginning
                                iochar = getchar();             //iterates past the '\\' to skip adding it to the array
                        }
                        else{
                                ungetc(tempchar, stdin);
                                iochar = getchar();             //Fix for input 12 of p2 autograder
                        }
                }

                /* Checks for a newline. Terminates the array and uses ungetc() to put the newline back onto the stack
                similar to how the metacharacter check works. This is required for the newline if-statement before the
                while loop to run. */
                if(iochar == '\n'){
                        *w = '\0';
                        ungetc(iochar, stdin);          //ungetc puts the newline back into stdin
                        break;
                }

                /* Checks for EOF. It sets the EOF boolean to true for a different check down the line. */
                if(tempchar == EOF){
                        isEOF = 1;
                }

                /* These three lines always run if none of the cases are true. */
                *w++ = (char) iochar;           //adds iochar to character array
                count++;                        //incremnts count
                iochar = getchar();             //iterates stdin to the next character
        }

        /* Checks for EOF. Terminates the array and returns -255. The nested if only occurs if EOF happens right
        after the last word. In that case, it terminates the array and returns the count instead. It then sets the
        EOF boolean back to false allowing for the base case to run the next time getword is called. */
        if(iochar == EOF){
                if(isEOF == 1){
                        *w = '\0';
                        isEOF = 0;
                        return count * isNeg;
                }
                *w = '\0';
                return -255;
        }