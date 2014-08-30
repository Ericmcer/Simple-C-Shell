#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>


#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */

char *COMMANDHISTORY[80];
int HISTORYSIZE = 0;

// setup() reads in the next command line, separating it into distinct tokens


void setup(char inputBuffer[], char *args[],int *background)
{
    int length, i, start, ct;     
   
    ct = 0;

    // read what the user enters on the command line
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE); 
    
    start = -1;
    if (length == 0)
        exit(0);            
    if (length < 0){
        perror("error reading the command");
        exit(-1);           
    }

    // examine each character in the inputBuffer 
    for (i=0;i<length;i++) {
        switch (inputBuffer[i]){
          case ' ':
          case '\t' :               
            if(start != -1){
                 args[ct] = &inputBuffer[start];
                ct++;
            }
            inputBuffer[i] = '\0'; // append null char to c-string
            start = -1;
            break;
          case '\n':                 
            if (start != -1){
                    args[ct] = &inputBuffer[start];    
                ct++;
            }
                inputBuffer[i] = '\0';
                args[ct] = NULL; 
            break;
          default :             
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&'){
                *background  = 1;
                start = -1;
                inputBuffer[i] = '\0';
            }
          }
     }   
     args[ct] = NULL; 
     
}




//whisper function-lowercase string after whisper call
void whisper(char *args[MAX_LINE/2+1]){
 
  int i = 1;

  while(args[i] != NULL){
      if(*args[i] == '\0'){
        i++;
        printf(" ");
      }
      else{
        putchar(tolower(*args[i]));
        args[i]++;
      }
  }
  printf("\n");

}

//History Function-echo last 10 commands entered
void history(){
  int nums = HISTORYSIZE;
  int i;

  if(nums < 9 || nums == 9){
    for(i = 0; i<nums; i++){
      printf("%d : %s\n", i+1,COMMANDHISTORY[i]);
     
    }
  }
  else if (nums > 9){
    for(i = nums-10; i<nums;i++){
      printf("%d : %s\n", i+1, COMMANDHISTORY[i]);
    }
  }  
}


//check tokens for recognized tokens, return 0 if unrecognized,
//this also handles the exit.
int processInput(char *args[MAX_LINE/2+1]){
  
  if(args[0] == NULL){
    return 0;
  }
  if (0 == strcmp(args[0],"whisper")){
    whisper(args);
    return 0;
  }
  if (0 == strcmp(args[0],"exit")){
    
      char buf[52];
      sprintf(buf, "ps -p %d -o pid,ppid,pcpu,pmem,etime,user,command", getpid()); 
      system(buf);
      exit(0);
    
  }
  if (0 == strcmp(args[0],"r")){
    history();
    return 0;
  }
  return 1;
  

}

//This will be called if ctrl-z is caught 
void signalHandler(int sig){
    char *tempArgs[1];
    tempArgs[0] = "r";
    processInput(tempArgs);
}


int main(void)
{    
    //########### Initial Setup ############### 
    char inputBuffer[MAX_LINE];      /* buffer to hold the command entered */
    int background;              /* equals 1 if a command is followed by '&' */
    char *args[(MAX_LINE/2)+1];  /* command line (of 80) has max of 40 arguments */
    int commandCounter = 1;      //number of commands given
    printf("Welcome to EMshell. My pid is %i\n", getpid());
    
      
    //########## Signal Handler Setup #############
      struct sigaction handler;
      handler.sa_handler = signalHandler;
      handler.sa_flags = SA_RESTART;
      sigaction(SIGTSTP, &handler, NULL);

      

    //#### Main loop, will process user input and execute commands as they are given #######

    while (1){            // Program terminates normally inside setup 
       background = 0;
       printf("emshell[%i]\n", commandCounter );
       setup(inputBuffer,args,&background);       // get next command 
       commandCounter++;
       
       //store commands for history
        if(HISTORYSIZE < 80){
          char *buffcopy = (char *) malloc(20 * sizeof(char));
          strcpy(buffcopy, inputBuffer);
          COMMANDHISTORY[HISTORYSIZE] = buffcopy;
          HISTORYSIZE++;
        }


       //if recognized command processInput returns 0, if not returns 1 and fork creates child to handle command
       if(processInput(args)){
         pid_t  pid;
         pid = fork();
         if (pid < 0) { /* error occurred */
           fprintf(stderr, "Fork Failed");
           return 1;
         }
         else if (pid == 0) { /* child process */
          execvp(args[0], args);
        }
        else { /* parent process */
       /* parent will wait for the child to complete */
         if(background = 1){ 
         waitpid(pid);
         printf("Child Complete\n");
         }
        }
      }    

      
    }
 
    
}
