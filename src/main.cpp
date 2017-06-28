#include "http_downloader_wrapper.h"

int main(int argc, char *argv[])
{
  if (argc <= 1) {
    std::cout << "usage: ./daobell url" << std::endl;
    return -1;
  }

  // http_downloader hd(argv[1]);
  // hd.download_it();

  http_downloader_wrapper hdw(argv[1], 2);
  hdw.download_file();

  return 0;
}
