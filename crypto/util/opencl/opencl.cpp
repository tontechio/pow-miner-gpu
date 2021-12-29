#include "sha256.h"
#include "opencl.h"
#include "miner.h"
#include "opencl_helper.h"
#include "../cruncher.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

namespace opencl {

void OpenCL::load_source(const char *filename) {
  FILE *fp = fopen(filename, "r");

  if (!fp) {
    LOG(ERROR) << "[ OpenCL: failed to load kernel source '" << filename << "' ]";
    exit(1);
  }

  source_str_ = (char *)malloc(MAX_SOURCE_SIZE);
  source_size_ = fread(source_str_, 1, MAX_SOURCE_SIZE, fp);
  fclose(fp);

  if (GET_VERBOSITY_LEVEL() >= VERBOSITY_NAME(INFO)) {
    LOG(PLAIN) << "[ OpenCL: loaded kernel source '" << filename << "' (" << source_size_ << " bytes) ]";
  }
}

void OpenCL::set_source(unsigned char *source, unsigned int length) {
  source_str_ = (char *)malloc(MAX_SOURCE_SIZE);
  memcpy(source_str_, source, length);
  source_size_ = length;

  if (GET_VERBOSITY_LEVEL() >= VERBOSITY_NAME(INFO)) {
    LOG(PLAIN) << "[ OpenCL: set kernel source (" << source_size_ << " bytes) ]";
  }
}

void OpenCL::print_devices() {
  cl_int cl_err = CL_SUCCESS;

  // platform
  CL_WRAPPER(clGetPlatformIDs(0, NULL, &platform_count_));
  platforms_ = (cl_platform_id *)malloc(platform_count_ * sizeof(cl_platform_id));
  CL_WRAPPER(clGetPlatformIDs(platform_count_, platforms_, NULL));

  // devices
  char buf[1024];
  num_devices_ = 0;

  for (uint p = 0; p < platform_count_; p++) {
    cl_err = clGetDeviceIDs(platforms_[p], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count_);
    if (cl_err != CL_SUCCESS) {
      LOG(PLAIN) << "[ OpenCL: platform #" << p << " ERROR on calling \"clGetDeviceIDs(...)\", error code = " << cl_err
                 << " ]";
      continue;
    }

    devices_ = (cl_device_id *)malloc(device_count_ * sizeof(cl_device_id));
    CL_WRAPPER(clGetDeviceIDs(platforms_[p], CL_DEVICE_TYPE_ALL, device_count_, devices_, NULL));

    for (uint i = 0; i < device_count_; i++) {
      CL_WRAPPER(clGetDeviceInfo(devices_[i], CL_DEVICE_NAME, sizeof(buf), buf, NULL));
      if (GET_VERBOSITY_LEVEL() >= VERBOSITY_NAME(INFO)) {
        LOG(PLAIN) << "[ OpenCL: platform #" << p << " device #" << i << " " << buf << " ]";
      }
      num_devices_++;
    }
  }
}

int OpenCL::get_num_devices() {
  return num_devices_;
}

void OpenCL::create_context(cl_uint platform_idx, cl_uint device_idx) {
  char buf[1024];

  if (devices_ != NULL)
    free(devices_);

  CL_WRAPPER(clGetDeviceIDs(platforms_[platform_idx], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count_));
  devices_ = (cl_device_id *)malloc(device_count_ * sizeof(cl_device_id));

  CL_WRAPPER(clGetDeviceIDs(platforms_[platform_idx], CL_DEVICE_TYPE_ALL, device_count_, devices_, NULL));
  CL_WRAPPER(clGetDeviceInfo(devices_[device_idx], CL_DEVICE_NAME, sizeof(buf), buf, NULL));

  CL_WRAPPER(clGetDeviceInfo(devices_[device_idx], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size_),
                             &max_work_group_size_, NULL));

  if (GET_VERBOSITY_LEVEL() >= VERBOSITY_NAME(INFO)) {
    LOG(PLAIN) << "[ OpenCL: create context for platform #" << platform_idx << " device #" << device_idx << " " << buf
               << ", max work group size is " << max_work_group_size_ << " ]";
  }

  cl_int ret;
  context_ = clCreateContext(NULL, 1, &devices_[device_idx], NULL, NULL, &ret);
  CL_WRAPPER(ret);

  device_idx_ = device_idx;
  platform_idx_ = platform_idx;
}

