#ifndef task_queue_h
#define task_queue_h

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename Msg, typename T> class task_queue {
	T task;
	std::vector<std::thread> threads;
	std::mutex mutex;
	std::condition_variable cv;
	std::queue<Msg> queue;
	bool quit = false;

	task_queue(const task_queue&) = delete;
	task_queue &operator=(const task_queue&) = delete;
	task_queue(task_queue&&) = delete;
	task_queue& operator=(task_queue&&) = delete;

public:
	task_queue(T &&task, size_t count = 4) : task{std::move(task)},
		threads{count} {
			for (auto &thread : threads) thread = std::thread{[this]() {
				std::unique_lock<std::mutex> lock{mutex};
				do {
					cv.wait(lock, [this]() { return queue.size() || quit; });
					if (!quit && queue.size()) {
						auto msg = std::move(queue.front());
						queue.pop();

						lock.unlock();
						this->task(msg);
						lock.lock();
					}
				} while (!quit);
			}};
		}
			
	~task_queue() {
		std::unique_lock<std::mutex> lock{mutex};
		quit = true;
		lock.unlock();
		cv.notify_all();
		for (auto &thread : threads) {
			if (thread.joinable()) thread.join();
		}
	}

	void dispatch(const Msg &msg) {
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(msg);
		lock.unlock();
		cv.notify_all();
	}

	void dispatch(Msg &&msg) {
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(std::move(msg));
		lock.unlock();
		cv.notify_all();
	}
};

template<typename Msg, typename T>
auto make_task_queue(T &&task, std::size_t count = 4) -> task_queue<Msg, T> {
	return {std::forward<T>(task), count};
}

#endif
