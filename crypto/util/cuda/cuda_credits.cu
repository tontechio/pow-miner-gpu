/* CreditsCoin [CRD] djm34 implementation - 2015 */

#include <stdio.h>
#include <memory.h>
#include <iostream>

#include "cuda_vector.h"
#include "miner.h"

uint64_t *d_BitNonce[MAX_CPUS];
uint64_t *d_BitVcpu[MAX_CPUS];
__constant__ uint8_t c_rdata[MAX_CPUS * 32 * MAX_GPU_THREADS];
__constant__ uint32_t pTarget[8];  // 8*4 = 32
__constant__ uint32_t c_data[48];  // 48*4 = 192

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// sha256 Transform function /////////////////////////

static __constant__ uint8 H256 = {0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
                                  0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};

static __constant__ uint32_t Ksha[64] = {
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
    0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
    0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
    0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
    0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2};

static __device__ __forceinline__ uint32_t bsg2_0(const uint32_t x) {
  uint32_t r1 = ROTR32(x, 2);
  uint32_t r2 = ROTR32(x, 13);
  uint32_t r3 = ROTR32(x, 22);
  return xor3b(r1, r2, r3);
}

static __device__ __forceinline__ uint32_t bsg2_1(const uint32_t x) {
  uint32_t r1 = ROTR32(x, 6);
  uint32_t r2 = ROTR32(x, 11);
  uint32_t r3 = ROTR32(x, 25);
  return xor3b(r1, r2, r3);
}

static __device__ __forceinline__ uint32_t ssg2_0(const uint32_t x) {
  uint32_t r1 = ROTR32(x, 7);
  uint32_t r2 = ROTR32(x, 18);
  uint32_t r3 = shr_t32(x, 3);
  return xor3b(r1, r2, r3);
}

static __device__ __forceinline__ uint32_t ssg2_1(const uint32_t x) {
  uint32_t r1 = ROTR32(x, 17);
  uint32_t r2 = ROTR32(x, 19);
  uint32_t r3 = shr_t32(x, 10);
  return xor3b(r1, r2, r3);
}

static __device__ __forceinline__ void sha2_step1(const uint32_t a, const uint32_t b, const uint32_t c, uint32_t &d,
                                                  const uint32_t e, const uint32_t f, const uint32_t g, uint32_t &h,
                                                  const uint32_t in, const uint32_t Kshared) {
  uint32_t t1, t2;
  uint32_t vxandx = xandx(e, f, g);
  uint32_t bsg21 = bsg2_1(e);
  uint32_t bsg20 = bsg2_0(a);
  uint32_t andorv = andor32(a, b, c);

  t1 = h + bsg21 + vxandx + Kshared + in;
  t2 = bsg20 + andorv;
  d = d + t1;
  h = t1 + t2;
}

static __device__ __forceinline__ void sha2_step2(const uint32_t a, const uint32_t b, const uint32_t c, uint32_t &d,
                                                  const uint32_t e, const uint32_t f, const uint32_t g, uint32_t &h,
                                                  uint32_t *in, const uint32_t pc, const uint32_t Kshared) {
  uint32_t t1, t2;

  int pcidx1 = (pc - 2) & 0xF;
  int pcidx2 = (pc - 7) & 0xF;
  int pcidx3 = (pc - 15) & 0xF;
  uint32_t inx0 = in[pc];
  uint32_t inx1 = in[pcidx1];
  uint32_t inx2 = in[pcidx2];
  uint32_t inx3 = in[pcidx3];

  uint32_t ssg21 = ssg2_1(inx1);
  uint32_t ssg20 = ssg2_0(inx3);
  uint32_t vxandx = xandx(e, f, g);
  uint32_t bsg21 = bsg2_1(e);
  uint32_t bsg20 = bsg2_0(a);
  uint32_t andorv = andor32(a, b, c);

  in[pc] = ssg21 + inx2 + ssg20 + inx0;

  t1 = h + bsg21 + vxandx + Kshared + in[pc];
  t2 = bsg20 + andorv;
  d = d + t1;
  h = t1 + t2;
}

