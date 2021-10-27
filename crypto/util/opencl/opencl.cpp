#include "sha256.h"
#include "opencl.h"
#include "miner.h"
#include "opencl_helper.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

namespace opencl {

void OpenCL::load_source(const char *filename) {
  FILE *fp;

  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "[ OpenCL: failed to load kernel source '%s' ]\n", filename);
    exit(1);
  }
  source_str_ = (char *)malloc(MAX_SOURCE_SIZE);
  source_size_ = fread(source_str_, 1, MAX_SOURCE_SIZE, fp);
  fclose(fp);
  printf("[ OpenCL: loaded kernel source '%s' (%lu bytes) ]\n", filename, source_size_);
}

void OpenCL::set_source(unsigned char *source, unsigned int length) {
  source_str_ = (char *)malloc(MAX_SOURCE_SIZE);
  memcpy(source_str_, source, length);
  source_size_ = length;
  printf("[ OpenCL: set kernel source (%lu bytes) ]\n", source_size_);
}

void OpenCL::print_devices() {
  // platform
  CL_WRAPPER(clGetPlatformIDs(0, NULL, &platform_count_));
  platforms_ = (cl_platform_id *)malloc(platform_count_ * sizeof(cl_platform_id));
  CL_WRAPPER(clGetPlatformIDs(platform_count_, platforms_, NULL));

  // devices
  char buf[1024];
  for (uint p = 0; p < platform_count_; p++) {
    CL_WRAPPER(clGetDeviceIDs(platforms_[p], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count_));
    devices_ = (cl_device_id *)malloc(device_count_ * sizeof(cl_device_id));
    CL_WRAPPER(clGetDeviceIDs(platforms_[p], CL_DEVICE_TYPE_ALL, device_count_, devices_, NULL));
    for (uint i = 0; i < device_count_; i++) {
      CL_WRAPPER(clGetDeviceInfo(devices_[i], CL_DEVICE_NAME, sizeof(buf), buf, NULL));
      printf("[ OpenCL: platform #%d device #%d %s ]\n", p, i, buf);
    }
  }
}

void OpenCL::create_context(cl_uint platform_idx, cl_uint device_idx) {
  char buf[1024];
  CL_WRAPPER(clGetDeviceInfo(devices_[device_idx], CL_DEVICE_NAME, sizeof(buf), buf, NULL));
  CL_WRAPPER(clGetDeviceInfo(devices_[device_idx], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size_),
                             &max_work_group_size_, NULL));

  printf("[ OpenCL: create context for platform #%d device #%d %s, max work group size is %lu ]\n", platform_idx,
         device_idx, buf, max_work_group_size_);

  cl_int ret;
  context_ = clCreateContext(NULL, 1, &devices_[device_idx], NULL, NULL, &ret);
  CL_WRAPPER(ret);

  device_idx_ = device_idx;
  platform_idx_ = platform_idx;
}

void OpenCL::create_kernel() {
  //  printf("[ OpenCL: create kernel ]\n");
  cl_int ret;
  program_ = clCreateProgramWithSource(context_, 1, (const char **)&source_str_, (const size_t *)&source_size_, &ret);
  CL_WRAPPER(ret);
  ret = clBuildProgram(program_, 1, &devices_[device_idx_], NULL, NULL, NULL);
  if (ret != CL_SUCCESS) {
    size_t blen = 0;
    CL_WRAPPER(clGetProgramBuildInfo(program_, devices_[device_idx_], CL_PROGRAM_BUILD_LOG, 0, NULL, &blen));
    
#ifndef _WIN32
    char buffer[blen];
#else
    char *buffer = (char *) malloc(blen);
#endif

    CL_WRAPPER(clGetProgramBuildInfo(program_, devices_[device_idx_], CL_PROGRAM_BUILD_LOG, blen, &buffer, NULL));

    printf("[ OpenCL: ERROR ]\n%s\n", buffer);
    
#ifdef _WIN32
	free(buffer);
#endif 

    exit(4);
  }

  kernel_ = clCreateKernel(program_, "sha256", &ret);
  CL_WRAPPER(ret);
  command_queue_ = clCreateCommandQueue(context_, devices_[device_idx_], 0, &ret);
  CL_WRAPPER(ret);
}

