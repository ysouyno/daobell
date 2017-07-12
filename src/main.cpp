#include "http_downloader.h"
#include "ftp_downloader.h"
#include "ftp_url_parser.h"

int main(int argc, char *argv[])
{
  if (argc <= 1) {
    std::cout << "usage: ./daobell url" << std::endl;
    return -1;
  }

  // http_downloader hd(argv[1], 2);
  // hd.download_file();

  // ftp_downloader();
  ftp_url_parser fup("ftp://127.0.0.1:21/1024_lnk.txt");
  fup.print_all();

  return 0;
}
