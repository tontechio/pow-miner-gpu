#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uchar;

#define MAX_GPUS 16
#define MAX_CPUS 1
#ifndef MAX_GPU_THREADS
#define MAX_GPU_THREADS 16
#endif

#if !HAVE_DECL_BE32ENC
static inline void be32enc(void *pp, uint32_t x) {
  uint8_t *p = (uint8_t *)pp;
  p[3] = x & 0xff;
  p[2] = (x >> 8) & 0xff;
  p[1] = (x >> 16) & 0xff;
  p[0] = (x >> 24) & 0xff;
}
#endif

#ifdef __cplusplus
}
#endif