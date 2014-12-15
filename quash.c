/* Author Bharath Chandra Elluru, Siva Pramod Bobbilli.
*Date:03/26/2014, Version:1.0.
*This program is implementing quash shell which mimics 
*some of the functionalities of bash shell.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define PATH "set path=" //used for set command is passed to quash shell.
#define DELIMITER ":"    //used while set command is run.
#define TRUE (!FALSE)
#define FALSE (0)
#define UNDEFINED 0
#define FRG 1
#define BKG 2
#define STP 3
int nextjid=1;  	//used for job id
extern int errno;
int err=0;
int status;
int fdout,fdin;		//filedescriptors used during redirection
char *cwdtemp= "";

//This structure is used to store a particular job
struct job_t{
pid_t pid;
int jid;
int state;
char *inputcmd;
};
struct job_t job_list[20];		//array used to store a list of jobs, maximum number of jobs are limited to 20

//function used to display listjob_t  of jobs to the user
void listjobs(struct job_t *job_list,int fd){
	int i;
	char buf[1024];
	for(i=0;i<20;i++){
		memset(buf,'\0',1024);
		if(job_list[i].pid != 0){
			sprintf(buf,"[%d] %d ",job_list[i].jid,job_list[i].pid);
			write(fd,buf,strlen(buf));
			memset(buf,'\0',1024);
			sprintf(buf,"%s\n",job_list[i].inputcmd);
			write(fd,buf,strlen(buf));
		}
	}
}

//function used to clear the jobs list
void clearjob(struct job_t *job){
	job->pid=0;
	job->jid=0;
	job->state=UNDEFINED;
	job->inputcmd='\0';
}

//function returns maxjid from the job list
int maxjid(struct job_t *job_list){
	int i,max=0;
	for(i=0;i<20;i++){
		if(job_list[i].jid>max)
			max=job_list[i].jid;
	}
	return max;
}

//function used to initialize the job list
void initjobs(struct job_t *job_list){
	int i;
	for(i=0;i<20;i++){
		clearjob(&job_list[i]);
	}
}

// function used to add job to job list
void addjob(struct job_t *job_list,pid_t pid, int state,char *input){
int i;
for(i=0;i<20;i++){
		if(job_list[i].pid==0){
			job_list[i].pid=pid;
			job_list[i].state=state;
			job_list[i].jid=nextjid++;
			job_list[i].inputcmd=input;
			if(nextjid>20){
			nextjid=1;
			}
		break;
		}
	}
}

// function used to delete job from job list
void deletejob(struct job_t *job_list,pid_t pid){
int i;
for(i=0;i<20;i++){
		if(job_list[i].pid==pid){
			clearjob(&job_list[i]);
			nextjid=maxjid(job_list)+1;
			break;
			}
		}
}

//function used to find job based on pid
struct job_t *getjob(struct job_t *job_list,pid_t pid){
	int i;
	for(i=0;i<20;i++){
		if(job_list[i].pid== pid)
			return &job_list[i];
	}
	return &job_list[i];
}

//function used to find job based on jid
struct job_t *getjobjid(struct job_t *job_list,char *usrcmdargs[]){
	int i;
	int j=atoi(usrcmdargs[2]);
	for(i=0;i<20;i++){
		if(job_list[i].jid== j)
			return &job_list[i];
	}
	return &job_list[i];
}

/*function used to handle the signal SIGINT, when user presses ctrl+c,
  we must still display quash>, not bash prompt*/
void handle_signal(int signo){
  printf("\n%s", cwdtemp);
  printf("~quash>");
  fflush(stdout);
}
//function used to handle SIGCHILD and delete the job from job list
void handle_sigchild(int signo){
  int status;
  pid_t pid;
  struct job_t *tempjob_list;
  while((pid = waitpid(-1,&status,WNOHANG|WUNTRACED)) > 0){
	if(WIFEXITED(status)){		//checks if child process is terminated normally
		tempjob_list=getjob(job_list,pid);
		printf("[%d] %d finished %s\n",tempjob_list->jid,tempjob_list->pid,tempjob_list->inputcmd);
		deletejob(job_list,pid);	//if child terminates normally, delete it from the jobs list
	}
	else if(WIFSIGNALED(status)){
		deletejob(job_list,pid);
	}
	else if(WIFSTOPPED(status)){
		tempjob_list=getjob(job_list,pid);
		tempjob_list->state=STP;
	}
  }
}

/*function used to split the command cmd by space(" ") and store them in an array 
  so that we can pass array to execvp command to execute the command*/
