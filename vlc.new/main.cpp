#include <iostream>
#include <vlc/vlc.h>

using namespace std;

auto main(int argc, char**argv) -> int {

  auto instance = libvlc_new(argc, argv);

  libvlc_release(instance);

  return 0;
}
