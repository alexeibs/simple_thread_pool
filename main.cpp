#include <iostream>

#include "pool.h"

int main() {
  using namespace simple_thread_pool;

  try {
    auto pool = createPool(ThreadPoolParams{
        .threads = 2
    });

    pool->addJob([]() {
        std::cout << "Job 1 finished.\n";
    });

    pool->addJob([]() {
        std::cout << "Job 2 finished.\n";
    });

    pool->start();
    pool->join();

  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
  } catch (...) {
    std::cerr << "Something wrong.\n";
  }

  return 0;
}