void OpenCL::create_kernel() {
  // printf("[ OpenCL: create kernel ]\n");
  cl_int ret;
  program_ = clCreateProgramWithSource(context_, 1, (const char **)&source_str_, (const size_t *)&source_size_, &ret);
  CL_WRAPPER(ret);

  ret = clBuildProgram(program_, 1, &devices_[device_idx_], NULL, NULL, NULL);
  if (ret != CL_SUCCESS) {
    size_t blen = 0;
    CL_WRAPPER(clGetProgramBuildInfo(program_, devices_[device_idx_], CL_PROGRAM_BUILD_LOG, 0, NULL, &blen));

    char *buffer = (char *)malloc(blen + 1);
    buffer[blen] = '\0';

    CL_WRAPPER(clGetProgramBuildInfo(program_, devices_[device_idx_], CL_PROGRAM_BUILD_LOG, blen, buffer, NULL));
    LOG(ERROR) << "[ OpenCL: ERROR ]\n" << buffer << "\n";

    free(buffer);
    exit(4);
  }

  kernel_ = clCreateKernel(program_, "bitcredit_gpu_hash", &ret);
  CL_WRAPPER(ret);

  command_queue_ = clCreateCommandQueue(context_, devices_[device_idx_], 0, &ret);
  CL_WRAPPER(ret);
}

void OpenCL::load_objects(uint32_t gpu_id, uint32_t cpu_id, uint32_t expired, const unsigned char *data,
                          const uint8_t *target, const unsigned char *rdata, uint32_t gpu_threads) {
  (void)cpu_id, (void)gpu_id;
  MsgData msg = bitcredit_prepare_msg(gpu_threads, expired, data, target, rdata);

  cl_int ret;

  buffer_msg_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, sizeof(msg), NULL, &ret);
  CL_WRAPPER(ret);

  buffer_result_ = clCreateBuffer(context_, CL_MEM_READ_WRITE, sizeof(DevHashResult), NULL, &ret);
  CL_WRAPPER(ret);

  CL_WRAPPER(clSetKernelArg(kernel_, 1, sizeof(buffer_msg_), (void *)&buffer_msg_));
  CL_WRAPPER(clSetKernelArg(kernel_, 2, sizeof(buffer_result_), (void *)&buffer_result_));

  CL_WRAPPER(clEnqueueWriteBuffer(command_queue_, buffer_msg_, CL_TRUE, 0, sizeof(msg), &msg, 0, NULL, NULL));
}

void OpenCL::release() {
  CL_WRAPPER(clReleaseCommandQueue(command_queue_));
  CL_WRAPPER(clReleaseKernel(kernel_));
  CL_WRAPPER(clReleaseProgram(program_));
  CL_WRAPPER(clReleaseContext(context_));
  CL_WRAPPER(clReleaseDevice(devices_[device_idx_]));

  CL_WRAPPER(clReleaseMemObject(buffer_result_));
  CL_WRAPPER(clReleaseMemObject(buffer_msg_));

  free(devices_);
  free(platforms_);
  free(source_str_);
}

HashResult OpenCL::scan_hash(uint cpu_id, uint32_t gpu_threads, uint64_t threads, uint64_t start_nonce, uint expired) {
  (void)expired;

  HashResult r;
  r.nonce = UINT64_MAX, r.vcpu = UINT64_MAX, r.cpu_id = cpu_id;

  uint64_t start = start_nonce / gpu_threads;
  CL_WRAPPER(clSetKernelArg(kernel_, 0, sizeof(start), (void *)&start));

  DevHashResult devresult;
  devresult.nonce = UINT64_MAX, devresult.vcpu = UINT32_MAX, devresult.found = 0;

  clEnqueueWriteBuffer(command_queue_, buffer_result_, CL_FALSE, 0, sizeof(devresult), &devresult, 0, NULL, NULL);

  size_t global_work_size[2] = {threads / gpu_threads, gpu_threads};
  CL_WRAPPER(clEnqueueNDRangeKernel(command_queue_, kernel_, 2, NULL, global_work_size, NULL, 0, NULL, NULL));

  CL_WRAPPER(clFinish(command_queue_));
  clEnqueueReadBuffer(command_queue_, buffer_result_, CL_TRUE, 0, sizeof(devresult), &devresult, 0, NULL, NULL);

  r.nonce = devresult.nonce;
  r.vcpu = (devresult.vcpu == UINT32_MAX) ? UINT64_MAX : devresult.vcpu;

  return r;
}

}  // namespace opencl
