#ifndef __CRUNCHER_H__
#define __CRUNCHER_H__


#define MAX_GPU_THREADS 16


#if defined(__OPENCL_VERSION__)
	typedef uint uint32_t;
	typedef ulong uint64_t;
#else
	#if defined(__NVCC__)
		#include <cuda_runtime.h>
		#include <device_launch_parameters.h>
	#endif

	#include <stdint.h>
#endif


#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))

#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))


typedef struct vec8u
{
	uint32_t v[8];
}
vec8u;


typedef struct vec16u
{
	uint32_t v[16];
}
vec16u;


typedef struct ThreadData
{
	vec8u state;
	uint32_t rdata[9];
}
ThreadData;


typedef struct MsgData
{
	uint32_t pseed[3];
	ThreadData thrdata[MAX_GPU_THREADS];
	vec8u target;
}
MsgData;


#if defined(__NVCC__)
__forceinline__ __device__ __host__
#else
inline
#endif
static vec8u sha256_transform(vec16u data, vec8u state)
{
	const uint32_t ksha[] = {
		0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
		0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
		0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
		0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
		0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
		0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
		0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
		0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
	};

	uint32_t m[64];

	for (int i = 0; i < 16; i += 1)
		m[i] = data.v[i];

	for (int i = 16; i < 64; i += 1)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	uint32_t a = state.v[0], b = state.v[1], c = state.v[2], d = state.v[3];
	uint32_t e = state.v[4], f = state.v[5], g = state.v[6], h = state.v[7];

#if defined(__CUDA_ARCH__) || defined(__OPENCL_VERSION__)
	#pragma unroll
#endif
	for (int i = 0; i < 64; i += 1)
	{
		uint32_t t1 = h + EP1(e) + CH(e, f, g) + ksha[i] + m[i];
		uint32_t t2 = EP0(a) + MAJ(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	state.v[0] += a, state.v[1] += b, state.v[2] += c, state.v[3] += d;
	state.v[4] += e, state.v[5] += f, state.v[6] += g, state.v[7] += h;

	return state;
}


#if !defined(__OPENCL_VERSION__)
static const vec8u h256 = {
	0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};


inline static void bitcredit_be32enc(void *pp, uint32_t x)
{
	uint8_t *p = (uint8_t *)pp;

	p[3] = x & 0xff;
	p[2] = (x >> 8) & 0xff;
	p[1] = (x >> 16) & 0xff;
	p[0] = (x >> 24) & 0xff;
}


inline static MsgData bitcredit_prepare_msg(uint32_t gpu_threads, uint32_t expired, const unsigned char *data,
											const void *target, const unsigned char *rdata)
{
	/*printf("gpu_threads: %u, expired: %04x\n", (unsigned)gpu_threads, (unsigned)expired);
	printf("data:\n");

	unsigned char padded_data[128];
	memset(padded_data, 0, sizeof(padded_data));
	memcpy(padded_data, data, 123);

	for (int i = 0; i < 32; i += 1)
	{
		for (int j = 0; j < 4; j += 1)
			printf("%02hhx ", padded_data[i * 4 + j]);
		printf("\n");
	}

	printf("target:\n");
	for (int i = 0; i < 32; i += 1)
		printf("%02hhx ", ((const unsigned char *)target)[i]);
	printf("\n");

	printf("rdata:\n");
	for (unsigned i = 0; i < gpu_threads; i += 1)
	{
		for (int j = 0; j < 32; j += 1)
			printf("%02hhx ", rdata[i * 32 + j]);
		printf("\n");
	}*/

	MsgData msg;

	for (int i = 0; i < 3; i += 1)
		bitcredit_be32enc(&msg.pseed[i], *((const uint32_t *)(data + 76) + i));

	for (int i = 0; i < 8; i += 1)
		bitcredit_be32enc(&msg.target.v[i], *((const uint32_t *)target + i));

	for (uint32_t gpu_thread = 0; gpu_thread < gpu_threads; gpu_thread += 1)
	{
		ThreadData *thrdata = &msg.thrdata[gpu_thread];
		const unsigned char *thr_rdata = rdata + gpu_thread * 32;

		vec8u state = h256;
		vec16u shadata;

		bitcredit_be32enc(&shadata.v[0x0], *((const uint32_t *)data + 0x0));

		uint32_t word;
		bitcredit_be32enc(&word, *((const uint32_t *)data + 0x1));

		shadata.v[0x1] = (word & 0xFFFFFF00) | (expired >> 24);
		shadata.v[0x2] = (expired << 8) | data[11];

		for (int i = 0x3; i <= 0x9; i += 1)
			bitcredit_be32enc(&shadata.v[i], *((const uint32_t *)data + i));

		bitcredit_be32enc(&word, *((const uint32_t *)data + 0xA));
		shadata.v[0xA] = (word & 0xFFFFFF00) | thr_rdata[0];

		for (int i = 0xB, j = 0; i <= 0xF; i += 1, j += 1)
			bitcredit_be32enc(&shadata.v[i], *((const uint32_t *)(thr_rdata + 1) + j));

		thrdata->state = sha256_transform(shadata, state);

		for (int i = 0x0, j = 0; i <= 0x1; i += 1, j += 1)
			bitcredit_be32enc(&thrdata->rdata[i], *((const uint32_t *)(thr_rdata + 21) + j));

		bitcredit_be32enc(&word, *(const uint32_t *)(thr_rdata + 28));
		thrdata->rdata[0x2] = (word << 8) | data[75];

		bitcredit_be32enc(&word, *((const uint32_t *)data + 0x16));
		thrdata->rdata[0x3] = (word & 0xFFFFFF00) | thr_rdata[0];

		for (int i = 0x4, j = 0; i <= 0x8; i += 1, j += 1)
			bitcredit_be32enc(&thrdata->rdata[i], *((const uint32_t *)(thr_rdata + 1) + j));
	}

	return msg;
}
#endif


typedef struct DevHashResult
{
	uint64_t nonce;
	uint32_t vcpu, found;
}
DevHashResult;


#if defined(__CUDACC__) || defined(__OPENCL_VERSION__)

#if defined(__OPENCL_VERSION__)
	__kernel void bitcredit_gpu_hash(
		uint64_t start_nonce, __constant const MsgData *c_msg, __global DevHashResult *result
	)
#else
	__constant__ MsgData c_msg;
	extern "C" __global__ void bitcredit_gpu_hash(uint64_t start_nonce, DevHashResult *result)
#endif
{
#if defined(__OPENCL_VERSION__)
	uint64_t idx = get_global_id(0);
	uint32_t vcpu = get_global_id(1);

	__constant const MsgData *msg = c_msg;
	__constant const ThreadData *thrdata = &msg->thrdata[vcpu];
#else
	uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;
	uint32_t vcpu = blockIdx.y;

	const MsgData *msg = &c_msg;
	const ThreadData *thrdata = &c_msg.thrdata[vcpu];
#endif

	vec8u state = thrdata->state;

	vec16u data;
	uint64_t nonce = start_nonce + idx;

	uint32_t rdata6 = thrdata->rdata[0], rdata7 = thrdata->rdata[1], rdata8 = thrdata->rdata[2];
	uint64_t rdata = (((uint64_t)rdata6 << 56) | ((uint64_t)rdata7 << 24) | ((uint64_t)rdata8 >> 8)) + nonce;

	rdata6 = (uint32_t)(rdata >> 56) | (rdata6 & 0xFFFFFF00);
	rdata7 = (uint32_t)(rdata >> 24);

	uint32_t rdata10 = (uint32_t)(rdata << 8) | 0x80;
	rdata8 = (uint32_t)(rdata << 8) | (rdata8 & 0xFF);

	data.v[0x0] = rdata6;
	data.v[0x1] = rdata7;
	data.v[0x2] = rdata8;
	data.v[0x3] = msg->pseed[0];
	data.v[0x4] = msg->pseed[1];
	data.v[0x5] = msg->pseed[2];
	data.v[0x6] = thrdata->rdata[3];
	data.v[0x7] = thrdata->rdata[4];
	data.v[0x8] = thrdata->rdata[5];
	data.v[0x9] = thrdata->rdata[6];
	data.v[0xA] = thrdata->rdata[7];
	data.v[0xB] = thrdata->rdata[8];
	data.v[0xC] = rdata6;
	data.v[0xD] = rdata7;
	data.v[0xE] = rdata10;
	data.v[0xF] = 0x00000000;

	/*if (vcpu == 0 && idx == 0)
		printf(
			"%04x %04x %04x %04x %04x %04x %04x %04x\n%04x %04x %04x %04x %04x %04x %04x %04x\n\n",
			data.v[0], data.v[1], data.v[2], data.v[3], data.v[4], data.v[5], data.v[6], data.v[7],
			data.v[8], data.v[9], data.v[10], data.v[11], data.v[12], data.v[13], data.v[14], data.v[15]
		);*/

	state = sha256_transform(data, state);

	data.v[0x0] = 0x00000000;
	data.v[0x1] = 0x00000000;
	data.v[0x2] = 0x00000000;
	data.v[0x3] = 0x00000000;
	data.v[0x4] = 0x00000000;
	data.v[0x5] = 0x00000000;
	data.v[0x6] = 0x00000000;
	data.v[0x7] = 0x00000000;
	data.v[0x8] = 0x00000000;
	data.v[0x9] = 0x00000000;
	data.v[0xA] = 0x00000000;
	data.v[0xB] = 0x00000000;
	data.v[0xC] = 0x00000000;
	data.v[0xD] = 0x00000000;
	data.v[0xE] = 0x00000000;
	data.v[0xF] = 0x000003d8;

	/*if (vcpu == 0 && idx == 0)
		printf(
			"%04x %04x %04x %04x %04x %04x %04x %04x\n%04x %04x %04x %04x %04x %04x %04x %04x\n\n",
			data.v[0], data.v[1], data.v[2], data.v[3], data.v[4], data.v[5], data.v[6], data.v[7],
			data.v[8], data.v[9], data.v[10], data.v[11], data.v[12], data.v[13], data.v[14], data.v[15]
		);*/

	state = sha256_transform(data, state);

	/*if (vcpu == 0 && idx == 0)
		printf(
			"%04x %04x %04x %04x %04x %04x %04x %04x\n\n",
			state.v[0], state.v[1], state.v[2], state.v[3], state.v[4], state.v[5], state.v[6], state.v[7]
		);*/

	for (int i = 0; i < 8; i += 1)
	{
		if (state.v[i] > msg->target.v[i])
			return;

		if (state.v[i] < msg->target.v[i])
		{
#if defined(__OPENCL_VERSION__)
			if (atomic_add(&result->found, 1) == 0)
#else
			if (atomicAdd(&result->found, 1) == 0)
#endif
				result->nonce = nonce, result->vcpu = vcpu;

			return;
		}
	}
}

#endif


#endif
