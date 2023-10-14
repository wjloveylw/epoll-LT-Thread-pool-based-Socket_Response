#pragma once
#ifndef _THREADPOOL2_H
#define _THREADPOOL2_H
#include <queue>
#include <iostream>
#include <pthread.h>
#include "TaskQueue.h"
//#include "TaskQueue.cpp"

template<typename T>
class ThreadPool
{
public:
	// �����̳߳ز���ʼ��
	ThreadPool(int min, int max);

	// ���̳߳���������
	void threadPoolAdd(Task<T> task);

	// ��ȡ�̳߳��й������߳���
	int ThreadPoolBusyNum();

	// ��ȡ�̳߳��л��ŵ��̵߳ĸ���
	int ThreadPoolLiveNum();

	// �����̳߳�
	~ThreadPool();
private:
	//static void* Worker(void* arg)
	//{
	//	ThreadPool* obj = static_cast<ThreadPool*>(arg);
	//	obj->worker(arg);  // ��֪��Ҫ��Ҫȡ��ַ
	//	return nullptr;
	//}
	//static void* Manager(void* arg)
	//{
	//	ThreadPool* obj = static_cast<ThreadPool*>(arg);
	//	obj->manager(arg);  // ��֪��Ҫ��Ҫȡ��ַ
	//	return nullptr;
	//}
	//void* worker(void* arg);
	//void* manager(void* arg);
	void threadExit();

	static void* worker(void* arg);  // ��̬����ֻ�ܷ������ڵľ�̬����
	static void* manager(void* arg);
	// �������
	Task2<T>* taskq;

	pthread_t managerID; // �������߳�ID
	pthread_t* threadIDs; //�������߳�ID
	int minNum;  // ��С�߳�����
	int maxNum;  // ����߳�����
	int busyNum; // æ���߳���
	int liveNum; // �����߳���
	int exitNum; // Ҫ���ٵ��߳���
	pthread_mutex_t mutexPool; // ���������̳߳�
	pthread_mutex_t mutexBusy; // ��busyNum����
	pthread_cond_t notEmpty; //��������ǲ��ǿ���

	bool shutdown; //�ǲ���Ҫ�����̳߳أ�����1��������0
	static const int NUMBER = 2;

};


///////////////////
void* worker(void* arg);

void* manager(void* arg);

using namespace std;
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include<stdio.h>

template<typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
	// 实锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
	do
	{
		taskq = new Task2<T>;
		if (taskq == nullptr)
		{
			cout << "malloc taskq fail..." << endl;
			break;
		}
		this->threadIDs = new pthread_t[max];  // 之前锟斤拷锟斤拷锟斤拷锟斤拷为锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷诖娌伙拷锟斤拷锟� 之前没锟斤拷锟斤拷[max]
		if (this->threadIDs == nullptr)
		{
			cout << "malloc threadIDs fail..." << endl;
			break;
		}
		memset(this->threadIDs, 0, sizeof(pthread_t) * max);
		this->minNum = min;
		this->maxNum = max;
		this->busyNum = 0;
		this->liveNum = min;
		this->exitNum = 0;

		if (pthread_mutex_init(&this->mutexPool, NULL) != 0 ||
			pthread_mutex_init(&this->mutexBusy, NULL) != 0 ||
			pthread_cond_init(&this->notEmpty, NULL) != 0)
		{
			cout << "mutex or cond init failed!" << endl;
			break;
		}

		this->shutdown = false;

		// 锟斤拷锟斤拷锟竭筹拷
		//pthread_create(&this->managerID, NULL, &ThreadPool::Manager, this);
		pthread_create(&this->managerID, NULL, manager, this);
		for (int i = 0; i < min; ++i)
		{
			//pthread_create(&this->threadIDs[i], NULL, &ThreadPool::Worker, this); //NULL锟斤拷锟斤拷要锟侥筹拷this
			pthread_create(&this->threadIDs[i], NULL, &worker, this); //NULL锟斤拷锟斤拷要锟侥筹拷this
		}

		this->shutdown = false;
		return;
	} while (0);
	
	// 锟酵凤拷锟斤拷源
	if (this->threadIDs)
	{
		delete[] this->threadIDs;
	}
	if (taskq) delete taskq;
}

//void ThreadPool::threadPoolAdd(callback func, void* arg)
template<typename T>
void ThreadPool<T>::threadPoolAdd(Task<T> task)
{
	if (this->shutdown)
	{
		return;
	}
	// 锟斤拷锟斤拷锟斤拷锟斤拷
	//this->taskq->addTask(func, arg);
	this->taskq->addTask(task);

	pthread_cond_signal(&this->notEmpty);
}

template<typename T>
int ThreadPool<T>::ThreadPoolBusyNum()
{
	pthread_mutex_lock(&this->mutexBusy);
	int busynum = this->busyNum;
	pthread_mutex_unlock(&this->mutexBusy);
	return busynum;
}

template<typename T>
int ThreadPool<T>::ThreadPoolLiveNum()
{
	pthread_mutex_lock(&this->mutexPool);
	int livenum = this->liveNum;
	pthread_mutex_unlock(&this->mutexPool);
	return livenum;
}

