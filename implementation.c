#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>




const char*begin="%%BEGIN\n";
const char*end="%%END\n";
const char*interstart="#INTERSTART\n";
const char*interstop="#INTERSTOP\n";

int begin_end_flag;
int inter_start_flag;
int line_type,line_status;

int pipe_indexes[20];
int pipe_flag;

int pipe_count;
int my_pipe[2];



char* params[20];
char* temp[20];
int temp_count=0;
char* split_params[20];
char *token;


int set_line_type(char* );
int set_line_status(int);
void process_file(char* filename);
void tokenize(char*);
void do_exec_once();
void get_pipe_indexes();
void do_pipe_exec();
void do_split_params();



void process_file(char* filename)
{
	
	FILE* fd = fopen(filename,"r");
	char buff[100],*m;
    

    begin_end_flag=0;
    inter_start_flag=0;

    while(1)
    {
    	m=fgets( buff, 100, fd);
    	
    	if(m == NULL)
        {
        	if(begin_end_flag)
        	{
        		printf("\nBatch file finished abruptly\n");
        	}
        	break;
        }
        

        line_type=set_line_type(buff);

        line_status=set_line_status(line_type);

        

        if(line_status==-2)
        {
        	tokenize(buff);
        }

    }
    
  
   	fclose(fd);
	   
   
}




int set_line_type(char* line)
{	
	// RETURN VALUE

	// 1 %BEGIN
	// 2 %END

	// 3 #INTERSTART
	// 4 #INTERSTOP

	// 0 anything else

	// 5 comment

	

int comment_start=-1,comment_end=-1;


	if(line[0]=='\n')
	{
		return 5;
	}



	if(line[0]=='%')
	{
		if(line[1]=='B')
		{
			for(int i=2;i<7;i++)
			{
				if(!(line[i]==begin[i+1]))
				{
					return 0;
				}
				else
				{
					if(i==6)
						return 1;
				}
			}
	
		}
		else if(line[1]=='E')
		{
			for(int i=2;i<5;i++)
			{
				if(!(line[i]==end[i+1]))
				{
					return 0;
				}
				else
				{
					if(i==4)
						return 2;
				}
			}
		}
		else
		{
			return 0;
		}
	}
	else if(line[0]=='#')
	{
		for(int i=1;i<8;i++)
			{
				if(!(line[i]==interstart[i]))
				{
					return 5;
				}
				
			}

		if(line[8]=='A'&&line[9]=='R'&&line[10]=='T'&&line[11]=='\n')
		{
			return 3;
		}
		else if(line[8]=='O'&&line[9]=='P'&&line[10]=='\n')
		{
			return 4;
		}
		else
		{
			return 5;
		}

	}
	 
	return 0;
}

int set_line_status(int val)
{
	// RETURN VALUE
	// 1 start to reading commands
	// 0 start ignoring commands
	// 3 start log
	// 4 stop log
	// -1 anything else when no b/w begin and end
	//    OR if Interstart or begin after a previous Interstart or begin hasn't met end or Interstop
	// -2 to be executed
	


	switch(val)
	{
		case 0:
		{	
			if(begin_end_flag==1)
				return -2;
			else
				return -1;
			break;
		}

		case 1:
		{
			if(val==1&&begin_end_flag==0)
			{
				begin_end_flag=1;
				return 1; 
			}
			else
			{
				return -1;
			}
			break;
		}

		case 2:
		{
			if(val==2&&begin_end_flag==1)
			{
				begin_end_flag=0;
				return 0;
			}
			else
			{
				return -1;
			}
			break;
		}

		case 3:
		{	
			if(val==3&&begin_end_flag==1)
			{
				if(inter_start_flag==0)
				{
					inter_start_flag=1;
					return 3; 
				}
				else
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}

			break;
		}

		case 4:
		{
			if(val==4&&begin_end_flag==1)
			{
				if(inter_start_flag==1)
				{
					inter_start_flag=0;
					return 4; 
				}
				else
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}
			break;
		}

		case 5:
		{
			return -1;
			break;
		}

		default:
		{
			return -2;
		}
	}

}





void tokenize(char *command)
{	
	char str[100];
	
   
  	int index=0;

  	strncpy(str,command,100);
  	token= strtok(str, " \n");

   while( token != NULL ) 
   {
      params[index]=token;
      index++;
      token = strtok(NULL, " \n");
   }

   params[index]=NULL;


   pipe_flag=0;

   for(int i=0;i<index;i++)
   {
   		for(int j=0;params[i][j]!='\0';j++)
   		{
   			if(params[i][j]=='|')
   			{
   				pipe_flag=1;
   				break;	
   			}
   		}

   		if(pipe_flag==1)
   			break;
   	}

   	

   if(pipe_flag==0)
   {	
   		
   		//do_exec_once();
   }
   else
   {	
   		get_pipe_indexes();
   		do_split_params();
   		do_exec_once();
   		do_pipe_exec();
   		
   }

}



void do_exec_once()
{


	pid_t child_id;
    child_id = fork();
    
    if(child_id == -1)
    {
        fprintf(stderr, "Fork error\n");
    }
    
    if(child_id == 0) 
    {

    	if(pipe_flag==1)
    	{
    		


    		close(1); 
        	dup(my_pipe[1]); 
        	close(my_pipe[0]);
        	close(my_pipe[1]);

        	execvp(temp[0], temp);
        	perror("Exec failed");
        	printf("\n");
        	exit(1);


        }

        
		execvp(params[0], params);      	 
        perror("Exec failed");
        printf("\n");
        exit(1);
    }
    

}



void get_pipe_indexes()
{
	int k=0;
	pipe_count=0;
	for(int i=0;params[i]!=NULL;i++)
   {
   		for(int j=0;params[i][j]!='\0';j++)
   		{
   			if(params[i][j]=='|')
   			{
   				pipe_indexes[k]=i;
   				k++;
   				pipe_count++;
   			}
   		}
   		
   	}
   	pipe_indexes[k]=-1;


}





void do_pipe_exec()
{

	

	for(int i=0;i<pipe_count;i++)
	{
		pid_t child_id;
    	child_id = fork();
    

   		if(child_id == -1)
    	{
        	fprintf(stderr, "Fork error\n");
    	}
    
    	if(child_id == 0) 
    	{

    		

    		if(pipe_indexes[i+1]!=-1)
    		{	

        		


        	}
        	else
        	{
     			
        		close(0); 
       			dup(my_pipe[0]); 
        		close(my_pipe[0]);
        		close(my_pipe[1]);


        	}



        	execvp(temp[pipe_indexes[i]+1], temp+pipe_indexes[i]+1);
        	perror("Exec failed");
        	printf("\n");
        	exit(1);
    	}


	}
	close(my_pipe[0]);
    close(my_pipe[1]);
    close(1);
    close(0);

 for(int i = 0; i < pipe_count+1; i++)
        wait(NULL);

}






void do_split_params()
{
	
	

	int flag=0;
	temp_count=0;
	for(int i=0;params[i]!=NULL;i++)
   {
   		
   		if(params[i][0]=='|')
   		{
   			temp[temp_count]=NULL;
   			temp_count++;
   				
   		}
   		else
   		{
   			temp[temp_count]=params[i];
   			temp_count++;
   		}
   		
   		
   	}
 
}
