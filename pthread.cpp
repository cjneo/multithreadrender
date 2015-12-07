#include <pthread.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h> 

#include <vector>
using namespace std;
// 线程ID 
pthread_t       ntid; 
// 互斥对象 
pthread_mutex_t mutex; 
 
int             count; 
 vector<int> vv;
volatile bool threadstop=true;
void printids(const char *s) 
{ 
    pid_t   pid; 
    pthread_t   tid; 
 
    pid = getpid(); 
    tid = pthread_self(); 
 
    printf("%s pid %u tid %u (0x%x)\n", s, (unsigned int)pid, 
        (unsigned int)tid, (unsigned int)tid); 
} 
 
// 线程函数 
void *thr_fn(void *arg) 
{ 
    printids("new thread begin\n"); 
 
    // 加锁 
    printids("new thread:"); 
 while(true){
    int i=0;
    if(threadstop){
	sleep(1);	
	}
    else{
    pthread_mutex_lock(&mutex); 
    
    for ( int i=0; i < vv.size(); ++i ) 
    { 
        printf("thr_fn runing %d\n", i);
	sleep(1);
	if(threadstop){
		pthread_mutex_unlock(&mutex);
		break;	
	} 
    }
	threadstop=true;
	}
 //pthread_mutex_unlock(&mutex); 
 }
    // 释放互斥锁 
    return ( (void*)100); 
} 
 
int main(void) 
{ 
    int err; 
 
    count = 0; 
    // 初始化互斥对象 
    pthread_mutex_init(&mutex, NULL); 
 
    // 创建线程 
    err = pthread_create(&ntid, NULL, thr_fn, NULL); 
    if ( 0 != err ) 
    { 
        printf("can't create thread:%s\n", strerror(err)); 
    } 
 
    // sleep(5); 
      
    printids("main thread:");
while(true){
threadstop=true;
pthread_mutex_lock(&mutex);

vv.clear();

    int i=0; 
    for ( ; i < 700; ++i ) 
    { 
        vv.push_back(i); 
    } 
   threadstop=false;
   pthread_mutex_unlock(&mutex);
//pthread_mutex_unlock(&mutex);
 sleep(10);
} 
     
 
    int *ret; 
    pthread_join(ntid, (void * *)&ret); 
    printf("thr_fn return %d\n", (int)(*ret)); 
 
    pthread_mutex_destroy(&mutex); 
 
 
    return 0; 
} 
