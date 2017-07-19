#include "http_downloader.h"
#include "ftp_downloader.h"
#include "ftp_url_parser.h"
#include "bencode_parser.h"

int main(int argc, char *argv[])
{
  if (argc <= 1) {
    std::cout << "usage: ./daobell url" << std::endl;
    return -1;
  }

  // http_downloader hd(argv[1]);
  // hd.download_file();

  // just test parse ftp url
  ftp_url_parser fup("ftp://127.0.0.1:21/w@w/1024_lnk.txt");
  fup.print_all();

  // ftp_downloader fd(argv[1]);
  // fd.download_it();

  // just test parse bencode
  std::ifstream file(argv[1], std::ios::binary);
  bencode_parser bp(file);
  // bp.print_all();

  std::shared_ptr<bencode_member> sp_bm = bp.get_value();
  bencode_collector bc(sp_bm);
  // bc.print_all();
  bc.print_multimap();

  return 0;
}
