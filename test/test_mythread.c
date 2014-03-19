#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <mythread.h>
#include <measure.h>

//rien - lock unlock - cond signal et recoit 

pthread_mutex_t mutex;
mythread_mutex_t my_mutex;

pthread_mutex_t mutex2;
mythread_mutex_t my_mutex2;
pthread_cond_t cond;
mythread_cond_t my_cond;

void* thread_func_rien(void *arg)
{
  return NULL;
}

void* pthread_func_lock(void *arg)
{
  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void* mythread_func_lock(void *arg)
{
  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void* pthread_func_cond_wait(void *arg)
{
  pthread_cond_wait(&cond,&mutex2);
  return NULL;
}

void* mythread_func_cond_wait(void *arg)
{
  mythread_cond_wait(&my_cond,&my_mutex2);
  return NULL;
}

void* pthread_func_cond_signal(void *arg)
{
  pthread_cond_signal(&cond);
  return NULL;
}

void* mythread_func_cond_signal(void *arg)
{
  mythread_cond_signal(&my_cond);
  return NULL;
}




int mythread_main(void *args)
{
  struct timeval timer;
  pthread_t thread1,thread2,thread3,thread4;
  mythread_t my_thread1,my_thread2,my_thread3,my_thread4;
  pthread_mutex_init(&mutex,NULL);
  mythread_mutex_init(&my_mutex,NULL);  
  pthread_mutex_init(&mutex2,NULL);
  mythread_mutex_init(&my_mutex2,NULL);  
  pthread_cond_init(&cond,NULL);
  mythread_cond_init(&my_cond,NULL);  

  start_timer(&timer);
  pthread_create(&thread1, NULL, &thread_func_rien, NULL);
  pthread_yield();
  printf("pthread rien=%d usec\n",(int)stop_timer(&timer));

  start_timer(&timer);
  mythread_create(&my_thread1, NULL, &thread_func_rien, NULL);
  mythread_yield();
  printf("mythread rien=%d usec\n",(int)stop_timer(&timer));

  start_timer(&timer);
  pthread_create(&thread2, NULL, &pthread_func_lock, NULL);
  pthread_yield();
  printf("pthread lock=%d usec\n",(int)stop_timer(&timer));

  start_timer(&timer);
  mythread_create(&my_thread2, NULL, &mythread_func_lock, NULL);
  mythread_yield();
  printf("mythread lock=%d usec\n",(int)stop_timer(&timer));

  start_timer(&timer);
  pthread_create(&thread3, NULL, &pthread_func_cond_wait, NULL);
  pthread_create(&thread4, NULL, &pthread_func_cond_signal, NULL);
  pthread_yield();
  printf("pthread cond=%d usec\n",(int)stop_timer(&timer));

  start_timer(&timer);
  mythread_create(&my_thread3, NULL, &mythread_func_cond_wait, NULL);
  mythread_create(&my_thread4, NULL, &mythread_func_cond_signal, NULL);  
  mythread_yield();
  printf("mythread cond=%d usec\n",(int)stop_timer(&timer));

  return 0;
}
