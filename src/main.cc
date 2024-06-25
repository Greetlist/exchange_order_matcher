#include <iostream>
#include <chrono>
#include "matcher.h"

int main(int argc, char** argv)
{
  Matcher m("", "600759", "SH", 2.5);
  m.Start();

  auto start = std::chrono::system_clock::now();
  while (true) {
    auto end = std::chrono::system_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 10000) {
      m.Stop();
      break;
    }
  }
  return 0;
}
