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