*v20211126-1, Dr. Elias, Dr. Andreas*

## Алгоритм работы майнера

Начнем с базы - как работает майнер с гивером.

Пример найденного решения:

```
/usr/bin/ton/crypto/pow-miner -vv -w 1 -t 43200 \
Ef80UXx731GHxVr0-LYf3DIViMerdo3uJLAG3ykQZFjXz2kW \
189652831762983393964625580816547478595 \
9916687918475570719027284474883172700405006565162163839650282600000 \
300647710720 kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN /tmp/mined.boc
[ expected required hashes for success: 11676488177 ]
0: GPU #0: SM 6.1 NVIDIA GeForce GTX 1080
0: GPU throughput: 33554432
0: hash[0:12958854353]: 0000 = 0000, 18191111 < 5e2a220a
0: data0[0:12958854353]: f24d69 6e65fc61 38bcea34 517c7bdf 5187c55a f4f8b61f dc321588 c7ab768d ee24b006 df291064 58d7cfba c342d743 629ae2ea 61a8122d 9711c73 f8605dac
0: data1[0:12958854353]: 481d08b8 cf788b19 1251818e adce88ad 28e847f8 57522cb cc7c43ba c342d743 629ae2ea 61a8122d 9711c73 f8605dac 481d08b8 cf788b19 12518180 0000
0: data2[0:12958854353]: 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 03d8
0: hash[0:12958854353]: 0000 18191111 9582f5a4 b5807a2c 3982dd02 3d568d3f 18d414a9 53c0a728
0: complexity[0:12958854353]: 0000 5e2a220a d09901dd 7a08fe66 b11bfe6c 4928567d 9ea4390b 4d9ff640
0: rdata[0:12958854353]: bac342d7 43629ae2 ea61a812 2d09711c73f8605d ac481d08 b8cf7888 14a9e4b0
0: FOUND! nonce=12958854353 vcpu=0 expired=1631108330
4D696E65FC6138BCEA34517C7BDF5187C55AF4F8B61FDC321588C7AB768DEE24B006DF29106458D7CFBAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8CF788B191251818EADCE88AD28E847F8057522CBCC7C43BAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8CF788B19125181
Saving 176 bytes of serialized external message into file /tmp/mined.boc
[ hashes computed: 12952010752 ]
[ speed: 7.20559e+08 hps ]
```

Разберем последовательно, как было получено решение. 

Переведем все параметры в hex формат:

```
op: Mine = 4d696e65
my address: Ef80UXx731GHxVr0-LYf3DIViMerdo3uJLAG2ikQZFjXzxCx 
            -1:34517c7bdf5187c55af4f8b61fdc321588c7ab768dee24b006da29106458d7cf
pow seed: 189652831762983393964625580816547478595 
          = 8EADCE88AD28E847F8057522CBCC7C43
pow complexity: 9916687918475570719027284474883172700405006565162163839650282600000 
                = 5E2A220AD09901DD7A08FE66B11BFE6C4928567D9EA4390B4D9FF640
miner address: kf-kkdY_B7p-77TLn2hUhM6QidWrrsl8FYWCIvBMpZKprBtN
expired at: 1631108330 = 6138bcea
rdata1: 32 случайных байта = bac342d743629ae2ea61a8122d09711c73f8605dac481d08b8cf788814a9e4b0
rdata2: равняется rdata1
```

