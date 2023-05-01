#pragma once

#include <functional>
#include <memory>

namespace simple_thread_pool {

using Job = std::function<void ()>;

struct ThreadPool {
  virtual ~ThreadPool() = default;

  virtual void start() = 0;

  // schedule a new job to the pool
  virtual void addJob(Job) = 0;

  // stop the scheduling and wait until all scheduled jobs are finished
  virtual void join() = 0;
};

struct ThreadPoolParams {
  size_t threads = 0;
};

std::shared_ptr<ThreadPool> createPool(const ThreadPoolParams&);

} // namespace simple_thread_pool
