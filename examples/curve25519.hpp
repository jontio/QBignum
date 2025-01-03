#pragma once

#include "montgomerycurve.hpp"

class Curve25519 : public MontgomeryCurve<320>
{
public:
    using BigNum = QBigNum<320>;
    using Curve = MontgomeryCurve<320>;
    using Point = Curve::Point;
    const Point G;
    const BigNum n;
    Curve25519()
        : Curve(
              BigNum("0x76d06"), // Coefficient 'a'
              BigNum("0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed") // Prime modulus
              ),
        G("0x09", "0x20ae19a1b8a086b4e01edd2c7748d14c923d4d7e6d7c61b229e9c5a27eced3d9"), // G
        n("0x1000000000000000000000000000000014def9dea2f79cd65812631a5cf5d3ed") // order
    {}

    Point operator*(const BigNum& other) const
    {
        return scalarMultiply(other, G);
    }
    Point operator*(const QString& other) const
    {
        return scalarMultiply(BigNum(other), G);
    }
    Point operator*(uint32_t other) const
    {
        return scalarMultiply(BigNum(other), G);
    }
    friend Point operator*(const QString &lhs, const Curve25519& rhs)
    {
        return rhs * lhs;
    }
    friend Point operator*(const BigNum &lhs, const Curve25519& rhs)
    {
        return rhs * lhs;
    }
    friend Point operator*(uint32_t lhs, const Curve25519& rhs)
    {
        return rhs * lhs;
    }

    QString generatePulicKey(const QString& private_key)
    {
        /* Keys are byte reversed order */
        BigNum pri_key = BigNum(private_key).reverseByteOrder(256 / 8);
        /* Clamp */
        pri_key.clearBit(0);
        pri_key.clearBit(1);
        pri_key.clearBit(2);
        pri_key.clearBit(255);
        pri_key.setBit(254);
        /* Calculate public key */
        auto point = scalarMultiply(pri_key, G);
        /* Reverse the byte order as hazmat likes */
        auto result = point.x.reverseByteOrder(256 / 8).toHexString();
        return result;
    }
};
