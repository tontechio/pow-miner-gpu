# GPU miner how-to

Invoke the pow-miner-cuda (pow-miner-opencl) utility as follows:

```
$ crypto/pow-miner-cuda -vv -g<gpu-id> -F<boost-factor> -t<timeout-in-sec> <your-wallet-address> <seed> <complexity> <iterations> <pow-giver-address> <boc-filename>
```

Here:

- `gpu-id`: GPU device ID
- `boost-factor`: 1..65536, the multiplier for throughput, affects the number of hashes processed per iteration on the GPU
- `timeout-in-sec`: max amount of seconds that the miner would run before admitting failure
- `your-wallet-address`: the address of your wallet (possibly not initialized yet), either in the masterchain or in the workchain (note that you need a masterchain wallet to control a validator)
- `seed` and `complexity` are the most recent values obtained by running get-method get-pow-params
- `pow-giver-address`: the address of the chosen proof-of-work giver smartcontract [https://ton.org/docs/#/howto/pow-givers?id=_1-proof-of-work-giver-smart-contracts](https://ton.org/docs/#/howto/pow-givers?id=_1-proof-of-work-giver-smart-contracts)
- `boc-filename` is the filename of the output file where the external message with the proof of work will be saved in the case of success.

For example, if you have one GPU device and your wallet address is `kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7`, you might run

```
$ crypto/pow-miner-cuda \
 -vv -g 0 -F 16 -t 43200 \
 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 \
 229760179690128740373110445116482216837 \
 53919893334301279589334030174039261347274288845081144962207220498432 \
 100000000000 \
 kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN \
 mined.boc
```

The program will run at least 100000000000 iterations in total and either terminate successfully (with zero exit code) and save the required proof of work into file `mined.boc`, or terminate with a non-zero exit code if no proof of work was found. 

### Example

```
$ crypto/pow-miner-cuda -vv -g 0 -F 16 -t 43200 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 229760179690128740373110445116482216837 53919893334301279589334030174039261347274288845081144962207220498432 100000000000 kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN mined.boc
GPU #0: SM 6.1 NVIDIA GeForce GTX 1080
[ 3][t 0][2021-11-03 08:17:45.924928685][pow-miner.cpp:341]	[ expected required hashes for success: 2147483648 ]
[ 3][t 0][2021-11-03 08:17:45.984839965][credits.cu:29]	[ GPU ID: 0, boost factor: 16, throughput: 16777216 ]
[ 3][t 0][2021-11-03 08:17:46.263656840][Miner.cpp:91]	[ passed: 0.339177s, hashes computed: 16777216 (0.781250%), speed: 49464521.532628 hps ]
[ 3][t 0][2021-11-03 08:17:47.442442507][Miner.cpp:96]	FOUND! GPU ID: 0, nonce=41835870, expired=1635928367
4D696E65006182492F5690D2AACC203003DBE333046683B698EF945FF250723C0F73297A2A1A41E2F1E84FE86B91016EDD41BCCDD0AED0D80950BB9582667CE3580325D2B388B66FD6ACDA33755876665780BAE9BE8A4D6385E84FE86B91016EDD41BCCDD0AED0D80950BB9582667CE3580325D2B388B66FD6
[ 3][t 0][2021-11-03 08:17:47.442606437][pow-miner.cpp:150]	Saving 176 bytes of serialized external message into file `mined.boc`
[ 3][t 0][2021-11-03 08:17:47.445684228][pow-miner.cpp:127]	[ passed: 1.521217s, hashes computed: 1030834254 (48.001961%), speed: 677637826.930832 hps ]
```

## TONLIB CLI wrapper with embedded GPU miner

The process automatically receives tasks from the specified <giver_addess>. During operation the process checks the parameters of the giver every 5 seconds. 
If they change, the task is restarted. If a solution is found, it sends it to the selected <giver_addess> and <my_address> is rewarded.

Invoke the tonlib-cuda-cli (tonlib-opencl-cli) utility as follows:

```
$  tonlib/tonlib-cuda-cli -v 3 -C <lite-server-config> -e 'pminer start <giver_addess> <my_address> <gpu-id> [boost-factor]'
```

Here:

- `lite-server-config`: last config from https://newton-blockchain.github.io/global.config.json
- `gpu-id`: GPU device ID
- `boost-factor`: 1..65536, the multiplier for throughput, affects the number of hashes processed per iteration on the GPU
- `giver_addess`: the address of the selected giver
- `my_address`: the address of your wallet (possibly not initialized yet), either in the masterchain or in the workchain (note that you need a masterchain wallet to control a validator)

### Example

```shell
tonlib/tonlib-cuda-cli -v 3 -C global.config.json -e 'pminer start Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f EQDU86V5wyPrLd4nQ0RHPcCLPZq_y1O5wFWyTsMw63vjXTOv 0 32'
[ 3][t 1][2021-11-03 08:19:02.658077236][TonlibClient.cpp:2126][!Tonlib]	Use init block from USER config: (-1,8000000000000000,10171687):8AB12DF708437E0698C03FBC033065633ACC64786B921495A773D4D6CE033B3E:95ACBF6D42945050C95D4F52EA0C7D180090165FAE2BECD7F12A87592F682D97
Tonlib is inited
GPU #0: SM 6.1 NVIDIA GeForce GTX 1080
[ 3][t 1][2021-11-03 08:19:02.979470929][tonlib-cli.cpp:867][!console]	Miner #1 created
synchronization: ???
synchronization: 0%
[ 3][t 1][2021-11-03 08:19:04.514834997][LastBlock.cpp:327][!LastBlock]	{"workchain":-1,"shard":-9223372036854775808,"seqno":10318834,"root_hash":"CXwGHSPE04WTPqLBjrHbEIy3I9AlfcWCrJggXic3Qh8=","file_hash":"il0OfYacxwqCdHPcqID3YnEiJDR/SIFGh9clOVIwDTE="}
[ 3][t 1][2021-11-03 08:19:45.802416800][LastBlock.cpp:327][!LastBlock]	{"workchain":-1,"shard":-9223372036854775808,"seqno":15798495,"root_hash":"1egKt0zv4YAoFrOBm6mayXxyCP/8DbF7ok3VNaZwg7M=","file_hash":"JHSUN5b/8zrUmA2fhY5q9pBRPk6jG9rsAlbemK9POK4="}
synchronization: DONE in 42.8s
[ 3][t 2][2021-11-03 08:19:45.900830827][tonlib-cli.cpp:787][!PowMiner]	pminer: got new options from Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f, seed=136536750502112786188876976884704719927, complexity=198610507763563516827057334560206191630488136735933071610139264
[ 3][t 2][2021-11-03 08:19:45.900888609][tonlib-cli.cpp:792][!PowMiner]	[ expected required hashes for success: 583010891725634 ]
[ 3][t 2][2021-11-03 08:19:45.900908806][tonlib-cli.cpp:671][!PowMiner]	pminer: start workers
[ 3][t 3][2021-11-03 08:19:45.975276164][credits.cu:29]	[ GPU ID: 0, boost factor: 32, throughput: 33554432 ]
[ 3][t 3][2021-11-03 08:19:46.025519602][Miner.cpp:91]	[ passed: 0.124603s, hashes computed: 33554432 (0.000006%), speed: 269290412.126255 hps ]
[ 3][t 3][2021-11-03 08:19:49.045791813][Miner.cpp:91]	[ passed: 3.144876s, hashes computed: 2348810240 (0.000403%), speed: 746868934.172745 hps ]
synchronization: 100%
synchronization: DONE in 211.6ms
[ 3][t 3][2021-11-03 08:19:52.058137085][Miner.cpp:91]	[ passed: 6.157221s, hashes computed: 4664066048 (0.000800%), speed: 757495266.521090 hps ]
[ 3][t 3][2021-11-03 08:19:55.071769856][Miner.cpp:91]	[ passed: 9.170854s, hashes computed: 6979321856 (0.001197%), speed: 761032912.824980 hps ]
```

## TONLIB CLI logging

To redirect the output to a file, add `&> pminer.log` to the command:

```
nohup tonlib/tonlib-cuda-cli -v 3 -C global.config.json -e 'pminer start Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 0 32' &> pminer.log
```

## TONLIB CLI automation

Actually, tonlib-*-cli miner does not know how to restart itself.
If the selected lightserver does not respond, the miner terminates with code 3.
This allows you to re-run it with another random lightserver from the config.

We suggest running tonlib-*-cli with an automatic restart in one of two ways:

### Systemd unit

Create a file `/etc/systemd/system/miner.service` with the following contents: 
```
[Unit]
Description=NewTON miner
After=network.target

[Service]
RestartSec=5
Restart=always
WorkingDirectory=/tonminer
ExecStart=/usr/bin/ton/tonlib/tonlib-cuda-cli -v 3 -C global.conf.json -e 'pminer start Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 0 32'

[Install]
WantedBy=multi-user.target
Alias=miner.service
```

Then start the service:

```shell
systemctl start miner
```

Use `tail -f /var/log/syslog` to view the service activity.

### Shell

Run the command in an infinite loop:

```shell
while true; do /usr/bin/ton/tonlib/tonlib-cuda-cli -v 3 -C global.conf.json -e 'pminer start Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 0 32'; done
````

## GPU Mining: Optimal Boost Factor

To determine the optimal boost factor, run a benchmark with following parameters. The short minings with different boost factors will be launched one by one. At the end, the best value of -F (boost factor) parameter will be displayed, at which the maximum hashrate was obtained for the specified period -t

```
$ crypto/pow-miner-opencl -vv -B -g 0 -t 10 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 229760179690128740373110445116482216837 5391989333430127958933403017403926134727428884508114496220722049840 10000000000

[ expected required hashes for success: 214748364800 ]
[ hashes computed: 32505856 ]
[ speed: 3.17562e+06 hps ]
```

And GPU hashrate:

```
$ crypto/pow-miner-cuda -vv -g 0 -B -F 16 -t 30 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 229760179690128740373110445116482216837 5391989333430127958933403017403926134727428884508114496220722049840 10000000000000000000
GPU #0: SM 6.1 NVIDIA GeForce GTX 1080
[ 3][t 0][2021-11-03 08:22:12.851690003][pow-miner.cpp:341]	[ expected required hashes for success: 21474836480 ]
[ 3][t 0][2021-11-03 08:22:12.851817849][credits.cu:29]	[ GPU ID: 0, boost factor: 1, throughput: 1048576 ]
[ 3][t 0][2021-11-03 08:22:12.932733662][Miner.cpp:91]	[ passed: 0.080981s, hashes computed: 1048576 (0.004883%), speed: 12948389.779773 hps ]
...
[ 3][t 0][2021-11-03 08:29:26.677193144][credits.cu:29]	[ GPU ID: 0, boost factor: 16384, throughput: 17179869184 ]
[ 3][t 0][2021-11-03 08:29:52.330313065][Miner.cpp:91]	[ passed: 25.653132s, hashes computed: 17179869184 (80.000000%), speed: 669698698.401948 hps ]
[ 3][t 0][2021-11-03 08:30:17.990905785][Miner.cpp:91]	[ passed: 51.313725s, hashes computed: 34359738368 (160.000000%), speed: 669601328.330153 hps ]
[ 3][t 0][2021-11-03 08:30:17.990967356][pow-miner.cpp:127]	[ passed: 51.313788s, hashes computed: 34359738368 (160.000000%), speed: 669600505.085364 hps ]
[ 3][t 0][2021-11-03 08:30:17.990976810][pow-miner.cpp:216]	[ *** best boost factor: 1, best speed: 701337339.395668 hps *** ]
```

In this case the best value of `-F` will be 1.
