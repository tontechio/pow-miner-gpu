#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE 0x10000000

#ifdef _WIN32
typedef unsigned int uint;
#endif

namespace opencl {

struct HashResult {
  uint64_t nonce;
  uint64_t vcpu;
  uint32_t cpu_id;
};

class OpenCL {
 public:
  OpenCL() = default;
  void load_source(const char *filename);
  void set_source(unsigned char *source, unsigned int length);
  void print_devices();
  int get_num_devices();
  void create_context(cl_uint platform_idx, cl_uint device_idx);
  void create_kernel(uint64_t ocl_throughput, uint64_t ocl_hpf);
  // void create_kernel();
  void load_objects(uint32_t gpu_id, uint32_t cpu_id, uint32_t expired, const unsigned char *data,
                    const uint8_t *target, const unsigned char *rdata, uint32_t gpu_threads);
  HashResult scan_hash(uint cpu_id, uint32_t gpu_threads, uint64_t threads, uint64_t start_nonce, uint expired);
  void release();

 private:
  int num_devices_ = 0;

 private:
  size_t source_size_;
  char *source_str_;

  cl_uint device_count_;
  cl_uint platform_count_;
  size_t max_work_group_size_;

  cl_platform_id *platforms_;
  cl_device_id *devices_;

  cl_uint platform_idx_;
  cl_uint device_idx_;

  cl_context context_;
  cl_program program_;
  cl_kernel kernel_;
  cl_command_queue command_queue_;

 private:
  cl_mem buffer_msg_;
  cl_mem buffer_result_;
};

}  // namespace opencl
