# Big integer number library for Qt

A template header file to allow arbitrarily large integer number arithmetic when using Qt. I’ve tried to make it match how python tread numbers so division have the same negative infinity rounding.

The test folder does the unit test for it. The example folder shows examples on how to use it.

## Example

```C++
QBigNum512 a, b;
a = “4583469587236457823649856238794569328745345”
b = “3452345923486982734876239856727836928”
PRINT << a + b;
```

See the example folder for how to use. It’s pretty intuitive.

## Compiling

I used a Linux PC, x86, qmake6 and gcc. I also used 128 ints so your compiler needs to support that. If you are using Windows the pro files will need changing.

For tests cd to the tests folder and type `qmake6` then `make`. This will build and run the tests.

For the examples cd to the examples folder and type `qmake6` then `make`. This will build and run the examples.

## Comparing with GMP

In the unit test it also compare speed of division with GMP (another bignum library) and it’s about the same speed. GMP took 3.5 seconds for 10000 big number divisions while QbigNum took 3.7 seconds.

## Postscript

If you find anything wrong with it please let me know.

Jonti 31/12/2024 Buy buy 2024

