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
int    list_c[MAX_CLIENT];
int		count = 0;
int tcount;
char    escape[ ] = "exit";
char    greeting[ ] = "Welcome to chatting room\n";
char    CODE200[ ] = "Sorry No More Connection\n";
char name[10][100];
char nickname[100];
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
        list_c[i] = INVALID_SOCK;
    while(1) {
        len = sizeof(c_addr);
        c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
        res = pushClient(c_socket);
        if(res < 0) { //MAX_CLIENT만큼 이미 클라이언트가 접속해 있다면,
            write(c_socket, CODE200, strlen(CODE200));
            close(c_socket);
        } else {
            write(c_socket, greeting, strlen(greeting));
				read(c_socket, nickname, 20 );
				strcpy(name[tcount],nickname);
            //pthread_create with do_chat function.
				pthread_create(&thread,NULL,do_chat,(void *)&c_socket);
        }
    }
}
void *do_chat(void *arg)
{
    int c_socket = *((int *)arg);
    char chatData[CHATDATA];
    int i, n;
    while(1) {
        memset(chatData, 0, sizeof(chatData));
        if((n = read(c_socket, chatData, sizeof(chatData))) > 0) {
			if(!strncmp(chatData,"/w",sizeof("/w"))){
				char *token;
				char tname[100];
				int i;
				token=strtok(chatData," ");
				token=strtok(NULL," ");
				for(i=0;i<count;i++){
					if(!strcmp(token,name[i]))
						write(list_c[i],chatData,sizeof(chatData));
				}
			}else{
            //write chatData to all clients
				for(i=0;i<count;i++){
					write(list_c[i],chatData,sizeof(chatData));
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
		list_c[count]=c_socket;
		count=tcount;
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
		if(list_c[i]==c_socket){
			list_c[i]=0;
			for(j=i;j<count;j++){
				if(j+1==count)
					break;
				if(j!=count){
					list_c[j]=list_c[j+1];
				}
			}
		}
	}
	pthread_mutex_lock(&mutex);
	count--;
	pthread_mutex_unlock(&mutex);
}
