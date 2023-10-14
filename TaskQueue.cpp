#include "TaskQueue.h"

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
	taskQ.push(Task<T>(f, arg));  // 通过有参构造直接生成Task<T>对象
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
	// 释放堆内存
	while (!this->taskQ.empty())
	{
		this->taskQ.pop();
	}
	return 0;
}