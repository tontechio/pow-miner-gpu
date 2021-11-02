#pragma once

#include "td/utils/Slice-decl.h"
#include "crypto/util/Miner.h"
#ifdef _WIN32
#include "pthread.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void restart_threads(void);

extern int scanhash_credits(int thr_id, int cpu_id, ton::HDataEnv H, const ton::Miner::Options &options, uint64_t *pdata, const uint32_t *ptarget,
                            uint64_t max_nonce, unsigned char* rdata);

extern int opt_n_threads;
extern struct thr_info *thr_info;

typedef unsigned char uchar;

#define MAX_GPUS 16
#define MAX_CPUS 1
#ifndef MAX_GPU_THREADS
#define MAX_GPU_THREADS 16
#endif
extern char *device_name[MAX_GPUS];
extern short device_map[MAX_GPUS];
extern long device_sm[MAX_GPUS];
extern uint32_t gpus_intensity[MAX_GPUS];
extern uint32_t device_intensity(int thr_id, const char *func, uint32_t defcount);

struct HashResult {
  uint64_t nonce;
  uint64_t vcpu;
  uint32_t cpu_id;
};

struct cgpu_info {
  uint8_t gpu_id;
  uint8_t thr_id;
  int accepted;
  int rejected;
  int hw_errors;
  double khashes;
  uint8_t intensity_int;
  uint8_t has_monitoring;
  float gpu_temp;
  uint16_t gpu_fan;
  uint16_t gpu_fan_rpm;
  uint16_t gpu_arch;
  int gpu_clock;
  int gpu_memclock;
  size_t gpu_mem;
  uint32_t gpu_usage;
  double gpu_vddc;
  int16_t gpu_pstate;
  int16_t gpu_bus;
  uint16_t gpu_vid;
  uint16_t gpu_pid;

  int8_t nvml_id;
  int8_t nvapi_id;

  char gpu_sn[64];
  char gpu_desc[64];
  float intensity;
  uint32_t throughput;
};

struct thr_info {
  int id;
  pthread_t pth;
  struct thread_q *q;
  struct cgpu_info gpu;
};

#if !HAVE_DECL_BE32ENC
static inline void be32enc(void *pp, uint32_t x) {
  uint8_t *p = (uint8_t *)pp;
  p[3] = x & 0xff;
  p[2] = (x >> 8) & 0xff;
  p[1] = (x >> 16) & 0xff;
  p[0] = (x >> 24) & 0xff;
}
#endif

#if !HAVE_DECL_LE32ENC
static inline void le32enc(void *pp, uint32_t x) {
  uint8_t *p = (uint8_t *)pp;
  p[0] = x & 0xff;
  p[1] = (x >> 8) & 0xff;
  p[2] = (x >> 16) & 0xff;
  p[3] = (x >> 24) & 0xff;
}
#endif

#ifdef __cplusplus
}
#endif
