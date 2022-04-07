//
// Created by admin on 2022/4/7.
//

#include "ThreadPool.h"
#include <assert.h>
#include <atomic>
#include <future>
#include <chrono>

// benchmark

using namespace std;
using LL = long long;
// M: 单生产者生产个数
constexpr int N = 1e8, M = 50;
atomic<int> finish(0);

// 消费者和生产者个数
constexpr int workers = 8, producers = 8;
constexpr int ALL = M * producers;

vector<future<LL>> futu[producers];
vector<promise<LL>> prom[producers];
vector<LL> resultThreadPool[producers];

vector<future<LL>> futuOneThread(ALL);
vector<promise<LL>> promOneThread(ALL);
vector<LL> resultOnThread;

vector<thread> produceThread;

void func(promise<LL>& p) {
	LL res = 0LL;
	for (int i = 1; i <= N; i ++ )
		res += i;
	p.set_value(res);
}

void prod(int i, ThreadPool& threadPool) {
	// 多生产者, 添加到任务队列中
	for (int j = 0; j < M; j ++ ) {
		futu[i][j] = prom[i][j].get_future();
		threadPool.addTask(func, ref(prom[i][j]));
	}
	// 生产者完成数 + 1
	finish.fetch_add(1);
}

void threadPoolRun() {
	std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();

	for (int i = 0; i < producers; i ++ ) {
		prom[i].resize(M);
		futu[i].resize(M);
	}

	ThreadPool threadPool(workers, 20);

	threadPool.start();

	produceThread.resize(producers);

	// 多生产者模式
	for (int i = 0; i < producers; i ++ )
		produceThread[i] = thread(prod, i, ref(threadPool));

	// CAS 判断是否生产完毕
	int cnt = producers;
	while (!finish.compare_exchange_weak(cnt, cnt)) {
		// 当finish != cnt时, cnt 会被置成 finish的当前值, 因此每次需要重置cnt
		cnt = producers;
	}

	// 结束线程池
	threadPool.stop();

	// 回收生产者线程
	for (auto& t : produceThread)
		t.join();

	for (int i = 0; i < producers; i ++ ) {
		for (auto& f : futu[i]) {
			LL cur = f.get();
			resultThreadPool[i].push_back(cur);
		}
	}

	for (int i = 0; i < producers; i ++ )
		for (int j = 0; j < resultThreadPool[i].size(); j ++ )
			assert(resultThreadPool[i][0] == resultThreadPool[i][j]);


	std::chrono::time_point<std::chrono::system_clock> end_time = std::chrono::system_clock::now();
	std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	cout << "\n ThreadPool cost time = " << time.count() << " ms\n";
}

void oneThreadRun() {
	std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();

	for (int i = 0; i < ALL; i ++ ) {
		futuOneThread[i] = promOneThread[i].get_future();
		func(ref(promOneThread[i]));
		LL cur = futuOneThread[i].get();
		resultOnThread.push_back(cur);
	}

	for (int i = 0; i < ALL; i ++ )
		assert(resultOnThread[0] == resultOnThread[i]);

	std::chrono::time_point<std::chrono::system_clock> end_time = std::chrono::system_clock::now();
	std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	cout << "\n oneThreadRun cost time = " << time.count() << " ms\n";
}


int main() {
	oneThreadRun();
	threadPoolRun();
	return 0;
}


