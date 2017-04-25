#include<iostream>
#include<Windows.h>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include"buffer.h"
using namespace std;

#define RAND_MAX_NUM 100

#define wait(x)    WaitForSingleObject(x,INFINITE)
#define signal(x)   ReleaseSemaphore(x,1,NULL)

buff_item buffer[BUFFER_SIZE];

HANDLE mutex;
HANDLE empty;
HANDLE full;


int counter=0;
int in=0;
int out=0;


void init_semp()
{
    mutex=CreateMutex(NULL,FALSE,NULL);
    empty=CreateSemaphore(NULL,BUFFER_SIZE,BUFFER_SIZE,NULL);
    full=CreateSemaphore(NULL,0,BUFFER_SIZE,NULL);
}

int insert_item(buff_item& item)
{
	try
	{
		wait(empty);
		wait(mutex);
		while(counter==BUFFER_SIZE);
		buffer[in]=item;
		in=(in+1)%BUFFER_SIZE;
		counter++;
		ReleaseMutex(mutex);
		signal(full);
		return 0;//success
	}
	catch(exception e)
	{
		return -1;//fail
	}
    
}

int remove_item(buff_item &item)
{
	try
	{
		wait(full);
		wait(mutex);
		while(counter==0);
		item=buffer[out];
		out=(out+1)%BUFFER_SIZE;
		counter--;
		ReleaseMutex(mutex);
		signal(empty);
		return 0;//success
	}
	catch(exception e)
	{
		return -1;//fail
	}
}

DWORD WINAPI producer(LPVOID Param)
{
	int mytoken=*(int *)Param;
	srand(int(time(0))+mytoken);
    buff_item randnum;
    while(true)
    {
        Sleep(rand()%RAND_MAX_NUM);//sleep random 0-100ms
        randnum=rand();
        printf("producer %d produced %d\n",mytoken,randnum);
        if(insert_item(randnum))
        {
            printf("report error condition\n");
        }
    }
}

DWORD WINAPI consumer(LPVOID Param)
{
    buff_item randnum;
	int mytoken=*(int *)Param;
	srand(int(time(0))-mytoken);
    while(true)
    {
        Sleep(rand()%RAND_MAX_NUM);//sleep random 0-100ms
        if(remove_item(randnum))
        {
            printf("report error condition\n");
        }
        else
        {
            printf("consumer %d consumed %d\n",mytoken,randnum);
        }
    }
}


int main(int argc,char *argv[])
{
    
    /* 1. Get command ,there being 3 argvs*/
	int sleep_time=1000;
	int producerAmount=3;
	int consumerAmount=3;
	if(argc!=4)
	{
		cout<<"Params set incorrect.\nUse default params.";
	}
	else
	{
		sleep_time=atoi(argv[1]);
		producerAmount=atoi(argv[2]);
		consumerAmount=atoi(argv[3]);
	}
	printf("sleep=%dms producer_threads=%d consumer_threads=%d\n",sleep_time,producerAmount,consumerAmount);
    /* 2. Initialize  */
    init_semp();

    /* 3. Create producer thread(s)*/
	int *producer_param=new int [producerAmount];
	  for(int i=0;i<producerAmount;i++)
	{
		DWORD producer_threadid;
		HANDLE producer_threadHandle;
		producer_param[i]=i;
		producer_threadHandle=CreateThread(NULL,0,producer,&producer_param[i],0,&producer_threadid);
	}

    /* 4. Create consumer thread(s)*/
	int *consumer_param=new int [consumerAmount];
	  for(int i=0;i<consumerAmount;i++)
	{
		DWORD consumer_threadid;
		HANDLE consumer_threadHandle;
		consumer_param[i]=i;
		consumer_threadHandle=CreateThread(NULL,0,consumer,&consumer_param[i],0,&consumer_threadid);
	}

    /* 5. Sleep */
    Sleep(sleep_time);

    /* 6. Exit */
    return 0;
}

