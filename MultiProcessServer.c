#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 10000

char buffer[100] = "Hi, I'm server";
//sizeof(buffer) => 100 (배열의 크기)
//strlen(buffer) => 15 (buffer에 저장된 문자열의 길)
char rcvBuffer[100];
int numClient = 0;

void do_service(int c_socket);
void sig_handler(int signo);

int main(){
	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	int len;
	int n;

	signal(SIGCHLD, sig_handler); //첫번째 인자 : 시그널번호, 두번째 인자:첫번째 인자의 시그널이 발생했을 때 실행되는 함수

	// 1. 서버 소켓 생성
	//서버 소켓 = 클라이언트의 접속 요청을 처리(허용)해 주기 위한 소켓
	s_socket = socket(PF_INET, SOCK_STREAM, 0); //TCP/IP 통신을 위한 서버 소켓 생성
	
	//2. 서버 소켓 주소 설정
	memset(&s_addr, 0, sizeof(s_addr)); //s_addr의 값을 모두 0으로  초기화
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP 주소 설정
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);

	//3. 서버 소켓바인딩
	if(bind(s_socket,(struct sockaddr *) &s_addr, sizeof(s_addr)) == -1){ 
		//바인딩 작업 실패 시, Cannot Bind 메시지 출력 후 프로그램 종료
		printf("Cannot Bind\n");
		return -1;
	}
	
	//4.listen() 함수 실행
	if(listen(s_socket, 5) == -1){
		printf("listen Fail\n");
		return -1;
	}

	//5. 클라이언트 요청 처리
	// 요청을 허용한 후, Hello World 메세지를 전송함
	while(1){ //무한 루프
		len = sizeof(c_addr);
		printf("클라이언트 접속을 기다리는 중....\n");
		c_socket = accept(s_socket, (struct sockaddr *)&c_addr, &len); 
		//클라이언트의 요청이 오면 허용(accept)해 주고, 해당 클라이언트와 통신할 수 있도록 클라이언트 소켓(c_socket)을 반환함.
		printf("/client is connected\n");
		numClient++;
		printf("현재 접속 중인 클라이언트 수:%d\n",numClient);
		int pid = fork();
		if(pid >0){//부모 프로세스
			continue;
		}else if(pid==0){ //자식 프로세스
			do_service(c_socket);
			numClient--;
			exit(0);
		}else{//fork() 함수 실패
			printf("fprk() failed\n");
			exit(0);
		}
	}
	close(s_socket);
	return 0;	
}

void do_service(int c_socket){
	int n;
	while(1){
		n = read(c_socket, rcvBuffer, sizeof(rcvBuffer));
		printf("rcvBuffer: %s\n",rcvBuffer);
		//rcvBuffer[n-1] = '\0'; 개행 문자 삭제
		if(strncasecmp(rcvBuffer,"quit",4)==0 || strncasecmp(rcvBuffer,"kill server",11)==0)
			break;
		else if(!strncasecmp(rcvBuffer, "안녕하세요",strlen("안녕하세요")))
			strcpy(buffer, "안녕하세요. 만나서 반가워요.");
		else if(!strncasecmp(rcvBuffer, "이름이 뭐야?",strlen("이름이 뭐야?")))
			strcpy(buffer, "내 이름은 Bot이야.");
		else if(!strncasecmp(rcvBuffer, "몇 살이야?",strlen("몇 살이야?")))
			strcpy(buffer, "나는 10살이야");
		else if(!strncasecmp(rcvBuffer, "strlen ",strlen("strlen ")))
			//문자열의 길이는 XX입니다.
			sprintf(buffer, "문자열의 길이는 %d입니다.",strlen(rcvBuffer)-7);
		else if(!strncasecmp(rcvBuffer, "exec ",strlen("exec "))){
			char *token;
			char com[100]="";
			int ex;
			token = strtok(rcvBuffer, " ");
			while(token != NULL){
				strcat(com,token);
				strcat(com," ");
				token = strtok(NULL," ");
			}
			ex = system(com);
			if(ex==0)
				strcpy(buffer, "command is executed.");
			else
				strcpy(buffer, "command failed.");
		}
		else if(!strncasecmp(rcvBuffer, "readfile ",strlen("readfile "))){
			FILE *fp;
			char *token;
			char buff[255];
			token = strtok(rcvBuffer, " ");
			token = strtok(NULL, " ");
			fp = fopen(token,"r");
			if(fp){
				while(fgets(buff,255,(FILE *)fp)){
					printf("%s",buff);
					write(c_socket, buff, strlen(buff));
				}
			}
			fclose(fp);
			strcpy(buffer, "파일 읽기 완료");
		}				
		else if(!strncasecmp(rcvBuffer, "strcmp ",strlen("strcmp "))){
			char *token;
			char *str[3];
			int idx = 0;
			
			token = strtok(rcvBuffer, " ");
			while(token != NULL){
				str[idx] = token;
				printf("str[%d] = %s\n", idx, str[idx]);
				idx++;
				token = strtok(NULL, " ");
			}
			if(idx < 3)
				strcpy(buffer, "문자열 비교를 위해서는 두 문자열이 필요합니다.");
			else if(!strcmp(str[1], str[2])) //같은 문자열이면 
				sprintf(buffer, "%s와 %s는 같은 문자열입니다.",str[1], str[2]);
			else
				sprintf(buffer, "%s와 %s는 다른 문자열입니다.",str[1], str[2]);
		}
		else
			strcpy(buffer, "무슨 말인지 모르겠습니다.");
		
		write(c_socket, buffer, strlen(buffer)); //클라이언트에게 buffer의 내용을 전송함
		}
}
void sig_handler(int signo){
	int pid;
	int status;
	pid = wait(&status);
	printf("pid[%d] is terminated.status = %d\n",pid,status);
	numClient--;
	printf("현재 접속 중인 클라이언트 수 : %d",numClient);
}