void split(char* cmd, char *args[]){
  char *p;
  int j=0;
  p=strtok(cmd," ");  		//tokanize the user input command by using space(" ") as delimiter and save it in p.
  while(p != NULL)
  {
    args[j]=p;
    j++;
    p=strtok(NULL," ");		
  }
  args[j]='\0';
}

void splitforpath(char* cmd, char *args[]){
  char *p;
  int j=0;
  p=strtok(cmd,"=");  		//tokanize the user input command by using space(" ") as delimiter and save it in p.
  while(p != NULL)
  {
    args[j]=p;
    j++;
    p=strtok(NULL," ");		
  }
  args[j]='\0';
}
/*function used to read a line */
char* readLine(char* line, size_t len){ 
  getline(&line, &len, stdin);            
  *(line+strlen(line)-1)='\0';
  return line;
}

/*function used to run user command using execvp*/
void run(char *input,char *usrcmdargs[],int is_background){
  int pid1;
  pid1=fork();
  struct job_t *tempjob_list;
  if(pid1 == 0){
    close(fdin);
    close(fdout);
    //execvp return -1 if command do not exist, if it is successful, it do not return any value
    err=execvp(usrcmdargs[0],usrcmdargs);	
    printf("errno is %d\n", errno);
    if(err == -1){
      printf("command do not exist\n");	
		exit(EXIT_FAILURE);
    }
    exit(0);
  }
  else if(is_background != 0){
	addjob(job_list,pid1,BKG,input);
	tempjob_list=getjob(job_list,pid1);
    printf("[%d] %d running in background\n",tempjob_list->jid,pid1);
  }
  else{
	//addjob(job_list,pid1,FRG,input);
    waitpid(pid1,&status, 0);
  }
}

//function used to changedirectory
void changedir(char *directory){
  if(!(directory)){
    chdir(getenv("HOME"));
  }
  else{
    if(chdir(directory) == -1){
      printf("Invalid Directory path\n");
    }
  }
}

//function used to check if the command consists of the second parameter passed to function
int checkforsymbol(char *input,char symbl){
  int i=0;
  int returnvalue=0;
  for(;i<strlen(input);i++){
    if(input[i] == symbl){
      returnvalue=i;
      break;
    }
  }
  return returnvalue;
}

//function used to run the command if the command consists single pipe
void runpipe(char *usrcmdargs[],char *usrcmdargs2[]){
  int mypipe[2];	
  pipe(mypipe);
  int pid2;
  pid2=fork();
  if(pid2 == 0){
    dup2(mypipe[1],STDOUT_FILENO);
    close(mypipe[0]);
    close(mypipe[1]);
    execvp(usrcmdargs[0],usrcmdargs);
    exit(0);
  }
  else{
    dup2(mypipe[0],STDIN_FILENO);
    close(mypipe[1]);
    close(mypipe[0]);
    waitpid(pid2,&status, 0);
    execvp(usrcmdargs2[0],usrcmdargs2);
  }
}

//function used to split the user command, check for pipe and execute the command
void execute_command(char *input, int is_background){		//function to execute a command
  char *input2=NULL;
  char *inputtemp=NULL;
  char *usrcmdargs[100];  		//array to save the user input after spliting by spaces
  char *usrcmdargs2[50]; 
  char *usrcmdsargspath[50];  
  int i;
  pid_t pid6;
  struct job_t *tempjob_list;
  inputtemp=(char *) malloc (100 + 1);
    strcpy(inputtemp,input);
  i=checkforsymbol(input,'|');
  if(i == 0){
    split(input, usrcmdargs);
    if(usrcmdargs[0]){		//checking for \n condition, when user is pressing enter.
      if(strcmp(usrcmdargs[0],"cd") == 0){
        changedir(usrcmdargs[1]);
      }
	  else if(strcmp(usrcmdargs[0],"kill") == 0){
		//pid6 = fork();
		//if(pid6 == 0){
		tempjob_list=getjobjid(job_list,usrcmdargs);
		kill(tempjob_list->pid,9);
		//}
		//else{
			//waitpid(pid6,&status,0);
		//}
	  }
      else if((strcmp(usrcmdargs[0], "exit") == 0) || (strcmp(usrcmdargs[0], "quit") == 0)) {                  
        exit(0);
      }
	  else if(strcmp(usrcmdargs[0], "jobs") == 0) {   
        listjobs(job_list,STDOUT_FILENO);
      }
	  else if(strcmp(usrcmdargs[0], "set") == 0) { 
		splitforpath(inputtemp,usrcmdsargspath);
		if((strncmp(usrcmdargs[1],"HOME=",5))==0){
			if((setenv("HOME",usrcmdsargspath[1],1))==-1){
				printf("could not set the given path to HOME");
			}
		}
		else if((strncmp(usrcmdargs[1],"PATH=",5))==0){
			if((setenv("PATH",usrcmdsargspath[1],1))==-1){
				printf("could not set the given path to PATH");
			}
		}
      }
      else{
        run(inputtemp,usrcmdargs, is_background);
      }
    }
  }
  else{
    input2 = &input[i+1];
    input[i]='\0';
    split(input, usrcmdargs);
    split(input2, usrcmdargs2);
	pid6=fork();
    if(pid6 == 0){             //creates a child process
      runpipe(usrcmdargs, usrcmdargs2);
    }
    else{
	  waitpid(pid6,&status,0);         //wait till the child process terminates
	  //addjob(job_list,pid6,FRG,inputtemp);	  
    }
  }
}

