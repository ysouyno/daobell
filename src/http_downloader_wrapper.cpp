#include "http_downloader_wrapper.h"

void *thread_proc(void *param)
{
  http_multi_threads_downloader *hmtd = (http_multi_threads_downloader *)param;
  std::cout << "thread_proc" << hmtd->get_current_thread_index() << std::endl;
  hmtd->download_it();

  return NULL;
}

void http_downloader_wrapper::download_file()
{
  log_i("download_file enter\n");
  log_i("thread_count_: %d\n", thread_count_);
  if (1 == thread_count_) {
    sp_hmtd_ = std::make_shared<http_multi_threads_downloader>(url_, thread_count_, 0);
    sp_hmtd_->download_it();
  }
  else {
    pthread_t tid[thread_count_];
    size_t tid_index = 0;
    vec_sp_hmtd_.erase(vec_sp_hmtd_.begin(), vec_sp_hmtd_.end());

    for (size_t i = 0; i < thread_count_; ++i) {
      vec_sp_hmtd_.push_back(std::make_shared<http_multi_threads_downloader>(url_, thread_count_, i));
    }

    for (std::vector<std::shared_ptr<http_multi_threads_downloader> >::iterator it = vec_sp_hmtd_.begin(); it != vec_sp_hmtd_.end(); ++it) {
    pthread_attr_t pat;
    pthread_attr_init(&pat);
    pthread_attr_setdetachstate(&pat, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid[tid_index], &pat, thread_proc, (void *)it->get());
    pthread_attr_destroy(&pat);
    tid_index++;
    }
  }

  getchar();
}

void http_downloader_wrapper::merge_file()
{
}