static __device__ __forceinline__ uint8 sha256_Transform2(uint16 in[1],
                                                          const uint8 &r)  // also known as sha2_round_body
{
  uint8 tmp = r;
#define a tmp.s0
#define b tmp.s1
#define c tmp.s2
#define d tmp.s3
#define e tmp.s4
#define f tmp.s5
#define g tmp.s6
#define h tmp.s7

  sha2_step1(a, b, c, d, e, f, g, h, in[0].s0, Ksha[0]);
  sha2_step1(h, a, b, c, d, e, f, g, in[0].s1, Ksha[1]);
  sha2_step1(g, h, a, b, c, d, e, f, in[0].s2, Ksha[2]);
  sha2_step1(f, g, h, a, b, c, d, e, in[0].s3, Ksha[3]);
  sha2_step1(e, f, g, h, a, b, c, d, in[0].s4, Ksha[4]);
  sha2_step1(d, e, f, g, h, a, b, c, in[0].s5, Ksha[5]);
  sha2_step1(c, d, e, f, g, h, a, b, in[0].s6, Ksha[6]);
  sha2_step1(b, c, d, e, f, g, h, a, in[0].s7, Ksha[7]);
  sha2_step1(a, b, c, d, e, f, g, h, in[0].s8, Ksha[8]);
  sha2_step1(h, a, b, c, d, e, f, g, in[0].s9, Ksha[9]);
  sha2_step1(g, h, a, b, c, d, e, f, in[0].sa, Ksha[10]);
  sha2_step1(f, g, h, a, b, c, d, e, in[0].sb, Ksha[11]);
  sha2_step1(e, f, g, h, a, b, c, d, in[0].sc, Ksha[12]);
  sha2_step1(d, e, f, g, h, a, b, c, in[0].sd, Ksha[13]);
  sha2_step1(c, d, e, f, g, h, a, b, in[0].se, Ksha[14]);
  sha2_step1(b, c, d, e, f, g, h, a, in[0].sf, Ksha[15]);

#pragma unroll
  for (int i = 0; i < 3; i++) {
    sha2_step2(a, b, c, d, e, f, g, h, (uint32_t *)in, 0, Ksha[16 + 16 * i]);
    sha2_step2(h, a, b, c, d, e, f, g, (uint32_t *)in, 1, Ksha[17 + 16 * i]);
    sha2_step2(g, h, a, b, c, d, e, f, (uint32_t *)in, 2, Ksha[18 + 16 * i]);
    sha2_step2(f, g, h, a, b, c, d, e, (uint32_t *)in, 3, Ksha[19 + 16 * i]);
    sha2_step2(e, f, g, h, a, b, c, d, (uint32_t *)in, 4, Ksha[20 + 16 * i]);
    sha2_step2(d, e, f, g, h, a, b, c, (uint32_t *)in, 5, Ksha[21 + 16 * i]);
    sha2_step2(c, d, e, f, g, h, a, b, (uint32_t *)in, 6, Ksha[22 + 16 * i]);
    sha2_step2(b, c, d, e, f, g, h, a, (uint32_t *)in, 7, Ksha[23 + 16 * i]);
    sha2_step2(a, b, c, d, e, f, g, h, (uint32_t *)in, 8, Ksha[24 + 16 * i]);
    sha2_step2(h, a, b, c, d, e, f, g, (uint32_t *)in, 9, Ksha[25 + 16 * i]);
    sha2_step2(g, h, a, b, c, d, e, f, (uint32_t *)in, 10, Ksha[26 + 16 * i]);
    sha2_step2(f, g, h, a, b, c, d, e, (uint32_t *)in, 11, Ksha[27 + 16 * i]);
    sha2_step2(e, f, g, h, a, b, c, d, (uint32_t *)in, 12, Ksha[28 + 16 * i]);
    sha2_step2(d, e, f, g, h, a, b, c, (uint32_t *)in, 13, Ksha[29 + 16 * i]);
    sha2_step2(c, d, e, f, g, h, a, b, (uint32_t *)in, 14, Ksha[30 + 16 * i]);
    sha2_step2(b, c, d, e, f, g, h, a, (uint32_t *)in, 15, Ksha[31 + 16 * i]);
  }

#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
  return (r + tmp);
}

