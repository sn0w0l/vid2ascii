#define _POSIX_C_SOURCE 200809L // for popen/pclose declarations

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#ifdef IMPL_BITMAP
#include "bitmap.h"
#else
#include "array.h"
#endif // IMPL_BITMAP

static inline void print_usage(const char *const argv0) {
  printf(
      "usage: %s [-f | --full-color (default is monochrome)] [-c | --color "
      "#rrggbb] [-b | --background #rrggbb] [-t | --tiles (default is ascii)] "
      "[-o | --output <filename> (default is output.mp4)] [-r | --fps <fps> "
      "(default is 30)] <input>\n",
      argv0);
}

int main(int argc, char *argv[]) {
  static struct option long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"full-color", no_argument, 0, 'f'},
      {"color", required_argument, 0, 'c'},
      {"background", required_argument, 0, 'b'},
      {"tiles", no_argument, 0, 't'},
      {"output", required_argument, 0, 'o'},
      {"fps", required_argument, 0, 'r'},
      {0, 0, 0, 0}};

  int opt;
  int option_index;

  bool full_color = false;
  const char *fg_hex = "#ffaa44";
  const char *bg_hex = "#241f10";
  const char *output_path = "output.mp4";
  int fps = 30;

  while ((opt = getopt_long(argc, argv, "vo:hftcbr:", long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'h':
      print_usage(argv[0]);
      return 0;
    case 'f':
      full_color = true;
      break;
    case 'c':
      fg_hex = optarg;
      break;
    case 'b':
      bg_hex = optarg;
      break;
    case 't':
      tiles = tile_list;
      break;
    case 'o':
      output_path = optarg;
      break;
    case 'r':
      fps = atoi(optarg);
      if (fps <= 0) {
        fprintf(stderr, "invalid fps: %s\n", optarg);
        return 1;
      }
      break;
    }
  }

  if (optind >= argc) {
    print_usage(argv[0]);
    return 1;
  }

  if (fg_hex[0] != '#' || bg_hex[0] != '#') {
    fprintf(stderr, "invalid color format: use #123abc\n");
    return 1;
  }

  const uint32_t fg_int = (uint32_t)strtoul(fg_hex + 1, NULL, 16);
  const uint8_t fg_rgb[] = {
      (uint8_t)((fg_int & 0x00FF0000) >> 16),
      (uint8_t)((fg_int & 0x0000FF00) >> 8),
      (uint8_t)(fg_int & 0x000000FF),
  };
  const uint32_t bg_int = (uint32_t)strtoul(bg_hex + 1, NULL, 16);
  const uint8_t bg_rgb[] = {
      (uint8_t)((bg_int & 0x00FF0000) >> 16),
      (uint8_t)((bg_int & 0x0000FF00) >> 8),
      (uint8_t)(bg_int & 0x000000FF),
  };

  const char *const input = argv[optind];

  const size_t width = (1920 >> 3), height = (1080 >> 3);
  const size_t out_width = width << 3, out_height = height << 3;

  const size_t frame_size = width * height * 3;
  const size_t out_frame_size = out_width * out_height * 3;

  uint8_t *const frame = malloc(frame_size);
  if (!frame) {
    fprintf(stderr, "Failed to allocate frame buffer\n");
    return 1;
  }

  uint8_t *const out_frame = malloc(out_frame_size);
  if (!out_frame) {
    fprintf(stderr, "Failed to allocate output frame buffer\n");
    free(frame);
    return 1;
  }

  char decode_cmd[4096];
  snprintf(decode_cmd, sizeof(decode_cmd),
           "ffmpeg "
           "-loglevel error "
           "-i \"%s\" "
           "-vf \"fps=%d,scale=%zu:%zu\" "
           "-pix_fmt rgb24 "
           "-f rawvideo "
           "-",
           input, fps, width, height);

  FILE *ffmpeg_in = popen(decode_cmd, "r");
  if (!ffmpeg_in) {
    fprintf(stderr, "Failed to start ffmpeg decoder\n");
    free(frame);
    free(out_frame);
    return 1;
  }

  char encode_cmd[4096];
  snprintf(encode_cmd, sizeof(encode_cmd),
           "ffmpeg "
           "-y "
           "-loglevel error "
           "-f rawvideo "
           "-pix_fmt rgb24 "
           "-s %zux%zu "
           "-r %d "
           "-i - "
           "-i \"%s\" "
           "-map 0:v "
           "-map 1:a? "
           "-c:v libx264 "
           "-pix_fmt yuv420p "
           "-c:a aac -b:a 192k "
           "-shortest "
           "\"%s\"",
           out_width, out_height, fps, input, output_path);

  FILE *ffmpeg_out = popen(encode_cmd, "w");
  if (!ffmpeg_out) {
    fprintf(stderr, "Failed to start ffmpeg encoder\n");
    pclose(ffmpeg_in);
    free(frame);
    free(out_frame);
    return 1;
  }

  size_t frame_index = 0;
  int status = 0;

  fprintf(stderr, "Video processing started. Please wait..");
  for (;;) {
    size_t got = 0;

    while (got < frame_size) {
      size_t n = fread(frame + got, 1, frame_size - got, ffmpeg_in);

      if (n == 0)
        goto done;

      got += n;
    }

    if (full_color) {
      color_transform(frame, out_frame, width, height);
    } else {
      transform(frame, out_frame, width, height, fg_rgb, bg_rgb);
    }

    size_t written = 0;
    while (written < out_frame_size) {
      size_t n =
          fwrite(out_frame + written, 1, out_frame_size - written, ffmpeg_out);
      if (n == 0) {
        fprintf(stderr, "Failed to write frame %zu to encoder\n", frame_index);
        status = 1;
        goto done;
      }
      written += n;
    }

    frame_index++;
    // fprintf(stderr, "\rencoded frame %zu", frame_index);
    // fflush(stderr);
  }

done:
  fprintf(stderr, "\n");

  pclose(ffmpeg_in);

  int encode_status = pclose(ffmpeg_out);
  if (encode_status != 0) {
    fprintf(stderr, "ffmpeg encoder exited with status %d\n", encode_status);
    status = 1;
  }

  free(frame);
  free(out_frame);

  if (status == 0) {
    fprintf(stderr, "wrote %zu frames to %s\n", frame_index, output_path);
  }

  return status;
}
