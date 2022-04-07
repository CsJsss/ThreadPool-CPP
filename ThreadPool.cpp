//
// Created by admin on 2022/4/6.
//
#include "ThreadPool.h"
#include <stdexcept>
#include <assert.h>

// 判断工作队列是否满
bool ThreadPool::isFull() const {
	return maxQueueSize_ == tasks_.size();
}

// 生产者, 向线程池添加任务
void ThreadPool::addToWorkingPool(Task task) {
	if (!running_)
		throw std::runtime_error ("ThreadPool must running when adding tasks.");

	// 如果线程池工作队列为空, 则主线程(管理线程)执行
	if (numOfThreads_ == 0) {
		task();
		return ;
	}

	std::unique_lock<std::mutex> lock(mtx_);
	// 保证最后一个任务能够被添加入任务队列.
	// 当工作队列为满时, 等待. while的作用时防止条件变量虚假唤醒
	// 如果线程池满且正在运行, 那就阻塞等待 notFull 条件变量
	while (isFull() and running_)
		notFull_.wait(lock);

	// 如果线程池不在运行, 结束
	if (!running_)
		return ;

	// 添加到工作队列中
	tasks_.emplace_back(move(task));
	// 唤醒工作线程
	notEmpty_.notify_one();
}


// 工作线程(消费者)
void ThreadPool::consumeTask() {
	while (true) {
		Task task;
		{
			std::unique_lock<std::mutex> lock(mtx_);
			// 当任务队列为空且线程池正在运行, 需要等待生产者添加任务
			while (tasks_.empty() and running_)
				notEmpty_.wait(lock);

			// 如果线程池停止且任务队列为空, 即结束该工作线程
			if (!running_ and tasks_.empty())
				return ;

			assert(tasks_.size());
			task = tasks_.front();
			tasks_.pop_front();
			notFull_.notify_one();
		}
		// 执行任务
		task();
	}
}

// 开启线程池
void ThreadPool::start () {
	running_ = true;
	// thread的创建推迟到 start()函数被调用
	for (int i = 0; i < numOfThreads_; i ++ )
		workers_.emplace_back(std::thread(&ThreadPool::consumeTask, this));
}


// 结束线程池
void ThreadPool::stop() {
	if (!running_)
		throw std::runtime_error ("ThreadPool must running when stop it.");

	{
		std::unique_lock<std::mutex> lock(mtx_);
		running_ = false;
		// 唤醒所有阻塞在 notEmpty 条件变量上的工作线程
		notEmpty_.notify_all();
		// 唤醒所有阻塞在 notFull 条件变量上的生产者线程
		notFull_.notify_all();
	}

	// 回收所有工作(消费者)线程
	for (auto& t : workers_)
		t.join();
}
