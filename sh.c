//SAM
//ANDRES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>


void runcmd(char *buf);
int getcmd(char *buf, int nbuf);
void openFile(char *command[]);
void signal_handler(int signo);
void child_signal_handler(int signo);
void pipeFile(char *command[]);


int main(void){
  static char buf[100];

  printf("Welcome!\n");

  signal(SIGINT, signal_handler);
  
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      if(chdir(buf+3) < 0){
        fprintf(stderr, "cannot cd %s\n", buf+3);
      }
      continue;
    }
    
    char *background[100];
    background[0] = strtok(buf, " ");

    printf("%li\n",sizeof(buf));

    for(int i = 1; i < sizeof(buf); i++){
      background[i] = strtok(NULL, " ");
      if(background[i] == "&"){
       execvp(background[0], background);
      }
    }
  
    
    if(fork() == 0){
      signal(SIGINT, child_signal_handler);
      printf("\n");
      runcmd(buf);
    }    
    wait(NULL);


  }

  exit(0);
}



//shell prompt
int getcmd(char *buf, int nbuf){
  char *s = 0;
  fprintf(stderr, "shell$ ");
  fflush(stdout);
  memset(buf, 0, nbuf);
  s = fgets(buf, nbuf, stdin);\
  if(s == 0){
    return -1;
  }
  buf[strcspn(buf, "\n")] = '\0';
  return 0;
}


//selects command to run
void runcmd(char *buf){
  char *command[100];
  bool isPipe = false;
  command[0] = strtok(buf, " ");

  int i = 1;
  while(command[i] != NULL){
    command[i] = strtok(NULL, " ");
    if(command[i] == "|"){
      isPipe = true;
    }
    i++;
  }

  printf("%d\n", isPipe);

  //list of comands
  if(isPipe){
    printf("%d-----\n", isPipe);
    pipeFile(command);

  } else if(strcmp(command[0], "ls") == 0) {
    execvp(command[0], command);

  } else if(strcmp(command[0], "pwd") == 0){
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);

  } else if(strcmp(command[0], "cat") == 0){
    openFile(command);

  } else if(strcmp(command[0], "clear") == 0){
    system("clear");

  } else {
    printf("WENT TO ELSE\n");
    if(command[0] == 0){
      exit(-1);
    }
    execvp(command[0], command);
    fprintf(stderr, "exec %s failed\n", command[0]);
  }
}




//pipe file
void pipeFile(char *command[]){
  printf("IN THE PIPE\n");
  int fd[2];
  char *commandParent[50], *commandChild[50];
  bool pipeFound = false;
  int check = 0;
  int i = 0;

  while(command[i] != NULL){
    if(strcmp(command[i], "|") == 0){
        pipeFound = true;
    }
    if(!pipeFound){
      commandParent[i] = command[i];
    }else{
      commandChild[check] = command[i];
      check++;
    }
    i++;
  }


  pipe(fd);

  if(fork()){
    dup2(fd[0], 0);
    close(fd[0]);
    close(fd[1]);
    execvp(commandChild[0], commandChild);
  } else {
    dup2(fd[1], 1);
    close(fd[0]);
    close(fd[1]);
    execvp(commandParent[0], commandParent);
  }

}



//cat command
void openFile(char *command[]){
  char buffer[1000];
  int read_size;

  if(command[1] == NULL){
    fprintf(stderr, "Error: usage: cat filename\n");
  }

  int file = open(command[1], O_RDONLY);

  if(file == 1){
    fprintf(stderr, "Error: %s: file not found\n", command[1]);
  }

  while((read_size = read(file, buffer, 1000)) > 0){
    write(1, &buffer, read_size);
  }
  
  close(file);
}


//handles children processes
void child_signal_handler(int signo){
 if(signo == SIGINT){
    fprintf(stdout, "Caught signal %d\n", signo);
    fflush(stdout);
    //signal(SIGINT, child_signal_handler);
  }

}


//handles processes signals
void signal_handler(int signo){
  if(signo == SIGINT){
    fprintf(stdout, "Caught signal %d\n", signo);
    fflush(stdout);
  }
}


