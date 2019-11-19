#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *func_thread(void *); //스레드가 수행할 함수
//void * : Java의 Object와 비슷한 개념. 모든 타입으로 형변환 가능

int main(){
	int status;
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, func_thread, NULL);

	printf("thread %x is created\n", thread_id);
	pthread_join(thread_id, (void **)&status);
	printf("main thread END\n");
	return 0;
}

void *func_thread(void *argv){
	int i=0;
	while(i<10){
		i++;
		printf("thread %dth executing....\n", i);
	}
}
