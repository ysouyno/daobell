#include "http_downloader.h"
#include "progress_bar.h"

void *http_download_thread(void *arg)
{
  http_multi_threads_downloader *hmtd = (http_multi_threads_downloader *)arg;
  hmtd->download_it();

  return NULL;
}

void http_downloader::download_file()
{
  log_t("thread_count_: %d\n", thread_count_);

  pthread_t tid[thread_count_];
  size_t tid_index = 0;
  vec_sp_hmtd_.erase(vec_sp_hmtd_.begin(), vec_sp_hmtd_.end());

  for (size_t i = 0; i < thread_count_; ++i) {
    vec_sp_hmtd_.push_back(std::make_shared<http_multi_threads_downloader>(url_, thread_count_, i));
  }

  progress_bar<std::vector<std::shared_ptr<http_multi_threads_downloader> > > pb(vec_sp_hmtd_.at(0)->get_file_size());

  for (std::vector<std::shared_ptr<http_multi_threads_downloader> >::iterator it = vec_sp_hmtd_.begin();
       it != vec_sp_hmtd_.end(); ++it) {
    pthread_attr_t pat;
    pthread_attr_init(&pat);
    pthread_attr_setdetachstate(&pat, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid[tid_index], &pat, http_download_thread, (void *)it->get());
    pthread_attr_destroy(&pat);
    tid_index++;
  }

  pb.print_progress(vec_sp_hmtd_);

  merge_file();
}

void http_downloader::merge_file()
{
  if (1 == thread_count_) {
    log_t("single thread just return\n");
    return;
  }

  if (vec_sp_hmtd_.size() != thread_count_) {
    log_e("thread object count not match with thread count\n");
    return;
  }

  FILE *fp = NULL;
  std::string file_name = vec_sp_hmtd_.at(0)->get_dest_file_name();
  log_t("merge file: %s\n", file_name.c_str());

  if ((fp = fopen(file_name.c_str(), "wb+")) < 0) {
    log_e("fopen() %s error: %d\n", file_name.c_str(), errno);
    return;
  }

  for (std::vector<std::shared_ptr<http_multi_threads_downloader> >::iterator it = vec_sp_hmtd_.begin();
       it != vec_sp_hmtd_.end(); ++it) {
    log_t("multi threads\n");

    FILE *fp_temp = NULL;
    int rt = 0;
    char buff[1024] = {0};

    // need use "rb", or while(true) will exec once in windows
    if ((fp_temp = fopen((*it)->get_dest_file_name_temp().c_str(), "rb")) < 0) {
      log_e("fopen() %s error: %d\n", (*it)->get_dest_file_name_temp().c_str(), errno);
      break;
    }

    while (true) {
      if ((rt = fread(buff, sizeof(char), sizeof(buff), fp_temp)) <= 0) {
        break;
      }

      fwrite(buff, sizeof(char), rt, fp);
      memset(buff, 0, sizeof(buff));
    }

    fclose(fp_temp);

    if (-1 == unlink((*it)->get_dest_file_name_temp().c_str())) {
      log_e("unlink() error: %d\n", errno);
    }
  }

  log_t("merge file done\n");

  fclose(fp);
}
