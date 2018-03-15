#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
typedef struct{

	char* args[61];
}command;
void getArgs(char string[],char** args)
{
   char* tokenPtr = strtok(string, " ,\t\n");
   while(tokenPtr!=NULL)
   {
      *args++ = tokenPtr;
      tokenPtr = strtok(NULL, " ,\t\n");

   }
   *args = '\0';
   return;
}
void getSingleArgs(char string[],char** args)
{
   char* tokenPtr = strtok(string, "|");
   while(tokenPtr!=NULL)
   {
   	 *args++ = tokenPtr;
   	 tokenPtr = strtok(NULL, "|");
   }
   *args='\0';
   return;
}
int buildCommandArray(command c[],char** args)
{
   int i=0;
   while(*args!='\0')
   {
       getArgs(*args++,c[i++].args);

   }
   return i;
}
void create_single_process(int in,int out,char** args)
{
   int x=fork();
   if(x==0)
   {
        if(in!=0)
        {
        	 dup2(in,0);
        	 close(in);
        }
        if(out!=1)
        {
        	 dup2(out,1);
        	 close(out);
        }
        execvp(args[0],args);

   }
}
void execute_pipe_command(command c[],int n)
{
   int i=0;
   int in,out;
   in = 0;
   int p[2];
   for(i=0;i<n-1;i++)
   {
       pipe(p);
       create_single_process(in,p[1],c[i].args); 
       close(p[1]);      
       in = p[0];
   }
   if(in!=0)
   {
   	 dup2(in,0);
   	 close(in);
   }
   execvp(c[i].args[0],c[i].args);
}
void execute(char** args)
{
   int status;
   int x = fork();
   if(x<0)
   {
       printf("Forking process failed\n");
       exit(1);

   }else if(x==0)
   {
       if(execvp(args[0],args)<0)
       {
       	 printf("Execvp failed\n");
       	 exit(0);
        }

   }else{

         while(wait(&status)!=x);

   }



}
int main()
{
   char co[1005];
   char* args[61];
   mkdir("testFolder",S_IRWXG|S_IRWXU|S_IROTH|S_IXOTH);
   int file = open("def.txt",O_RDONLY);
   int saved = dup(0);
   dup2(file,0);
   char ch;
   scanf(" %c",&ch);
   printf("%c\n",ch);
   fflush(stdin);
   close(file);
   dup2(saved,0);
   scanf(" %c",&ch);
   printf("%c\n",ch);
   return 0;


}