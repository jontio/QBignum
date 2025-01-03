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

## Unit test ouput on my n100

```
********* Start testing of TestQBigNum512 *********
Config: Using QtTest library 6.4.2, Qt 6.4.2 (x86_64-little_endian-lp64 shared (dynamic) release build; by GCC 13.2.0), ubuntu 24.04
PASS   : TestQBigNum512::initTestCase()
PASS   : TestQBigNum512::testDefaultConstructor()
PASS   : TestQBigNum512::testConstructorWithValue()
PASS   : TestQBigNum512::testConstructorWithHex()
PASS   : TestQBigNum512::testAssignment()
PASS   : TestQBigNum512::testToHex()
PASS   : TestQBigNum512::testIncrement()
PASS   : TestQBigNum512::testDecrement()
PASS   : TestQBigNum512::testCompareAbs()
PASS   : TestQBigNum512::testAddition()
PASS   : TestQBigNum512::testAdditionByScalar()
PASS   : TestQBigNum512::testSubtraction()
PASS   : TestQBigNum512::testSubtractionByScalar()
PASS   : TestQBigNum512::testMultiplicationByScalar()
PASS   : TestQBigNum512::testMultiplication()
PASS   : TestQBigNum512::testLeftShift()
PASS   : TestQBigNum512::testRightShift()
PASS   : TestQBigNum512::testShiftLeft()
PASS   : TestQBigNum512::testComparisonOperators()
PASS   : TestQBigNum512::testToDecimal()
PASS   : TestQBigNum512::testFromDecimal()
PASS   : TestQBigNum512::testDevision()
PASS   : TestQBigNum512::testModulo()
PASS   : TestQBigNum512::testPowMod()
PASS   : TestQBigNum512::testInverseMod()
PASS   : TestQBigNum512::testDivisionWithGMP()
QDEBUG : TestQBigNum512::testDivisionSpeedWithGMP() Loading time for 10000 iterations: 23 ms
QDEBUG : TestQBigNum512::testDivisionSpeedWithGMP() GMP division time for 10000 iterations: 2693 ms
QDEBUG : TestQBigNum512::testDivisionSpeedWithGMP() BigNum division time for 10000 iterations: 2975 ms
PASS   : TestQBigNum512::testDivisionSpeedWithGMP()
QDEBUG : TestQBigNum512::testGCD() gcd_slow 30000 iterations: 2495 ms
QDEBUG : TestQBigNum512::testGCD() gcd 30000 iterations: 1134 ms
PASS   : TestQBigNum512::testGCD()
QDEBUG : TestQBigNum512::testMillerRabin() found 100 random primes of length upto 511 bits in 6941 ms
PASS   : TestQBigNum512::testMillerRabin()
QDEBUG : TestQBigNum512::testTonelli() found 20 random quadratic residuals for 20 random primes in 3786 ms
PASS   : TestQBigNum512::testTonelli()
PASS   : TestQBigNum512::cleanupTestCase()
Totals: 31 passed, 0 failed, 0 skipped, 0 blacklisted, 24068ms
********* Finished testing of TestQBigNum512 *********
```

## How division works

Initially, I asked chat GPT. It’s solution was simply going bit by bit through the numerator (dividend). So for a 256-bit number, it would have to do 256 iterations before calculating the result. Chat GPT is pretty bad with C++ code, so I asked it to do it in Python.

```python
def long_division(numerator: int, denominator: int):
    """
    Perform long division on large integers represented as decimal numbers.

    Args:
        numerator (int): Decimal integer for the numerator.
        denominator (int): Decimal integer for the denominator.

    Returns:
        tuple: (quotient, remainder) as decimal integers.
    """
    if denominator == 0:
        raise ValueError("Division by zero is not allowed.")

    # Initialize quotient and remainder
    quotient = 0
    remainder = 0

    # Get the number of bits in the numerator
    num_bits = numerator.bit_length()

    # Iterate over each bit from MSB to LSB
    for i in range(num_bits - 1, -1, -1):  # Start from MSB (num_bits-1) to LSB (0)
        # Shift remainder left by 1 bit and bring down the i-th bit of numerator
        remainder = (remainder << 1) | ((numerator >> i) & 1)

        # If remainder >= denominator, subtract denominator from remainder and set the i-th bit of quotient
        if remainder >= denominator:
            remainder -= denominator
            quotient |= (1 << i)  # Set the i-th bit of quotient to 1

    # Return the quotient and remainder as decimal integers
    return quotient, remainder


# Example usage
numerator = 81  # Example numerator in decimal
denominator = 7  # Example denominator in decimal

quotient, remainder = long_division(numerator, denominator)

print("Quotient:", quotient)
print("Remainder:", remainder)
```

