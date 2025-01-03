#include <QtCore>

#pragma once

#ifndef USEIMMINTRIN
#define USEIMMINTRIN (1)
#endif

#ifdef __x86_64__
#if (defined(__GNUC__) || defined(__clang__)) && USEIMMINTRIN
#define HAS_ADCINTRIN 1
#include <immintrin.h>  // GCC and Clang support
#endif
#endif

#define NUM_WORDS(bits) (((bits) + 63) / 64)

template <size_t Bits>
class QBigNum
{
private:
    QList<uint64_t> data;

    template <size_t ABits, size_t BBits>
    static void copy(const QBigNum<ABits>& from, QBigNum<BBits>& to)
    {
        size_t minWords = qMin(NUM_WORDS(ABits), NUM_WORDS(BBits));
        for (size_t i = 0; i < minWords; ++i)
        {
            to[i] = from[i];
        }
        if (from.isNegative())
        {
            for (size_t i = minWords; i < NUM_WORDS(BBits); ++i)
            {
                to[i] = ~0ULL;
            }
        }

        if (to.isNegative() != from.isNegative())
        {
            throw std::overflow_error("Copy overflow");
        }
    }

protected:
public:
    static constexpr int NUM_BITS = Bits;
    static constexpr int NUM_WORDS = NUM_WORDS(Bits);

    QBigNum()
    {
        data.fill(0, NUM_WORDS);
    }

    QBigNum(int64_t scalar)
    {
        data.fill(0, NUM_WORDS);
        data[0] = std::abs(scalar);

        if (scalar < 0)
        {
            *this = -*this;
        }
    }

    QBigNum(const QString& number)
    {
        if (number.trimmed().toUpper().startsWith("0X") ||
            number.trimmed().toUpper().startsWith("-0X"))
        {
            *this = fromHex(number);
        }
        else
        {
            *this = fromDecimal(number);
        }
    }

    /* Assignment */

    QBigNum& operator=(int64_t scalar)
    {
        *this = QBigNum(scalar);
        return *this;
    }

    QBigNum& operator=(const QString& number)
    {
        *this = QBigNum(number);
        return *this;
    }

    /* Shift operations */

    QBigNum& operator<<=(int bits)
    {
        if (bits == 0)
        {
            return *this;
        }

        // Handle shifting full words first
        int shift_words = bits / 64;
        int bit_shift = bits % 64;

        if (shift_words > 0)
        {
            for (int i = NUM_WORDS - 1; i >= shift_words; --i)
            {
                data[i] = data[i - shift_words];
            }
            for (int i = 0; i < shift_words; ++i)
            {
                data[i] = 0; // Clear low words
            }
        }

        if (bit_shift > 0)
        {
            for (int i = NUM_WORDS - 1; i > 0; --i)
            {
                data[i] = (data[i] << bit_shift) | (data[i - 1] >> (64 - bit_shift));
            }
            data[0] <<= bit_shift;
        }

        return *this;
    }

    QBigNum& operator>>=(int bits)
    {
        if (bits == 0)
        {
            return *this;
        }

        // Handle shifting full words first
        int shift_words = bits / 64;
        int bit_shift = bits % 64;

        if (shift_words > 0)
        {
            for (int i = 0; i < NUM_WORDS - shift_words; ++i)
            {
                data[i] = data[i + shift_words];
            }
            for (int i = NUM_WORDS - shift_words; i < NUM_WORDS; ++i)
            {
                data[i] = 0; // Clear high words
            }
        }

        if (bit_shift > 0)
        {
            for (int i = 0; i < NUM_WORDS - 1; ++i)
            {
                data[i] = (data[i] >> bit_shift) | (data[i + 1] << (64 - bit_shift));
            }
            data[NUM_WORDS - 1] >>= bit_shift;
        }

        return *this;
    }

    /* Scaler operations */
    QBigNum& operator*=(int64_t scalar)
    {
        if (scalar < 0)
        {
            scalar *= -1;
            *this = -*this;
        }
        __uint128_t carry = 0;  // Using __uint128_t to hold 128-bit intermediate results

        // Loop through each word in the QBigNum
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            // Multiply the current word by the scalar and add any carry from the previous iteration
            __uint128_t result = static_cast<__uint128_t>(data[i]) * scalar + carry;

            // Store the lower 64 bits of the result in the current word
            data[i] = static_cast<uint64_t>(result);

            // Calculate the carry for the next word (higher 64 bits)
            carry = result >> 64;  // The carry is the upper 64 bits of the 128-bit result
        }
        return *this;
    }

    QBigNum& operator+=(int64_t scalar)
    {
        if (scalar == 0)
        {
            return *this; // No change for zero
        }

        // Handle negative scalar
        if (scalar < 0)
        {
            // Convert scalar to positive for subtraction
            uint64_t absScalar = static_cast<uint64_t>(-scalar);
            return *this -= absScalar; // Use subtraction for negative numbers
        }

        // Handle positive scalar
#if defined(HAS_ADCINTRIN)
        uint64_t carry = _addcarry_u64(0, data[0], scalar, (unsigned long long*)&data[0]);
        for (int i = 1; i < NUM_WORDS; ++i)
        {
            carry = _addcarry_u64(carry, data[i], 0, (unsigned long long*)&data[i]);
        }
#else
        uint64_t carry = static_cast<uint64_t>(scalar);

        for (int i = 0; i < NUM_WORDS && carry != 0; ++i)
        {
            // Add carry to the current word and compute the new carry
            uint64_t sum = static_cast<uint64_t>(data[i]) + carry;
            data[i] = static_cast<uint64_t>(sum);
            carry = (sum < carry) ? 1 : 0; // Carry occurs if overflow happened
        }
#endif
        return *this;
    }

    QBigNum& operator-=(int64_t scalar)
    {
        if (scalar == 0)
        {
            return *this; // No operation needed for zero
        }

        if (scalar < 0)
        {
            // Subtracting a negative is equivalent to addition
            *this += static_cast<uint64_t>(-scalar);
            return *this;
        }

#if defined(HAS_ADCINTRIN)
        uint64_t borrow = _subborrow_u64(0, data[0], scalar, (unsigned long long*)&data[0]);
        for (int i = 1; i < NUM_WORDS; ++i)
        {
            borrow = _subborrow_u64(borrow, data[i], 0, (unsigned long long*)&data[i]);
        }
#else
        uint64_t borrow = static_cast<uint64_t>(scalar);
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            uint64_t current = static_cast<uint64_t>(data[i]);
            uint64_t diff = current - borrow;
            data[i] = static_cast<uint64_t>(diff);
            borrow = (current < borrow) ? 1 : 0;
        }
