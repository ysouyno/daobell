#include "http_downloader.h"
#include "ftp_downloader.h"
#include "ftp_url_parser.h"
#include "bencode_parser.h"
#include "bencode_to_map.h"
#include "bencode_reader.h"
#include "torrent_helper.h"
#include "sha1.h"
#include "torrent_downloader.h"

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

  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();
  bencode_to_map btm(sp_bvb);
  btm.print_multimap();

  bencode_reader br(sp_bvb);
  std::string announce_str = br.get_announce();
  std::cout << "announce: " << announce_str << std::endl;

  // just for test torrent_info class
  auto ti = std::make_shared<torrent_info>();
  get_announce(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));

  long long piece_length = get_piece_length(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));
  printf("piece length: %lld\n", piece_length);

  long long creation_date = get_creation_date(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));
  printf("creation date: %lld\n", creation_date);

  // test parse bencode from const char *p
  const char *str_test = "d8:announce41:http://bttracker.debian.org:6969/announce7:comment35:\"Debian CD from cdimage.debian.org\"13:creation datei1497709363e9:httpseedsl85:http://cdimage.debian.org/cdimage/release/9.0.0/iso-cd/debian-9.0.0-amd64-netinst.iso85:http://cdimage.debian.org/cdimage/archive/9.0.0/iso-cd/debian-9.0.0-amd64-netinst.isoe4:infod6:lengthi304087040e4:name30:debian-9.0.0-amd64-netinst.iso12:piece lengthi262144e6:pieces2:abee";
  bencode_parser bp_const_char(str_test);

  std::string sha1_str = "test";
  char sha1_result[DIGEST_LEN] = {0};
  sha1_compute(sha1_str.c_str(), sha1_str.size(), sha1_result);
  for (size_t i = 0; i < DIGEST_LEN; i++) {
    printf("%02X", (unsigned char)sha1_result[i]);
  }
  printf("\n");

  // test torrent_downloader class
  torrent_downloader td;
  td.download_it("/home/ysouyno/dnld/debian-9.0.0-amd64-netinst.iso.torrent", "/home/ysouyno/dnld");

  return 0;
}
