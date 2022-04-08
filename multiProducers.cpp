//
// Created by admin on 2022/4/6.
//

#include "ThreadPool.h"
#include <assert.h>
#include <atomic>
#include <future>
#include <chrono>

// 多生产者 线程池

using namespace std;
using LL = long long;
// M: 单生产者生产个数
constexpr int N = 1e8, M = 50;
atomic<int> finish(0);

// 消费者和生产者个数
constexpr int workers = 8, producers = 4;

vector<future<LL>> futu[producers];
vector<promise<LL>> prom[producers];
vector<LL> result[producers];

vector<thread> produceThread;

void func(promise<LL>& p) {
	this_thread::sleep_for(chrono::seconds(2));
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

int main() {
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
			result[i].push_back(cur);
		}
	}

	for (int i = 0; i < producers; i ++ ) {
		for (int j = 0; j < result[i].size(); j ++ ) {
			cout << i << ' ' << j << ' ' << result[i][j] << '\n';
			assert(result[i][0] == result[i][j]);
		}
	}

	return 0;
}