//main function which will be called first, envp[] consists all the environment variables
int main(int argc, char *argv[],char *envp[])
{
  char *input=NULL;	//stores user input
  
  int j,k;
  char *filename=NULL;
  pid_t pid3,pid4;
  char *cwd;
  char *usrcmdargs[100];
  int is_background;		//used to determine a background process
  FILE *fp;
  char *s;

  input = (char *) malloc (100 + 1);
  initjobs(job_list);
  while (1) {
	printf("in main %s\n",getcwd(NULL, 64));
    if ((cwd = getcwd(NULL, 64)) == NULL) {
      perror("pwd");
      exit(2);
    }
	
	printf("in main\n");
    cwdtemp=cwd;
    signal(SIGINT,SIG_IGN);           //capture SIGINT signal and ignore its default function.
    signal(SIGINT,handle_signal);	  //capture SIGINT signal and redirect to handle_signal.
	//signal(SIGCHLD,SIG_IGN); 
	signal(SIGCHLD,handle_sigchild);	  
    printf("%s", cwd);
    printf("~quash>");
    input = readLine(input,100);
	
    is_background = checkforsymbol(input,'&');
    if(is_background !=0){
      input[is_background] = '\0';
    }
    j=checkforsymbol(input,'>');		//checking if output redirection is present in command
    k=checkforsymbol(input,'<');		//checking if output redirection is present in command
    if(j !=0){
      filename=&input[j+1];
      input[j]='\0';			//execute the output redirection block
      if(filename){
        pid3=fork();
        if(pid3 == 0){             //creates a child process			
          fdout = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
          dup2(fdout, STDOUT_FILENO);
          execute_command(input,is_background);
          close(fdout);
          exit(0);
        }
        else if(is_background !=0){
          printf("[1] %d\n",pid3);
        }
        else{
          waitpid(pid3,&status, 0);
        }
      }
      else{
        printf("Please enter the file name");
      }
    }
    else if(k !=0){
      filename=&input[k+1];
      input[k]='\0';			//execute the input redirection block	
      if(strcmp(input,"quash") == 0){	//block used to read commands from a file and execute them
        fp = fopen(filename, "r");
        while(fgets(input, 1024, fp) != NULL){
		  s=strchr(input,'\n');
		  if(s != NULL){			// if \n is present in the command, then remove it and replacing by \0
          input[strlen(input)-1] = '\0';
		  }
	  execute_command(input,is_background);
        }
	fclose(fp);
      }
     else{				//block used to perform input redirection operation
        pid4=fork();	
        if(pid4 == 0){	
          fdin = open(filename,O_RDONLY);
          dup2(fdin, STDIN_FILENO);
          close(fdin);
          split(input, usrcmdargs);
          if(usrcmdargs[0]){		//checking for \n condition, when user is pressing enter.
            if(strcmp(usrcmdargs[0],"cd") == 0){
              changedir(usrcmdargs[1]);
            }
            else if((strcmp(usrcmdargs[0], "exit") == 0) || (strcmp(usrcmdargs[0], "quit") == 0)) {                  
              exit(0);
            }
            else{
              err=execvp(usrcmdargs[0],usrcmdargs);
              printf("errno is %d\n", errno);
			  if(err == -1){
				printf("command do not exist\n");
				return EXIT_FAILURE;
			  }
            }
          }
		  exit(0);
        }
        else if(is_background !=0){
          printf("[1] %d\n",pid4);
        }
        else{
          waitpid(pid4,&status, 0);
        }
      }
    }
    else{
      execute_command(input,is_background);
    }
  }
  free(input);
}
