# GPU miner how-to

Invoke the pow-miner-cuda (pow-miner-opencl) utility as follows:

```
$ crypto/pow-miner-cuda -vv -g<gpu-id> -G<gpu-threads> -t<timeout-in-sec> <your-wallet-address> <seed> <complexity> <iterations> <pow-giver-address> <boc-filename>
```

Here:

- `gpu-id`: GPU device ID
- `gpu-threads`: 1..1792, the number of virtual CPU cores simultaneously hashed in a GPU kernel
- `timeout-in-sec`: max amount of seconds that the miner would run before admitting failure
- `your-wallet-address`: the address of your wallet (possibly not initialized yet), either in the masterchain or in the workchain (note that you need a masterchain wallet to control a validator)
- `seed` and `complexity` are the most recent values obtained by running get-method get-pow-params
- `pow-giver-address`: the address of the chosen proof-of-work giver smartcontract [https://ton.org/docs/#/howto/pow-givers?id=_1-proof-of-work-giver-smart-contracts](https://ton.org/docs/#/howto/pow-givers?id=_1-proof-of-work-giver-smart-contracts)
- `boc-filename` is the filename of the output file where the external message with the proof of work will be saved in the case of success.

For example, if you have one GPU device and your wallet address is `kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7`, you might run

```
$ crypto/pow-miner-cuda \
 -vv -g 0 -G 16 -t 43200 \
 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 \
 229760179690128740373110445116482216837 \
 53919893334301279589334030174039261347274288845081144962207220498432 \
 100000000000 \
 kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN \
 mined.boc
```

The program will run at least 100000000000 iterations in total (distributed to all 16 threads) and either terminate successfully (with zero exit code) and save the required proof of work into file `mined.boc`, or terminate with a non-zero exit code if no proof of work was found. 

### Example

```
$ crypto/pow-miner-cuda -vv -g 0 -G 16 -t 43200 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 229760179690128740373110445116482216837 53919893334301279589334030174039261347274288845081144962207220498432 100000000000 kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN mined.boc
GPU #0: SM 6.1 NVIDIA GeForce GTX 1080
[ expected required hashes for success: 2147483648 ]
[ GPU ID: 0, CPU thread: 0, GPU threads: 16, throughput: 33554432 ]
FOUND! GPU ID: 0, CPU thread: 0, VCPU: 4, nonce=289312917, expired=1631801029
4D696E650061434EC55690D2AACC203003DBE333046683B698EF945FF250723C0F73297A2A1A41E2F130C8A157E676C2D20E3B1421CCB511EA59A41D163F2A4A6384E398BC97949C5DACDA33755876665780BAE9BE8A4D638530C8A157E676C2D20E3B1421CCB511EA59A41D163F2A4A6384E398BC97949C5D
Saving 176 bytes of serialized external message into file `mined.boc`
[ hashes computed: 5754208852 ]
[ speed: 8.58523e+08 hps ]
```

## TONLIB CLI wrapper with embedded GPU miner

The process automatically receives tasks from the specified <giver_addess>. During operation the process checks the parameters of the giver every 5 seconds. 
If they change, the task is restarted. If a solution is found, it sends it to the selected <giver_addess> and <my_address> is rewarded.

Invoke the tonlib-cuda-cli (tonlib-opencl-cli) utility as follows:

```
$  tonlib/tonlib-cuda-cli -v 3 -C <lite-server-config> -e 'pminer start <giver_addess> <my_address> <gpu-id> [gpu-threads]'
```

Here:

- `lite-server-config`: last config from https://newton-blockchain.github.io/global.config.json
- `gpu-id`: GPU device ID
- `gpu-threads`: 1..1792, the number of virtual CPU cores simultaneously hashed in a GPU kernel
- `giver_addess`: the address of the selected giver
- `my_address`: the address of your wallet (possibly not initialized yet), either in the masterchain or in the workchain (note that you need a masterchain wallet to control a validator)

### Example

```shell
tonlib/tonlib-cuda-cli -v 3 -C global.config.json -e 'pminer start Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f EQDU86V5wyPrLd4nQ0RHPcCLPZq_y1O5wFWyTsMw63vjXTOv 0 32'
[ 3][t 1][2021-10-19 14:27:57.281760534][TonlibClient.cpp:2121][!Tonlib]	Use init block from USER config: (-1,8000000000000000,15412272):71F1A9213A7C743B50A7F19DD4B4A5F0B69ABE940DCDF506216CC9CFD1586895:1384276A92CA54021B0E54C77CBC097486F1DFB5B81B62D7C1A6EA1B0C591878
Tonlib is inited
GPU #0: SM 6.1 NVIDIA GeForce GTX 1080
Miner #1 created
synchronization: ???
synchronization: 0%
[ 3][t 2][2021-10-19 14:27:57.948185682][LastBlock.cpp:327][!LastBlock]	{"workchain":-1,"shard":-9223372036854775808,"seqno":15433803,"root_hash":"RCM2vts8dIAtZ3BMWbkbcxXcrLapGo7xEhtxvBYtNpw=","file_hash":"j++FfmxyHXy+tYVq07hQ3kR/irKmIEGpVu9Wu18PsX4="}
synchronization: DONE in 652.8ms
[ 2][t 1][2021-10-19 14:27:58.044737131][TonlibClient.cpp:682][!GetAccountState]	Unknown code hash: 2s9GOKuPdHJC16F3zmLcwbF87f6YV9l6LQuRrdLuUFo=
pminer: got new options
[ expected required hashes for success: 481636794870846 ]
pminer: start workers
[ GPU ID: 0, CPU thread: 0, GPU threads: 32, throughput: 33554432 ]
```

## GPU Mining: Optimal Number of GPU Threads

GPU Miner has the parameter `-G <gpu-threads>`, the number of logical GPU threads that is used for mining. Default value is 8. 

It works the same way as if you were running pow miner on G CPU threads.

The GPU miner starts from #G different random points and then goes through the values one by one in search for the solution.

The search speed is limited by the GPU performance and is divided in proportion to the all starting values.

Playing around with the number of logical GPU threads can significantly increase hashrate (of course it can also decrease it).

However, since the solutions to the problem are evenly distributed over the set of solutions, a large number of starting points increases the probability of finding a solution.

Ideally you should have so many GPU threads that each thread is faster than the CPU thread. And it must be a multiple of 8.

Let's calculate a CPU hashrate on a simple task:

```
$ crypto/pow-miner -vv -w 1 -t 10 \
  kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 \
  229760179690128740373110445116482216837 \
  539198933343012795893340301740392613472742888450811449622072204984 \
  100000000000
[ expected required hashes for success: 214748364800 ]
[ hashes computed: 32505856 ]
[ speed: 3.17562e+06 hps ]
```

And GPU hashrate:

```
$ crypto/pow-miner-opencl -vv -g 2 -G 1 -t 10 \
  kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 \
  229760179690128740373110445116482216837 \
  539198933343012795893340301740392613472742888450811449622072204984 \
  100000000000
[ expected required hashes for success: 214748364800 ]
[ OpenCL: set kernel source (17937 bytes) ]
[ OpenCL: platform #0 device #0 Intel(R) Core(TM) i9-9880H CPU @ 2.30GHz ]
[ OpenCL: platform #0 device #1 Intel(R) UHD Graphics 630 ]
[ OpenCL: platform #0 device #2 AMD Radeon Pro 5500M Compute Engine ]
[ OpenCL: create context for platform #0 device #2 AMD Radeon Pro 5500M Compute Engine, max work group size is 256 ]
[ GPU ID: 2, CPU thread: 0, GPU threads: 1, throughput: 4194304 ]
[ hashes computed: 3091202048 ]
[ speed: 3.08938e+08 hps ]
```

`3.08938e+08 / 3.17562e+06 = 97`

In this case the best value of `-G` will be 64.
