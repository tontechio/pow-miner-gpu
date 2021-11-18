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
    LOG(INFO) << "device not found!";
    exit(4);
  }
  int rc = scanhash_credits(options.gpu_id, cpu_id, H, options, pdata, target, options.max_iterations, rdata);

  // found
  if (rc != 0) {
    auto result = ton::build_mine_result(cpu_id, H, options, rdata, pdata[0], pdata[1], (uint32_t)pdata[2]);
    if (result) {
      return result;
    }
  }
  return {};
}

}  // namespace cuda
