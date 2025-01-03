#pragma once

/* Not constant time. Timing attacks could be used.
 * Also need a few more bits to prevent overflow (adding upto 3 signed bignums) */

#include "qbignum.hpp"

template <size_t Bits>
class MontgomeryCurve
{
public:
    using BigNum = QBigNum<Bits>;

    class Point
    {
    public:
        Point()
        {
        }
        Point(const QString &x, const QString &y)
        {
            this->x = x;
            this->y = y;
        }
        Point(const BigNum &x, const BigNum &y)
        {
            this->x = x;
            this->y = y;
        }
        BigNum x, y;
        operator QString() const
        {
            return "(" + x.toDecimalString() + ", " + y.toDecimalString() + ")";
        }
        bool operator==(const Point& other) const
        {
            if ((x != other.x) || (y != other.y))
            {
                return false;
            }
            return true; // Equal case
        }
        bool operator!=(const Point& other) const
        {
            return !(*this == other);
        }
    };

    MontgomeryCurve(const BigNum &a, const BigNum &p)
        : modulus(p), curveA(a)
    {
    }

    Point pointDouble(const Point &point) const
    {
        const BigNum BigNum2 = BigNum(2);
        const BigNum BigNum3 = BigNum(3);
        const BigNum BigNumMinus1 = BigNum(-1);
        Point result;
        const BigNum &y = point.y;
        const BigNum &x = point.x;
        BigNum &xResult = result.x;
        BigNum &yResult = result.y;
        if (y == 0)
        {
            return result;
        }

        //(x * x * 3 + curveA * x * 2 + BigNum(1)) % modulus;
        BigNum numerator = BigNum::mulMod(x, x * BigNum3, modulus);
        numerator       += BigNum::mulMod(curveA, x * BigNum2, modulus);
        numerator++;
        numerator       %= modulus;
        BigNum denominator = BigNum::powMod(y * BigNum2, BigNumMinus1, modulus);
        BigNum lamb = BigNum::mulMod(numerator, denominator, modulus);

        //BigNum xDouble = (lamb * lamb - x * 2 - curveA) % modulus;
        // BigNum yDouble = (-(y + lamb * (xDouble - x))) % modulus;

        xResult  = BigNum::mulMod(lamb, lamb,  modulus);
        xResult -= x * BigNum2 + curveA;
        xResult %= modulus;

        yResult  = BigNum::mulMod(lamb, (xResult - x),  modulus);
        yResult += y;
        yResult  = -yResult;
        yResult %= modulus;

        return result;
    }

    Point pointAdd(const Point &point1, const Point &point2) const
    {
        Point result;
        const BigNum &y1 = point1.y;
        const BigNum &x1 = point1.x;
        const BigNum &y2 = point2.y;
        const BigNum &x2 = point2.x;
        BigNum &xResult = result.x;
        BigNum &yResult = result.y;

        if (x1 == 0 && y1 == 0)
        {
            return point2;
        }
        if (x2 == 0 && y2 == 0)
        {
            return point1;
        }

        BigNum numerator = (y2 - y1) % modulus;
        BigNum denominator = (x2 - x1).inverseMod(modulus);
        BigNum lamb = BigNum::mulMod(numerator, denominator, modulus);

        //BigNum x3 = (lamb * lamb - x1 - x2 - curveA) % modulus;
        //BigNum y3 = (-(y1 + lamb * (x3 - x1))) % modulus;

        xResult  = BigNum::mulMod(lamb, lamb, modulus);
        xResult -= x1;
        xResult -= x2;
        xResult -= curveA;
        xResult %= modulus;

        yResult  = BigNum::mulMod(lamb, (x1 - xResult), modulus);
        yResult -= y1;
        yResult %= modulus;

        return result;
    }

    /* Not constant time. Timing attacks could be used */
    Point scalarMultiply(const BigNum &k, const Point &point) const
    {
        Point result;
        Point current = point;

        BigNum kCopy = k;
        while (kCopy != 0)
        {
            if (kCopy[0] & 1)
            {
                result = pointAdd(result, current);
            }

            current = pointDouble(current);

            kCopy >>= 1;
        }

        return result;
    }

    Point getPointGivenX(const BigNum &x)
    {
        Point point;
        auto y_squared = BigNum::powMod(x, BigNum(3), modulus);
        y_squared += BigNum::mulMod(curveA, BigNum::mulMod(x, x, modulus), modulus);
        y_squared += x;
        y_squared %= modulus;
        if (BigNum::legendre(y_squared, modulus) != 1)
        {
            // fail to find y
            return point;
        }
        point.x = x;
        point.y = BigNum::tonelli(y_squared, modulus);
        return point;
    }

    bool isOnCurve(const Point &point) const
    {
        const BigNum &y = point.y;
        const BigNum &x = point.x;
        auto left_side = BigNum::mulMod(y, y, modulus);
        auto right_side = BigNum::powMod(x, BigNum(3), modulus);
        right_side += BigNum::mulMod(curveA, BigNum::mulMod(x, x, modulus), modulus);
        right_side += x;
        right_side %= modulus;
        return (left_side == right_side);
    }

private:
    BigNum modulus;
    BigNum curveA;
};