#endif

        return *this;
    }

    /* QBigNum operations */

    QPair<QBigNum, QBigNum> operator/(const QBigNum& divisor) const
    {
        QBigNum q;
        QBigNum r = *this;
        QBigNum d = divisor;

        bool nflag = false;
        if (isNegative())
        {
            nflag = !nflag;
            r = -r;
        }
        if (d.isNegative())
        {
            nflag = !nflag;
            d = -d;
        }

        /* Find first words that aren't zero or just use the zeroth word */
        int kr, kd;
        for (kd = NUM_WORDS - 1; kd > 0 && d.data[kd] == 0; --kd)
        {
            //
        }
        for (kr = NUM_WORDS - 1; kr > 0 && r.data[kr] == 0; --kr)
        {
            //
        }

        if (kd == 0 && d.data[kd] == 0)
        {
            throw std::overflow_error("Division by zero");
        }

        /* Calculate shift that divisor would like to maximize number of bits in chunk_divisor but dont want to make
         * it -ve */
        __uint128_t chunk_divisor = (__uint128_t)d.data[kd] + 1;
        int shift = (chunk_divisor < (__uint128_t)0xFFFFFFFFFFFFFFFFULL) ? __builtin_clzll(chunk_divisor) : 0;
        if (kd == NUM_WORDS - 1)
        {
            shift--;
        }

        /* If that shift is too big for the numerator then reduce the shift such that the numerator wont overflow.
         * the +1 is for the sign bit as we dont want to turn a +ve number into a -ve number */
        int r_length = (r.bitLength() + shift + 1);
        if (r_length > (int)Bits)
        {
            shift = shift - (r_length - (int)Bits);
        }

        /* Normalize */
        r <<= shift;
        d <<= shift;

        chunk_divisor = (__uint128_t)d.data[kd] + 1;

        /* kr might have gone up by one word during the shift */
        for (kr = qMin(kr + 1, NUM_WORDS - 1); kr > 0 && r.data[kr] == 0; --kr)
        {
            //
        }

        /* Long division */
        while(true)
        {
            __uint128_t chunk_dividend = 0;
            QBigNum subnumber;

            /* Find the first word of remainder */
            for (; kr > 0 && r.data[kr] == 0; --kr)
            {
                //
            }
            for (int k = kr; k >= kd; --k)
            {
                chunk_dividend |= r.data[k];
                if (chunk_dividend < chunk_divisor)
                {
                    chunk_dividend <<= 64;
                    continue;
                }

                uint64_t temp_result = chunk_dividend / chunk_divisor;
                subnumber <<= 64;
                subnumber |= temp_result;
                chunk_dividend -= temp_result * chunk_divisor;
                chunk_dividend <<= 64;
            }

            if (subnumber == 0)
            {
                if (r >= d)
                {
                    q++;
                    r -= d;
                }
                break;
            }

            r -= (subnumber * d);
            q += subnumber;
        }

        /* Denormalize */
        r >>= shift;

        if (nflag)
        {
            q = -q;
        }

        /* Sign of r is dictated by divisor */
        if (divisor.isNegative())
        {
            r = -r;
        }

        /* If signs don't match and there is a remainder */
        if (divisor.isNegative() != isNegative() && r != 0)
        {
            q--;
            r = divisor - r;
        }

        /* should be r = dividend - q * divisor */

        return QPair<QBigNum, QBigNum>(q, r);
    }

    QPair<QBigNum, QBigNum> operator/(int64_t scalar) const
    {
        QBigNum divisor(scalar);
        return (*this / divisor);
    }

    QBigNum div(const QBigNum& divisor) const
    {
        auto [quotient, remainder] = *this / divisor;
        return quotient;
    }

    QBigNum div(int64_t divisor) const
    {
        return this->div(QBigNum(divisor));
    }

    static QBigNum div(QBigNum dividend, QBigNum divisor)
    {
        auto [quotient, remainder] = dividend / divisor;
        return quotient;
    }

    static QBigNum div(const QString& dividend, const QString& divisor)
    {
        auto [quotient, remainder] = QBigNum(dividend) / QBigNum(divisor);
        return quotient;
    }

    QBigNum& operator|=(uint64_t scalar)
    {
        data[0] |= scalar;
        return *this;
    }

    QBigNum& operator&=(uint64_t scalar)
    {
        data[0] &= scalar;
        return *this;
    }

    static QBigNum abs(const QBigNum& num)
    {
        QBigNum result = num;
        if (result.isNegative())
        {
            return -result;
        }
        return result;
    }

    static QBigNum abs(const QString& num)
    {
        return QBigNum::abs(QBigNum(num));
    }

    static QBigNum legendre(const QBigNum& a, const QBigNum& p)
    {
        return QBigNum::powMod(a, (p - 1).div(2), p);
    }

    static QBigNum legendre(const QString& a, const QString& p)
    {
        return QBigNum::legendre(QBigNum(a), QBigNum(p));
    }

    static QBigNum legendre(int64_t a, int64_t p)
    {
        return QBigNum::legendre(QBigNum(a), QBigNum(p));
    }

    /* prime number test */
    static bool millerRabin(const QBigNum& n, int k = 44)
    {
        if (n <= 1)
        {
            return false;
        }
        if (n == 2 || n == 3)
        {
            return true; // n is prime
        }
        if (n % 2 == 0)
        {
            return false; // Even numbers > 2 are not prime
        }

        // Small prime divisors check
        std::vector<int> smallPrimes = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};
        for (int prime : smallPrimes)
        {
            QBigNum primeBN = QBigNum(prime);
            if (n % primeBN == 0 && n != primeBN)
            {
                return false;
            }
        }

        // Write n - 1 as d * 2^r
        QBigNum d = n - 1;
        int r = 0;
        while (d % 2 == 0)
        {
            d /= 2;
            r++;
        }

        // Perform k iterations of the test
        for (int i = 0; i < k; ++i)
        {
            // Generate random a in range [2, n - 2]
            QBigNum a = QBigNum::randomInRange(2, n - 2);

            // Compute x = a^d % n
            QBigNum x = QBigNum::powMod(a, d, n);

            if (x == 1 || x == n - 1)
            {
                continue;
            }

            bool isComposite = true;
            for (int j = 0; j < r - 1; ++j)
            {
                x = QBigNum::mulMod(x, x, n);
                if (x == n - 1)
                {
                    isComposite = false;
                    break;
                }
            }

            if (isComposite)
            {
                return false;
            }
        }

        return true; // n is probably prime
    }

    static bool millerRabin(const QString& n, int k = 44)
    {
        return QBigNum::millerRabin(QBigNum(n), k);
    }

    static bool millerRabin(int64_t n, int k = 44)
    {
        return QBigNum::millerRabin(QBigNum(n), k);
    }

    /* generalization of legendre but for compisitte numbers so no use for tonelli */
    static int jacobi(const QBigNum& a, const QBigNum& n)
    {
        QBigNum aCopy = a % n;
        QBigNum nCopy = n;

        // Handle base cases
        if (aCopy == 0)
        {
            return 0; // Jacobi symbol (0/n) = 0
        }
        if (aCopy == 1)
        {
            return 1; // Jacobi symbol (1/n) = 1
        }

        int result = 1;

        // When a is negative, make it positive and flip the sign of result
        if (aCopy < 0)
        {
            aCopy = -aCopy;
            if (nCopy % 4 == 3)
            {
                result = -result; // Flip sign if n mod 4 is 3
            }
        }

        // Apply the law of quadratic reciprocity
        while (aCopy != 0)
        {
            while (aCopy % 2 == 0)
            {
                aCopy = aCopy.div(2);
                if (nCopy % 8 == 3 || nCopy % 8 == 5)
                {
                    result = -result; // Flip sign when n mod 8 is 3 or 5
                }
            }

            // Swap a and n if n > a
            if (n > a)
            {
                QBigNum tmp = aCopy;
                aCopy = nCopy;
                nCopy = tmp;
            }

            if (aCopy % 4 == 3 && nCopy % 4 == 3)
            {
                result = -result; // Apply quadratic reciprocity
            }

            aCopy = aCopy % nCopy;
        }

        // If a is 1, return 1, otherwise return the result
        return (nCopy == 1) ? result : 0;
    }

    /* p needs to be prime for this algo wil find quadratic residuals */
    static QBigNum tonelli(const QBigNum& n, const QBigNum& p)
    {
        if (n == 1 || n == 0)
        {
            return n;
        }

        if (legendre(n, p) != 1)
        {
            throw std::invalid_argument("Not a square (mod p)");
        }

        static QBigNum p_last;
        static bool p_last_is_prime = false;

        if (p != p_last)
        {
            p_last = p;
            p_last_is_prime = QBigNum::millerRabin(p_last);
        }

        if (!p_last_is_prime)
        {
            throw std::invalid_argument("p isn't prime");
        }

        QBigNum q = p - 1;
        uint s = 0;

        // Factorize p - 1 as q * 2^s
        while ((q % 2) == 0)
        {
            q /= 2;
            s++;
        }

        if (s == 1)
        {
            return QBigNum::powMod(n, (p + 1).div(4), p);
        }

        // Find a non-residue z
        QBigNum z = 2;
        while (legendre(z, p) != p - 1)
        {
            z++;
        }

        QBigNum c = QBigNum::powMod(z, q, p);
        QBigNum r = QBigNum::powMod(n, (q + 1).div(2), p);
        QBigNum t = QBigNum::powMod(n, q, p);
        uint m = s;

        while ((t - 1) % p != 0)
        {
            QBigNum t2 = t;
            uint i = 0;

            for (i = 1; i < m; ++i)
            {
                t2 = QBigNum::mulMod(t2, t2, p);
                if ((t2 - 1) % p == 0)
                {
                    break;
                }
            }

            QBigNum b = QBigNum::powMod(c, QBigNum(1) << (m - i - 1), p);
            r = QBigNum::mulMod(r, b, p);
            c = QBigNum::mulMod(b, b, p);
            t = QBigNum::mulMod(t, c, p);
            m = i;
        }

        return r;
    }

    static QBigNum tonelli(const QString& n, const QString& p)
    {
        return QBigNum::tonelli(QBigNum(n), QBigNum(p));
    }

    static QBigNum tonelli(int64_t n, int64_t p)
    {
        return QBigNum::tonelli(QBigNum(n), QBigNum(p));
    }

    /* Doesn't check for overflow */
    QBigNum& operator*=(const QBigNum& other)
    {
        QBigNum result;
        // Multiply each word of *this by each word of other
        for (size_t i = 0; i < NUM_WORDS; ++i)
        {
            __uint128_t carry = 0;
            for (size_t j = 0; i + j < NUM_WORDS; ++j)
            {
                __uint128_t product = (__uint128_t)this->data[i] * other.data[j] + result.data[i + j] + carry;
                result.data[i + j] = static_cast<uint64_t>(product);  // Store lower 64 bits
                carry = product >> 64;  // Carry for next higher bits
            }
        }
        *this = result;
        return *this;  // Return the modified object
    }

    QBigNum& operator-=(const QBigNum& other)
    {
#if defined(HAS_ADCINTRIN)
        uint8_t borrow = 0;
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            borrow = _subborrow_u64(borrow, data[i], other.data[i], (unsigned long long*)&data[i]);
        }
