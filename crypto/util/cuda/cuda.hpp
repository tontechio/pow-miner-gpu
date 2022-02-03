#pragma once

int cuda_num_devices();
void cuda_devicenames();
void cuda_reset_device(int gpu_id, bool *init);
void cuda_shutdown();
int cuda_finddevice(char *name);
void print_cuda_devices();
void cuda_print_devices();
