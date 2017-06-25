#include "http_downloader.h"

int main(int argc, char *argv[])
{
  if (argc <= 1) {
    std::cout << "usage: ./daobell url" << std::endl;
    return -1;
  }

  http_downloader hd(argv[1]);
  hd.download_it();

  return 0;
}
