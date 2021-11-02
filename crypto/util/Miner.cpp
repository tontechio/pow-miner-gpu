/*
    This file is part of TON Blockchain Library.

    TON Blockchain Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    TON Blockchain Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TON Blockchain Library.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2017-2020 Telegram Systems LLP
*/
#include "Miner.h"

#include "td/utils/Random.h"
#include "td/utils/misc.h"
#include "td/utils/crypto.h"
#include "td/utils/port/Clocks.h"
#include <openssl/sha.h>

namespace ton {
td::optional<std::string> Miner::run(const Options &options) {
  HDataEnv H;
  H.init(options.my_address, td::Slice(options.seed.data(), options.seed.size()));

  td::Slice data = H.as_slice();
  CHECK(data.size() == 123);

  constexpr size_t prefix_size = 72;
  constexpr size_t guard_pos = prefix_size - (72 - 28);
  CHECK(0 <= guard_pos && guard_pos < 32);
  size_t got_prefix_size = (const unsigned char *)H.body.rdata1 + guard_pos + 1 - (const unsigned char *)&H;
  CHECK(prefix_size == got_prefix_size);

  auto head = data.substr(0, prefix_size);
  auto tail = data.substr(prefix_size);

  SHA256_CTX shactx1, shactx2;
  std::array<td::uint8, 32> hash;
  SHA256_Init(&shactx1);
  auto guard = head.back();

  td::int64 i = 0, i0 = 0;
  for (; i < options.max_iterations; i++) {
    if (!(i & 0xfffff) || head.back() != guard) {
      if (options.token_) {
        break;
      }
      if (options.hashes_computed) {
        *options.hashes_computed += i - i0;
      }
      i0 = i;
      if (options.expire_at && options.expire_at.value().is_in_past(td::Timestamp::now())) {
        break;
      }
      H.body.set_expire((unsigned)td::Clocks::system() + 900);
      guard = head.back();
      SHA256_Init(&shactx1);
      SHA256_Update(&shactx1, head.ubegin(), head.size());
    }
    shactx2 = shactx1;
    SHA256_Update(&shactx2, tail.ubegin(), tail.size());
    SHA256_Final(hash.data(), &shactx2);

    if (memcmp(hash.data(), options.complexity.data(), 32) < 0) {
      // FOUND
      if (options.hashes_computed) {
        *options.hashes_computed += i - i0;
      }
      return H.body.as_slice().str();
    }
    H.body.inc();
  }
  if (options.hashes_computed) {
    *options.hashes_computed += i - i0;
  }
  return {};
}

void Miner::print_stats(td::Timestamp start_at, td::uint64 hashes_computed, td::uint64 hashes_expected) {
  auto passed = td::Timestamp::now().at() - start_at.at();
  if (passed < 1e-9) {
    passed = 1;
  }
  LOG(INFO) << "[ passed: "<< passed << "s, hashes computed: " << hashes_computed << " (" << static_cast<double>(hashes_computed)/static_cast<double>(hashes_expected)*100 << "%), speed: " << static_cast<double>(hashes_computed) / passed << " hps ]";
};

td::optional<std::string> build_mine_result(int cpu_id, ton::HDataEnv H, const ton::Miner::Options &options,
                                            unsigned char *rdata, uint64_t nonce, uint64_t vcpu, uint32_t expired) {
  LOG(INFO) << "FOUND! GPU ID: " << options.gpu_id << ", nonce=" << nonce << ", expired=" << expired;

  //    std::cout << cpu_id << ": "<< "rdata[" << vcpu << "]: ";
  //    for (int i = 0; i < 32; i++) {
  //      printf("%02x", rdata[32 * vcpu + i]);
  //    }
  //    std::cout << std::endl;

  // read last 8 bytes of rdata1
  uint64_t rdata1 = (uint64_t)rdata[32 * vcpu + 24] << (8 * 7) | (uint64_t)rdata[32 * vcpu + 25] << (8 * 6) |
                    (uint64_t)rdata[32 * vcpu + 26] << (8 * 5) | (uint64_t)rdata[32 * vcpu + 27] << (8 * 4) |
                    (uint64_t)rdata[32 * vcpu + 28] << (8 * 3) | (uint64_t)rdata[32 * vcpu + 29] << (8 * 2) |
                    (uint64_t)rdata[32 * vcpu + 30] << (8 * 1) | (uint64_t)rdata[32 * vcpu + 31];

  rdata1 += nonce;  // add nonce

  // write rdata1
  for (int i = 0; i <= 23; i++) {
    H.body.rdata1[i] = rdata[32 * vcpu + i];
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
  H.body.set_expire(expired);

  return H.body.as_slice().str();
}
}  // namespace ton