#else
        __uint128_t borrow = 0;
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            uint64_t diff = data[i] - other.data[i] - borrow;
            borrow = (data[i] < other.data[i] + borrow) ? 1 : 0;
            data[i] = diff;
        }
#endif
        return *this;
    }

    QBigNum& operator+=(const QBigNum& other)
    {
#if defined(HAS_ADCINTRIN)
        uint8_t carry = 0;
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            carry = _addcarry_u64(carry, data[i], other.data[i], (unsigned long long*)&data[i]);
        }
#else
        __int128_t carry = 0;
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            __uint128_t sum = (__int128_t)data[i] + (__int128_t)other.data[i] + carry;
            data[i] = sum;
            carry = sum >> 64;
        }
#endif
        return *this;
    }

    QBigNum& operator++()
    {
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            if (++data[i] != 0)
            {
                break; // No carry needed
            }
        }
        return *this;
    }

    QBigNum& operator--()
    {
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            if (data[i]-- != 0)
            {
                break; // No borrow needed
            }
        }
        return *this;
    }

    static QBigNum mulMod(QBigNum a, QBigNum b, QBigNum m)
    {
        QBigNum<(2*Bits)> resultData; // 1024-bit intermediate result
        QBigNum<(2*Bits)> mbig;
        QBigNum res;

        bool negFlag = false;
        if (a.isNegative())
        {
            a = -a;
            negFlag = !negFlag;
        }
        if (b.isNegative())
        {
            b = -b;
            negFlag = !negFlag;
        }

        // Multiply each word of *this by each word of other
        for (size_t i = 0; i < NUM_WORDS; ++i)
        {
            __uint128_t carry = 0;
            for (size_t j = 0; j < NUM_WORDS; ++j)
            {
                __uint128_t product = (__uint128_t)a[i] * b[j] + resultData[i + j] + carry;
                resultData[i + j] = static_cast<uint64_t>(product);  // Store lower 64 bits
                carry = product >> 64;  // Carry for next higher bits
            }
            resultData[i + NUM_WORDS] = static_cast<uint64_t>(carry);  // Store the last carry
        }

        if (negFlag)
        {
            resultData = -resultData;
        }

        uint bitCount = qMax(resultData.bitLength(), m.bitLength());
        if (bitCount >= Bits)
        {
            /* Do the mod operation in the bigger space */
            copy(m, mbig);
            resultData %= mbig;
            copy(resultData, res);
        }
        else
        {
            copy(resultData, res);
            res %= m;
        }

        return res;
    }

    static QBigNum mulMod(const QString& a, const QString& b, const QString& m)
    {
        return QBigNum::mulMod(QBigNum(a), QBigNum(b), QBigNum(m));
    }

    static QBigNum mulMod(int64_t a, int64_t b, int64_t m)
    {
        return QBigNum::mulMod(QBigNum(a), QBigNum(b), QBigNum(m));
    }

    // Non-const version: allows modification of data
    uint64_t& operator[](size_t index)
    {
        if (index >= NUM_WORDS)
        {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    // Const version: read-only access
    const uint64_t& operator[](size_t index) const
    {
        if (index >= NUM_WORDS)
        {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    static QBigNum powMod(const QBigNum& base, const QBigNum& exp, const QBigNum& mod)
    {
        QBigNum result = base.powMod(exp, mod);
        return result;
    }

    static QBigNum powMod(const QString& base, const QString& exp, const QString& mod)
    {
        QBigNum result = QBigNum(base).powMod(QBigNum(exp), QBigNum(mod));
        return result;
    }

    static QBigNum powMod(int64_t base, int64_t exp, const int64_t mod)
    {
        QBigNum result = QBigNum(base).powMod(QBigNum(exp), QBigNum(mod));
        return result;
    }

    QBigNum powMod(const QBigNum& exp, const QBigNum& mod) const
    {
        if (mod == 0)
        {
            throw std::invalid_argument("Modulus cannot be zero.");
        }

        if (exp < 0)
        {
            QBigNum base_inv = inverseMod(mod);
            return powMod(base_inv, -exp, mod);
        }

        QBigNum result = 1;    // Initialize result to 1
        QBigNum b = *this % mod; // Reduce base modulo mod
        QBigNum e = exp;       // Copy exponent for manipulation

        while (e > 0)
        {
            // If exponent is odd, multiply base with result
            if (e.data[0] & 1)
            {
                result = mulMod(result, b, mod);
            }
            e >>= 1;       // Divide exponent by 2
            b = mulMod(b, b, mod);// Square the base and reduce modulo mod
        }

        return result;
    }

    QBigNum inverseMod(const QBigNum& mod) const
    {
        if (mod == 0)
        {
            throw std::invalid_argument("Modulus cannot be zero.");
        }

        if (mod < 0)
        {
            QBigNum result = inverseMod(-mod) + mod;
            return result;
        }

        QBigNum a = *this % mod; // Reduce `this` modulo `mod`
        QBigNum m = mod;
        QBigNum x0 = 0, x1 = 1; // Coefficients for the Extended Euclidean Algorithm
        QBigNum t;

        if (mod == 1)
        {
            return 0;
        }

        while (a > 1)
        {

            if (m == 0)
            {
                throw std::invalid_argument("Inverse does not exist.");
            }

            auto [q, r] = a / m;
            a = m;
            m = r;

            // Update `x0` and `x1`
            t = x0;
            x0 = x1 - q * x0; // TODO: can this overflow ?
            x1 = t;
        }

        // If `a` is not 1, no modular inverse exists
        if (a != 1)
        {
            throw std::invalid_argument("Inverse does not exist.");
        }

        // Make sure result is positive
        if (x1 < 0)
        {
            x1 += mod;
        }

        return x1;
    }

    static QBigNum gcd(QBigNum a, QBigNum b)
    {
        if (a == 0)
        {
            return b;
        }
        if (b == 0)
        {
            return a;
        }

        a = QBigNum::abs(a);
        b = QBigNum::abs(b);

        uint shift = 0;

        // Remove common factors of 2
        while (((a.data[0] | b.data[0]) & 1) == 0) // Both are even
        {
            a >>= 1;
            b >>= 1;
            shift++;
        }

        while ((a.data[0] & 1) == 0) // Remove factors of 2 from a
        {
            a >>= 1;
        }

        while (b != 0)
        {
            while ((b.data[0] & 1) == 0) // Remove factors of 2 from b
            {
                b >>= 1;
            }

            if (a > b)
            {
                QBigNum temp = a;
                a = b;
                b = temp;
            }

            b = b - a; // Reduce b
        }

        // Restore common factors of 2
        return a << shift;
    }

    static QBigNum gcd(const QString& a, const QString& b)
    {
        return gcd(QBigNum(a), QBigNum(b));
    }

    static QBigNum gcd(int64_t a, int64_t b)
    {
        return gcd(QBigNum(a), QBigNum(b));
    }

    static QBigNum gcd_slow(QBigNum a, QBigNum b)
    {
        if (a == 0)
        {
            return b;
        }
        if (b == 0)
        {
            return a;
        }
        a = QBigNum::abs(a);
        b = QBigNum::abs(b);
        while (b != 0 && a != 0)
        {
            if (b < a)
            {
                a %= b;
            }
            else
            {
                b %= a;
            }
        }
        if (a == 0)
        {
            return b;
        }
        return a;
    }

    static QBigNum gcd_slow(const QString& a, const QString& b)
    {
        return gcd_slow(QBigNum(a), QBigNum(b));
    }

    static QBigNum gcd_slow(int64_t a, int64_t b)
    {
        return gcd_slow(QBigNum(a), QBigNum(b));
    }

    /* Friends */

    friend QBigNum operator-(int64_t lhs, const QBigNum& rhs)
    {
        return QBigNum(lhs) - rhs;
    }

    friend QBigNum operator+(int64_t lhs, const QBigNum& rhs)
    {
        return QBigNum(lhs) + rhs;
    }

    friend QBigNum operator*(int64_t lhs, const QBigNum& rhs)
    {
        return QBigNum(lhs) * rhs;
    }

    friend QBigNum operator%(int64_t lhs, const QBigNum& rhs)
    {
        return QBigNum(lhs) % rhs;
    }

    /* Hany operations */

    QBigNum operator-() const
    {
        return this->twosComplement();
    }

    QBigNum operator<<(int bits) const
    {
        QBigNum result = *this;
        result <<= bits;
        return result;
    }

    QBigNum operator>>(int bits) const
    {
        QBigNum result = *this;
        result >>= bits;
        return result;
    }

    QBigNum operator+(int64_t scalar) const
    {
        QBigNum result = *this;
        result += scalar;
        return result;
    }

    QBigNum operator-(int64_t scalar) const
    {
        QBigNum result = *this;
        result -= scalar;
        return result;
    }

    QBigNum operator*(int64_t scalar) const
    {
        QBigNum result = *this;
        result *= scalar;
        return result;
    }

    QBigNum operator+(const QBigNum& other) const
    {
        QBigNum result = *this;
        result += other;
        return result;
    }

    QBigNum operator-(const QBigNum& other) const
    {
        QBigNum result = *this;
        result -= other;
        return result;
    }

    QBigNum& operator/=(const QBigNum& divisor)
    {
        auto [quotient, remainder] = *this / divisor;
        *this = quotient;
        return *this;
    }

    QBigNum operator++(int)
    {
        QBigNum temp = *this;
        ++(*this); // Use pre-increment
        return temp;
    }

    QBigNum operator--(int)
    {
        QBigNum temp = *this;
        --(*this); // Use pre-decrement
        return temp;
    }

    QBigNum& operator%=(const QBigNum& divisor)
    {
        auto [quotient, remainder] = *this / divisor;
        *this = remainder;
        return *this;
    }

    QBigNum operator%(const QBigNum& divisor) const
    {
        QBigNum remainder = *this;
        remainder %= divisor;
        return remainder;
    }

    QBigNum operator*(const QBigNum& other) const
    {
        QBigNum result = *this;
        result *= other;
        return result;
    }

    QBigNum& operator%=(int64_t scalar)
    {
        QBigNum divisor(scalar);
        *this %= divisor;
        return *this;
    }

    QBigNum& operator/=(int64_t scalar)
    {
        QBigNum divisor(scalar);
        auto [quotient, remainder] = *this / divisor;
        *this = quotient;
        return *this;
    }

    QBigNum operator%(int64_t scalar) const
    {
        QBigNum divisor(scalar);
        QBigNum remainder = *this;
        remainder %= divisor;
        return remainder;
    }

    /* Comparisons. */

    bool operator==(const QString &decimalStr) const
    {
        QBigNum other = decimalStr;
        return *this == other;
    }

    bool operator==(int64_t scalar) const
    {
        QBigNum other = QBigNum(scalar);
        return *this == other;
    }

    bool operator<(int64_t scalar) const
    {
        QBigNum other = QBigNum(scalar);
        return *this < other;
    }

    bool operator>(int64_t scalar) const
    {
        QBigNum other = QBigNum(scalar);
        return *this > other;
    }

    bool operator<=(int64_t scalar) const
    {
        QBigNum other = QBigNum(scalar);
        return *this <= other;
    }

    bool operator>=(int64_t scalar) const
    {
        QBigNum other = QBigNum(scalar);
        return *this >= other;
    }

    bool operator>=(const QBigNum& other) const
    {
        // Handle sign mismatch
        if (isNegative() != other.isNegative())
        {
            return !isNegative();
        }
        for (int i = NUM_WORDS - 1; i >= 0; --i)
        {
            if (data[i] > other.data[i]) return true;
            if (data[i] < other.data[i]) return false;
        }
        return true; // Equal case
    }

    bool operator<=(const QBigNum& other) const
    {
        // Handle sign mismatch
        if (isNegative() != other.isNegative())
        {
            return isNegative();
        }
        for (int i = NUM_WORDS - 1; i >= 0; --i)
        {
            if (data[i] > other.data[i]) return false;
            if (data[i] < other.data[i]) return true;
        }
        return true; // Equal case
    }

    bool operator<(const QBigNum& other) const
    {
        return !(*this>=other);
    }

    bool operator>(const QBigNum& other) const
    {
        return !(*this<=other);
    }

    bool operator==(const QBigNum& other) const
    {
        if (isNegative() != other.isNegative())
        {
            return false;
        }
        for (int i = NUM_WORDS - 1; i >= 0; --i)
        {
            if (data[i] > other.data[i]) return false;
            if (data[i] < other.data[i]) return false;
        }
        return true; // Equal case
    }

    bool operator!=(const QBigNum& other) const
    {
        return !(*this == other);
    }

    int compareAbs(const QBigNum& other) const
    {
        for (int i = NUM_WORDS - 1; i >= 0; i--)
        {
            uint64_t thisWord = isNegative() ? ~data[i] + (i == 0 ? 1 : 0) : data[i];
            uint64_t otherWord = other.isNegative() ? ~other.data[i] + (i == 0 ? 1 : 0) : other.data[i];

            if (thisWord != otherWord)
            {
                return (thisWord > otherWord) ? 1 : -1;
            }
        }
        return 0; // Absolute values are equal
    }

    /* Conversion */

    operator QString() const
    {
        return toDecimalString();
    }

    int64_t toInt64() const
    {
        return (int64_t)data[0];
    }

    template <size_t TargetBits>
    QBigNum<TargetBits> convertTo() const
    {
        QBigNum<TargetBits> result;
        size_t minWords = qMin(NUM_WORDS(Bits), NUM_WORDS(TargetBits));
        for (size_t i = 0; i < minWords; ++i)
        {
            result[i] = this->data[i];
        }
        return result;
    }

    /* Misc */

    // Static function to get the maximum value QBigNum can represent
    static QBigNum max()
    {
        QBigNum maxVal;

        maxVal.data.fill(UINT64_MAX);  // Fill all uint64_t words with 0xFFFFFFFFFFFFFFFF

        // Set the sign bit (most significant bit) to 0
        maxVal.data.back() &= ~(static_cast<uint64_t>(1) << 63);  // Clear the sign bit (MSB)

        return maxVal;
    }

    // Static function to get the minimum value QBigNum can represent
    static QBigNum min()
    {
        QBigNum minVal;

        // The most negative value in two's complement has the highest bit set to 1 and all others set to 0
        minVal.data.fill(NUM_WORDS, 0);
        minVal.data.back() = static_cast<uint64_t>(1) << 63; // Set the sign bit (MSB) to 1

        return minVal;
    }

    void debug(const QString &comment = QString()) const
    {
        if (comment.isEmpty())
        {
            qDebug() << toDecimalString();
        }
        else
        {
            qDebug() << comment << ":" << toDecimalString();
        }
    }

    void setNegative(bool isNegative)
    {
        if (isNegative)
        {
            // Set the most significant bit (MSB) of the highest word
            data[NUM_WORDS - 1] |= (1ULL << 63); // Assuming 64-bit words
        }
        else
        {
            // Clear the most significant bit (MSB) of the highest word
            data[NUM_WORDS - 1] &= ~(1ULL << 63); // Assuming 64-bit words
        }
    }

    QBigNum twosComplement() const
    {
        QBigNum num = *this;
        __uint128_t carry = 1; // Start with the carry for adding 1
        for (int i = 0; i < NUM_WORDS; ++i)
        {
            // Invert the bits of the current word and add the carry
            __uint128_t temp = ~num.data[i] + carry;

            // Store the lower 64 bits back into the current word
            num.data[i] = static_cast<uint64_t>(temp);

            // Compute the carry for the next word
            carry = temp >> 64; // Extract the carry from the higher bits
        }
        return num;
    }

    int bitLength() const
    {

        // Handle negative numbers by considering their two's complement representation
        if (isNegative())
        {
            // For negative numbers, calculate the bit length of the positive equivalent (ignoring the sign bit)
            QBigNum<Bits> positiveNum = *this;
            positiveNum = abs(positiveNum); // Get the absolute value
            return positiveNum.bitLength();
        }

        for (int i = NUM_WORDS - 1; i >= 0; --i)
        {
            if (data[i] != 0)
            {
                 return (64 * i) + (64 - __builtin_clzll(data[i])) ;
            }
        }
        return 0;
    }

    bool isNegative() const
    {
        return (data[NUM_WORDS - 1] >> 63);
    }

    QString toWordString() const
    {
        QString result;
        for (int i = NUM_WORDS - 1; i >= 0; --i)
        {
            result += QString("0x%1 ").arg(data[i], 16, 16, QChar('0'));
        }
        result = result.trimmed();
        return result;
    }

    QString toHexString() const
    {
        QString result;
        QBigNum temp = *this;
        if (temp.isNegative())
        {
            result += "-";
            temp = -temp;
        }
        result += "0x";
        bool isprinting = false;
        for (int i = NUM_WORDS - 1; i >= 0; --i)
        {
            if (!isprinting && temp.data[i] == 0)
            {
                continue;
            }
            if (isprinting)
            {
                result += QString("%1").arg(temp.data[i], 16, 16, QChar('0'));
            }
            else
            {
                result += QString("%1").arg(temp.data[i], 0, 16, QChar('0'));
            }
            isprinting = true;
        }
        if (!isprinting)
        {
            result += "00";
        }
        return result;
    }

    QString toDecimalString() const
    {
        QString result;
        QBigNum temp = *this; // Copy of the original number for manipulation

        // Handle zero as a special case
        if (temp == 0)
        {
            return "0";
        }

        temp = QBigNum::abs(temp);

        // Convert the absolute value to a decimal string
        while (temp > 0)
        {
            auto [quotient, remainder] = temp / 10;
            temp = quotient;
            result.prepend(QString::number(remainder[0]));
        }

        // Add the negative sign if necessary
        if (isNegative())
        {
            result.prepend('-');
        }

        return result;
    }

    static QBigNum fromDecimal(const QString& decimalStr)
    {
        QBigNum result;
        QString sanitizedDecimal = decimalStr.trimmed().toUpper().remove(' ');
        QString num = sanitizedDecimal.trimmed();

        if (num.isEmpty())
        {
            throw std::invalid_argument("Invalid decimal string provided to BigNum.");
        }

        // Check if the number is negative
        bool isNegative = false;
        if (num[0] == '-')
        {
            isNegative = true;
            num = num.mid(1); // Remove the '-' sign for further processing
        }

        if (num.isEmpty() || !std::all_of(num.begin(), num.end(), [](QChar c) { return c.isDigit(); }))
        {
            throw std::invalid_argument("Invalid decimal string provided to BigNum.");
        }

        QBigNum base;
        base.data[0] = 1; // Base starts as 1

        for (int i = num.length() - 1; i >= 0; --i)
        {
            int digit = num[i].digitValue();
            if (digit < 0)
            {
                throw std::invalid_argument("Invalid character in decimal string.");
            }

            // Multiply current base by the digit
            result += base * static_cast<uint64_t>(digit);

            if (result.isNegative())
            {
                throw std::overflow_error("Decimal number exceeds the maximum value for BigNum.");
            }

            // Update the base (multiply by 10)
            base *= 10;
        }

        if (isNegative)
        {
            result = -result;
        }

        return result;
    }

    static QBigNum fromHex(const QString& hex)
    {
        QBigNum result;
        QString sanitizedHex = hex.trimmed().toUpper().remove(' ');

        bool string_is_negative = false;
        if (sanitizedHex.startsWith("-"))
        {
            string_is_negative = true;
            sanitizedHex = sanitizedHex.mid(1);
        }

        // Remove "0x" if present
        if (sanitizedHex.startsWith("0X"))
        {
            sanitizedHex = sanitizedHex.mid(2);
        }

        // Pad the string to a multiple of 16 characters
        while (sanitizedHex.length() % 16 != 0)
        {
            sanitizedHex.prepend('0');
        }

        // Parse the hex string into 64-bit chunks
        int index = 0;
        for (int i = sanitizedHex.length(); i > 0 && index <= NUM_WORDS; i -= 16)
        {
            if (index == NUM_WORDS)
            {
                throw std::overflow_error("Hexidecimal number exceeds the maximum value for BigNum.");
            }

            QString chunk = sanitizedHex.mid(i - 16, 16);
            bool ok;
            result.data[index++] = chunk.toULongLong(&ok, 16);

            if (!ok)
            {
                throw std::invalid_argument("Invalid hex string provided to BigNum.");
            }
        }

        if (result.isNegative())
        {
            throw std::overflow_error("Hexidecimal number exceeds the maximum value for BigNum.");
        }

        if (string_is_negative)
        {
            result = -result;
        }

        return result;
    }

    static QBigNum randomInRange(const QBigNum& min, const QBigNum& max)
    {
        if (min > max)
        {
            throw std::invalid_argument("min must be <= max");
        }

        QBigNum range = max - min + 1; // Inclusive range

        QBigNum result = 0;
        QBigNum multiplier = 1;

        while (range > 0)
        {
            // Generate a random 64-bit integer using QRandomGenerator
            uint64_t randomPart = QRandomGenerator::global()->generate64();

            // Ensure randomPart fits within the current range
            QBigNum part = QBigNum(randomPart) % range;

            // Accumulate the random result
            result += part * multiplier;

            // Adjust the range for the next iteration
            range >>= 64;
            multiplier <<= 64;
        }

        return min + result % (max - min + 1);
    }

    // Randomize QBigNum with specified number of bits
    static QBigNum randomize(int numBits, bool negative)
    {
        if (numBits <= 0)
        {
            return QBigNum();
        }

        QBigNum result;
        int numWords = (numBits + 63) / 64; // Determine how many 64-bit words needed
        int remainingBits = numBits % 64;

        // Fill the QBigNum with random words using QRandomGenerator
        for (int i = 0; i < numWords; ++i)
        {
            result.data[i] = QRandomGenerator::global()->generate64();
        }

        // Mask unused bits in the highest word
        if (remainingBits > 0)
        {
            quint64 mask = (1ULL << remainingBits) - 1;
            result.data[numWords - 1] &= mask;
        }

        result.data[NUM_WORDS - 1] &= ~(1ULL<<63);
        if (negative)
        {
            result = -result;
        }

        return result;
    }

    static QString UInt128ToHexString(const __uint128_t& value)
    {
        // Split the 128-bit value into high and low 64-bit parts
        uint64_t high = static_cast<uint64_t>(value >> 64);
        uint64_t low = static_cast<uint64_t>(value);

        // Format the output as a hexadecimal string
        QString hexString = QString("0x%1%2")
                                .arg(high, 16, 16, QChar('0'))
                                .arg(low, 16, 16, QChar('0'));

        return hexString;
    }

    static QString UInt64ToHexString(uint64_t value)
    {
        // Format the value as a 64-bit hexadecimal string
        QString hexString = QString("0x%1").arg(value, 16, 16, QChar('0'));
        return hexString;
    }

};

#define DEFINE_NAMESPACE_QBIGNUM(BITS)                              \
namespace QBigNumUtils##BITS                                         \
{                                                                    \
        using BigNum = QBigNum<BITS>;                                    \
                                                                     \
        BigNum abs(const BigNum& num) { return BigNum::abs(num); } \
        BigNum abs(const QString& num) { return BigNum::abs(num); } \
                                                                     \
        BigNum legendre(const BigNum& a, const BigNum& p) { return BigNum::legendre(a, p); } \
        BigNum legendre(const QString& a, const QString& p) { return BigNum::legendre(a, p); } \
        BigNum legendre(int64_t a, int64_t p) { return BigNum::legendre(a, p); } \
                                                                     \
        BigNum mulMod(const BigNum& a, const BigNum& b, const BigNum& mod) { return BigNum::mulMod(a, b, mod); }           \
        BigNum mulMod(const QString& a, const QString& b, const QString& mod) { return BigNum::mulMod(a, b, mod); }           \
        BigNum mulMod(int64_t a, int64_t b, int64_t mod) { return BigNum::mulMod(a, b, mod); }           \
                                                                    \
        BigNum powMod(const BigNum& base, const BigNum& exp, const BigNum& mod) { return BigNum::powMod(base, exp, mod); }           \
        BigNum powMod(const QString& base, const QString& exp, const QString& mod) { return BigNum::powMod(base, exp, mod); }           \
        BigNum powMod(int64_t base, int64_t exp, int64_t mod) { return BigNum::powMod(base, exp, mod); }           \
                                                                    \
        BigNum gcd(const BigNum& a, const BigNum& b) { return BigNum::gcd(a, b); } \
        BigNum gcd(const QString& a, const QString& b) { return BigNum::gcd(a, b); } \
        BigNum gcd(int64_t a, int64_t b) { return BigNum::gcd(a, b); } \
                                                                    \
        BigNum div(BigNum dividend, BigNum divisor)  { return BigNum::div(dividend, divisor); } \
        BigNum div(const QString& dividend, const QString& divisor)  { return BigNum::div(dividend, divisor); } \
                                                                    \
        BigNum tonelli(const BigNum& n, const BigNum& p) { return BigNum::tonelli(n, p); }\
        BigNum tonelli(const QString& n, const QString& p) { return BigNum::tonelli(n, p); }\
        BigNum tonelli(int64_t n, int64_t p) { return BigNum::tonelli(n, p); }\
                                                                    \
        bool millerRabin(const BigNum& n, int k = 44) { return BigNum::millerRabin(n, k); } \
        bool millerRabin(const QString& n, int k = 44) { return BigNum::millerRabin(n, k); } \
        bool millerRabin(int64_t n, int k = 44) { return BigNum::millerRabin(n, k); } \
} \
typedef QBigNum<BITS> QBigNum##BITS

#define DEFINE_USING_NAMESPACE_QBIGNUM(BITS) \
    DEFINE_NAMESPACE_QBIGNUM(BITS); \
    using namespace QBigNumUtils##BITS

typedef QBigNum<256> QBigNum256;
typedef QBigNum<512> QBigNum512;
typedef QBigNum<1024> QBigNum1024;
