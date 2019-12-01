#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
void *do_chat(void *); //채팅 메세지를 보내는 함수
int pushClient(int); //새로운 클라이언트가 접속했을 때 클라이언트 정보 추가
int popClient(int); //클라이언트가 종료했을 때 클라이언트 정보 삭제
pthread_t thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_CLIENT 10
#define CHATDATA 1024
#define INVALID_SOCK -1
#define PORT 9000
struct list{
	int socket;
	char name[20];
}    list_c[MAX_CLIENT];
int		count = 0;
char    escape[ ] = "exit";
char    greeting[ ] = "Welcome to chatting room\n";
char    CODE200[ ] = "Sorry No More Connection\n";
int main(int argc, char *argv[ ])
{
    int c_socket, s_socket;
    struct sockaddr_in s_addr, c_addr;
    int    len;
    int    i, j, n;
    int    res;
    if(pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Can not create mutex\n");
        return -1;
    }
    s_socket = socket(PF_INET, SOCK_STREAM, 0);
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    if(bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1) {
        printf("Can not Bind\n");
        return -1;
    }
    if(listen(s_socket, MAX_CLIENT) == -1) {
        printf("listen Fail\n");
        return -1;
    }
    for(i = 0; i < MAX_CLIENT; i++)
        list_c[i].socket = INVALID_SOCK;
    while(1) {
        len = sizeof(c_addr);
        c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
        res = pushClient(c_socket);
        if(res < 0) { //MAX_CLIENT만큼 이미 클라이언트가 접속해 있다면,
            write(c_socket, CODE200, strlen(CODE200));
            close(c_socket);
        } else {
			int nVal = count - 1;
			if(nVal < 0)
			{	
				nVal = 0;
			}
			read(c_socket, list_c[nVal].name, sizeof(list_c[nVal].name));
			write(c_socket, greeting, strlen(greeting));
           //pthread_create with do_chat function.
			pthread_create(&thread,NULL,do_chat,(void *)&c_socket);
        }
    }
}
void *do_chat(void *arg)
{
    int c_socket = *((int *)arg);
    char chatData[CHATDATA];
    int i, n, j;
    while(1) {
        memset(chatData, 0, sizeof(chatData));
        if((n = read(c_socket, chatData, sizeof(chatData))) > 0) {
            //write chatData to all clients
			//if(!strncmp(chatData,"/w ",sizeof("/w "))){
			//	char *token;
			//	char tname[20];
			//	int i;
			//	token = strtok(chatData," ");
			//	token = strtok(NULL," ");
			//	for(i=0;i<count;i++){
			//		if(!strcmp(token,list_c[i].name)){
			//			token = strtok(NULL,"\0");
			//			sprintf(chatData,"[%s] %s ",list_c[i].name,chatData);
			//			write(list_c[i].socket,chatData,sizeof(chatData));
			//		}
			//		else{
				//		sprintf(chatData,"%d %s %s ",strcmp(token,list_c[i].name),token,list_c[i].name);
				//		write(list_c[i].socket,chatData,sizeof(chatData));
				//	}
				//}
			//} else 
			//{
			//	for(i=0;i<count;i++){
			//		write(list_c[i].socket,chatData,sizeof(chatData));
			//	}
			//}
			if(!strncmp(chatData,"/w ",strlen("/w ")))
			{
				char *tempS;
				char temp[20];
				tempS = strtok(chatData," ");
				tempS = strtok(NULL," ");
				for(i=0;i<count;i++)
				{
					//sprintf(temp,"%d %s ",count, list_c[i].name);
					//write(list_c[i].socket,temp,sizeof(list_c[i].name));
					//write(list_c[i].socket,tempname,sizeof(tempname));
					if(!strcmp(tempS,list_c[i].name))
					{
						for(j=0;j<count;j++){
							if(list_c[j].socket==c_socket)
								strcpy(temp,list_c[j].name);
						}
						tempS = strtok(NULL,"\0");
						sprintf(chatData,"[%s] %s",temp,tempS);
					  write(list_c[i].socket,chatData,sizeof(chatData));
					}
				}	
			}
			else
		  	 {
				for(i=0;i<count;i++)
				{
					write(list_c[i].socket,chatData,sizeof(chatData));
					//write(list_c[i].socket,list_c[i].name,sizeof(list_c[i].name));
				}
			}


            if(strstr(chatData, escape) != NULL) {
                popClient(c_socket);
                break;
            }
        }
    }
}
int pushClient(int c_socket) {
    //ADD c_socket to list_c array.
	//return -1, if list_c is full.
	if(count<MAX_CLIENT){
		list_c[count].socket=c_socket;
		pthread_mutex_lock(&mutex);
		count++;
		pthread_mutex_unlock(&mutex);
		return count;
	}else{
		return -1;
	}
    //return the index of list_c which c_socket is added.
}
int popClient(int c_socket)
{
	int i,j;
    close(c_socket);
    //REMOVE c_socket from list_c array.
	for(i=0;i<count;i++){
		if(list_c[i].socket==c_socket){
			list_c[i].socket=0;
			for(j=i;j<count;j++){
				if(j+1==count)
					break;
				if(j!=count){
					list_c[j].socket=list_c[j+1].socket;
				}
			}
		}
	}
	pthread_mutex_lock(&mutex);
	count--;
	pthread_mutex_unlock(&mutex);
}
