#include "threadpool.h"
#include <queue>
#include <iostream>
#include <pthread.h>
using namespace std;
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include<stdio.h>

template<typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
	// ʵ�����������?
	do
	{
		taskq = new Task2<T>;
		if (taskq == nullptr)
		{
			cout << "malloc taskq fail..." << endl;
			break;
		}
		this->threadIDs = new pthread_t[max];  // ֮ǰ��������Ϊ����������ڴ治����? ֮ǰû����[max]
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

		// �����߳�
		//pthread_create(&this->managerID, NULL, &ThreadPool::Manager, this);
		pthread_create(&this->managerID, NULL, manager, this);
		for (int i = 0; i < min; ++i)
		{
			//pthread_create(&this->threadIDs[i], NULL, &ThreadPool::Worker, this); //NULL����Ҫ�ĳ�this
			pthread_create(&this->threadIDs[i], NULL, &worker, this); //NULL����Ҫ�ĳ�this
		}

		this->shutdown = 0;
		return;
	} while (0);
	
	// �ͷ���Դ
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
	// ��������
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
		//��ǰ��������Ƿ�Ϊ��?
		while (pool->taskq->taskNumber() == 0 && !pool->shutdown)
		{
			//���������߳�
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);  // ���� &pool->notEmpty , �⿪ &pool->mutexPool

			// �ж��Ƿ�Ҫ�����߳�
			if (pool->exitNum > 0)
			{
				pool->exitNum--;
				if (pool->liveNum > pool->minNum)
				{
					pool->liveNum--;
					pthread_mutex_unlock(&pool->mutexPool);
					pool->threadExit();
				}
			}
		}

		//�ж��̳߳��Ƿ񱻹ر���
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			pool->threadExit();
		}

		//�����������ȡ��һ������?
		Task<T> task = pool->taskq->takeTask();
		//����
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
	ThreadPool* pool = static_cast<ThreadPool*>(arg); // ����һ��Ӧ��һ��
	while (!pool->shutdown)
	{
		//ÿ��3s���һ��?
		sleep(3);

		// ȡ���̳߳�������������͵�ǰ�̵߳�����?
		pthread_mutex_lock(&pool->mutexPool);
		int queueSize = pool->taskq->taskNumber();
		int liveNum = pool->liveNum;
		pthread_mutex_unlock(&pool->mutexPool);

		// ȡ��æ���̵߳�����
		pthread_mutex_lock(&pool->mutexBusy);
		int busyNum = pool->busyNum;
		pthread_mutex_unlock(&pool->mutexBusy);

		// �����߳�
		// ����ĸ���?>�����̸߳��� && �����߳���<����߳���?
		if (queueSize > liveNum && liveNum < pool->maxNum)
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
		// �����߳�
		// ����ĸ���?>�����̸߳��� && �����߳���<����߳���?
		if (busyNum * 2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			pool->exitNum = NUMBER;
			pthread_mutex_unlock(&pool->mutexPool);
			// �ù������߳���ɱ
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
	//�ر��̳߳�
	this->shutdown = true;
	//�������չ������߳�
	pthread_join(this->managerID, NULL);
	// �����������������߳�
	for (int i = 0; i < this->liveNum; ++i)
	{
		pthread_cond_signal(&this->notEmpty);
	}
	// �ͷŶ��ڴ�
	if(taskq)
	{
		this->taskq->freeQ();
		delete taskq;  // ����ط����ܻᱨ����Ҳ���ܼ��˲�û��?
	}
	if (this->threadIDs)
	{
		delete []this->threadIDs;
	}
	pthread_mutex_destroy(&this->mutexPool);
	pthread_mutex_destroy(&this->mutexBusy);
	pthread_cond_destroy(&this->notEmpty);
}