expired at проверяется гивером [https://github.com/newton-blockchain/ton/blob/master/crypto/smartcont/pow-testgiver-code.fc#L16](https://github.com/newton-blockchain/ton/blob/master/crypto/smartcont/pow-testgiver-code.fc#L16) и не должно отличаться от времени валидатора более чем на 1024 секунд (~17 минут) при проверке решения.

Формируется массив длиной 123 байта, далее будем называть его "стартовым значением" или "стартовой точкой":

```
2 bytes              [0:1]    = 00f2
4 bytes, op          [2:5]    = 4d696e65
1 byte, flags        [6:6]    = fc
4 bytes, expired at  [7:10]   = 6138bcea
32 bytes, my address [11:42]  = 34517c7bdf5187c55af4f8b61fdc321588c7ab768dee24b006df29106458d7cf
32 bytes, rdata1     [43:74]  = bac342d743629ae2ea61a8122d09711c73f8605dac481d08b8cf788814a9e4b0
16 bytes, pow seed   [75:90]  = 8EADCE88AD28E847F8057522CBCC7C43
32 bytes, rdata1     [91:122] = bac342d743629ae2ea61a8122d09711c73f8605dac481d08b8cf788814a9e4b0
```

Все вместе:

`00f24d696e65fc6138bcea34517c7bdf5187c55af4f8b61fdc321588c7ab768dee24b006df29
106458d7cfbac342d743629ae2ea61a8122d09711c73f8605dac481d08b8cf788814a9e4b08E
ADCE88AD28E847F8057522CBCC7C43bac342d743629ae2ea61a8122d09711c73f8605dac481d
08b8cf788814a9e4b0`

Вычисляется sha256 полученного массива: `hash = 3c11a1a246dc81ef411efb800644f183ebb169fd5035aa0379fd7f5fdd615685`
Полученный *hash* сравниваем с *pow complexity*, если *hash < pow complexity*, то этот массив является решением. 
Если нет, то увеличиваем *rdata1* и *rdata2* на единицу и повторяем хэширование и сравнение до тех пор, пока не найдем решение или не произведем заданное количество итераций.

В нашем случае решение было найдено на 12958854353 итерации, найденный `hash = 00000000 18191111 9582f5a4 b5807a2c 3982dd02 3d568d3f 18d414a9 53c0a728 < complexity = 00000000 5e2a220a d09901dd 7a08fe66 b11bfe6c 4928567d 9ea4390b 4d9ff640`, ответом является строка (в ответе не используются первые два байта): `4D696E65FC6138BCEA34517C7BDF5187C55AF4F8B61FDC321588C7AB768DEE24B006DF291064
58D7CFBAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8CF788B191251818EADCE
88AD28E847F8057522CBCC7C43BAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8
CF788B19125181`

Подробно:

```
4 bytes, op          4D696E65
1 byte, flags        FC
4 bytes, expired at  6138BCEA
32 bytes, my address 34517C7BDF5187C55AF4F8B61FDC321588C7AB768DEE24B006DF29106458D7CF
32 bytes, rdata1     BAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8CF788B19125181
16 bytes, pow seed   8EADCE88AD28E847F8057522CBCC7C43
32 bytes, rdata1     BAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8CF788B19125181
```

Расчет sha256 на gpu производится в соответствии с алгоритмом https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf

Из особенностей надо отметить то, что для вычисления sha256 исходное значение длиной 123 байта надо дополнить 1 битом, нулями и в конце записать длину хэшируемого значения, что в итоге дает нам массив data размером 192 байта (64*3). Это три блока по 64 байта, поэтому нужно произвести хэширование три раза.

Для параллельного вычисления хэшей, мы формируем стартовое значение с пустыми *expired at*, *rdata1*, *rdata2*, генерируем N случайных стартовых *rdata1*. В последней версии GPU-майнера `N = 16`.

Записываем в constant memory gpu массив из стартовых значений *rdata1*, *complexity*, дополненное значение *data* 192 байта, текущую позицию *i* (*nonce*) и количество хэшей для расчета.

Размер constant memory в gpu позволяет нам итерировать до 1024-1360 стартовых значений одновременно, но на практике такое большое количество не может быть одновременно обработано ядрами ГПУ.

Каждый тред на gpu вычисляет хэш независимо, получает индекс треда, текущий nonce (итерацию, `nonce = 0 .. max_iterations`), берет из памяти *rdata1* (по смещению, соответствующему треду), прибавляет *nonce*, подставляет в *data* значения *expired at*, *rdata1_nonced*, *rdata2_nonced*, вычисляет хэш, сравнивает с *complexity*. Если решение найдено, то оно записывается в ответ. После обработки группы значений проверям, найден ли ответ в этой группе и либо формируем boc с решением, либо вычисляем следующую группу хэшей. Размер группы определяется переменной *throughput* (`boost_factor * 2^19`).
