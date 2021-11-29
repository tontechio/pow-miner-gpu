## CUDA POW Miner

Inspired by [tpruvot/ccminer](https://github.com/tpruvot/ccminer) project.

This variant was tested and built on Linux (Ubuntu 18.04.5 LTS)

#### System Requirements for CUDA POW Miner

To use CUDA on your system, you will need the following installed:

* CUDA-capable GPU (Nvidia)
* A supported version of Linux with a gcc compiler and toolchain
* NVIDIA CUDA Toolkit:
   - CUDA-Runtime-Libraries 
   - Driver components-Display Driver

## OpenCL POW Miner

This variant was tested and built on Linux (Ubuntu 18.04.5 LTS), macOS Big Sur (11.5.2)

#### System Requirements for OpenCL POW Miner

To use OpenCL on your system, you will need the following installed:

* A device that supports OpenCL (AMD)
* OpenCL headers and libraries included in the OpenCL SDK

## Building Guide for Linux

1. Install prerequisites

   ```shell
    sudo apt install -y build-essential git make cmake \
        clang libgflags-dev zlib1g-dev libssl-dev \
        libreadline-dev libmicrohttpd-dev pkg-config \
        libgsl-dev python3 python3-dev python3-pip
    sudo pip3 install psutil crc16 requests
    ```

2. Install latest stable release of CMake:
   [https://cmake.org/download/](https://cmake.org/download/)
   

3. Install requirements (depends on GPU device)
   
   a) CUDA-Runtime-Libraries and Driver components-Display Driver: [https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)

   b) OpenCL SDK from your favourite vendor. If you don't know which one you need, start from here: [https://github.com/KhronosGroup/OpenCL-SDK](https://github.com/KhronosGroup/OpenCL-SDK) or install it from repository:

   ```shell
   apt-get install opencl-headers ocl-icd-libopencl1 ocl-icd-opencl-dev
   ```

4. Clone repository:
    
    ```shell
    cd /usr/src
    git clone --recursive https://github.com/tontechio/pow-miner-gpu.git
    cd /usr/src/pow-miner-gpu
    git checkout main
    ```
   
5. Build binaries (depends on GPU device):

   a) CUDA POW Miner (`pow-miner`, `pow-miner-cuda`, `tonlib-cli`, `tonlib-cuda-cli`, `lite-client`):
    
    ```shell
    mkdir /usr/bin/ton
    cd /usr/bin/ton
    export CCACHE_DISABLE=1
    export CUDA_HOME=/usr/local/cuda
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda/lib64:/usr/local/cuda/extras/CUPTI/lib64
    export PATH=$PATH:$CUDA_HOME/bin
    cmake -DCMAKE_BUILD_TYPE=Release -DMINERCUDA=true /usr/src/pow-miner-gpu
    make -j 8 pow-miner pow-miner-cuda tonlib-cli tonlib-cuda-cli lite-client
    ```

   b) OpenCL POW Miner (`pow-miner`, `pow-miner-opencl`, `tonlib-cli`, `tonlib-opencl-cli`, `lite-client`):

    ```shell
    mkdir /usr/bin/ton
    cd /usr/bin/ton
    export CCACHE_DISABLE=1
    cmake -DCMAKE_BUILD_TYPE=Release -DMINEROPENCL=true /usr/src/pow-miner-gpu
    make -j 8 pow-miner pow-miner-opencl tonlib-cli tonlib-opencl-cli lite-client
    ```

6. Download global config for tonlib-cli's, lite-client:

   ```shell
   wget https://newton-blockchain.github.io/global.config.json
   ```
   
All done, now you have binaries for mining (depends on GPU device):

### `/usr/bin/ton/crypto/pow-miner`

This is a standard TON pow-miner designed for CPU. Can be used in conjunction with GPU-compatible miner on the same system simultaneously.

```shell
usage: /usr/bin/ton/crypto/pow-miner [-v][-B][-w<threads>] [-t<timeout>][-e<expire-at>] <my-address> <pow-seed> <pow-complexity> <iterations> [<miner-addr> <output-ext-msg-boc>] [-V]
Outputs a valid <rdata> value for proof-of-work testgiver after computing at most <iterations> hashes or terminates with non-zero exit code
```

