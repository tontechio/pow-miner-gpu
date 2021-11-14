<style type="text/css" rel="stylesheet">
body {
  font:14px/22px Helvetica, Arial, sans-serif;
}
</style>
## Usage

### tonlib-cuda-cli (tonlib-opencl-cli)

This utility:
- get actual job from giver-contract
- run mining job
- send job result to giver-contract

When job has been done quicker than others and delivered quicker than others then giver-contract will send 100 toncoin to your personal wallet address.

Each success job indicated by `FOUND!` message in the output (or log if enabled)

```
$  /opt/ton-miner/tonlib-cuda-cli \
  -v <log-level> \
  -C <lite-server-config> \
  -e 'pminer start <giver_address> <my_address> <gpu-id> [boost-factor] [platform-id]' \
  [-l <logfile>]
```

Options and parameters:

- `-v`: log/output verbosity; use `2` for less messages or `3` for more
- `-C`: lite-servers network config file name, previously downloaded `global.config.json` (it's recommended to re-download it once a week at least)
- `<gpu-id>`: GPU device ID
- `[platform-id]`: optional GPU platform ID (OpenCl only), `0` by default; needed for some systems with more than one OpenCL's "platform" present in the system
- `[boost-factor]`: optional, `32` by default, accept values 1..65536; the multiplier for throughput, affects the number of hashes processed per iteration on the GPU
- `<giver_address>`: the address of the desired giver to mine
- `<my_address>`: the address of your wallet (possibly not initialized yet)
- `-l`: log flie name; all output goes to log file if it's specified



#### [Back](./../index.md)

