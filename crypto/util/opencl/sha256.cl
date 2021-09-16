/*
 * Copyright (c) 2010, Michal Tomlein
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *     2. Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Original code by Angel Marin, Paul Johnston.
 * https://raw.githubusercontent.com/michal-tomlein/sha256crack/master/sha256.cl
 */

#define uint64_t unsigned long
#define uint32_t unsigned int
#define uint8_t unsigned char

// prototypes
uint rotr(uint x, int n);
uint ch(uint x, uint y, uint z);
uint maj(uint x, uint y, uint z);
uint sigma0(uint x);
uint sigma1(uint x);
uint gamma0(uint x);
uint gamma1(uint x);
void sha256core(uint message[], uint n, uint source_binlength, uint H[]);

uint rotr(uint x, int n) {
  if (n < 32)
    return (x >> n) | (x << (32 - n));
  return x;
}

uint ch(uint x, uint y, uint z) {
  return (x & y) ^ (~x & z);
}

uint maj(uint x, uint y, uint z) {
  return (x & y) ^ (x & z) ^ (y & z);
}

uint sigma0(uint x) {
  return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

uint sigma1(uint x) {
  return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

uint gamma0(uint x) {
  return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

uint gamma1(uint x) {
  return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

void sha256core(uint message[], uint n, uint source_binlength, uint H[]) {
  uint K[] = {0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
              0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
              0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
              0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
              0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
              0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
              0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
              0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2};
  H[0] = 0x6A09E667;
  H[1] = 0xBB67AE85;
  H[2] = 0x3C6EF372;
  H[3] = 0xA54FF53A;
  H[4] = 0x510E527F;
  H[5] = 0x9B05688C;
  H[6] = 0x1F83D9AB;
  H[7] = 0x5BE0CD19;

  uint a, b, c, d, e, f, g, h;
  uint T1, T2;

  //    for (int j=0; j<n; j++) {
  //      printf("message[%d]: ", j);
  //      for (uint i = 0; i < 16; i += 16) {
  //        for (uint t = 0; t < 64; t++) {
  //            if (t < 16) printf("%08x ", message[t+i + j*16]);
  //        }
  //      }
  //      printf("\n");
  //    }

#pragma unroll
  for (uint j = 0; j < n; j++) {
    uint W[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#pragma unroll
    for (uint i = 0; i < 16; i += 16) {
      a = H[0];
      b = H[1];
      c = H[2];
      d = H[3];
      e = H[4];
      f = H[5];
      g = H[6];
      h = H[7];
#pragma unroll
      for (uint t = 0; t < 64; t++) {
        //printf("%d ", t + i + j * 64);
        if (t < 16)
          W[t] = message[t + i + j * 16];
        else
          W[t] = gamma1(W[t - 2]) + W[t - 7] + gamma0(W[t - 15]) + W[t - 16];
        T1 = h + sigma1(e) + ch(e, f, g) + K[t] + W[t];
        T2 = sigma0(a) + maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;

        //            printf("t=%d: a=%08x b=%08x c=%08x d=%08x e=%08x f=%08x g=%08x h=%08x\n", t, a, b, c, d, e, f , g, h);
      }
      H[0] += a;
      H[1] += b;
      H[2] += c;
      H[3] += d;
      H[4] += e;
      H[5] += f;
      H[6] += g;
      H[7] += h;

      //        printf("H[%d]: H0=%08x H1=%08x H2=%08x H3=%08x H4=%08x H5=%08x H6=%08x H7=%08x\n", j, H[0], H[1], H[2], H[3], H[4], H[5] , H[6], H[7]);
    }
  }
}

__kernel void sha256(__global const uchar *c_rdata, __global const uint32_t *c_data, __global const uint32_t *pTarget,
                     __global const uint32_t *gpu_threads, __global const uint32_t *cpu_id,
                     __global const uint64_t *threads, __global const uint64_t *startNonce,
                     __global const uint32_t *expired, __global uint64_t *result) {
  uint length = 48;
  uint H[8];

  int vcpu = get_global_id(1);
  uint64_t thread = get_global_id(0);
  uint64_t nonce = startNonce[0] + thread;

  // read rdata from offset
  // 24:31 bytes
  uint64_t rdata1 = ((uint64_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 24]) << 56 |
                    ((uint64_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 25]) << 48 |
                    ((uint64_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 26]) << 40 |
                    ((uint64_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 27]) << 32 |
                    ((uint64_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 28]) << 24 |
                    ((uint64_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 29]) << 16 |
                    ((uint64_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 30]) << 8 |
                    ((uint64_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 31]);

  // increment rdata1 & rdata2
  rdata1 += nonce;

  uint32_t rdata01 = (c_data[1] & ~(0xff)) | (uint8_t)(expired[0] >> 24);
  uint32_t rdata02 = ((expired[0] << 8) & ~(0xff)) | (uint8_t)(c_data[2]);

  uint32_t rdata10 = (c_data[10] & ~(0xff)) | (uint8_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 0];
  uint32_t rdata11 = (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 1] << 24 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 2] << 16 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 3] << 8 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 4];
  uint32_t rdata12 = (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 5] << 24 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 6] << 16 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 7] << 8 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 8];
  uint32_t rdata13 = (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 9] << 24 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 10] << 16 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 11] << 8 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 12];
  uint32_t rdata14 = (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 13] << 24 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 14] << 16 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 15] << 8 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 16];
  uint32_t rdata15 = (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 17] << 24 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 18] << 16 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 19] << 8 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 20];
  uint32_t rdata16 = (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 21] << 24 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 22] << 16 |
                     (uint32_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 23] << 8 | (uint8_t)(rdata1 >> 56);
  uint32_t rdata17 = (uint32_t)(rdata1 >> 24);
  uint32_t rdata18 = ((uint32_t)(rdata1 << 8) & ~(0xff)) | (uint8_t)(c_data[18]);

  uint32_t rdata22 = (c_data[22] & ~(0xff)) | (uint8_t)c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 0];
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

  uint binb[] = {c_data[0],  rdata01,    rdata02,    c_data[3],  c_data[4],  c_data[5],  c_data[6],  c_data[7],
                 c_data[8],  c_data[9],  rdata10,    rdata11,    rdata12,    rdata13,    rdata14,    rdata15,
                 rdata16,    rdata17,    rdata18,    c_data[19], c_data[20], c_data[21], rdata22,    rdata23,
                 rdata24,    rdata25,    rdata26,    rdata27,    rdata28,    rdata29,    rdata30,    c_data[31],
                 c_data[32], c_data[33], c_data[34], c_data[35], c_data[36], c_data[37], c_data[38], c_data[39],
                 c_data[40], c_data[41], c_data[42], c_data[43], c_data[44], c_data[45], c_data[46], c_data[47]};
  uint n = 3;

  //  printf("data0[%d:%lu]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n", vcpu,
  //         nonce, c_data[0], rdata01, rdata02, c_data[3], c_data[4], c_data[5], c_data[6], c_data[7], c_data[8],
  //         c_data[9], rdata10, rdata11, rdata12, rdata13, rdata14, rdata15);
  //  printf("data1[%d:%lu]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n", vcpu,
  //         nonce, rdata16, rdata17, rdata18, c_data[19], c_data[20], c_data[21], rdata22, rdata23, rdata24, rdata25,
  //         rdata26, rdata27, rdata28, rdata29, rdata30, c_data[31]);
  //  printf("data2[%d:%lu]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n", vcpu, nonce,
  //         c_data[32], c_data[33], c_data[34], c_data[35], c_data[36], c_data[37], c_data[38], c_data[39], c_data[40],
  //         c_data[41], c_data[42], c_data[43], c_data[44], c_data[45], c_data[46], c_data[47]);

  //     printf("binb [%d]: ", 16 * n);
  //     for (i = 0; i < 16 * n; i++) {
  //       printf("%08x ", binb[i]);
  //     }
  //     printf("\n");

  sha256core(binb, n, length, H);

  //  printf("H[%d:%d:%lu]: H0=%08x H1=%08x H2=%08x H3=%08x H4=%08x H5=%08x H6=%08x H7=%08x\n", cpu_id[0], vcpu, nonce, H[0], H[1], H[2], H[3], H[4], H[5] , H[6], H[7]);

  // alternative memcmp
  // check s0
  if (H[0] < pTarget[0]) {
    result[0] = nonce;
    result[1] = vcpu;
  } else if (H[0] > pTarget[0]) {
    return;
  } else if (H[0] == pTarget[0]) {
    // check s1
    if (H[1] < pTarget[1]) {
      result[0] = nonce;
      result[1] = vcpu;

      //      printf("%d: hash[%d:%lld]: %08x = %08x, %04x < %04x\n", cpu_id[0], vcpu, nonce, H[0], pTarget[0], H[1],
      //             pTarget[1]);
      //
      //      printf("%d: data0[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
      //             cpu_id[0], vcpu, nonce, c_data[0], rdata01, rdata02, c_data[3], c_data[4], c_data[5], c_data[6], c_data[7],
      //             c_data[8], c_data[9], rdata10, rdata11, rdata12, rdata13, rdata14, rdata15);
      //
      //      printf("%d: data1[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
      //             cpu_id[0], vcpu, nonce, rdata16, rdata17, rdata18, c_data[19], c_data[20], c_data[21], rdata22, rdata23,
      //             rdata24, rdata25, rdata26, rdata27, rdata28, rdata29, rdata30, c_data[31]);
      //
      //      printf("%d: data2[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
      //             cpu_id[0], vcpu, nonce, c_data[32], c_data[33], c_data[34], c_data[35], c_data[36], c_data[37], c_data[38],
      //             c_data[39], c_data[40], c_data[41], c_data[42], c_data[43], c_data[44], c_data[45], c_data[46],
      //             c_data[47]);
      //
      //      printf("%d: hash[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x\n", cpu_id[0], vcpu, nonce, H[0], H[1], H[2],
      //             H[3], H[4], H[5], H[6], H[7]);
      //
      //      printf("%d: complexity[%d:%lld]: %04x %04x %04x %04x %04x %04x %04x %04x\n", cpu_id[0], vcpu, nonce, pTarget[0],
      //             pTarget[1], pTarget[2], pTarget[3], pTarget[4], pTarget[5], pTarget[6], pTarget[7]);
      //
      //      printf("%d: rdata[%d:%lld]: %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", cpu_id[0], vcpu,
      //             nonce, c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 0],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 1], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 2],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 3], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 4],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 5], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 6],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 7], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 8],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 9], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 10],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 11], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 12],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 13], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 14],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 15]);
      //
      //      printf("%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 16], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 17],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 18], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 19],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 20], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 21],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 22], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 23],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 24], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 25],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 26], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 27],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 28], c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 29],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 30],
      //             c_rdata[cpu_id[0] * 32 * gpu_threads[0] + 32 * vcpu + 31]);

    } else if (H[1] > pTarget[1]) {
      return;
    } else if (H[1] == pTarget[1]) {
      // check s2
      if (H[2] < pTarget[2]) {
        result[0] = nonce;
        result[1] = vcpu;
      } else if (H[2] > pTarget[2]) {
        return;
      } else if (H[2] == pTarget[2]) {
        // check s3
        if (H[3] < pTarget[3]) {
          result[0] = nonce;
          result[1] = vcpu;
        } else if (H[3] > pTarget[3]) {
          return;
        } else if (H[3] == pTarget[3]) {
          // check s4
          if (H[4] < pTarget[4]) {
            result[0] = nonce;
            result[1] = vcpu;
          } else if (H[4] > pTarget[4]) {
            return;
          } else if (H[4] == pTarget[4]) {
            // check s5
            if (H[5] < pTarget[5]) {
              result[0] = nonce;
              result[1] = vcpu;
            } else if (H[5] > pTarget[5]) {
              return;
            } else if (H[5] == pTarget[5]) {
              // check s6
              if (H[6] < pTarget[6]) {
                result[0] = nonce;
                result[1] = vcpu;
              } else if (H[6] > pTarget[6]) {
                return;
              } else if (H[6] == pTarget[6]) {
                // check s7
                if (H[7] < pTarget[7]) {
                  result[0] = nonce;
                  result[1] = vcpu;
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