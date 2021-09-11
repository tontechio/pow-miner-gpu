# GPU miner how-to

Invoke the pow-miner-cuda utility as follows:

```
$ crypto/pow-miner-cuda -vv -w<num-threads> -g<gpu-id> -t<timeout-in-sec> <your-wallet-address> <seed> <complexity> <iterations> <pow-giver-address> <boc-filename>
```

Here:

- `num-threads`: the number of threads that you want to use for mining, each running thread emulates 8 CPU cores
- `gpu-id`: GPU device ID
- `timeout-in-sec`: max amount of seconds that the miner would run before admitting failure
- `your-wallet-address`: the address of your wallet (possibly not initialized yet), either in the masterchain or in the workchain (note that you need a masterchain wallet to control a validator)
- `seed` and `complexity` are the most recent values obtained by running get-method get-pow-params
- `pow-giver-address`: the address of the chosen proof-of-work giver smartcontract [https://ton.org/docs/#/howto/pow-givers?id=_1-proof-of-work-giver-smart-contracts](https://ton.org/docs/#/howto/pow-givers?id=_1-proof-of-work-giver-smart-contracts)
- `boc-filename` is the filename of the output file where the external message with the proof of work will be saved in the case of success.

For example, if you have one GPU device and your wallet address is `kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7`, you might run

```
$ crypto/pow-miner-cuda \
 -vv -g 0 -w 16 -t 43200 \
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
$ crypto/pow-miner-cuda -g 0 -w 16 -t 43200 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 229760179690128740373110445116482216837 53919893334301279589334030174039261347274288845081144962207220498432 100000000000 kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN mined.boc

GPU #0: SM 6.1 NVIDIA GeForce GTX 1080
[ GPU ID: [ GPU ID: 00, CPU thread: , CPU thread: 2, VCPUS: 08, VCPUS: , throughput: 8, throughput: 2097152 ]2097152 ]

[ GPU ID: 0, CPU thread: 7, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 5, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 4, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 3, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 13, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 12, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 11, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 9, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 8, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 14, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 15, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 10, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 6, VCPUS: 8, throughput: 2097152 ]
[ GPU ID: 0, CPU thread: 1, VCPUS: 8, throughput: 2097152 ]
FOUND! GPU ID: 0, CPU thread: 10, VCPU: 2, nonce=6855761, expired=1631361273
4D696E6500613C98F95690D2AACC203003DBE333046683B698EF945FF250723C0F73297A2A1A41E2F1987CDA201B0EF8E8C72B7FAB993A88548E85F391D88A2A654FACBDD61D9348A0ACDA33755876665780BAE9BE8A4D6385987CDA201B0EF8E8C72B7FAB993A88548E85F391D88A2A654FACBDD61D9348A0
Saving 176 bytes of serialized external message into file `mined.boc`
```