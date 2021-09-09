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
td::optional<std::string> Miner::run(const Options& options, const int thread_id) {
  HDataEnv H;
  H.init(options.my_address, td::Slice(options.seed.data(), options.seed.size()));

  td::Slice data = H.as_slice();
  CHECK(data.size() == 123);

  constexpr size_t prefix_size = 72;
  constexpr size_t guard_pos = prefix_size - (72 - 28);
  CHECK(0 <= guard_pos && guard_pos < 32);
  size_t got_prefix_size = (const unsigned char*)H.body.rdata1 + guard_pos + 1 - (const unsigned char*)&H;
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
}  // namespace ton
