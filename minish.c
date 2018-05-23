#include <stdio.h>		//printf
#include <stdlib.h>		//atoi
#include <string.h>		//puts,gets
#include <ctype.h>		//isspace
#include <unistd.h>		//execvp,setpid,getpid,getcwd,fork	
#include <signal.h>		//SIGINT
#include <sys/types.h>		//pid_t
#include <sys/wait.h>		//waitpid implementation

char *args[100];
char commandline[256];
int procid[20]={0};
char proc[100][20];
int counter=0;

int exeCommand();
int run(char *command);
char * skipSpace(char* x);

pid_t fgpsid=0;

void inthandler(int sig_num)		//signal handler for the parent(main) process.
{
	if(fgpsid!=0)
	kill(fgpsid, SIGKILL);
	return;
}


int main()
{
	signal(SIGINT,inthandler);	//parent register for the signal handling - SIGINT which is ctrl + c signal

	while(1)	//infinite loop for parent process
	{
		printf("minish >");
		fflush(stdin); //fflush() discards any buffered data that has been fetched from the underlying file, but has not been consumed 					by the application.
		char *command;
		
		command=fgets(commandline,1024,stdin);
		int cc=run(command);
	}
	return 0;
}

void showbg()				// shows the list of all processes running in background from array
{
	int k=0,l=0,status,ans;
	printf("\nList of all background processes\n");
	printf("\nProcess Id	Command	      Status\n");
	printf("-------------------------------------");
	
	for(k=0;procid[k]!=0;k++)
	{
		if(procid[k]!=1)
		{
			ans=waitpid(procid[k], &status, WNOHANG);
			if(ans==0)
			printf("\n%d           %s	      Running\n",procid[k],proc[k]);
			else
			printf("\n%d  	       %s	      Finished\n",procid[k],proc[k]);
		}
	}
}


void updatebg(pid_t x)		//Update the background process list if any background process bring to foreground by fg command
{
	for(int k=0;procid[k]!=0;k++)
	{
		if(procid[k]==x)
		{
			procid[k]=1;
			break;
		}	
	}
}


void killbg()			//kill the background processes and orphan processes when exit calls
{
	int k=0,l=0,status,ans;
	for(k=0;procid[k]!=0;k++)
	{
		ans=waitpid(procid[k], &status, WNOHANG);
		if(ans==0 && procid[k]!=0)
		kill(procid[k],SIGKILL);
	}
}

int exeCommand()		// Executes the commands with execvp system calls
{
	pid_t pid='\0',result;
	int flag=0,flag2=0,flag3=0,flag4=0;	
	int i=0;
	char p[100];
	
	
	while(args[i]!='\0')
	{
		strcpy(p,args[i]);

		if(strcmp(p,"exit")==0)		//if exit call occur it will call killbg() before exit
		{
			
			killbg();
			exit(0);
		}	
		if(strcmp(p,"&")==0)		//if & occurs as a last command, flag is set and & is replaced with null
		{
			args[i]='\0';
			flag=1;
			break;		
		}
		if(strcmp(p,"listjobs")==0)	//listjobs will call showbg()
		{
			showbg();flag2=1;break;
		}	
		if(strcmp(p,"fg")==0)		// takes bg process to foreground
		{
			flag3=1;break;
		}
		if(strcmp(p,"pwd")==0)		// flag for implementation of pwd
		{
			flag4=1;break;
		}
		i++;
	}
	
	if((args[0] != NULL) && strcmp(args[0],"cd")==0)	// implementation for cd command
	{
		if(chdir(args[1])==0) 
		{
			printf("directory Changed\n");
			return 0;
		}
		else
		{
			printf("No such directory found\n");
			return 0;	
		}
	}
	
	if(flag4==1)						// implementation of pwd command
	{
		char cwd[200];
		if (getcwd(cwd, sizeof(cwd)) == NULL)
		      perror("getcwd() error");
    		else
		      printf("current working directory is: %s\n", cwd);
		return 0;
	}
			
	if(flag3==1 && args[1]!=NULL)				//if any bg process bring to fg, parent will wait for that to finish
	{
			int returnStatus;
			fgpsid=atoi(args[1]);			// set foreground process id to current process,for signal handling
			updatebg(atoi(args[1]));		// update the bg process list
			result=waitpid(atoi(args[1]), &returnStatus,0);   	//wait for process
			return 0;
	}
	
	if(flag2==1){printf("\n");return 0;}
		
	pid = fork();			//parent fork a new process
	
	if (pid == -1) 
	{
		fprintf(stderr, "fork failed\n"); 
		exit(1); 
	}
 
	if(pid>0 && flag==0)		// if no & sign in the command than fgpsid is a new created process id
	{
		fgpsid=pid;
	}	

	if(pid==0) 			// grab the child
	{
		setpgid(pid,0);

		if (execvp( args[0], args) == -1)
		{
			perror(args[0]);//printf("Command not found\n");
			exit(EXIT_FAILURE); 
		}	
	}
	
	if(flag!=1)	// if flag==0 then parent must wait for child
	{
			int returnStatus;
			result=waitpid(pid, &returnStatus, 0);		// wait for child to complete, library -sys/types and sys/wait
			fflush(stdin);
	}

	if(flag==1 && pid>0)	// Add new background process id and command for that to the array list and increment the counter
	{
		char p[100],*s;
		procid[counter]=pid;
		strcpy(p,args[0]);
		strcpy(proc[counter],p);

		counter++;
	}
	
	
	return 0;
}

int run(char *command)			//separates the arguments and store it in individual string
{

	char* findnextspace;
	char* findnewline;

	int m=0;
	
	command=skipSpace(command);				// skip white space from starting
	
	findnewline=strchr(command,'\n');			// last character of command is \n which needs to be replaced with null

	if(findnewline!='\0')
	{
		findnewline[0]='\0';
	}
	
	findnextspace=strchr(command,' ');			// breaks whole argument by space between them	

	if(findnextspace !='\0')
	{
		while(findnextspace !='\0')				
		{
			findnextspace[0]='\0';			
			args[m]=command;
			m++;
			command=skipSpace(findnextspace+1);	
			findnextspace=strchr(command,' ');			
		}
	}
	if(command[0]!='\0')
	{	
		args[m]=command;
		m++;
	}	
	args[m]='\0';
	return exeCommand();		
}

char * skipSpace(char* x)					//remove space from the command
{	
	while(isspace(*x))
	++x;
	return x;
}
