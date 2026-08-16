#ifdef IMPL_BITMAP
#ifndef BITMAP_H
#define BITMAP_H

#include <stddef.h>
#include <stdint.h>

static const uint64_t char_list[16] = {
    0x0ULL,                // ' '
    0x0000000000303000ULL, // '.'
    0x0000000000303060ULL, // ','
    0x0030300000303000ULL, // ':'
    0x0030300000303060ULL, // ';'
    0x0000FC0000FC0000ULL, // '='
    0x6030180C18306000ULL, // '>'
    0x78CC0C1830003000ULL, // '?'
    0x7830303030307800ULL, // 'I'
    0xFCCC0C1830303000ULL, // '7'
    0x3C66C0C0C0663C00ULL, // 'C'
    0xF06060606266FE00ULL, // 'L'
    0xC6C66C38386CC600ULL, // 'X'
    0x78CCCCCCDC781C00ULL, // 'Q'
    0x7CC6DEDEDEC07800ULL, // '@'
    0xC6EEFEFED6C6C600ULL, // 'M'
};

static const uint64_t tile_list[16] = {
    0x0ULL,                // '0'
    0x0000001818000000ULL, // '4'
    0x8100001818000081ULL, // '8'
    0x0060600606606000ULL, // '12'
    0x00247620046E2400ULL, // '16'
    0x0066661818666600ULL, // '20'
    0x1866669189666618ULL, // '26'
    0xAA55A2AA55A2AA55ULL, // '30'
    0x49B6B649B6B649B6ULL, // '34'
    0xCCEC7733CCEE3733ULL, // '38'
    0xF7CAB55DBAAD53EFULL, // '42'
    0x7EDBB5DFFBADDB7EULL, // '48'
    0xFFDB99FFFF99DBFFULL, // '52'
    0x77FFFFDD77FFFFDDULL, // '56'
    0x77FFFFFF77FFFFFFULL, // '60'
    0xFFFFFFFFFFFFFFFFULL, // '64'
};

extern const uint64_t *tiles;

void transform(uint8_t *const in_frame, uint8_t *const out_frame,
               const size_t w, const size_t h, const uint8_t fg[3],
               const uint8_t bg[3]);

void color_transform(uint8_t *const in_frame, uint8_t *const out_frame,
                     const size_t w, const size_t h);

#endif // !BITMAP_H
#endif // IMPL_BITMAP
