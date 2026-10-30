#ifndef SEESO_SAMPLE_CSV_LOGGER_HPP_
#define SEESO_SAMPLE_CSV_LOGGER_HPP_

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

// Owned logger for gaze data. Quote fields so commas and quotes survive.
class CSVLogger {
 public:
  explicit CSVLogger(const std::string& filename = "gaze.csv")
      : stream_(filename, std::ios::out | std::ios::trunc) {}

  void add_header(const std::string& header) {
    std::lock_guard<std::mutex> lock(mutex_);
    headers_.push_back(header);
  }

  bool write_headers() {
    std::lock_guard<std::mutex> lock(mutex_);
    return write_unlocked(headers_);
  }

  bool write_row(const std::vector<std::string>& fields) {
    std::lock_guard<std::mutex> lock(mutex_);
    return write_unlocked(fields);
  }

 private:
  bool write_unlocked(const std::vector<std::string>& fields) {
    for (std::size_t i = 0; i < fields.size(); ++i) {
      if (i) stream_ << ',';
      stream_ << '"';
      for (char c : fields[i]) {
        if (c == '"') stream_ << '"';
        stream_ << c;
      }
      stream_ << '"';
    }
    stream_ << '\n';
    stream_.flush();
    return static_cast<bool>(stream_);
  }

  std::ofstream stream_;
  std::vector<std::string> headers_;
  std::mutex mutex_;
};

#endif
