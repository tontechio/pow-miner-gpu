## CUDA POW Miner

Inspired by [tpruvot/ccminer](https://github.com/tpruvot/ccminer) project.

This variant was tested and built on Linux (Ubuntu 18.04.5 LTS)

## System Requirements

To use CUDA on your system, you will need the following installed:

* CUDA-capable GPU (Nvidia)
* A supported version of Linux with a gcc compiler and toolchain
* NVIDIA CUDA Toolkit

## Building Guide for Linux

1. Install CUDA Toolkit and Driver:
[https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)
   
2. Install latest stable release of CMake:
   [https://cmake.org/download/](https://cmake.org/download/)
   
3. Build CUDA POW Miner:

    Install prerequisites:
    
    ```shell
    sudo apt install -y build-essential git make cmake \
        clang libgflags-dev zlib1g-dev libssl-dev \
        libreadline-dev libmicrohttpd-dev pkg-config \
        libgsl-dev python3 python3-dev python3-pip
    sudo pip3 install psutil crc16 requests
    ```
   
    Clone repository:
    
    ```shell
    cd /usr/src
    git clone --recursive https://github.com/tontechio/pow-miner-gpu.git
    cd /usr/src/pow-miner-gpu
    ```
   
    Build `pow-miner` and `lite-client`:
    
    ```shell
    mkdir /usr/bin/ton
    cd /usr/bin/ton
    export CCACHE_DISABLE=1
    export CUDA_HOME=/usr/local/cuda
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda/lib64:/usr/local/cuda/extras/CUPTI/lib64
    export PATH=$PATH:$CUDA_HOME/bin
    cmake -DCMAKE_BUILD_TYPE=Release -DMINERCUDA=true /usr/src/pow-miner-gpu
    make -j 8 pow-miner pow-miner-cuda lite-client
    ```

All done, now you have three binaries for mining:

### `/usr/bin/ton/crypto/pow-miner`

This is a standard TON pow-miner designed for CPU. Can be used in conjunction with GPU-compatible miner on the same system simultaneously.

```shell
usage: /usr/bin/ton/crypto/pow-miner [-v][-B][-w<threads>] [-t<timeout>] <my-address> <pow-seed> <pow-complexity> <iterations> [<miner-addr> <output-ext-msg-boc>] [-V]
Outputs a valid <rdata> value for proof-of-work testgiver after computing at most <iterations> hashes or terminates with non-zero exit code
```

### `/usr/bin/ton/crypto/pow-miner-cuda`

This is a GPU-miner compatible with Nvidia hardware. Can be used in multi-GPU environments (see `-g` option).
Dry run it to see the list of available GPUs in the system.

```shell
usage: crypto/pow-miner-cuda [-v][-B][-w<threads>][-g<gpu-id>] [-t<timeout>] <my-address> <pow-seed> <pow-complexity> <iterations> [<miner-addr> <output-ext-msg-boc>] [-V]
Outputs a valid <rdata> value for proof-of-work testgiver after computing at most <iterations> hashes or terminates with non-zero exit code
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
