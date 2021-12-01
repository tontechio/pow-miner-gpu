<style type="text/css" rel="stylesheet">
body {
  font:14px/22px Helvetica, Arial, sans-serif;
}
</style>
*v20211126-1, Dr. Elias, Dr. Andreas*

## Miner operation algorithm

Let's start with the basics: miner and giver interaction.

Sample hash computation:

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

Let's analyze step-by-step how this result was obtained. 

First, let's convert all parameters to the hex format:

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
rdata1: 32 random bytes = bac342d743629ae2ea61a8122d09711c73f8605dac481d08b8cf788814a9e4b0
rdata2: equal to rdata1
```

the **expired at** value is verified by the giver (see [https://github.com/newton-blockchain/ton/blob/master/crypto/smartcont/pow-testgiver-code.fc#L16](https://github.com/newton-blockchain/ton/blob/master/crypto/smartcont/pow-testgiver-code.fc#L16)); the value must n't diverge from the validator time by more than 1024 seconds (~17 minutes) at the point when a suggested solution is being verified.

First, a 123-byte array is formed; let's call it the "initial value" or the "starting point":

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

or, concatenated:

`00f24d696e65fc6138bcea34517c7bdf5187c55af4f8b61fdc321588c7ab768dee24b006df29
106458d7cfbac342d743629ae2ea61a8122d09711c73f8605dac481d08b8cf788814a9e4b08E
ADCE88AD28E847F8057522CBCC7C43bac342d743629ae2ea61a8122d09711c73f8605dac481d
08b8cf788814a9e4b0`

Then the sha256 hash of the resulting array is computed: `hash = 3c11a1a246dc81ef411efb800644f183ebb169fd5035aa0379fd7f5fdd615685`
The obtained *hash* is comparable to the *pow complexity*, and if *hash < pow complexity*, this array is the solution. 
Otherwise,  *rdata1* and *rdata2* are incremented by 1 and hashing is repeated until the solution is found or the predefined number of iterations is carried out.

In our case the valid hash was found at iteration 12958854353; here it is: `hash = 00000000 18191111 9582f5a4 b5807a2c 3982dd02 3d568d3f 18d414a9 53c0a728 < complexity = 00000000 5e2a220a d09901dd 7a08fe66 b11bfe6c 4928567d 9ea4390b 4d9ff640`, and the following string is taken as the solution (first two bytes are omitted): `4D696E65FC6138BCEA34517C7BDF5187C55AF4F8B61FDC321588C7AB768DEE24B006DF291064
58D7CFBAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8CF788B191251818EADCE
88AD28E847F8057522CBCC7C43BAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8
CF788B19125181`

Breakdown:

```
4 bytes, op          4D696E65
1 byte, flags        FC
4 bytes, expired at  6138BCEA
32 bytes, my address 34517C7BDF5187C55AF4F8B61FDC321588C7AB768DEE24B006DF29106458D7CF
32 bytes, rdata1     BAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8CF788B19125181
16 bytes, pow seed   8EADCE88AD28E847F8057522CBCC7C43
32 bytes, rdata1     BAC342D743629AE2EA61A8122D09711C73F8605DAC481D08B8CF788B19125181
```

 sha256 hashing is performed at a GPU according the algorithm specified here  https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf

There are some particularities to point out. 

Thus, for an sha256 computation, the initial value of 123 bytes has to be extended by 1 bit and zeros, the length of the hashed value is recorded in the end. The result is a *data* array of 192 bytes (64*3). These are three 64-byte blocks, therefore hashing is performed thrice.

For parallel hash computation, a miner forms an initial value with empty *expired at*, *rdata1*,*rdata2*, then generate an N  number of random starting *rdata1*. In the latest GPU-miner version `N = 16`.

Then the array of initial  *rdata1*, *complexity* values, 192 bytes of the extended data array, the current *i* (nonce) position and the number of hashes for computation are recorded to constant memory gpu.

The constant memory size in GPU allows us to simultaneously iterate up to 1024-1360 initial values, but in practice GPU cores cannot simultaneously processes a number this big.

Each GPU thread independently computes a hash: gets a thread index, current nonce value (iteration, `nonce = 0 .. max_iterations`), takes *rdata1* from the memory (by shift, by a corresponding thread), adds *nonce*, puts *expired at*, *rdata1_nonced* and *rdata2_nonced* values to *data* , computes hash and compares it to *complexity*. If it is meet the condition *hash < complexity*, it is recorded as the solution. After a group of values is processed, a miner checks whether it has the valid solution and forms a .boc file with it, if yes, or computes the next group of hashes, if no. The group size is determined by  *throughput* variable (`boost_factor * 2^19`).




#### [Back](./../index_ru.md)
