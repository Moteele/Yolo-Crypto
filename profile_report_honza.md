# Profiling report
This document shows some performance numbers of core functions. 

* compiler used: g++ 9.2.0
* standard flags: -std=c++14 -pg

## String to hex
Result after 1000 iterations. -O2 flag is useful.

### standard flags
* total time: 1.69s

| % time | time/s | function                                                      |
|--------|--------|---------------------------------------------------------------|
| 23.67  |  0.40  | std::ios_base::setf(std::_Ios_Fmtflags, std::_Ios_Fmtflags)   |
| 20.71  |  0.35  | std::hex(std::ios_base&)                                      |
| 12.13  |  0.21  | std::operator|=(std::_Ios_Fmtflags&, std::_Ios_Fmtflags)      |
|  5.62  |  0.10  | std::operator&=(std::_Ios_Fmtflags&, std::_Ios_Fmtflags)      |

### with -O2 flag
* total time: 0.32s

| % time | time/s | function                                                                                            |
|--------|--------|-----------------------------------------------------------------------------------------------------|
| 100.00 |  0.32  | stringToHex(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) |


## Hex to string
Result after 100000 iterations. It is much faster than the reverse. Interestingly, the -O2 flag does harm performance. 

### standard flags
* total time: 0.35s

| % time | time/s | function                                                                                            |
|--------|--------|-----------------------------------------------------------------------------------------------------|
| 45.71  | 0.16   | hexToString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
| 14.29  | 0.05   | std::ios_base::setf(std::_Ios_Fmtflags, std::_Ios_Fmtflags)
| 11.43  | 0.04   | std::hex(std::ios_base&)
|  8.57  | 0.03   | std::operator|=(std::_Ios_Fmtflags&, std::_Ios_Fmtflags)
|  5.71  | 0.02   | std::operator&(std::_Ios_Fmtflags, std::_Ios_Fmtflags)
|  5.71  | 0.02   | std::operator|(std::_Ios_Fmtflags, std::_Ios_Fmtflags)

### with -O2 flag
* total time: 0.62s

| % time | time/s | function                                                                                            |
|--------|--------|-----------------------------------------------------------------------------------------------------|
| 90.32  | 0.56   | hexToString(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
|  4.84  | 0.03   | void std::vector<std::unique_ptr<Catch::IStreamingReporter, std::default_delete<Catch::IStreamingReporter> >, std::allocator<std::unique_ptr<Catch::IStreamingReporter, std::default_delete<Catch::IStreamingReporter> > > >::emplace_back<std::unique_ptr<Catch::IStreamingReporter, std::default_delete<Catch::IStreamingReporter> > >(std::unique_ptr<Catch::IStreamingReporter, std::default_delete<Catch::IStreamingReporter> >&&)
|  4.84  | 0.03   | void std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct<char const*>(char const*, char const*, std::forward_iterator_tag)




## ECDH profiling
Most of the time was spent on squaring and multiplying field elements on the curve. These functions are from the openssl library and are hand optimized in assembly. Because of this, flag -O2 doesn't help here. These times are after 10000 iterations of ecdh function. 

### standard flags
* total time: 0.82s

| % time | time/s | function         |
|--------|--------|------------------|
| 37.80  |  0.31  | x25519_fe64_sqr  |
| 19.51  |  0.16  | fe_mul           |
|  9.76  |  0.08  | fe_sq            |
|  9.15  |  0.08  | x25519_fe64_mul  |

### with -O2 flag
* total time: 0.80s

| % time | time/s | function          |
|--------|--------|-------------------|
| 36.25  |  0.29  | fe_mul            |
| 27.50  |  0.22  | x25519_fe64_sqr   |
|  8.75  |  0.07  | x25519_fe64_mul   |
|  7.50  |  0.06  | fe_sq             |
|  5.00  |  0.04  | x25519_fe64_add   |
|  5.00  |  0.04  | x25519_scalar_mult|

## XEdDSA
Most of the time is again spent on elliptic curve arithmetic functions from openssl. These times are after 10000 iterations

### XEdDSA sign, standard flags
* total time: 3.82s

| % time | time/s | function                               |
|--------|--------|----------------------------------------|
| 32.41  | 1.28   | fe_cmov(int*, int const*, unsigned int)|
| 27.85  | 1.10   | fe_mul(int*, int const*, int const*)   |
| 12.66  | 0.50   | fe_sq(int*, int const*)                |
|  5.57  | 0.22   | fe_mul                                 |

### XEdDSA sign, with -O2 flag
* total time: 1.26s

| % time | time/s | function                                           |
|--------|--------|----------------------------------------------------|
| 34.92  | 0.44   | fe_mul(int*, int const*, int const*)               |
| 18.25  | 0.23   | cmov(ge_precomp*, ge_precomp const*, unsigned char)|
| 16.67  | 0.21   | fe_mul                                             |
|  7.14  | 0.09   | fe_sq                                              |
|  6.35  | 0.08   | fe_sq(int*, int const*)                            |

### XEdDSA sign + verify, standard flags
* total time: 7.73s

| % time | time/s | function                                           |
|--------|--------|----------------------------------------------------|
| 34.93  |  2.70  | fe_mul(int*, int const*, int const*)               |
| 22.51  |  1.74  | fe_sq(int*, int const*)                            |
| 15.14  |  1.17  | fe_cmov(int*, int const*, unsigned int)            |
|  7.24  |  0.56  | fe_sub(int*, int const*, int const*)               |
|  6.86  |  0.53  | fe_add(int*, int const*, int const*)               |

### XEdDSA sign + verify, with -O2 flag
* total time: 2.41s

| % time | time/s | function                                           |
|--------|--------|----------------------------------------------------|
| 37.97  |  0.92  | fe_mul(int*, int const*, int const*)               |
| 23.86  |  0.58  | fe_sq(int*, int const*)                            |
|  8.71  |  0.21  | ge_p2_dbl(ge_p1p1*, ge_p2 const*)                  |
|  7.47  |  0.18  | cmov(ge_precomp*, ge_precomp const*, unsigned char)|
|  7.47  |  0.18  | fe_mul                                             |