### `/usr/bin/ton/crypto/pow-miner-cuda`

This is a GPU-miner compatible with Nvidia hardware. Can be used in multi-GPU environments (see `-g` option). Dry run it
to see the list of available GPUs in the system.

```shell
usage: crypto/pow-miner-cuda [-v][-B][-g<gpu-id>][-p<platform-id>][-F<boost factor>] [-t<timeout>][-e<expire-at>] <my-address> <pow-seed> <pow-complexity> <iterations> [<miner-addr> <output-ext-msg-boc>] [-V]
Outputs a valid <rdata> value for proof-of-work testgiver after computing at most <iterations> hashes or terminates with non-zero exit code
```

### `/usr/bin/ton/crypto/pow-miner-opencl`

This is a GPU-miner compatible with AMD hardware. Can be used in multi-GPU environments (see `-g` option). Dry run it
to see the list of available GPUs in the system.

```shell
usage: crypto/pow-miner-cuda [-v][-B][-g<gpu-id>][-p<platform-id>][-F<boost factor>] [-t<timeout>][-e<expire-at>] <my-address> <pow-seed> <pow-complexity> <iterations> [<miner-addr> <output-ext-msg-boc>] [-V]
Outputs a valid <rdata> value for proof-of-work testgiver after computing at most <iterations> hashes or terminates with non-zero exit code
```

### `/usr/bin/ton/tonlib/tonlib-cli`

This is a standard TONLIB CLI with embedded miner designed for CPU. Can be used in conjunction with GPU-compatible miner on the same system simultaneously.

```shell
usage: /usr/bin/ton/tonlib/tonlib-cli [-v] -C <lite-server-config> -e 'pminer start <giver_addess> <my_address> [cpu-threads]'
Starts miner daemon. It uses the specified number of [cpu-threads], lightserver is selected randomly. 
The process automatically receives tasks from the specified <giver_addess>. During operation the process checks the parameters of the giver every 5 seconds. 
If they change, the task is restarted. If a solution is found, it sends it to the selected <giver_addess> and <my_address> is rewarded.
Instead of giver address you can specify the strategy "auto"/"random". 
```

### `/usr/bin/ton/tonlib/tonlib-cuda-cli`

This is a TONLIB CLI with embedded GPU-miner compatible with Nvidia hardware. Can be used in multi-GPU environments (see `<gpu-id>` option). Dry run it
to see the list of available GPUs in the system.

```shell
§usage: /usr/bin/ton/tonlib/tonlib-cuda-cli [-v] -C <lite-server-config> -e 'pminer start <giver_addess> <my_address> <gpu-id> [boost-factor] [platform-id]'
The behavior is the same as the tonlib-cli CPU miner.
```

### `/usr/bin/ton/tonlib/tonlib-opencl-cli`

This is a TONLIB CLI with embedded GPU-miner compatible with AMD hardware. Can be used in multi-GPU environments (see `<gpu-id>` option). Dry run it
to see the list of available GPUs in the system.

```shell
usage: /usr/bin/ton/tonlib/tonlib-opencl-cli [-v] -C <lite-server-config> -e 'pminer start <giver_addess> <my_address> <gpu-id> [boost-factor] [platform-id]'
The behavior is the same as the tonlib-cli CPU miner.
```

### `/usr/bin/ton/lite-client/lite-client`

```shell
Test Lite Client for TON Blockchain. Options:
  -h, --help               prints_help
  -C, --global-config<arg> file to read global config
  -r, --disable-readline
  -R, --enable-readline
  -D, --db<arg>            root for dbs
  -L, --print-limit<arg>   sets maximum count of recursively printed objects
  -v, --verbosity<arg>     set verbosity level
  -V, --version            shows lite-client build information
  -i, --idx<arg>           set liteserver idx
  -a, --addr<arg>          connect to ip:port
  -c, --cmd<arg>           schedule command
  -t, --timeout<arg>       timeout in batch mode
  -p, --pub<arg>           remote public key
  -d, --daemonize          set SIGHUP
  -l, --logname<arg>       log to file
```
