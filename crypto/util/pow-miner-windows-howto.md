# GPU miner for Windows how-to

## TONLIB CLI wrapper with embedded GPU miner

The process automatically receives tasks from the specified `<giver_addess>`. During operation the process checks the parameters of the giver every 5 seconds.
If they change, the task is restarted. If a solution is found, it sends it to the selected `<giver_addess>` and `<my_address>` is rewarded.

If you want more flexibility or control you can also use the `pow-miner-cuda.exe` (`pow-miner-opencl.exe`) as described at [pow-miner-howto.md](pow-miner-howto.md).

### Step by step:

1. Read basic info about mining on the https://ton.org/mining page. Here you can find giver addresses and some stats.
2. Install GPU Drivers:
    - For CUDA-capable GPU (Nvidia): https://docs.nvidia.com/cuda/cuda-installation-guide-microsoft-windows/index.html
    - For OpenCL-capable (AMD): http://support.amd.com/en-us/download
3. Download Wallet app from https://ton.org/wallets and create a wallet for the rewards.
4. Download last Windows release from https://github.com/tontechio/pow-miner-gpu/releases
5. Download last global config from https://newton-blockchain.github.io/global.config.json
6. Invoke the `tonlib-cuda-cli.exe` (`tonlib-opencl-cli.exe`) utility as follows:

```
> tonlib-cuda-cli.exe -v 3 -C <global-config> -e "pminer start <giver_addess> <my_address> <gpu-id> [boost-factor]"
```

## TONLIB CLI logging

To redirect the output to a file, add `>> pminer.log` to the command:

```
> tonlib-cuda-cli.exe -v 3 -C global.config.json -e "pminer start Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 0 32" >> pminer.log
```

## TONLIB CLI automation

Actually, `tonlib-*-cli` miner does not know how to restart itself.
If the selected lightserver does not respond, the miner terminates with code 3.
This allows you to re-run it with another random lightserver from the config.

We suggest running `tonlib-*-cli` with an automatic restart in one of two ways:

### Batch Script

An infinite loop in Batch Script refers to the repetition of a command infinitely. The only way to stop an infinitely loop in Windows Batch Script is by either pressing *Ctrl + C* or by closing the program.

Create a batch file with given contents and run it:

```shell
:a
tonlib-cuda-cli.exe -v 3 -C global.conf.json -e "pminer start Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 0 32"
goto a
```

### As a service

TBD
