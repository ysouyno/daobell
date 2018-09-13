#include <stdio.h>
#include "bt_downloader.h"

int main(int argc, char *argv[])
{
  bt_download("/home/ysouyno/dnld/hello_multi_file.torrent",
              "/home/ysouyno/dnld");

  return 0;
}
