#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_LINE 80 //The maximum length command
#define MAX_COMMANDS 20 // size of commands, which history array can store

char history[MAX_COMMANDS][MAX_LINE];	
char display_history [MAX_COMMANDS][MAX_LINE];	

int CountHistory = 0;
char* ofile;		//flag for Output file
char* apofile;		//flag for Append file
char* ifile;		//flag for Input file
char* ipipe;		//flag for pipe
//To SaveHistory
void SaveHistory(char Input[]) 
{
	int i = 0;		
	strcpy(history[CountHistory % MAX_COMMANDS], Input);	// add command
	while (Input[i] != '\n' && Input[i] != '\0')			// add display_history 
	{
		display_history[CountHistory % MAX_COMMANDS][i] = Input[i];
		i++;
	}
	display_history[CountHistory % MAX_COMMANDS][i] = '\0';
	CountHistory+=1;
}
//To PrintHistory
void historyfeature(char Input[])
{
	int temp;
	int i;
	if (CountHistory < MAX_COMMANDS)
	{
		temp = CountHistory;
	}
	else
	{
		temp = MAX_COMMANDS;
	}
	for (i = 0; i < temp; i++) 
	{
		printf("%d \t %s\n", i, display_history[i]);
	}
}
//Execute_Function
int setup(char inputBuffer[], char *args[],int *background,char* args_pipe[])
{
	char **args2;	
	args2 = malloc(41 * sizeof(char*));
    int i,j,k,h;
    int check,check1,check2,check3;
    check=0;
    check1=0;
    check2=0;
    check3=0;
    int command_number;
    int count_flag;
    ofile = NULL;
    apofile=NULL;
    ifile=NULL;
    ipipe=NULL;
    count_flag=0;
    do
    {
    	printf("osh>");
		fflush(stdout);
		fgets(inputBuffer, 80, stdin);
    }while(inputBuffer[0]=='\n');
 	if (strncmp(inputBuffer, "exit", 4) == 0)
 	{
		return 0;
 	}
	if (inputBuffer[0] == '!') 
	{
		if (CountHistory == 0) 
		{
			printf("No commands in history\n");
			return 1;
		}
		else if (inputBuffer[1] == '!') 
		{
			strcpy(inputBuffer,history[(CountHistory - 1) % MAX_COMMANDS]);		
		}
		else if (isdigit(inputBuffer[1])) 
		{ 
			command_number = atoi(&inputBuffer[1]);
			strcpy(inputBuffer,history[command_number]);
		}
	}
    if (strncmp(inputBuffer,"history",7) == 0) 
	{
		historyfeature(inputBuffer);
		SaveHistory(inputBuffer);
		return 2;
	}
	if(inputBuffer[0]!='\n')
	{
		SaveHistory(inputBuffer);
	}	
 	i = 0;
    char* token;
    token = strtok(inputBuffer, " \t\r\a\n");
    while (token != NULL && i <= MAX_LINE) 
    {
        args2[i] = token;              /* build command array */
        if((args2[i][0]=='>' && args2[i][1]==0)||(args2[i][0]=='1' && args2[i][1]=='>' && args2[i][2]==0)||(args2[i][0]=='>' && args2[i][1]=='|' && args2[i][2]==0))
        {
        	check=1;
        	count_flag=i;
        }
        else if((args2[i][0]=='>' && args2[i][1]=='>')||(args2[i][0]=='1' && args2[i][1]=='>' && args2[i][2]=='>'))
        {
        	check1=1;
        	count_flag=i;
        }
        else if((args2[i][0]=='<' && args2[i][1]==0))
        {
        	check2=1;
        	count_flag=i;
        }
        else if((args2[i][0]=='|' && args2[i][1]==0))
        {
        	check3=1;
        	count_flag=i;
        }
        i+=1;
        token = strtok(NULL, " \t\r\a\n");
        if(check!=0)
        {
        	ofile=token;
        	check=0;
        }
        if(check1!=0)
        {
        	apofile=token;
        	check1=0;
        }
        if(check2!=0)
        {
        	ifile=token;
        	check2=0;
        }
        if(check3!=0)
        {
        	ipipe=token;
        	check3=0;
        }
    }

    if((ofile==NULL)&&(apofile==NULL)&&(ifile==NULL)&&(ipipe==NULL))
    {
    	for(j=0;j<i;j++)
	    {
	    	args[j]=args2[j];
	    }
	    if (!strcmp(args[i - 1], "&"))    // Concurrency Test
	    {
	        *background=1;
	        args[i - 1] = NULL;
	    }
    	args[i] = NULL;
    	args_pipe[0]=NULL;
    }
    else if(ipipe!=NULL)
    {
    	k=0;
    	for(j=0;j<count_flag;j++)
    	{
    		args[j] = args2[j];
    	}
    	args[count_flag] = NULL;
    	for(h=count_flag+1;h<i;h++)
    	{
    		args_pipe[k]=args2[h];
    		k+=1;
    	}
    	args_pipe[k]=NULL;
    }
    else if((ofile!=NULL)||(apofile!=NULL)||(ifile!=NULL))
    {
    	for (j = 0; j < count_flag; j++)
        {
            args[j] = args2[j];
        }
        if (!strcmp(args[count_flag - 1], "&"))    // Concurrency Test
	    {
	        *background=1;
	        args[count_flag - 1] = NULL;
	    }
    	args[count_flag] = NULL;
    	args_pipe[0]=NULL;
    }
   free(args2);
   free(token);
    return 1;
}
int main(void)
{
	char inputBuffer[MAX_LINE]; //input command
	int background;             	
	char *args[MAX_LINE/2 + 1];	// command line arguments 
	char* args_pipe[MAX_LINE/2 + 1];
	pid_t child;            		
	int status;           		// result execvp 
	int shouldrun = 1;			//flag to determine when to exit program
    while (shouldrun){            		
		background = 0;
		shouldrun = setup(inputBuffer,args,&background,args_pipe);      
		if(shouldrun==2)
		{
			shouldrun=1;
			continue;
		}
		if (shouldrun==1) {
			child = fork();        
			switch (child) {
				case -1: 
					perror("Fork failed");
					break;
				case 0: //child

					if(ipipe!=NULL)
					{
						int pipefd[2];
						pipe(pipefd);
						pid_t cchild;
						cchild = fork();  
						switch(cchild){
							case -1:
								perror("Fork failed");
								break;
							case 0:
								close(1);
								dup(pipefd[1]);
								close(pipefd[0]);
								close(pipefd[1]);
								execvp(args[0],args);
								break;
							default:
								close(0);
								dup(pipefd[0]);
								close(pipefd[0]);
								close(pipefd[1]);
								execvp(args_pipe[0],args_pipe);
								break;
						}
						exit(0);
					}

					if(ofile!=NULL)
					{
						int fd2;
						if ((fd2 = open(ofile, O_WRONLY | O_CREAT | O_TRUNC,0777)) < 0) 
						{
                    		perror("Cannot open output file.");
                    		exit(0);
                		}
                		dup2(fd2, STDOUT_FILENO);
                		close(fd2);
					}

					if(apofile!=NULL)
					{
						int fd2;
						if ((fd2 = open(apofile, O_RDWR | O_APPEND | O_CREAT,0777)) < 0) 
						{
                    		perror("Cannot open output file.");
                    		exit(0);
                		}
                		dup2(fd2, STDOUT_FILENO);
                		close(fd2);
					}
					if (ifile != NULL) 
					{
                		int fd2 = open(ifile, O_RDONLY,0777);
		                if (dup2(fd2, STDIN_FILENO) == -1) 
		                {
		                    fprintf(stderr, "dup2 failed");
		                }
                		close(fd2);
            		}
					status = execvp(args[0],args);
					signal(SIGINT, SIG_DFL);
					if (status != 0)
					{
						perror("error in execvp");
						exit(-2); 					
                    }
					break;		
				default :  //parent
					if (background == 0) // handle parent,wait child 
						while (child != wait(NULL)) 
							;
			}
		}
		
    }
	return 0;
}