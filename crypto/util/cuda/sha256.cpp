#include <iostream>

#include "miner.h"
#include "sha256.h"
#include "cuda.hpp"
#include <cuda_runtime.h>

#include "td/utils/optional.h"
#include "tdutils/td/utils/misc.h"
#include "crypto/util/Miner.h"

char *device_name[MAX_GPUS];
short device_map[MAX_GPUS] = {0};
long device_sm[MAX_GPUS] = {0};
uint32_t gpus_intensity[MAX_GPUS] = {0};
int opt_n_threads = 0;

namespace cuda {

td::optional<std::string> SHA256::run(ton::HDataEnv H, unsigned char *rdata, const ton::Miner::Options &options,
                                      int cpu_id) {
  uint64_t pdata[3] = {0, 0, 0};  // {nonce, vcpu, expire}
  uint32_t target[8];

  // mine
  if (device_name[options.gpu_id] == NULL) {
    std::cout << "not found!" << std::endl;
    exit(4);
  }
  int rc = scanhash_credits(options.gpu_id, cpu_id, H, options, pdata, target, options.max_iterations, rdata);

  // found
  if (rc != 0) {
    std::cout << "FOUND! GPU ID: " << options.gpu_id << ", CPU thread: " << cpu_id << ", VCPU: " << pdata[1]
              << ", nonce=" << pdata[0] << ", expired=" << pdata[2] << std::endl;

    //    std::cout << cpu_id << ": "<< "rdata[" << pdata[1] << "]: ";
    //    for (int i = 0; i < 32; i++) {
    //      printf("%02x", rdata[32 * pdata[1] + i]);
    //    }
    //    std::cout << std::endl;

    // read last 8 bytes of rdata1
    uint64_t rdata1 = (uint64_t)rdata[32 * pdata[1] + 24] << (8 * 7) | (uint64_t)rdata[32 * pdata[1] + 25] << (8 * 6) |
                      (uint64_t)rdata[32 * pdata[1] + 26] << (8 * 5) | (uint64_t)rdata[32 * pdata[1] + 27] << (8 * 4) |
                      (uint64_t)rdata[32 * pdata[1] + 28] << (8 * 3) | (uint64_t)rdata[32 * pdata[1] + 29] << (8 * 2) |
                      (uint64_t)rdata[32 * pdata[1] + 30] << (8 * 1) | (uint64_t)rdata[32 * pdata[1] + 31];

    rdata1 += pdata[0];  // add nonce

    // write rdata1
    for (int i = 0; i <= 23; i++) {
      H.body.rdata1[i] = rdata[32 * pdata[1] + i];
    }
    H.body.rdata1[24] = (uint8_t)(rdata1 >> 7 * 8);
    H.body.rdata1[25] = (uint8_t)(rdata1 >> 6 * 8);
    H.body.rdata1[26] = (uint8_t)(rdata1 >> 5 * 8);
    H.body.rdata1[27] = (uint8_t)(rdata1 >> 4 * 8);
    H.body.rdata1[28] = (uint8_t)(rdata1 >> 3 * 8);
    H.body.rdata1[29] = (uint8_t)(rdata1 >> 2 * 8);
    H.body.rdata1[30] = (uint8_t)(rdata1 >> 1 * 8);
    H.body.rdata1[31] = (uint8_t)(rdata1);
    // write back rdata2
    std::memcpy(H.body.rdata2, H.body.rdata1, 32);

    // set expire
    H.body.set_expire((uint32_t)pdata[2]);

    return H.body.as_slice().str();
  }
  return {};
}

}  // namespace cuda