void OpenCL::load_objects(uint32_t gpu_id, uint32_t cpu_id, unsigned char *data, const uint8_t *target,
                          unsigned char *rdata, uint32_t gpu_threads) {

#ifndef _WIN32
  int len = 123, n = 3;
#else
  static const int len = 123, n = 3;
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

  //  std::cout << "PaddedMessage[" << 16 * n * sizeof(uint32_t) << "]: ";
  //  for (int z = 0; z < 16 * n; z++)
  //    printf("%08x ", endiandata[z]);
  //  std::cout << std::endl;

  cl_int ret;
  buffer_rdata_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(cl_uchar) * 32 * gpu_threads, NULL, &ret);
  CL_WRAPPER(ret);
  buffer_data_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(cl_uint) * 16 * 3, NULL, &ret);
  CL_WRAPPER(ret);
  buffer_target_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(cl_uint) * 8, NULL, &ret);
  CL_WRAPPER(ret);
  buffer_gpu_threads_= clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(cl_uint), NULL, &ret);
  CL_WRAPPER(ret);
  buffer_cpu_id_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(cl_uint), NULL, &ret);
  CL_WRAPPER(ret);
  buffer_threads_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(cl_ulong), NULL, &ret);
  CL_WRAPPER(ret);
  buffer_start_nonce_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(cl_ulong), NULL, &ret);
  CL_WRAPPER(ret);
  buffer_expired_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(cl_uint), NULL, &ret);
  CL_WRAPPER(ret);
  buffer_result_ = clCreateBuffer(context_, CL_MEM_READ_WRITE, sizeof(cl_ulong) * 2, NULL, &ret);
  CL_WRAPPER(ret);

  CL_WRAPPER(clEnqueueWriteBuffer(command_queue_, buffer_rdata_, CL_TRUE, 0, sizeof(cl_uchar) * 32 * gpu_threads, rdata, 0,
                                  NULL, NULL));
  CL_WRAPPER(clEnqueueWriteBuffer(command_queue_, buffer_data_, CL_TRUE, 0, sizeof(cl_uint) * 16 * 3, endiandata, 0,
                                  NULL, NULL));
  CL_WRAPPER(clEnqueueWriteBuffer(command_queue_, buffer_target_, CL_TRUE, 0, sizeof(cl_uint) * 8, endiantarget, 0,
                                  NULL, NULL));
  CL_WRAPPER(clEnqueueWriteBuffer(command_queue_, buffer_gpu_threads_, CL_TRUE, 0, sizeof(cl_uint), &gpu_threads, 0,
                                  NULL, NULL));

  CL_WRAPPER(clSetKernelArg(kernel_, 0, sizeof(buffer_rdata_), (void *)&buffer_rdata_));
  CL_WRAPPER(clSetKernelArg(kernel_, 1, sizeof(buffer_data_), (void *)&buffer_data_));
  CL_WRAPPER(clSetKernelArg(kernel_, 2, sizeof(buffer_target_), (void *)&buffer_target_));
  CL_WRAPPER(clSetKernelArg(kernel_, 3, sizeof(buffer_gpu_threads_), (void *)&buffer_gpu_threads_));
  CL_WRAPPER(clSetKernelArg(kernel_, 4, sizeof(buffer_cpu_id_), (void *)&buffer_cpu_id_));
  CL_WRAPPER(clSetKernelArg(kernel_, 5, sizeof(buffer_threads_), (void *)&buffer_threads_));
  CL_WRAPPER(clSetKernelArg(kernel_, 6, sizeof(buffer_start_nonce_), (void *)&buffer_start_nonce_));
  CL_WRAPPER(clSetKernelArg(kernel_, 7, sizeof(buffer_expired_), (void *)&buffer_expired_));
  CL_WRAPPER(clSetKernelArg(kernel_, 8, sizeof(buffer_result_), (void *)&buffer_result_));
}

HashResult OpenCL::scan_hash(uint cpu_id, uint32_t gpu_threads, td::uint64 threads, td::uint64 start_nonce, uint expired) {
  td::uint64 start = (start_nonce / gpu_threads);
  cl_ulong result[2] = {UINT64_MAX, UINT64_MAX};

  CL_WRAPPER(clEnqueueWriteBuffer(command_queue_, buffer_cpu_id_, CL_TRUE, 0, sizeof(cl_uint), &cpu_id, 0, NULL, NULL));
  CL_WRAPPER(
      clEnqueueWriteBuffer(command_queue_, buffer_threads_, CL_TRUE, 0, sizeof(cl_ulong), &threads, 0, NULL, NULL));
  CL_WRAPPER(
      clEnqueueWriteBuffer(command_queue_, buffer_start_nonce_, CL_TRUE, 0, sizeof(cl_ulong), &start, 0, NULL, NULL));
  CL_WRAPPER(
      clEnqueueWriteBuffer(command_queue_, buffer_expired_, CL_TRUE, 0, sizeof(cl_uint), &expired, 0, NULL, NULL));
  CL_WRAPPER(clEnqueueWriteBuffer(command_queue_, buffer_result_, CL_TRUE, 0, sizeof(result), result, 0, NULL, NULL));

  size_t global_work_size[2] = {threads / gpu_threads, gpu_threads};

  CL_WRAPPER(clEnqueueNDRangeKernel(command_queue_, kernel_, 2, NULL, global_work_size, NULL, 0, NULL, NULL));
  CL_WRAPPER(clFinish(command_queue_));
  clEnqueueReadBuffer(command_queue_, buffer_result_, CL_TRUE, 0, sizeof(result), result, 0, NULL, NULL);

  HashResult r;
  r.nonce = result[0];
  r.vcpu = result[1];
  r.cpu_id = cpu_id;
  return r;
}

}  // namespace opencl
