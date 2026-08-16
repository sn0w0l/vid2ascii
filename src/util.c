#include "util.h"

uint8_t quantize(const uint8_t v) {
  return (uint8_t)((v >= 249) ? 15 : (v + 7) >> 4);
}
