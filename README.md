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

Tested on:

| GPU | Hashrate |
|-----|:---------|
NVIDIA GTX1080 | 8.58523e+08
NVIDIA GTX1080 Ti | 9.21737e+08
NVIDIA GTX1660 Ti | 7.16364e+08
NVIDIA RTX2060 |
NVIDIA RTX2070 |
NVIDIA RTX3060 Ti | 1.138870e+09
NVIDIA RTX3080 | 2.018200e+09
NVIDIA RTX3090 | 2.385980e+09 
NVIDIA Tesla T4 |
