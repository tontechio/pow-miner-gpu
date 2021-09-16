# GPU miner how-to

Invoke the pow-miner-cuda (pow-miner-opencl) utility as follows:

```
$ crypto/pow-miner-cuda -vv -g<gpu-id> -G<gpu-threads> -t<timeout-in-sec> <your-wallet-address> <seed> <complexity> <iterations> <pow-giver-address> <boc-filename>
```

Here:

- `gpu-id`: GPU device ID
- `gpu-threads`: the number of virtual CPU cores simultaneously hashed in a GPU kernel
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

## Example

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
