﻿
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <map>
#include <iostream>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#else
#include <win_usleep.hpp>
#include <win_gettimeofday.hpp>
#endif

// include thrust
#ifndef __cplusplus
#include <thrust/version.h>
#include <thrust/remove.h>
#include <thrust/device_vector.h>
#include <thrust/iterator/constant_iterator.h>
#else
#include <ctype.h>
#endif

#include "miner.h"
#include "cuda.hpp"

#include <cuda_runtime.h>

// CUDA Devices on the System
int cuda_num_devices() {
  int version;
  cudaError_t err = cudaDriverGetVersion(&version);
  if (err != cudaSuccess) {
    std::cerr << "Unable to query CUDA driver version! Is an nVidia driver installed?" << std::endl;
    exit(1);
  }

  int maj = version / 1000, min = version % 100;  // same as in deviceQuery sample
  if (maj < 5 || (maj == 5 && min < 5)) {
    std::cerr << "Driver does not support CUDA 5.5 API! Update your nVidia driver!" << std::endl;
    exit(1);
  }

  int GPU_N;
  err = cudaGetDeviceCount(&GPU_N);
  if (err != cudaSuccess) {
    std::cerr << "Unable to query number of CUDA devices! Is an nVidia driver installed?" << std::endl;
    exit(1);
  }
  return GPU_N;
}

void cuda_devicenames() {
  cudaError_t err;
  int GPU_N;
  err = cudaGetDeviceCount(&GPU_N);
  if (err != cudaSuccess) {
    std::cerr << "Unable to query number of CUDA devices! Is an nVidia driver installed?" << std::endl;
    exit(1);
  }

  GPU_N = std::min(MAX_GPUS, GPU_N);
  for (int i = 0; i < GPU_N; i++) {
    cudaDeviceProp props;
    cudaGetDeviceProperties(&props, device_map[i]);

    device_name[i] = strdup(props.name);
    device_sm[i] = (props.major * 100 + props.minor * 10);
  }
}

// ton-stratum-miner method
void print_cuda_devices() {
  int ngpus = cuda_num_devices();

  for (int n = 0; n < ngpus; n++) {
    int m = device_map[n];
    cudaDeviceProp props;
    cudaGetDeviceProperties(&props, m);
    LOG(PLAIN) << "CUDA: device_id #" << m << " device_name " << props.name;
  }
}

void cuda_print_devices() {
  int ngpus = cuda_num_devices();
  for (int n = 0; n < ngpus; n++) {
    int m = device_map[n];
    cudaDeviceProp props;
    cudaGetDeviceProperties(&props, m);
    if (GET_VERBOSITY_LEVEL() < VERBOSITY_NAME(INFO)) {
      return;
    }
    if (!opt_n_threads || n < opt_n_threads) {
      LOG(PLAIN) << "[ GPU #" << m << ": SM " << props.major << "." << props.minor << " " << props.name << " ]";
    }
  }
}

void cuda_shutdown() {
  cudaDeviceSynchronize();
  cudaDeviceReset();
}

static bool substringsearch(const char *haystack, const char *needle, int &match) {
  int hlen = (int)strlen(haystack);
  int nlen = (int)strlen(needle);
  for (int i = 0; i < hlen; ++i) {
    if (haystack[i] == ' ')
      continue;
    int j = 0, x = 0;
    while (j < nlen) {
      if (haystack[i + x] == ' ') {
        ++x;
        continue;
      }
      if (needle[j] == ' ') {
        ++j;
        continue;
      }
      if (needle[j] == '#')
        return ++match == needle[j + 1] - '0';
      if (tolower(haystack[i + x]) != tolower(needle[j]))
        break;
      ++j;
      ++x;
    }
    if (j == nlen)
      return true;
  }
  return false;
}

// CUDA Gerät nach Namen finden (gibt Geräte-Index zurück oder -1)
int cuda_finddevice(char *name) {
  int num = cuda_num_devices();
  int match = 0;
  for (int i = 0; i < num; ++i) {
    cudaDeviceProp props;
    if (cudaGetDeviceProperties(&props, i) == cudaSuccess)
      if (substringsearch(props.name, name, match))
        return i;
  }
  return -1;
}

uint32_t device_intensity(int gpu_id, const char *func, uint32_t defcount) {
  uint32_t throughput = gpus_intensity[gpu_id] ? gpus_intensity[gpu_id] : defcount;
//  api_set_throughput(gpu_id, throughput);
  return throughput;
}

// Zeitsynchronisations-Routine von cudaminer mit CPU sleep
// Note: if you disable all of these calls, CPU usage will hit 100%
typedef struct {
  double value[8];
} tsumarray;
cudaError_t MyStreamSynchronize(cudaStream_t stream, int situation, int gpu_id) {
  cudaError_t result = cudaSuccess;
  if (situation >= 0) {
    static std::map<int, tsumarray> tsum;

    double a = 0.95, b = 0.05;
    if (tsum.find(situation) == tsum.end()) {
      a = 0.5;
      b = 0.5;
    }  // faster initial convergence

    double tsync = 0.0;
    double tsleep = 0.95 * tsum[situation].value[gpu_id];
    if (cudaStreamQuery(stream) == cudaErrorNotReady) {
      usleep((useconds_t)(1e6 * tsleep));
      struct timeval tv_start, tv_end;
      gettimeofday(&tv_start, NULL);
      result = cudaStreamSynchronize(stream);
      gettimeofday(&tv_end, NULL);
      tsync = 1e-6 * (tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec);
    }
    if (tsync >= 0)
      tsum[situation].value[gpu_id] = a * tsum[situation].value[gpu_id] + b * (tsleep + tsync);
  } else
    result = cudaStreamSynchronize(stream);
  return result;
}

int cuda_gpu_clocks(struct cgpu_info *gpu) {
  cudaDeviceProp props;
  if (cudaGetDeviceProperties(&props, gpu->gpu_id) == cudaSuccess) {
    gpu->gpu_clock = props.clockRate;
    gpu->gpu_memclock = props.memoryClockRate;
    gpu->gpu_mem = props.totalGlobalMem;
    return 0;
  }
  return -1;
}

// if we use 2 threads on the same gpu, we need to reinit the threads
void cuda_reset_device(int gpu_id, bool *init) {
  int dev_id = device_map[gpu_id];
  cudaSetDevice(dev_id);
  if (init != NULL) {
    // with init array, its meant to be used in algo's scan code...
    for (int i = 0; i < MAX_GPUS; i++) {
      if (device_map[i] == dev_id) {
        init[i] = false;
      }
    }
    // force exit from algo's scan loops/function
#ifndef _WIN32
    restart_threads();
#endif
    cudaDeviceSynchronize();
    while (cudaStreamQuery(NULL) == cudaErrorNotReady)
      usleep(1000);
  }
  cudaDeviceReset();
}

void cudaReportHardwareFailure(int gpu_id, cudaError_t err, const char *func) {
  LOG(ERROR) << "[ GPU #" << device_map[gpu_id] << ": " << func << " " << cudaGetErrorString(err) << " ]";
}