Considering the computer I am on is a 64 bit computer and has the ability to divide a 64-bit number in one instruction It seemed silly going through one bit at a time through 256 bit number…

```asm
; Dividend: RDX:RAX, Divisor: RCX
MOV RAX, dividend_low  ; Lower 32 bits of the 64-bit dividend
MOV RDX, dividend_high ; Upper 32 bits of the 64-bit dividend
MOV RCX, divisor       ; Divisor in RCX

DIV RCX  ; Result: RAX = quotient, RDX = remainder
```

So I decided to think about how to do it from scratch. I could barely remember how we used to do long division in primary school; so yes, that was a waste of time. I remember it well enough that I could still divide small numbers into big numbers but would struggle dividing big numbers into other big numbers. So I thought, let’s keep it simple, imagine I know up to my 99 times table, and thus can divide any 2 digit number into any other 2 digit number. 73 / 12 = 6 r 1 , no problem. But I couldn’t divide 73111 by 103. Also, on computers, multiplying or dividing by any power of 2 is just shifting bits to the left or the right, so that we can consider as easy. First, let’s multiply the denominator by a number such that the first digit of it almost is as big as it can be before overflowing and is a power of 2. 8 works so 73111 / 103 = 8 * 73111 / ( 103 * 8 ) = 584888 / 824. That’s just shifting the bits 3 to the left. This won’t change the quotient, but it will multiply the remainder by 8. We know  584888 / 824 >= 584888 / 900 >= 5848 / 9  and  5848 / 9 is something I remember how to do from primary school. 58 / 9 = 6 r 4, 5848 - 600 * 9 = 448, 44 / 9 = 4 r 8, 448 – 40 * 9 = 88, 88 / 9 = 9 r 7, so 5848 / 9 = 649 r 7. This means we can remove 649 * 824 from 584888 and not alter the remainder. ( 584888 -  649 * 824) / 824 = 50112 / 824. This is reduced the number by an order of magnitude. Let’s repeat the process, until we can’t do it anymore.  501 / 9 = 55 r 6, so ( 50112 – 55 * 824 ) / 824 = 4792 / 824, 47 / 9 = 5 r 2 so ( 4792 – 5 * 824 ) / 824 = 672 / 842. At this point we have to stop as the numerator is smaller than the denominator. We then have to scale the remainder 672 by 8 (3 shifts to the right) to obtain the remainder of 73111 / 103 which we’re looking for. 672 / 8 = 84. For the quotient we have to add up the number of multiples of 824 we have removed getting this remainder, so 649 + 55 + 5 = 709. That means the solution is 73111 / 103 = 709 r 84. That’s pretty cool as we don’t have to divide a number greater than 99 by any other number greater than 9. So to sum up…

```
73111 / 103 → 8 * ( 73111 / 103 ) =  584888 / 824 (This step is called normalization.)
q = 0, rr = 584888
584888 / 824 → 5848 / 9 = 649 r ? (use long division from school)
584888 -  649 * 824 = 50112
q = 649, rr = 50112
50112 / 824 → 501 / 9 = 55 r ?
50112   -  55 * 824   = 4792
q = 649 + 55 = 704, rr = 4792
4792 / 824 → 47 / 9 = 5 r ?
4792     -  5 * 824    = 672
q = 704 + 5 = 709, rr = 672
rr < 824 so stop
r = rr / 8 = 672 / 8 = 84
→  73111 / 103 = 709 r 84
```

For my computer implementation I didn't use numbers from 0 to 99, I used 128bit numbers, apart from that, this is basically how my algorithm works. The normalization step is very important to speed up the algorithm. Without it, the algorithm is no better than dividing long division bit by bit, as chat GPT gave me. I had a quick play around with this algorithm and the one chat GPT gave me but in C++, and I achieved around about 300% improvement in speed. I’m not sure what this algorithm is called.

## Postscript

If you find anything wrong with it please let me know.

Jonti 1/1/2025 hello 2025

