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
#pragma once

#include "block/block.h"
#include "td/utils/CancellationToken.h"
#include "td/utils/optional.h"
#include "td/utils/Time.h"
#include "td/utils/Random.h"
#include <atomic>
#include <array>
#include <iomanip>

namespace ton {
class Miner {
 public:
  struct Options {
    block::StdAddress my_address;
    std::array<td::uint8, 16> seed;
    std::array<td::uint8, 32> complexity;
    td::optional<td::Timestamp> expire_at;
    td::int64 max_iterations = std::numeric_limits<td::int64>::max();
    std::atomic<td::uint64>* hashes_computed{nullptr};
    td::CancellationToken token_;
    td::int32 gpu_id;
    td::int32 threads;
  };

  static td::optional<std::string> run(const Options& options, const int thread_id);
};

class MinerCuda : public Miner {
 public:
  static td::optional<std::string> run(const Options& options, const int thread_id);
};

#pragma pack(push, 1)
struct HData {
  unsigned char op[4];
  signed char flags = -4;
  unsigned char expire[4] = {}, myaddr[32] = {}, rdata1[32] = {}, pseed[16] = {}, rdata2[32] = {};
  void inc() {
    for (long i = 31; !(rdata1[i] = ++(rdata2[i])); --i) {
    }
  }
  void set_expire(unsigned x) {
    for (int i = 3; i >= 0; --i) {
      expire[i] = (x & 0xff);
      x >>= 8;
    }
  }

  td::Slice as_slice() const {
    return td::Slice(reinterpret_cast<const td::uint8*>(this), sizeof(*this));
  }
};

struct HDataEnv {
  unsigned char d1 = 0, d2 = sizeof(HData) * 2;
  HData body;

  td::Slice as_slice() const {
    return td::Slice(reinterpret_cast<const td::uint8*>(this), sizeof(*this));
  }

  void init(const block::StdAddress& my_address, td::Slice seed) {
    std::memcpy(body.myaddr, my_address.addr.data(), sizeof(body.myaddr));
    body.flags = static_cast<td::int8>(my_address.workchain * 4 + my_address.bounceable);
    CHECK(seed.size() == 16);
    std::memcpy(body.pseed, seed.data(), 16);
    std::memcpy(body.op, "Mine", 4);

#ifdef MINERCUDA
    // empty rdata1
#else
    td::Random::secure_bytes(body.rdata1, 32);
#endif
    std::memcpy(body.rdata2, body.rdata1, 32);
  }
};

static_assert(std::is_trivially_copyable<HDataEnv>::value, "HDataEnv must be a trivial type");
#pragma pack(pop)
}  // namespace ton
