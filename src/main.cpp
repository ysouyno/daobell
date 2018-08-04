#include <stdio.h>
#include "bt_downloader.h"

int main(int argc, char *argv[])
{
  bt_download("/home/ysouyno/dnld/spacemacs.tar.gz.torrent", "/home/ysouyno/dnld");

  return 0;
}
