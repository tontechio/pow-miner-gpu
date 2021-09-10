/* creditscoin SHA256 djm34 implementation - 2015 */

#include <iostream>

#include "miner.h"
#include "cuda_helper.h"
#include "td/utils/Slice-decl.h"
#include "td/utils/misc.h"

extern void bitcredit_setBlockTarget(uint32_t cpu_id, unsigned char *data, const void *ptarget, unsigned char *rdata);
extern void bitcredit_cpu_init(uint32_t gpu_id, uint32_t cpu_id, uint64_t threads);
extern HashResult bitcredit_cpu_hash(uint32_t gpu_id, uint32_t cpu_id, uint64_t threads, uint64_t startNounce, uint32_t expired);

static bool init = false;

extern "C" int scanhash_credits(int gpu_id, int cpu_id, ton::HDataEnv H, const ton::Miner::Options &options, uint64_t *pdata,
                                const uint32_t *ptarget, uint64_t max_nonce, unsigned char *rdata) {
  td::Slice data = H.as_slice();
  constexpr size_t prefix_size = 72;
  td::Slice head = data.substr(0, prefix_size);
  td::Slice tail = data.substr(prefix_size);
  char guard = head.back();

  // throughput
  td::uint64 throughput = device_intensity(gpu_id, __func__, 1U << 25); // 256*256*64*8
  if (options.max_iterations < throughput) {
    throughput = options.max_iterations;
  }
  std::cout << cpu_id << ": " << "GPU throughput: " << throughput << ", VCPUS: " << MAX_VCPUS << std::endl;

  // cuda device
  if (!init) {
    cudaSetDevice(device_map[gpu_id]);
    cudaSetDeviceFlags(cudaDeviceScheduleBlockingSync);
    cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);
    init = true;
  }

  // allocate mem
  bitcredit_cpu_init(gpu_id, cpu_id, throughput);

  // set data
  // std::cout << "data: " << hex_encode(data) << std::endl;
  unsigned char input[123], complexity[32];
  memcpy(input, data.ubegin(), data.size());
  bitcredit_setBlockTarget(cpu_id, input, options.complexity.data(), rdata);

  uint32_t expired;
  td::int64 i = 0;
  for (; i < options.max_iterations; i += throughput) {
    expired = (uint32_t)td::Clocks::system() + 900;
    HashResult foundNonce = bitcredit_cpu_hash(gpu_id, cpu_id, throughput, i, expired);
    if (foundNonce.nonce != UINT64_MAX) {
      pdata[0] = foundNonce.nonce;
      pdata[1] = foundNonce.vcpu;
      pdata[2] = expired;
      if (options.hashes_computed) {
        *options.hashes_computed += i + foundNonce.nonce * foundNonce.vcpu;
      }
      return 1;
    }
    if ((foundNonce.nonce + throughput) > UINT64_MAX) {
      pdata[0] = UINT64_MAX;
      pdata[1] = foundNonce.vcpu;
      pdata[2] = expired;
      if (options.hashes_computed) {
        *options.hashes_computed += i + UINT64_MAX * foundNonce.vcpu;
      }
      return 0;
    }
    if (options.token_) {
      break;
    }
    if (options.expire_at && options.expire_at.value().is_in_past(td::Timestamp::now())) {
      break;
    }
  }
  if (options.hashes_computed) {
    *options.hashes_computed += i;
  }
  return 0;
}