__global__ __launch_bounds__(256, 3) void bitcredit_gpu_hash(uint32_t gpu_threads, uint32_t cpu_id, uint64_t threads,
                                                             uint64_t startNonce, uint32_t expired,
                                                             uint64_t *NonceVector, uint64_t *VcpuVector) {
  // 2d grid of 1d blocks
  // int blockId = blockIdx.y * gridDim.x + blockIdx.x;
  // int threadId = blockId * blockDim.x + threadIdx.x;
  int vcpu = blockIdx.y;
  uint64_t thread = blockIdx.x * blockDim.x + threadIdx.x;
  uint64_t nonce = startNonce + thread;

  uint16 data[1];
  uint8 state = H256;

  // PaddedMessage[192]:
  // 00f24d69 6e650000 000000aa aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa aaaaaa00 00000000 00000000 00000000 00000000 00000000
  // 00000000 00000000 000000e6 40a697b2 9adcc54c 26404abe 70352f00 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000080 00000000
  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 000003d8

  // read rdata from offset
  // 24:31 bytes
  uint64_t rdata1 = ((uint64_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 24]) << 56 |
                    ((uint64_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 25]) << 48 |
                    ((uint64_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 26]) << 40 |
                    ((uint64_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 27]) << 32 |
                    ((uint64_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 28]) << 24 |
                    ((uint64_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 29]) << 16 |
                    ((uint64_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 30]) << 8 |
                    ((uint64_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 31]);

  // increment rdata1 & rdata2
  rdata1 += nonce;

  uint32_t rdata01 = (c_data[1] & ~(0xff)) | (uint8_t)(expired >> 24);
  uint32_t rdata02 = ((expired << 8) & ~(0xff)) | (uint8_t)(c_data[2]);

  uint32_t rdata10 = (c_data[10] & ~(0xff)) | (uint8_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 0];
  uint32_t rdata11 = (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 1] << 24 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 2] << 16 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 3] << 8 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 4];
  uint32_t rdata12 = (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 5] << 24 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 6] << 16 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 7] << 8 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 8];
  uint32_t rdata13 = (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 9] << 24 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 10] << 16 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 11] << 8 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 12];
  uint32_t rdata14 = (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 13] << 24 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 14] << 16 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 15] << 8 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 16];
  uint32_t rdata15 = (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 17] << 24 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 18] << 16 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 19] << 8 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 20];
  uint32_t rdata16 = (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 21] << 24 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 22] << 16 |
                     (uint32_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 23] << 8 | (uint8_t)(rdata1 >> 56);
  uint32_t rdata17 = (uint32_t)(rdata1 >> 24);
  uint32_t rdata18 = ((uint32_t)(rdata1 << 8) & ~(0xff)) | (uint8_t)(c_data[18]);

  uint32_t rdata22 = (c_data[22] & ~(0xff)) | (uint8_t)c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 0];
  uint32_t rdata23 = rdata11;
  uint32_t rdata24 = rdata12;
  uint32_t rdata25 = rdata13;
  uint32_t rdata26 = rdata14;
  uint32_t rdata27 = rdata15;
  uint32_t rdata28 = rdata16;
  uint32_t rdata29 = rdata17;
  uint32_t rdata30 = ((uint32_t)(rdata1 << 8) & ~(0xff)) | (uint8_t)(c_data[30]);
  //  printf(
  //      "[%d:%lld]: rdata1=%016llX rdata01=%04x rdata02=%04x rdata16=%04x rdata17=%04x rdata18=%04x rdata28=%04x "
  //      "rdata29=%04x rdata30=%04x\n",
  //      vcpu, nonce, rdata1, rdata01, rdata02, rdata16, rdata17, rdata18, rdata28, rdata29, rdata30);

  // first block
  data[0].s0 = c_data[0];
  data[0].s1 = rdata01;  //c_data[1];
  data[0].s2 = rdata02;  //c_data[2];
  data[0].s3 = c_data[3];
  data[0].s4 = c_data[4];
  data[0].s5 = c_data[5];
  data[0].s6 = c_data[6];
  data[0].s7 = c_data[7];
  data[0].s8 = c_data[8];
  data[0].s9 = c_data[9];
  data[0].sa = rdata10;  //c_data[10];
  data[0].sb = rdata11;  //c_data[11];
  data[0].sc = rdata12;  //c_data[12];
  data[0].sd = rdata13;  //c_data[13];
  data[0].se = rdata14;  //c_data[14];
  data[0].sf = rdata15;  //c_data[15];
  //  printf("data0[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n", vcpu,
  //         nonce, c_data[0], rdata01, rdata02, c_data[3], c_data[4], c_data[5], c_data[6], c_data[7], c_data[8],
  //         c_data[9], rdata10, rdata11, rdata12, rdata13, rdata14, rdata15);
  state = sha256_Transform2(data, state);

  // second block
  data[0].s0 = rdata16;  //c_data[16];
  data[0].s1 = rdata17;  //c_data[17];
  data[0].s2 = rdata18;  //c_data[18];
  data[0].s3 = c_data[19];
  data[0].s4 = c_data[20];
  data[0].s5 = c_data[21];
  data[0].s6 = rdata22;  //c_data[22];
  data[0].s7 = rdata23;  //c_data[23];
  data[0].s8 = rdata24;  //c_data[24];
  data[0].s9 = rdata25;  //c_data[25];
  data[0].sa = rdata26;  //c_data[26];
  data[0].sb = rdata27;  //c_data[27];
  data[0].sc = rdata28;  //c_data[28];
  data[0].sd = rdata29;  //c_data[29];
  data[0].se = rdata30;  //c_data[30];
  data[0].sf = c_data[31];
  //  printf("data1[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n", vcpu,
  //         nonce, rdata16, rdata17, rdata18, c_data[19], c_data[20], c_data[21], rdata22, rdata23, rdata24, rdata25,
  //         rdata26, rdata27, rdata28, rdata29, rdata30, c_data[31]);
  state = sha256_Transform2(data, state);

  // third block
  data[0].s0 = c_data[32];
  data[0].s1 = c_data[33];
  data[0].s2 = c_data[34];
  data[0].s3 = c_data[35];
  data[0].s4 = c_data[36];
  data[0].s5 = c_data[37];
  data[0].s6 = c_data[38];
  data[0].s7 = c_data[39];
  data[0].s8 = c_data[40];
  data[0].s9 = c_data[41];
  data[0].sa = c_data[42];
  data[0].sb = c_data[43];
  data[0].sc = c_data[44];
  data[0].sd = c_data[45];
  data[0].se = c_data[46];
  data[0].sf = c_data[47];
  //  printf("data2[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n", vcpu, nonce,
  //         c_data[32], c_data[33], c_data[34], c_data[35], c_data[36], c_data[37], c_data[38], c_data[39], c_data[40],
  //         c_data[41], c_data[42], c_data[43], c_data[44], c_data[45], c_data[46], c_data[47]);
  state = sha256_Transform2(data, state);

  //  printf("hash[%lld]: %04x %04x %04x %04x %04x %04x %04x %04x\n", nonce, state.s0, state.s1, state.s2, state.s3,
  //         state.s4, state.s5, state.s6, state.s7);

  //  printf("complexity[%lld]: %04x %04x %04x %04x %04x %04x %04x %04x\n", nonce, pTarget[0], pTarget[1], pTarget[2],
  //         pTarget[3], pTarget[4], pTarget[5], pTarget[6], pTarget[7]);

  // alternative memcmp
  // check s0
  if (state.s0 < pTarget[0]) {
    NonceVector[0] = nonce;
    VcpuVector[0] = vcpu;
  } else if (state.s0 > pTarget[0])
    return;
  else if (state.s0 == pTarget[0]) {
    // check s1
    if (state.s1 < pTarget[1]) {
      //      printf("%d: hash[%d:%lld]: %04x = %04x, %04x < %04x\n", cpu_id, vcpu, nonce, state.s0, pTarget[0], state.s1,
      //             pTarget[1]);
      //
      //      printf("%d: data0[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
      //             cpu_id, vcpu, nonce, c_data[0], rdata01, rdata02, c_data[3], c_data[4], c_data[5], c_data[6], c_data[7],
      //             c_data[8], c_data[9], rdata10, rdata11, rdata12, rdata13, rdata14, rdata15);
      //
      //      printf("%d: data1[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
      //             cpu_id, vcpu, nonce, rdata16, rdata17, rdata18, c_data[19], c_data[20], c_data[21], rdata22, rdata23,
      //             rdata24, rdata25, rdata26, rdata27, rdata28, rdata29, rdata30, c_data[31]);
      //
      //      printf("%d: data2[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
      //             cpu_id, vcpu, nonce, c_data[32], c_data[33], c_data[34], c_data[35], c_data[36], c_data[37], c_data[38],
      //             c_data[39], c_data[40], c_data[41], c_data[42], c_data[43], c_data[44], c_data[45], c_data[46],
      //             c_data[47]);
      //
      //      printf("%d: hash[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x\n", cpu_id, vcpu, nonce, state.s0, state.s1,
      //             state.s2, state.s3, state.s4, state.s5, state.s6, state.s7);
      //
      //      printf("%d: complexity[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x\n", cpu_id, vcpu, nonce, pTarget[0],
      //             pTarget[1], pTarget[2], pTarget[3], pTarget[4], pTarget[5], pTarget[6], pTarget[7]);
      //
      //      printf("%d: rdata[%d:%lld]: %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", cpu_id, vcpu,
      //             nonce, c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 0], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 1],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 2], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 3],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 4], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 5],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 6], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 7],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 8], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 9],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 10], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 11],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 12], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 13],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 14], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 15]);
      //
      //      printf("%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 16], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 17],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 18], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 19],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 20], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 21],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 22], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 23],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 24], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 25],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 26], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 27],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 28], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 29],
      //             c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 30], c_rdata[cpu_id * 32 * gpu_threads + 32 * vcpu + 31]);

      NonceVector[0] = nonce;
      VcpuVector[0] = vcpu;
    } else if (state.s1 > pTarget[1]) {
      return;
    } else if (state.s1 == pTarget[1]) {
      // check s2
      if (state.s2 < pTarget[2]) {
        NonceVector[0] = nonce;
        VcpuVector[0] = vcpu;
      } else if (state.s2 > pTarget[2]) {
        return;
      } else if (state.s2 == pTarget[2]) {
        // check s3
        if (state.s3 < pTarget[3]) {
          NonceVector[0] = nonce;
          VcpuVector[0] = vcpu;
        } else if (state.s3 > pTarget[3]) {
          return;
        } else if (state.s3 == pTarget[3]) {
          // check s4
          if (state.s4 < pTarget[4]) {
            NonceVector[0] = nonce;
            VcpuVector[0] = vcpu;
          } else if (state.s4 > pTarget[4]) {
            return;
          } else if (state.s4 == pTarget[4]) {
            // check s5
            if (state.s5 < pTarget[5]) {
              NonceVector[0] = nonce;
              VcpuVector[0] = vcpu;
            } else if (state.s5 > pTarget[5]) {
              return;
            } else if (state.s5 == pTarget[5]) {
              // check s6
              if (state.s6 < pTarget[6]) {
                NonceVector[0] = nonce;
                VcpuVector[0] = vcpu;
              } else if (state.s6 > pTarget[6]) {
                return;
              } else if (state.s6 == pTarget[6]) {
                // check s7
                if (state.s7 < pTarget[7]) {
                  NonceVector[0] = nonce;
                  VcpuVector[0] = vcpu;
                } else {
                  return;
                }
              }
            }
          }
        }
      }
    }
  }
}