template<typename T>
void* ThreadPool<T>::worker(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	while (1)
	{
		pthread_mutex_lock(&pool->mutexPool);
		//锟斤拷前锟斤拷锟斤拷锟斤拷锟斤拷欠锟轿拷锟�
		while (pool->taskq->taskNumber() == 0 && !pool->shutdown)
		{
			//锟斤拷锟斤拷锟斤拷锟斤拷锟竭筹拷
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);  // 锟斤拷锟斤拷 &pool->notEmpty , 锟解开 &pool->mutexPool

			// 锟叫讹拷锟角凤拷要锟斤拷锟斤拷锟竭筹拷
			if (pool->exitNum > 0)
			{
				pool->exitNum--;
				if (pool->liveNum > pool->minNum)
				{
					pool->liveNum--;
					pthread_mutex_unlock(&pool->mutexPool);
					//printf("#######################\n");
					pool->threadExit();
				}
			}
		}

		//锟叫讹拷锟竭程筹拷锟角否被关憋拷锟斤拷
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			//printf("---------------------\n");
			pool->threadExit();
		}

		//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟饺★拷锟揭伙拷锟斤拷锟斤拷锟�
		Task<T> task = pool->taskq->takeTask();
		//锟斤拷锟斤拷
		pthread_mutex_unlock(&pool->mutexPool);

		printf("thread %ld start working...\n", pthread_self());
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexBusy);

		task.function(task.arg); 
		delete task.arg;
		task.arg = NULL;
		
		cout << "thread " << to_string(pthread_self()) << " end working..." << endl;
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexBusy);
	}
	return NULL;
}

template<typename T>
void* ThreadPool<T>::manager(void* arg)
{
	//ThreadPool* pool = (ThreadPool*)arg;
	ThreadPool* pool = static_cast<ThreadPool*>(arg); // 锟斤拷锟斤拷一锟斤拷应锟斤拷一锟斤拷
	while (!pool->shutdown)
	{
		//每锟斤拷3s锟斤拷锟揭伙拷锟�
		sleep(3);

		// 取锟斤拷锟竭程筹拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷偷锟角帮拷叱痰锟斤拷锟斤拷锟�
		pthread_mutex_lock(&pool->mutexPool);
		int queueSize = pool->taskq->taskNumber();
		int liveNum = pool->liveNum;
		pthread_mutex_unlock(&pool->mutexPool);

		// 取锟斤拷忙锟斤拷锟竭程碉拷锟斤拷锟斤拷
		pthread_mutex_lock(&pool->mutexBusy);
		int busyNum = pool->busyNum;
		pthread_mutex_unlock(&pool->mutexBusy);

		// 锟斤拷锟斤拷锟竭筹拷
		// 锟斤拷锟斤拷母锟斤拷锟�>锟斤拷锟斤拷锟竭程革拷锟斤拷 && 锟斤拷锟斤拷锟竭筹拷锟斤拷<锟斤拷锟斤拷叱锟斤拷锟�
		if (queueSize > liveNum-busyNum && liveNum < pool->maxNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			int counter = 0;
			for (int i = 0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; i++)
			{
				if (pool->threadIDs[i] == 0)
				{
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					counter++;
					pool->liveNum++;
				}
			}
			pthread_mutex_unlock(&pool->mutexPool);
		}
		// 锟斤拷锟斤拷锟竭筹拷
		// 锟斤拷锟斤拷母锟斤拷锟�>锟斤拷锟斤拷锟竭程革拷锟斤拷 && 锟斤拷锟斤拷锟竭筹拷锟斤拷<锟斤拷锟斤拷叱锟斤拷锟�
		if (busyNum * 2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			pool->exitNum = NUMBER;
			pthread_mutex_unlock(&pool->mutexPool);
			// 锟矫癸拷锟斤拷锟斤拷锟竭筹拷锟斤拷杀
			for (int i = 0; i < NUMBER; i++)
			{
				pthread_cond_signal(&pool->notEmpty);
			}

		}
	}
	return nullptr;

}

template<typename T>
void ThreadPool<T>::threadExit()
{
	pthread_t tid = pthread_self();
	for (int i = 0; i < this->maxNum; ++i)
	{
		if (this->threadIDs[i] == tid)
		{
			this->threadIDs[i] = 0;
			cout << "threadExit() called, " << to_string(tid) << " exiting..." <<endl;
			break;
		}
	}
	pthread_exit(NULL);
}

template<typename T>
ThreadPool<T>::~ThreadPool()
{
	//printf("*******************\n");
	//锟截憋拷锟竭程筹拷
	this->shutdown = true;
	//锟斤拷锟斤拷锟斤拷锟秸癸拷锟斤拷锟斤拷锟竭筹拷
	pthread_join(this->managerID, NULL);
	// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟竭筹拷
	for (int i = 0; i < this->liveNum; ++i)
	{
		pthread_cond_signal(&this->notEmpty);
	}
	// 锟酵放讹拷锟节达拷
	if(taskq)
	{
		this->taskq->freeQ();
		delete taskq;  // 锟斤拷锟斤拷胤锟斤拷锟斤拷芑岜拷锟斤拷锟揭诧拷锟斤拷芗锟斤拷瞬锟矫伙拷锟�
	}
	if (this->threadIDs)
	{
		delete []this->threadIDs;
	}
	pthread_mutex_destroy(&this->mutexPool);
	pthread_mutex_destroy(&this->mutexBusy);
	pthread_cond_destroy(&this->notEmpty);
}

#endif
