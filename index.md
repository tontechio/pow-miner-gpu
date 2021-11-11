## "Soft" Pull Request rules

* Thou shall not merge your own PRs, at least one person should review the PR and merge it (4-eyes rule)
* Thou shall make sure that workflows are cleanly completed for your PR before considering merge

## Workflows responsibility
If a CI workflow fails not because of your changes but workflow issues, try to fix it yourself or contact one of the persons listed below via Telegram messenger:

* **C/C++ CI (ccpp-linux.yml)**: TBD
* **C/C++ CI Win64 Compile (ccpp-win64.yml)**: TBD

## GPU POW miner

GPU pow-miner located at `crypto/util`, check [pow-miner.md](crypto/util/pow-miner.md) and [pow-miner-howto.md](crypto/util/pow-miner-howto.md) for details.

TONLIB CLI with embedded GPU-miner located at `tonlib/tonlib`, check [pow-miner-howto.md](crypto/util/pow-miner-howto.md#tonlib-cli-wrapper-with-embedded-gpu-miner) for details.

GPU miner executable files for Windows can be downloaded from https://github.com/tontechio/pow-miner-gpu/releases, check [pow-miner-windows-howto.md](crypto/util/pow-miner-windows-howto.md) for details.

HW Supported:
- Nvidia: nVidia GT640+ or newer, Quadro series with Kepler chip or newer (FX not supported)
- Radeon HD78xx series and newer, AMD GPU GCN 1.0+
- HD4000 or newer

Tested on:

| GPU | Hashrate |
|-----|:---------|
NVIDIA GTX1060 | 3.8e+08
NVIDIA GTX1070 | 5.2e+08
NVIDIA GTX1070 Ti | 7.1e+08
NVIDIA GTX1080 | 8.58523e+08
NVIDIA GTX1080 Ti | 9.21737e+08
NVIDIA GTX1660 Ti | 7.16364e+08
NVIDIA GTX1660 Super | 6.41743e+08
NVIDIA RTX2060 Super | 8.67233e+08
NVIDIA RTX2070 |
NVIDIA RTX2080 Super | 1.34181e+09
NVIDIA RTX3060 | 8.45722e+08
NVIDIA RTX3060 Ti | 1.238870e+09
NVIDIA RTX3070 | 1.45591e+09
NVIDIA RTX3070 Ti | 1.594258e+09
NVIDIA RTX3080 | 2.018200e+09
NVIDIA RTX3090 | 2.385980e+09 
NVIDIA Tesla T4 |
AMD Radeon 580 | 4.2e+08
AMD Radeon 6600 | 6.6e+08
