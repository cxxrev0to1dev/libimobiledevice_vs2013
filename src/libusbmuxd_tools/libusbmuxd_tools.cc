#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#pragma comment(lib,"libusbmuxd.lib")
#include <windows.h>
#define ssize_t size_t
#else
#include <arpa/inet.h>
#endif

#include <libusbmuxd/Multi_usbmuxd.h>

int main(int argc, char *argv[])
{
  usbmuxd::Multi_usbmuxd multi_usbmuxd;
#if defined(WIN32) || defined(__CYGWIN__)
  multi_usbmuxd.AddDevice("a6fe1aff70858002fb7f13115925df47553ae2d2", 27015, 22);
#else
  multi_usbmuxd.AddLinuxDevice("a6fe1aff70858002fb7f13115925df47553ae2d2", "/var/run/usbmuxdxxxxxxxxx", 22);
#endif
  multi_usbmuxd.ActivationDevice("a6fe1aff70858002fb7f13115925df47553ae2d2");
  if (multi_usbmuxd.ActivationStatus("a6fe1aff70858002fb7f13115925df47553ae2d2") == usbmuxd::Multi_usbmuxd::ActivationStatusTable::kOK)
    getchar();
  return -1;
}

