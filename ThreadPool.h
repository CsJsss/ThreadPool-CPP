//
// Created by admin on 2022/4/6.
//

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
	using Task = std::function<void()>;
	explicit ThreadPool(size_t numOfThreads = 0, size_t maxQueueSize = 20)
	: numOfThreads_(numOfThreads), maxQueueSize_(maxQueueSize), running_(false) {}

	// 禁止赋值构造和拷贝赋值
	ThreadPool (const ThreadPool& ) = delete;
	ThreadPool& operator = (const ThreadPool&) = delete;

	// 开启线程池
	void start ();

	// 结束线程池
	void stop();

	// 析构函数
	~ThreadPool() {
		if (running_)
			stop();
	}

	// 添加任务接口
	template<typename _Callable, typename... _Args>
	void addTask(_Callable&& __f, _Args&&... __args) {
		// 封装好后添加到线程池中
		Task task = std::bind(std::forward<_Callable>(__f),std::forward<_Args...>(__args...));
		addToWorkingPool(task);
	}

private:
	void addToWorkingPool(Task task);
	bool isFull() const ;

	// 工作线程(消费者)
	void consumeTask() ;

	// 线程池工作线程
	std::vector<std::thread> workers_;

	// 任务队列
	std::deque<Task> tasks_;

	// 工作池最大个数
	size_t maxQueueSize_;
	// 线程池工作线程个数
	size_t numOfThreads_;

	// 线程池工作状态
	bool running_;

	// 互斥锁和条件变量实现消费者和生产者的唤醒, 以及线程工作状态的切换(结束线程池)
	std::mutex mtx_;
	std::condition_variable notFull_, notEmpty_;
};


#endif //THREADPOOL_THREADPOOL_H
