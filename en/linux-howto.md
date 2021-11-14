<style type="text/css" rel="stylesheet">
body {
  font:14px/22px Helvetica, Arial, sans-serif;
}
</style>
# Linux installation

## Requirements

### Personal wallet

Download and install any official wallet from [ton.org/wallets](https://ton.org/wallets).
Follow instructions to obtain your address.

**NB: Don't use any bot or exchange address for mining**

### Nvidia drivers

- download and install latest [CUDA toolkit](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)

### OpenCL (AMD) drivers

- download and install OpenCL SDK

   ```shell
   apt-get install opencl-headers ocl-icd-libopencl1 ocl-icd-opencl-dev
   ```
   
   or get it directly from [KhronosGroup/OpenCL-SDK](https://github.com/KhronosGroup/OpenCL-SDK)

## Installation

1. [Download](https://github.com/tontechio/pow-miner-gpu/releases/latest) latest build for your operating system and GPU make
1. Let's suppose it is `minertools-cuda-ubuntu-20.04-x86-64.tar.gz`
1. Create miner directory: `sudo mkdir -p /opt/ton-miner; sudo chown $USER /opt/ton-miner`
1. Extract downloaded file (change `PATH/TO` to the actual local path) `tar xzf PATH/TO/minertools-cuda-ubuntu-20.04-x86-64.tar.gz /opt/ton-miner/`
1. Download TON network config `cd /opt/ton-miner && curl -L -O https://newton-blockchain.github.io/global.config.json`


## Configuration

### "Giver" contracts

TON mining is a giving a reward for a job from "giver" smartcontract. It's not about "mining blocks" i.e. network activity support. It's a way to spread coins between users and borrow base capital for future validating directly or via *nominator* contract.

There are 10 giver contracts addresses (more info [here](https://ton.org/mining)):

```
1.  kf-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWraV
2.  kf8JfFUEJhhpRW80_jqD7zzQteH6EBHOzxiOhygRhBdt4z2N
3.  kf8kO6K6Qh6YM4ddjRYYlvVAK7IgyW8Zet-4ZvNrVsmQ4EOF
4.  kf9iWhwk9GwAXjtwKG-vN7rmXT3hLIT23RBY6KhVaynRrIK7
5.  kf_NSzfDJI1A3rOM0GQm7xsoUXHTgmdhN5-OrGD8uwL2JMvQ
6.  kf8gf1PQy4u2kURl-Gz4LbS29eaN4sVdrVQkPO-JL80VhOe6
7.  kf-P_TOdwcCh0AXHhBpICDMxStxHenWdLCDLNH5QcNpwMHJ8
8.  kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN
9.  kf91o4NNTryJ-Cw3sDGt9OTiafmETdVFUMvylQdFPoOxIsLm
10. kf8SYc83pm5JkGt0p3TQRkuiM58O9Cr3waUtR9OoFq716lN-
```

### Get list of GPU

Run `/opt/ton-miner/pow-miner-cuda` (or `pow-miner-opencl` for AMD) to get list of available GPUs and it's ID

Cuda (Nvidia) gpus `#0` and `#1` stands for gpu-id `0` and gpu-id `1`:

```
$ /opt/ton-miner/pow-miner-cuda
...
[ GPU #0: SM 6.1 NVIDIA GeForce GTX 1080 Ti ]
[ GPU #1: SM 6.1 NVIDIA GeForce GTX 1080 Ti ]
...
```

OpenCL (AMD):

```
$ /opt/ton-miner/pow-miner-opencl
...
[ OpenCL: platform #0 device #0 Intel(R) Core(TM) i9-9880H CPU @ 2.30GHz ]
[ OpenCL: platform #0 device #1 Intel(R) UHD Graphics 630 ]
[ OpenCL: platform #0 device #2 AMD Radeon Pro 5500M Compute Engine ]
...
```

Here GPU addressed by platform-id `0` and gpu-id `2`.
For CUDA the "platform" is always `0` (zero).


### Performance tuning

In general miner tried to load your GPU as much as possible. If this is not enough and temperature and/or GPU load is low then you need to make performance test and tune the "boost factor" option.

Run performance test - replace `<gpu-id>` and `<platform-id>` to address required GPU device.
This test takes time, so you can adjust run time limit for each using `-t seconds` option.

```shell
$ /opt/ton-miner/pow-miner-cuda -vv \
  -g<gpu-id> \
  -p<platform-id> \
  -B -F 16 -t 10 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 \
  229760179690128740373110445116482216837 \
  5391989333430127958933403017403926134727428884508114496220722049840 \
  10000000000000000000

...

*************************************************
***
***   best boost factor: 32
***   best speed:        8.9e+08 hps
***
*************************************************

...
```

To get more accurate results run it few times or increase `-t` value to heat up the GPU properly.
Don't worry if you'll get inconsistent results, just peak up most frequent and lower value.

### System service

Systemctl will automatically restart mining then:
- lost connection to lite-server
- mining interrupted
- job finished and ready for the next job from giver-contract

Use this template to create system service file.
Replace placeholders by your values and save file to `/etc/systemd/system/miner_gpu0.service`

```
[Unit]
Description=TON miner
After=network.target

[Service]
RestartSec=5
Restart=always
WorkingDirectory=/opt/ton-miner
ExecStart=/opt/ton-miner/tonlib-cuda-cli -v 2 -C global.conf.json -e 'pminer start <GIVER_ADDRESS> <MY_ADDRESS> <GPU_ID> <BOOST_FACTOR> <PLATFORM_ID>'

[Install]
WantedBy=multi-user.target
Alias=miner_gpu0.service
```

To start the service:

```shell
systemctl start miner_gpu0
```

Use `tail -f /var/log/syslog` to view the service activity.

Enhancements:
- you can create as much service files as much GPU you have. Copy with different names (`miner_gpu1.service`, `miner_gpu2.service`, ...) and change `Alias` inside the service file
- you can use dedicated logging using `-l` option (this stop writing to syslog and will use specified log files). Don't forget to setup logrotate for mining logs.























#### [Back](./../index.md)

