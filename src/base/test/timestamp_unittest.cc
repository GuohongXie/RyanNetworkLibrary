#include "timestamp.h"

#include <stdio.h>

#include <vector>

void PassByConstReference(const Timestamp& x) {
  printf("%s\n", x.ToString().c_str());
}

void PassByValue(Timestamp x) { printf("%s\n", x.ToString().c_str()); }

void Benchmark() {
  const int kNumber = 1000 * 1000;

  std::vector<Timestamp> stamps;
  stamps.reserve(kNumber);
  for (int i = 0; i < kNumber; ++i) {
    stamps.push_back(Timestamp::Now());
  }
  printf("%s\n", stamps.front().ToString().c_str());
  printf("%s\n", stamps.back().ToString().c_str());
  printf("%f\n", timeDifference(stamps.back(), stamps.front()));

  int increments[100] = {0};
  int64_t start = stamps.front().micro_seconds_since_epoch();
  for (int i = 1; i < kNumber; ++i) {
    int64_t next = stamps[i].micro_seconds_since_epoch();
    int64_t inc = next - start;
    start = next;
    if (inc < 0) {
      printf("reverse!\n");
    } else if (inc < 100) {
      ++increments[inc];
    } else {
      printf("big gap %d\n", static_cast<int>(inc));
    }
  }

  for (int i = 0; i < 100; ++i) {
    printf("%2d: %d\n", i, increments[i]);
  }
}

int main() {
  Timestamp now(Timestamp::Now());
  printf("%s\n", now.ToString().c_str());
  PassByValue(now);
  PassByConstReference(now);
  Benchmark();
}
