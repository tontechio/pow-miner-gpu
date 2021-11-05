# GPU miner how-to

Invoke the pow-miner-cuda (pow-miner-opencl) utility as follows:

```
$ crypto/pow-miner-cuda -vv -g<gpu-id> -p<platform-id> -F<boost-factor> -t<timeout-in-sec> <your-wallet-address> <seed> <complexity> <iterations> <pow-giver-address> <boc-filename>
```

Here:

- `gpu-id`: GPU device ID
- `platform-id`: GPU platform ID (OpenCl only)
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
$  tonlib/tonlib-cuda-cli -v 3 -C <lite-server-config> -e 'pminer start <giver_addess> <my_address> <gpu-id> [boost-factor] [platform-id]'
```

Here:

- `lite-server-config`: last config from https://newton-blockchain.github.io/global.config.json
- `gpu-id`: GPU device ID
- `platform-id`: GPU platform ID (OpenCl only)
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

To redirect the output to a file, add `-l pminer.log` to the command:

```
nohup tonlib/tonlib-cuda-cli -v 3 -C global.config.json -e 'pminer start Ef-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWg0f kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 0 32' -l pminer.log
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
[ 3][t 0][2021-11-03 08:22:39.937701735][Miner.cpp:91]	[ passed: 27.085950s, hashes computed: 19111346176 (88.994141%), speed: 705581532.922791 hps ]
[ 3][t 0][2021-11-03 08:22:42.852615576][pow-miner.cpp:127]	[ passed: 30.000864s, hashes computed: 21040726016 (97.978516%), speed: 701337339.395668 hps ]
[ 3][t 0][2021-11-03 08:22:42.852697840][credits.cu:29]	[ GPU ID: 0, boost factor: 2, throughput: 2097152 ]
[ 3][t 0][2021-11-03 08:22:42.855891363][Miner.cpp:91]	[ passed: 0.003218s, hashes computed: 2097152 (0.009766%), speed: 651674374.177311 hps ]
...
[ 3][t 0][2021-11-03 08:23:09.872089223][Miner.cpp:91]	[ passed: 27.019415s, hashes computed: 17968398336 (83.671875%), speed: 665018040.686022 hps ]
[ 3][t 0][2021-11-03 08:23:12.852908960][pow-miner.cpp:127]	[ passed: 30.000235s, hashes computed: 19950206976 (92.900391%), speed: 665001697.800553 hps ]
[ 3][t 0][2021-11-03 08:23:12.852991736][credits.cu:29]	[ GPU ID: 0, boost factor: 4, throughput: 4194304 ]
[ 3][t 0][2021-11-03 08:23:12.859321468][Miner.cpp:91]	[ passed: 0.006342s, hashes computed: 4194304 (0.019531%), speed: 661375728.990548 hps ]
...
[ 3][t 0][2021-11-03 08:23:39.889989796][Miner.cpp:91]	[ passed: 27.037009s, hashes computed: 18035507200 (83.984375%), speed: 667067384.529870 hps ]
[ 3][t 0][2021-11-03 08:23:42.857800247][pow-miner.cpp:127]	[ passed: 30.004820s, hashes computed: 20015218688 (93.203125%), speed: 667066782.382317 hps ]
[ 3][t 0][2021-11-03 08:23:42.857872070][credits.cu:29]	[ GPU ID: 0, boost factor: 8, throughput: 8388608 ]
[ 3][t 0][2021-11-03 08:23:42.870484476][Miner.cpp:91]	[ passed: 0.012624s, hashes computed: 8388608 (0.039062%), speed: 664508148.707123 hps ]
...
[ 3][t 0][2021-11-03 08:24:09.975848138][Miner.cpp:91]	[ passed: 27.117987s, hashes computed: 18127781888 (84.414062%), speed: 668478162.974692 hps ]
[ 3][t 0][2021-11-03 08:24:12.862072478][pow-miner.cpp:127]	[ passed: 30.004211s, hashes computed: 20057161728 (93.398438%), speed: 668478225.316951 hps ]
[ 3][t 0][2021-11-03 08:24:12.862142336][credits.cu:29]	[ GPU ID: 0, boost factor: 16, throughput: 16777216 ]
[ 3][t 0][2021-11-03 08:24:12.887259833][Miner.cpp:91]	[ passed: 0.025129s, hashes computed: 16777216 (0.078125%), speed: 667635973.906866 hps ]
...
[ 3][t 0][2021-11-03 08:24:39.973024207][Miner.cpp:91]	[ passed: 27.110893s, hashes computed: 18136170496 (84.453125%), speed: 668962487.602823 hps ]
[ 3][t 0][2021-11-03 08:24:42.882139337][pow-miner.cpp:127]	[ passed: 30.020008s, hashes computed: 20082327552 (93.515625%), speed: 668964767.355512 hps ]
[ 3][t 0][2021-11-03 08:24:42.882212701][credits.cu:29]	[ GPU ID: 0, boost factor: 32, throughput: 33554432 ]
[ 3][t 0][2021-11-03 08:24:42.932465944][Miner.cpp:91]	[ passed: 0.050265s, hashes computed: 33554432 (0.156250%), speed: 667555894.155316 hps ]
...
[ 3][t 0][2021-11-03 08:25:10.012296510][Miner.cpp:91]	[ passed: 27.130095s, hashes computed: 18152947712 (84.531250%), speed: 669107418.918688 hps ]
[ 3][t 0][2021-11-03 08:25:12.921294240][pow-miner.cpp:127]	[ passed: 30.039092s, hashes computed: 20099104768 (93.593750%), speed: 669098280.248961 hps ]
[ 3][t 0][2021-11-03 08:25:12.921374576][credits.cu:29]	[ GPU ID: 0, boost factor: 64, throughput: 67108864 ]
[ 3][t 0][2021-11-03 08:25:13.021700100][Miner.cpp:91]	[ passed: 0.100337s, hashes computed: 67108864 (0.312500%), speed: 668832394.111104 hps ]
...
[ 3][t 0][2021-11-03 08:25:40.093140249][Miner.cpp:91]	[ passed: 27.171777s, hashes computed: 18186502144 (84.687500%), speed: 669315891.633579 hps ]
[ 3][t 0][2021-11-03 08:25:43.001369297][pow-miner.cpp:127]	[ passed: 30.080006s, hashes computed: 20132659200 (93.750000%), speed: 669303687.827205 hps ]
[ 3][t 0][2021-11-03 08:25:43.001452042][credits.cu:29]	[ GPU ID: 0, boost factor: 128, throughput: 134217728 ]
[ 3][t 0][2021-11-03 08:25:43.202033234][Miner.cpp:91]	[ passed: 0.200604s, hashes computed: 134217728 (0.625000%), speed: 669066543.603036 hps ]
...
[ 3][t 0][2021-11-03 08:26:10.271042178][Miner.cpp:91]	[ passed: 27.269613s, hashes computed: 18253611008 (85.000000%), speed: 669375504.073207 hps ]
[ 3][t 0][2021-11-03 08:26:13.079003597][pow-miner.cpp:127]	[ passed: 30.077575s, hashes computed: 20132659200 (93.750000%), speed: 669357801.544288 hps ]
[ 3][t 0][2021-11-03 08:26:13.079084315][credits.cu:29]	[ GPU ID: 0, boost factor: 256, throughput: 268435456 ]
[ 3][t 0][2021-11-03 08:26:13.480117223][Miner.cpp:91]	[ passed: 0.401055s, hashes computed: 268435456 (1.250000%), speed: 669323213.013368 hps ]
...
[ 3][t 0][2021-11-03 08:26:42.345520169][Miner.cpp:91]	[ passed: 29.266458s, hashes computed: 19595788288 (91.250000%), speed: 669564741.192203 hps ]
[ 3][t 0][2021-11-03 08:26:43.147167495][pow-miner.cpp:127]	[ passed: 30.068106s, hashes computed: 20132659200 (93.750000%), speed: 669568593.458583 hps ]
[ 3][t 0][2021-11-03 08:26:43.147238375][credits.cu:29]	[ GPU ID: 0, boost factor: 512, throughput: 536870912 ]
[ 3][t 0][2021-11-03 08:26:43.948772400][Miner.cpp:91]	[ passed: 0.801544s, hashes computed: 536870912 (2.500000%), speed: 669795517.703030 hps ]
...
[ 3][t 0][2021-11-03 08:27:12.805253268][Miner.cpp:91]	[ passed: 29.658026s, hashes computed: 19864223744 (92.500000%), speed: 669775662.906090 hps ]
[ 3][t 0][2021-11-03 08:27:13.607240540][pow-miner.cpp:127]	[ passed: 30.460013s, hashes computed: 20401094656 (95.000000%), speed: 669766445.885378 hps ]
[ 3][t 0][2021-11-03 08:27:13.607309120][credits.cu:29]	[ GPU ID: 0, boost factor: 1024, throughput: 1073741824 ]
[ 3][t 0][2021-11-03 08:27:15.211307191][Miner.cpp:91]	[ passed: 1.604009s, hashes computed: 1073741824 (5.000000%), speed: 669411430.497654 hps ]
...
[ 3][t 0][2021-11-03 08:27:44.072308442][Miner.cpp:91]	[ passed: 30.465010s, hashes computed: 20401094656 (95.000000%), speed: 669656590.373546 hps ]
[ 3][t 0][2021-11-03 08:27:44.072378506][pow-miner.cpp:127]	[ passed: 30.465081s, hashes computed: 20401094656 (95.000000%), speed: 669655022.416330 hps ]
[ 3][t 0][2021-11-03 08:27:44.072400015][credits.cu:29]	[ GPU ID: 0, boost factor: 2048, throughput: 2147483648 ]
[ 3][t 0][2021-11-03 08:27:47.278819307][Miner.cpp:91]	[ passed: 3.206430s, hashes computed: 2147483648 (10.000000%), speed: 669742948.724877 hps ]
...
[ 3][t 0][2021-11-03 08:28:16.143197922][Miner.cpp:91]	[ passed: 32.070808s, hashes computed: 21474836480 (100.000000%), speed: 669606957.704288 hps ]
[ 3][t 0][2021-11-03 08:28:16.143272834][pow-miner.cpp:127]	[ passed: 32.070885s, hashes computed: 21474836480 (100.000000%), speed: 669605362.572068 hps ]
[ 3][t 0][2021-11-03 08:28:16.143293638][credits.cu:29]	[ GPU ID: 0, boost factor: 4096, throughput: 4294967296 ]
[ 3][t 0][2021-11-03 08:28:22.556410249][Miner.cpp:91]	[ passed: 6.413127s, hashes computed: 4294967296 (20.000000%), speed: 669714994.272392 hps ]
...
[ 3][t 0][2021-11-03 08:28:48.205944819][Miner.cpp:91]	[ passed: 32.062661s, hashes computed: 21474836480 (100.000000%), speed: 669777111.198473 hps ]
[ 3][t 0][2021-11-03 08:28:48.206020913][pow-miner.cpp:127]	[ passed: 32.062739s, hashes computed: 21474836480 (100.000000%), speed: 669775489.832911 hps ]
[ 3][t 0][2021-11-03 08:28:48.206056728][credits.cu:29]	[ GPU ID: 0, boost factor: 8192, throughput: 8589934592 ]
[ 3][t 0][2021-11-03 08:29:01.028938832][Miner.cpp:91]	[ passed: 12.822897s, hashes computed: 8589934592 (40.000000%), speed: 669890323.703375 hps ]
[ 3][t 0][2021-11-03 08:29:13.852496351][Miner.cpp:91]	[ passed: 25.646454s, hashes computed: 17179869184 (80.000000%), speed: 669873076.879754 hps ]
[ 3][t 0][2021-11-03 08:29:26.677096023][Miner.cpp:91]	[ passed: 38.471054s, hashes computed: 25769803776 (120.000000%), speed: 669849181.966987 hps ]
[ 3][t 0][2021-11-03 08:29:26.677169562][pow-miner.cpp:127]	[ passed: 38.471129s, hashes computed: 25769803776 (120.000000%), speed: 669847871.820834 hps ]
[ 3][t 0][2021-11-03 08:29:26.677193144][credits.cu:29]	[ GPU ID: 0, boost factor: 16384, throughput: 17179869184 ]
[ 3][t 0][2021-11-03 08:29:52.330313065][Miner.cpp:91]	[ passed: 25.653132s, hashes computed: 17179869184 (80.000000%), speed: 669698698.401948 hps ]
[ 3][t 0][2021-11-03 08:30:17.990905785][Miner.cpp:91]	[ passed: 51.313725s, hashes computed: 34359738368 (160.000000%), speed: 669601328.330153 hps ]
[ 3][t 0][2021-11-03 08:30:17.990967356][pow-miner.cpp:127]	[ passed: 51.313788s, hashes computed: 34359738368 (160.000000%), speed: 669600505.085364 hps ]
[ 1][t 0][2021-11-03 08:30:17.990976810][pow-miner.cpp:216]        
[ 1][t 0][2021-11-03 08:30:17.990976820][pow-miner.cpp:217]        *************************************************
[ 1][t 0][2021-11-03 08:30:17.990976813][pow-miner.cpp:218]        ***
[ 1][t 0][2021-11-03 08:30:17.990976840][pow-miner.cpp:219]        ***   best boost factor: 1
[ 1][t 0][2021-11-03 08:30:17.990976815][pow-miner.cpp:220]        ***   best speed:        701337339.395668 hps
[ 1][t 0][2021-11-03 08:30:17.990976816][pow-miner.cpp:221]        ***
[ 1][t 0][2021-11-03 08:30:17.990976820][pow-miner.cpp:222]        *************************************************
[ 1][t 0][2021-11-03 08:30:17.990976826][pow-miner.cpp:223]        
```

In this case the best value of `-F` will be 1.
