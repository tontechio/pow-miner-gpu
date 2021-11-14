<style type="text/css" rel="stylesheet">
body {
  font:14px/22px Helvetica, Arial, sans-serif;
}
</style>

# Установка в Linux

## Требования

### Личный кошелек

Скачайте и установите любой из кошельков с официального сайта [ton.org/wallets](https://ton.org/wallets).
Следуйте инструкциям по установке кошелька и получите свой личный адрес в сети ТОН.

**ВАЖНО: не используйте кошельки/адреса бирж и ботов для получения вознаграждения от майнинга**

### Драйверы для владельцев Nvidia

- скачайте и установите последний [CUDA toolkit](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)

### Драйверы для владельцев AMD (OpenCL)

- скачайте и установите OpenCL SDK

   ```shell
   apt-get install opencl-headers ocl-icd-libopencl1 ocl-icd-opencl-dev
   ```
   
   или напрямую из [KhronosGroup/OpenCL-SDK](https://github.com/KhronosGroup/OpenCL-SDK)

## Установка

1. [Скачайте](https://github.com/tontechio/pow-miner-gpu/releases/latest) последнюю версию майнера для вашей операционной системы и видеокарты
1. Для примера возьмем Ubuntu 20.04 и Nvidia `minertools-cuda-ubuntu-20.04-x86-64.tar.gz`
1. Создайте директорию для майнера: `sudo mkdir -p /opt/ton-miner; sudo chown $USER /opt/ton-miner`
1. Распакуйте архив с майнером (измените `PATH/TO` на путь к скаченному архиву) `tar xzf PATH/TO/minertools-cuda-ubuntu-20.04-x86-64.tar.gz /opt/ton-miner/`
1. Сохраните файл для подключения с сети TON `cd /opt/ton-miner && curl -L -O https://newton-blockchain.github.io/global.config.json`


## Настройка

### Контракты-гиверы (giver)

Майнинг в ТОНе - это получение вознаграждение за решение синтетической задачи, полученной от контракта-гивера. Это не относится к "майнингу блоков" или поддержке, участию в работе сети ТОН. Это способ распространить монеты toncoin среди пользователей для формирования базового капитала валидатора или для участия в валидации сети посредством готовящегося контракта-номинатора.

В сети работают 10 контрактов-гиверов с адресами ниже (подробнее [тут](https://ton.org/mining)):

```
1.  kf-FV4QTxLl-7Ct3E6MqOtMt-RGXMxi27g4I645lw6MTWraV
2.  kf8JfFUEJhhpRW80_jqD7zzQteH6EBHOzxiOhygRhBdt4z2N
3.  kf8kO6K6Qh6YM4ddjRYYlvVAK7IgyW8Zet-4ZvNrVsmQ4EOF
4.  kf9iWhwk9GwAXjtwKG-vN7rmXT3hLIT23RBY6KhVaynRrIK7
5.  kf_NSzfDJI1A3rOM0GQm7xsoUXHTgmdhN5-OrGD8uwL2JMvQ
6.  kf8gf1PQy4u2kURl-Gz4LbS29eaN4sVdrVQkPO-JL80VhOe6
7.  kf-P_TOdwcCh0AXHhBpICDMxStxHenWdLCDLNH5QcNpwMHJ8
8.  kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN
9.  kf91o4NNTryJ-Cw3sDGt9OTiafmETdVFUMvylQdFPoOxIsLm
10. kf8SYc83pm5JkGt0p3TQRkuiM58O9Cr3waUtR9OoFq716lN-
```

### Список доступных GPU

Выполните `/opt/ton-miner/pow-miner-cuda` (или `pow-miner-opencl` для AMD) чтобы получить список доступных майнеру GPU и их идентификаторов.

Cuda (Nvidia) gpus `#0` and `#1` stands for gpu-id `0` and gpu-id `1`:
Драйвер Cuda (Nvidia) маркирует ГПУ как `#0` and `#1`, что означает, что в параметрах GPU_ID значения `0` и `1` будут использовать соответствующую карту:

```
$ /opt/ton-miner/pow-miner-cuda
...
[ GPU #0: SM 6.1 NVIDIA GeForce GTX 1080 Ti ]
[ GPU #1: SM 6.1 NVIDIA GeForce GTX 1080 Ti ]
...
```

OpenCL (AMD):

```
$ /opt/ton-miner/pow-miner-opencl
...
[ OpenCL: platform #0 device #0 Intel(R) Core(TM) i9-9880H CPU @ 2.30GHz ]
[ OpenCL: platform #0 device #1 Intel(R) UHD Graphics 630 ]
[ OpenCL: platform #0 device #2 AMD Radeon Pro 5500M Compute Engine ]
...
```

В OpenCL для указания устройства используется параметр `platform` и `device`.
В примере выше для AMD-карты параметры будут platform-id `0` и gpu-id `2`.

*У пользователей Nvidia/CUDA значение platform не показывается и всегда равно `0` (ноль)*

### Настройка производительности

Майнер "из коробки" старается максимально нагрузить ГПУ. Если этого недостаточно, температура или загрузка GPU слишком низкая, тогда необходимо настроить производительность майнера на конкретной ГПУ-карте путем запуска теста производительности и получения оптимального значения параметра *boost factor*.

Запустите тест производительности - замените в примере `<gpu-id>` и `<platform-id>` на значения карты, которую нужно протестировать.
Этот тест занимает время. Длительность можно отрегулировать параметром `-t seconds` (количество секунд на каждый прогон теста).

```shell
$ /opt/ton-miner/pow-miner-cuda -vv \
  -g<gpu-id> \
  -p<platform-id> \
  -B -F 16 -t 10 kQBWkNKqzCAwA9vjMwRmg7aY75Rf8lByPA9zKXoqGkHi8SM7 \
  229760179690128740373110445116482216837 \
  5391989333430127958933403017403926134727428884508114496220722049840 \
  10000000000000000000

...

*************************************************
***
***   best boost factor: 32
***   best speed:        8.9e+08 hps
***
*************************************************

...
```

Для получения более точных результатов выполните тест несколько раз и/или увеличьте `-t`.
Если результаты отличаются от теста к тесту, но используйте наименьший *best boost factor*

### System service

Systemctl автоматически перезапускает майнер при:
- потере соединения с сетью ТОН
- непредвиденной остановке майнинга
- завершении вычислений и отправке решения гиверу

Используйте шаблон ниже для создания системного сервиса.
Замените соответствующие значения на собственные и сохраните файл как `/etc/systemd/system/miner_gpu0.service`

```
[Unit]
Description=TON miner
After=network.target

[Service]
RestartSec=5
Restart=always
WorkingDirectory=/opt/ton-miner
ExecStart=/opt/ton-miner/tonlib-cuda-cli -v 2 -C global.conf.json -e 'pminer start <GIVER_ADDRESS> <MY_ADDRESS> <GPU_ID> <BOOST_FACTOR> <PLATFORM_ID>'

[Install]
WantedBy=multi-user.target
Alias=miner_gpu0.service
```

Для запуска сервиса майнера:

```shell
systemctl start miner_gpu0
```

Активность майнера можно увидеть в системном журнале командой `tail -f /var/log/syslog`.

Варианты доработок:
- вы можете создать по сервису для каждого GPU в системе. Скопируйте уже созданный выше файл под другим именем (`miner_gpu1.service`, `miner_gpu2.service`, ...), отредактируйте необходимые значения (гивер, гпу, etc) и имя сервиса (`Alias`)
- помимо системного журнала майнер может вести свой собственный лог, который включается параметром `-l <path to log>` (это выключит вывод в системный журнал или консоль и включит запись в указанный файл). Не забудьте включить ротацию логов майнера (logrotate)























#### [Back](./../index_ru.md)

