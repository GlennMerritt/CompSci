#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/wait.h>
int pid=0;
char * cwd;
int runCommand(char * command){
	if(strlen(command)<1)return -1;
	//printf(">%s\n",command);
	int tl = 1;
	int i=0;
	int o=0;
	int of=0;
	int redirect=0;
	int cf=0;
	char * filepath;
	char * hiddenPath;
	while(i<strlen(command)){
		if(i+1<strlen(command)&&command[i]=='>'){
			if(i+2<strlen(command)&&command[i+1]=='>'){
				redirect=2;
				filepath = &command[i+2];
				//int q=0;
				while(filepath[0]==' '||filepath[0]=='>'){
				filepath++;
				}
				command[i]='\0';
				break;
			}
			redirect=1;
			filepath = &command[i+1];
			while(filepath[0]==' '||filepath[0]=='>'){
				filepath++;
			}
			command[i]='\0';
			break;
		}
		if(command[i]=='|')tl++;
		i++;
	}
	if(redirect==2){
		o = open(filepath,O_WRONLY|O_CREAT|O_EXCL,S_IRUSR | S_IWUSR);
		if(o>=0){
			close(o);
			remove(filepath);
			hiddenPath = (char *)malloc(strlen(filepath)+3);
			int v=0;
			int u=0;
			for(v=0;v<strlen(filepath);v++){
				if(filepath[v]=='/'||filepath[v]=='\\')
					u=v;
			}
			strcat(hiddenPath,filepath);
			hiddenPath[u+1]='.';
			strcpy(&hiddenPath[u+2],&filepath[u+1]);
			remove(hiddenPath);
			o = open(hiddenPath,O_RDWR|O_CREAT|O_APPEND,0600);
			cf=1;
		}
		else if(o<0&&errno==EEXIST){
			o = open(filepath,O_RDWR|O_CREAT|O_APPEND,0600);
		}
		if(o!=-1){
			of = dup(fileno(stdout));
			dup2(o,fileno(stdout));
				
		}else redirect=0;
	}
	if(redirect==1){
		o = open(filepath,O_WRONLY|O_CREAT|O_EXCL,S_IRUSR | S_IWUSR);
		if(o>=0){
			close(o);
			remove(filepath);
			hiddenPath = (char *)malloc(strlen(filepath)+3);
			int v=0;
			int u=0;
			for(v=0;v<strlen(filepath);v++){
				if(filepath[v]=='/'||filepath[v]=='\\')
					u=v;
			}
			strcat(hiddenPath,filepath);
			hiddenPath[u+1]='.';
			strcpy(&hiddenPath[u+2],&filepath[u+1]);
			remove(hiddenPath);
			o = open(hiddenPath,O_RDWR|O_CREAT|O_APPEND,0600);
			cf=1;
		}
		else if(o<0&&errno==EEXIST){
			remove(filepath);
			o = open(filepath,O_RDWR|O_CREAT|O_APPEND,0600);
		}
		if(o!=-1){
			
			of = dup(fileno(stdout));
			dup2(o,fileno(stdout));
				
		}else redirect=0;
	}
	char *tokens[tl];
	char * token = strtok(command,"|");
	i=0;
	while(token!=NULL){
		tokens[i]=token;
		token = strtok(NULL,"|");
		i++;
	}
	char * output=NULL;
	int inpipe[2];
	for(int i=0;i<tl;i++){
		char *args[128];
		if(strlen((char *)&tokens[i])<1){
			break;
		}
		char * arg = strtok(tokens[i]," ");
		int n=0;
		while(arg!=NULL){
			args[n]=arg;
			arg = strtok(NULL," ");
			n++;
		}
		args[n]=NULL;
		//printf("%s\n",args[0]);
		int io[2];
		
		
		
		if(output!=NULL){
			//inpipe[0]=0;
			//inpipe[1]=0;
			//args[n]=output;
			if(strlen(output)>1&&output[strlen(output)-2]=='\n'){
				output[strlen(output)-2]=='\0';
			}
			//args[n+1]=NULL;
		}if(!strcmp(args[0],"cd")){
			if(output!=NULL)chdir(output);
			else chdir(args[1]);
			continue;
		}
		pipe(io);
		pipe(inpipe);
		pid = fork();
		
		
		
		if(pid==0){
			if(output!=NULL){
				
				while((dup2(inpipe[0],STDIN_FILENO)==-1)&&(errno == EINTR)){}
				
			}close(inpipe[1]);
			close(inpipe[0]);
			if(i+1!=tl){
				while((dup2(io[1],STDOUT_FILENO)==-1)&&(errno == EINTR)){}
				close(io[0]);
				close(io[1]);
			}
				//printf("pid%d\n",pid);
				//printf("arg0%s\n",args[0]);
			execvp(args[0],args);
			exit(-1);
		}if(output!=NULL){
				//printf("output%s\n",output);
				write(inpipe[1],output,strlen(output));
				write(inpipe[1],"\n",strlen("\n"));
				}
				close(inpipe[1]);
				close(inpipe[0]);
				close(io[1]);
		if(i+1!=tl){
				
				
				char buffer[1024];
				int size=0;
				output = (char *)malloc(size*sizeof(char));
				while(1){
					int status = read(io[0],buffer,sizeof(buffer));
					if(status<=0)break;
					else{
						char * tmp = (char *)malloc((size+status)*sizeof(char));
						memcpy(tmp,output,size);
						free(output);
						output=tmp;
						memcpy(output+size,buffer,status);
						output[status-1]='\0';
						size+=status;
						
						//memcpy(output,buffer,status);
					}
				}
				bzero(buffer,1024);
		}
				close(io[0]);
		wait(NULL);
		pid=0;
	}
	
	if(redirect>0){
		fflush(stdout);
		close(o);
		dup2(of,fileno(stdout));
		close(of);
		if(cf==1){
			
			rename(hiddenPath,filepath);
			free(hiddenPath);
		}
	}
	
	if(output!=NULL)free(output);
	return 0;
}
void handler(int s){
if(pid>0)
	kill(pid,SIGINT);
	
}
int main(int argc, char ** argv){
	signal(SIGINT, handler);
	cwd = (char *)malloc(PATH_MAX);
	char * input = NULL;
	size_t chars = 0;
	while(1){
		bzero(cwd, PATH_MAX);
		getcwd(cwd, PATH_MAX);
		printf("%s $ ",cwd);
		getline(&input,&chars,stdin);
		//printf("\n");
		if(strstr(input,";;")||strstr(input,"||")||strstr(input,">>>")){
			printf("Syntax error\n");
			continue;
		}
		if(!strcmp(input,"exit\n"))return 0;
		if(input[0]==';'||input[0]=='|'||input[0]=='>'){
			printf("Syntax error\n");
			continue;
		
		}
		int tl = 1;
		int i=0;
		while(i<strlen(input)){
			if(input[i]==';')tl++;i++;
		}
		char *tokens[tl];
		//for(i=0;i<tl;i++){
		//	tokens[i]=(char *)malloc(strlen(input)*sizeof(char));
		//}
		char * token = strtok(input,";");
		i=0;
		while(token!=NULL){
			tokens[i]=token;
			token = strtok(NULL,";");
			i++;
		}
		
		for(i=0;i<tl;i++){
			if(i==tl-1)tokens[i][strlen(tokens[i])-1]='\0';
			
			runCommand(tokens[i]);
		}
		//free(input);
	}
	free(cwd);
	return 0;
}
