#pragma once
#ifndef _TASKQUEUE_H
#define _TASKQUEUE_H
#include <queue>
#include <iostream>
#include <pthread.h>
using callback = void (*)(void* arg);

template<typename T>
struct Task
{
	Task<T>()
	{
		function = nullptr;
		arg = nullptr;
	}
	Task<T>(callback f, void* arg)
	{
		//this->arg = (T*)arg;
		this->arg = static_cast<T*>(arg);
		function = f;
	}
	callback function;
	T* arg;
};

template<typename T>
class Task2
{
public:
	Task2();
	~Task2();

	//��������
	void addTask(Task<T> task);
	void addTask(callback f, void* arg);
	// ȡ��һ������
	Task<T> takeTask();
	// ��ȡ��ǰ����ĸ���
	inline size_t taskNumber()
	{
		return taskQ.size();
	}

	int freeQ();
private:
	pthread_mutex_t m_mutex;
	std::queue<Task<T>> taskQ;
};


template<typename T>
Task2<T>::Task2()
{
	pthread_mutex_init(&m_mutex, NULL);
}

template<typename T>
Task2<T>::~Task2()
{
	pthread_mutex_destroy(&m_mutex);
}

template<typename T>
void Task2<T>::addTask(Task<T> task)
{
	pthread_mutex_lock(&m_mutex);
	taskQ.push(task);
	pthread_mutex_unlock(&m_mutex);
}

template<typename T>
void Task2<T>::addTask(callback f, void* arg)
{
	pthread_mutex_lock(&m_mutex);
	taskQ.push(Task<T>(f, arg));  // ͨ���вι���ֱ������Task<T>����
	pthread_mutex_unlock(&m_mutex);
}

template<typename T>
Task<T> Task2<T>::takeTask()
{
	Task<T> t;
	pthread_mutex_lock(&m_mutex);
	if(!taskQ.empty())
	{
		t = taskQ.front();
		taskQ.pop();
	}
	pthread_mutex_unlock(&m_mutex);
	return t;
}

template<typename T>
int Task2<T>::freeQ()
{
	// �ͷŶ��ڴ�
	while (!this->taskQ.empty())
	{
		this->taskQ.pop();
	}
	return 0;
}
#endif