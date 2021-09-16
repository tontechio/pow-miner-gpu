#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "td/utils/optional.h"
#include "td/utils/misc.h"
#include "crypto/util/Miner.h"

namespace opencl {
class SHA256 {
 public:
  SHA256() = default;
  td::optional<std::string> run(ton::HDataEnv H, unsigned char* rdata, const ton::Miner::Options& options, int cpu_id);
};
}  // namespace opencl