__host__ void bitcredit_cpu_init(uint32_t gpu_id, uint32_t cpu_id, uint64_t threads) {
  cudaMalloc(&d_BitNonce[cpu_id], sizeof(uint64_t));
  cudaMalloc(&d_BitVcpu[cpu_id], sizeof(uint64_t));
}

__host__ bool bitcredit_setBlockTarget(uint32_t gpu_id, uint32_t gpu_threads, uint32_t cpu_id, unsigned char *data, const void *target,
                                       unsigned char *rdata) {
  
#ifndef _WIN32
  int len = 123, n = 3;
#else
  int len = 123;
  const int n = 3;
#endif
  uint32_t PaddedMessage[16 * n];  // bring balance to the force, 512*3 bits
  memset(PaddedMessage, 0, 16 * n * sizeof(uint32_t));
  memcpy(PaddedMessage, data, len);
  ((uchar *)PaddedMessage)[len] = 0x80;  // guard bit after data
  uint32_t endiandata[16 * n];
  for (int k = 0; k < 16 * n; k++)
    be32enc(&endiandata[k], ((uint32_t *)PaddedMessage)[k]);
  ((uint32_t *)endiandata)[16 * n - 1] = len * 8;  // size to the end

  uint32_t endiantarget[8];
  for (int k = 0; k < 8; k++)
    be32enc(&endiantarget[k], ((uint32_t *)target)[k]);

  //    std::cout << "PaddedMessage[" << 16 * n * sizeof(uint32_t) << "]: ";
  //    for (int z = 0; z < 16 * n; z++)
  //      printf("%08x ", endiandata[z]);
  //    std::cout << std::endl;

  CUDA_CALL_OR_RET_X(cudaMemcpyToSymbol(pTarget, endiantarget, 8 * sizeof(uint32_t), 0, cudaMemcpyHostToDevice), false);
  CUDA_CALL_OR_RET_X(cudaMemcpyToSymbol(c_data, endiandata, 16 * n * sizeof(uint32_t), 0, cudaMemcpyHostToDevice),
                     false);
  CUDA_CALL_OR_RET_X(cudaMemcpyToSymbol(c_rdata, rdata, 32 * gpu_threads * sizeof(uint8_t),
                                        (32 * gpu_threads * cpu_id) * sizeof(uint8_t), cudaMemcpyHostToDevice),
                     false);

  return true;
}

