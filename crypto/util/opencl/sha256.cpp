#include <iostream>

#include "sha256.h"
#include "miner.h"
#include "opencl.h"
#include "sha256_cl.h"

#include "crypto/util/Miner.h"

namespace opencl {

td::optional<std::string> SHA256::run(ton::HDataEnv H, unsigned char *rdata, const ton::Miner::Options &options,
                                      int cpu_id) {
  // opencl
  auto opencl = OpenCL();
  //opencl.load_source("sha256.cl");
  opencl.set_source(sha256_cl, sha256_cl_len);
  opencl.print_devices();
  opencl.create_context(0, options.gpu_id);
  opencl.create_kernel();

  // data
  td::Slice data = H.as_slice();

  td::uint64 throughput = ((td::uint64)options.gpu_threads) * 256 * 256 * 64;
  if (options.max_iterations < throughput) {
    throughput = options.max_iterations;
  }
  std::cout << "[ GPU ID: " << options.gpu_id << ", CPU thread: " << cpu_id << ", GPU threads: " << options.gpu_threads
            << ", throughput: " << throughput << " ]" << std::endl;

  // set data
  //std::cout << "data: " << hex_encode(data) << std::endl;
  unsigned char input[123], complexity[32];
  memcpy(input, data.ubegin(), data.size());
  opencl.load_objects(options.gpu_id, cpu_id, input, options.complexity.data(), rdata, options.gpu_threads);

  uint32_t expired;
  td::int64 i = 0;
  for (; i < options.max_iterations; i += throughput) {
    expired = (uint32_t)td::Clocks::system() + 900;
    HashResult foundNonce = opencl.scan_hash(cpu_id, options.gpu_threads, throughput, i, expired);
    if (foundNonce.nonce != UINT64_MAX) {
      if (options.hashes_computed) {
        *options.hashes_computed += i + foundNonce.nonce * foundNonce.vcpu;
      }
      return ton::build_mine_result(cpu_id, H, options, rdata, foundNonce.nonce, foundNonce.vcpu, expired);
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

  return {};
}

}  // namespace opencl
