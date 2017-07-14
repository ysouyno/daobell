#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

template <typename T>
class progress_bar
{
public:
  explicit progress_bar(size_t total) : total_(total)
  {
  }

  ~progress_bar() = default;

  void print_progress(T &t)
  {
    typedef typename T::iterator ITER;
    size_t prev = 0;
    double download_speed = 0;
    size_t time_remaining = 0;
    time_t old;
    time_t now;

    time(&old);

    while (true) {
      size_t n = 0;
      for (ITER it = t.begin(); it != t.end(); ++it) {
        n += (*it)->get_downloaded_size();
      }

      if (n == prev) {
        continue;
      }

      time(&now);
      int temp = now - old;
      if (0 == temp) {
        continue;
      }

      download_speed = n / (temp * 1024.0);
      time_remaining = (total_ - n) / (download_speed * 1024.0);

      if (this->run(n, download_speed, time_remaining)) {
        break;
      }

      prev = n;
    }
  }

private:
  bool run(size_t downloaded_size, double download_speed, size_t time_remaining)
  {
    double j = 0;
    double i = (double)downloaded_size / total_ * 50.0;

    printf("[");
    for (j = 0; j < i; ++j) {
      printf("#");
    }
    for (; j < 50.0; ++j) {
      printf(" ");
    }
    printf("]");

    if (download_speed < 1024) {
      printf("%5.2f%% %.0f kb/s time remaining(s): %zu",
             i * 2, download_speed, time_remaining);
    }
    else {
      printf("%5.2f%% %.0f mb/s time remaining(s): %zu",
             i * 2, download_speed / 1024.0, time_remaining);
    }

    if (50.0 == i || downloaded_size >= total_) {
      printf(" [Complete]\n");
      return true;
    }

    printf("\r");
    fflush(stdout);

    return false;
  }

  size_t total_;
};

#endif /* PROGRESS_BAR_H */
