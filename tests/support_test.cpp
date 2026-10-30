#ifdef NDEBUG
#undef NDEBUG
#endif
#include "priority_mutex.h"
#include "simple_signal.h"
#include "utils/CSVLogger.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>

int main() {
  sample::PriorityMutex mutex;
  mutex.lock_low();
  bool acquired = true;
  std::thread contender([&] { acquired = mutex.try_to_lock_high(); });
  contender.join();
  assert(!acquired);
  mutex.unlock_low();
  // Failed high-priority attempts must not block later low-priority access.
  assert(mutex.try_to_lock_low());
  mutex.unlock_low();

  int total = 0;
  sample::signal<void(int)> signal;
  auto connection = signal.connect([&](int value) { total += value; });
  signal(3);
  connection.disconnect();
  signal(4);
  assert(total == 3);

  const std::string filename = "csv-logger-test.csv";
  {
    CSVLogger logger(filename);
    logger.add_header("value");
    assert(logger.write_headers());
    assert(logger.write_row({"a,b", "say \"hello\""}));
  }
  std::ifstream input(filename, std::ios::binary);
  const std::string contents((std::istreambuf_iterator<char>(input)), {});
  assert(contents.find("\"a,b\",\"say \"\"hello\"\"\"") != std::string::npos);
  input.close();
  std::remove(filename.c_str());
}
