#ifndef IMPL_BITMAP
#include "array.h"
#include "util.h"

const bool (*tiles)[64] = char_list;

void transform(uint8_t *const in_frame, uint8_t *const out_frame,
               const size_t w, const size_t h, const uint8_t fg[3],
               const uint8_t bg[3]) {
  const size_t out_w = w << 3;

  for (size_t y = 0; y < h; y++) {
    for (size_t x = 0; x < w; x++) {
      const size_t src = (y * w + x) * 3;

      const uint8_t r = in_frame[src + 0];
      const uint8_t g = in_frame[src + 1];
      const uint8_t b = in_frame[src + 2];

      const uint8_t gray = (uint8_t)((77 * (uint32_t)r + 150 * (uint32_t)g +
                                      29 * (uint32_t)b + 127) >>
                                     8);

      const uint8_t intensity = quantize(gray);

      for (size_t dy = 0; dy < 8; dy++) {
        for (size_t dx = 0; dx < 8; dx++) {
          const size_t ox = (x << 3) + dx;
          const size_t oy = (y << 3) + dy;
          const size_t tile_pos = ((7 - dy) << 3) + (7 - dx);

          const size_t dst = (oy * out_w + ox) * 3;

          const bool huh = tiles[intensity][tile_pos];

          out_frame[dst + 0] = huh ? fg[0] : bg[0];
          out_frame[dst + 1] = huh ? fg[1] : bg[1];
          out_frame[dst + 2] = huh ? fg[2] : bg[2];
        }
      }
    }
  }
}

void color_transform(uint8_t *const in_frame, uint8_t *const out_frame,
                     const size_t w, const size_t h) {
  const size_t out_w = w << 3;

  for (size_t y = 0; y < h; y++) {
    for (size_t x = 0; x < w; x++) {
      const size_t src = (y * w + x) * 3;

      const uint8_t r = quantize(in_frame[src + 0]);
      const uint8_t g = quantize(in_frame[src + 1]);
      const uint8_t b = quantize(in_frame[src + 2]);

      for (size_t dy = 0; dy < 8; dy++) {
        for (size_t dx = 0; dx < 8; dx++) {
          const size_t ox = (x << 3) + dx;
          const size_t oy = (y << 3) + dy;
          const size_t tile_pos = ((7 - dy) << 3) + (7 - dx);

          const size_t dst = (oy * out_w + ox) * 3;

          out_frame[dst + 0] = tiles[r][tile_pos] ? 255 : 0;
          out_frame[dst + 1] = tiles[g][tile_pos] ? 255 : 0;
          out_frame[dst + 2] = tiles[b][tile_pos] ? 255 : 0;
        }
      }
    }
  }
}
#endif // !IMPL_BITMAP
