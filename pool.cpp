#include "pool.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

namespace simple_thread_pool {

struct JobList {
  void push(Job job) {
    std::lock_guard<std::mutex> lock(mutex_);
    jobs_.push_back(std::move(job));
  }

  Job pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (jobs_.empty()) {
      return Job{};
    }
    Job result = std::move(jobs_.front());
    jobs_.pop_front();
    return result;
  }

private:
  std::mutex mutex_;
  std::list<Job> jobs_;
};

struct ThreadPoolImpl : ThreadPool {
  explicit ThreadPoolImpl(const ThreadPoolParams& p)
    : params_(p)
  {
    if (p.threads == 0) {
      throw std::runtime_error("threads number can be below zero");
    }
  }

  ~ThreadPoolImpl() override final {
    join();
  }
   
  // schedule a new job to the pool
  void addJob(Job job) override final {
    std::cout << "addJob();\n";
    std::unique_lock lock(mutex_);
    if (stopped_.load()) {
      lock.unlock();
      std::cerr << "pool is stopped\n";
      return;
    }
    jobList_.push(std::move(job));
  }

  void start() override final {
    std::cout << "start()\n";
    std::unique_lock lock(mutex_);
    if (!threads_.empty()) {
      lock.unlock();
      std::cerr << "pool is already running\n";
      return;
    }
    if (stopped_.load()) {
      lock.unlock();
      std::cerr << "pool is already stopped\n";
      return;
    }
    for (size_t i = 0; i < params_.threads; ++i) {
      threads_.push_back(
          std::make_unique<std::thread>(&ThreadPoolImpl::threadFunc, this));
    }
  }

  // stop the scheduling and wait until all scheduled jobs are finished
  void join() override final {
    std::unique_lock lock(mutex_);
    bool expected = false;
    if (stopped_.compare_exchange_strong(expected, true)) {
      ThreadList tmp;
      tmp.swap(threads_);
      lock.unlock();
      std::cout << "join()\n";
      for (auto& t : tmp) {
        t->join();
      }
    }
  }

private:
  void threadFunc() {
    std::cout << "Starting a new worker, tid = "
              << std::this_thread::get_id() << "\n";
    while (true) {
      auto job = jobList_.pop();
      if (job) {
        job();
        std::cout << "Finished job by worker, tid = "
                  << std::this_thread::get_id() << "\n";
      } else if (stopped_.load()) {
        std::cout << "Stopped worker, tid = "
                  << std::this_thread::get_id() << "\n";
        return;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  using ThreadList = std::vector<std::unique_ptr<std::thread>>;

  const ThreadPoolParams params_;
  std::mutex mutex_;
  JobList jobList_;
  std::atomic<bool> stopped_{false};
  ThreadList threads_;
};

std::shared_ptr<ThreadPool> createPool(const ThreadPoolParams& p) {
  return std::make_shared<ThreadPoolImpl>(p);
}

} // namespace simple_thread_pool