__host__ HashResult bitcredit_cpu_hash(uint32_t gpu_id, uint32_t cpu_id, uint32_t gpu_threads, uint64_t threads, uint64_t startNounce,
                                       uint32_t expired) {
  uint64_t result[MAX_CPUS];
  uint64_t vcpu[MAX_CPUS];

  const int threadsperblock = 256;

  HashResult r;
  r.nonce = UINT64_MAX;

  memset(result, UINT64_MAX, sizeof(result));
  memset(vcpu, UINT64_MAX, sizeof(vcpu));
  CUDA_CALL_OR_RET_X(cudaMemset(d_BitNonce[cpu_id], UINT64_MAX, sizeof(uint64_t)), r);
  CUDA_CALL_OR_RET_X(cudaMemset(d_BitVcpu[cpu_id], UINT64_MAX, sizeof(uint64_t)), r);

  dim3 grid(threads / threadsperblock / gpu_threads, gpu_threads);
  dim3 block(threadsperblock);

  bitcredit_gpu_hash<<<grid, block>>>(gpu_threads, cpu_id, threads, (startNounce / gpu_threads), expired, d_BitNonce[cpu_id],
                                      d_BitVcpu[cpu_id]);
  cudaDeviceSynchronize();

  CUDA_CALL_OR_RET_X(cudaMemcpy(&result[cpu_id], d_BitNonce[cpu_id], sizeof(uint64_t), cudaMemcpyDeviceToHost), r);
  CUDA_CALL_OR_RET_X(cudaMemcpy(&vcpu[cpu_id], d_BitVcpu[cpu_id], sizeof(uint64_t), cudaMemcpyDeviceToHost), r);
  r.nonce = result[cpu_id];
  r.vcpu = vcpu[cpu_id];
  r.cpu_id = cpu_id;
  return r;
}
