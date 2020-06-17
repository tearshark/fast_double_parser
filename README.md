# fast_double_parser
[![Build Status](https://cloud.drone.io/api/badges/lemire/fast_double_parser/status.svg)](https://cloud.drone.io/lemire/fast_double_parser) [![Build status](https://ci.appveyor.com/api/projects/status/y7215jgem4ggswnj/branch/master?svg=true)](https://ci.appveyor.com/project/lemire/fast-double-parser/branch/master)


Fast function to parse strings containing decimal numbers into double-precision (binary64) floating-point values.  That is, given the string "1.0e10", it should return a 64-bit floating-point value equal to 10000000000. We do not sacrifice accuracy. The function will match exactly (down the smallest bit) the result of a standard function like strtod.

We support all major compilers: Visual Studio, GNU GCC, LLVM Clang. We require C++11.

## Why should I expect this function to be faster?

Parsing strings into binary numbers (IEEE 754) is surprisingly difficult. Parsing a single number can take hundreds of instructions and CPU cycles, if not thousands. It is relatively easy to parse numbers faster if you sacrifice accuracy (e.g., tolerate 1 ULP errors), but we are interested in "perfect" parsing.

Instead of trying to solve the general problem, we cover what we believe are the most common scenarios, providing really fast parsing. We fall back on the standard library for the difficult cases. We believe that, in this manner, we achieve the best performance on some of the most important cases. 

We have benchmarked our parser on a collection of strings from a sample geojson file (canada.json). Here are some of our results:


| parser                                | MB/s |
| ------------------------------------- | ---- |
| simd_double_parser | 1279.46 MB/s |
| fast_double_parser                    | 736.44 MB/s |
| abseil, from_chars                    | 494.11 MB/s |
| double_conversion                     | 285.80 MB/s |
| strtod                    | 170.99 MB/s |

(configuration: clang version 10.0.0-4ubuntu1, I7-8700K OC 4.4GHz)

We expect string numbers to follow [RFC 7159](https://tools.ietf.org/html/rfc7159). In particular,
the parser will reject overly large values that would not fit in binary64. It will not produce
NaN or infinite values.

## Requirements

You should be able to just drop  the header file into your project, it is a header-only library.

If you want to run our benchmarks, you should have

- Windows, Linux or macOS; presumably other systems can be supported as well
- A recent C++ compiler
- A recent cmake (cmake 3.11 or better) is necessary for the benchmarks 

## Usage (benchmarks)

```
cd build
cmake ..
cmake --build . --config Release  
ctest .
./benchmark
```
Under Windows, the last like should be `./Release/benchmark.exe`.


## Sample results


```
$ ./out/build/WSL-Clang-Release/benchmark
parsing random integers in the range [0,1)


=== trial 1 ===
fast_double_parser  769.77 MB/s
simd_double_parser  1077.16 MB/s
strtod         109.04 MB/s
abslfromch     267.04 MB/s
absl           242.39 MB/s
double-conv    339.36 MB/s


=== trial 2 ===
fast_double_parser  772.42 MB/s
simd_double_parser  1063.91 MB/s
strtod         109.65 MB/s
abslfromch     267.13 MB/s
absl           242.62 MB/s
double-conv    348.11 MB/s
```

```
$ ./out/build/WSL-Clang-Release/benchmark benchmarks/data/canada.txt
read 111126 lines


=== trial 1 ===
fast_double_parser  734.14 MB/s
simd_double_parser  1279.46 MB/s
strtod         170.04 MB/s
abslfromch     477.66 MB/s
absl           461.43 MB/s
double-conv    263.21 MB/s


=== trial 2 ===
fast_double_parser  736.44 MB/s
simd_double_parser  1211.09 MB/s
strtod         170.99 MB/s
abslfromch     494.11 MB/s
absl           470.44 MB/s
double-conv    285.80 MB/s
```

## API

The current API is simple enough:

```C++
#include "fast_double_parser.h" // the file is in the include directory

double x;
char * string = ...
size_t length = ...
bool isok = fast_double_parser::parse_number(string, &x, string + length);
```

You must check the value of the boolean (`isok`): if it is false, then the function refused to parse.



```c++
#include "simd_double_parser.h" // the file is in the include directory

simd_double_parser::number_value x;
simd_double_parser::parser_result isok;
char * string = ...
size_t length = ...
std::tie(x, isok) = simd_double_parser::parser(string, string + length);
if (isok != simd_double_parser::parser_result::Invalid)
	...
```

You must check the value of the parser_result(`isok`): if it is Invalid, then the function refused to parse.



## Credit

Contributions are invited.

This is joint work with Michael Eisel.
