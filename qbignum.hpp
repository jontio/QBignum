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

#define NUM_WORDS(bits) ((bits + 63) / 64)

template <size_t Bits>
class QBigNum
{
private:
    QList<uint64_t> data;   
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

    QBigNum& operator|=(uint64_t scalar)
    {
        data[0] |= scalar;
        return *this;
    }

    QBigNum abs() const
    {
        if (!isNegative())
        {
            return *this; // Already positive
        }

        // Convert two's complement to its absolute value
        return -*this;
    }

    QBigNum& operator*=(const QBigNum& other)
    {
        QList<uint64_t> resultData(2 * NUM_WORDS, 0); // 1024-bit intermediate result

        // Multiply each word of *this by each word of other
        for (size_t i = 0; i < NUM_WORDS; ++i)
        {
            __uint128_t carry = 0;
            for (size_t j = 0; j < NUM_WORDS; ++j)
            {
                __uint128_t product = (__uint128_t)this->data[i] * other.data[j] + resultData[i + j] + carry;
                resultData[i + j] = static_cast<uint64_t>(product);  // Store lower 64 bits
                carry = product >> 64;  // Carry for next higher bits
            }
            resultData[i + NUM_WORDS] = static_cast<uint64_t>(carry);  // Store the last carry
        }

        // Truncate to 512 bits and assign back to *this
        for (size_t i = 0; i < NUM_WORDS; ++i)
        {
            this->data[i] = resultData[i];  // Assign the lower 512 bits of result
        }

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
                result = (result * b) % mod;
            }
            e >>= 1;       // Divide exponent by 2
            b = (b * b) % mod; // Square the base and reduce modulo mod
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
            x0 = x1 - q * x0;
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

    uint64_t get_lsw() const
    {
        return data.front();
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

        temp = temp.abs();

        // Convert the absolute value to a decimal string
        while (temp > 0)
        {
            auto [quotient, remainder] = temp / 10;
            temp = quotient;
            result.prepend(QString::number(remainder.get_lsw()));
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

typedef QBigNum<512> QBigNum512;
