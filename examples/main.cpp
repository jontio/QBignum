#include "qbignum.hpp"

#define PRINT qDebug().noquote()

int main()
{
    QBigNum512 a, b, sum, diff;
    PRINT << "\nQBigNum Example Project\n";

    /* Add two decimal numbers */
    a = "12345678901234567890";
    b = "98765432109876543210";
    sum = a + b;
    PRINT << "sum:" << a << "+" << b << "==" << sum;

    /* Subtract two hexidecimal numbers and print as decimal */
    a = "0xabdbabadbdfbadbfad6adfb6adf6d87f687e7b78eb87be78cb7eb87ce6b87ec687687b8a77b78dbd7db878b87bc8bc8cb87cb";
    b = "0xabdb4bdbabcbbadcbadbcbacbbadcad7cba8d7cba87cbad8c7badc87a7c67a8d7c6ad65ca976c5a9cb6a59bdc7babd98c7badbc5ad97c6a";
    diff = a - b;
    PRINT << "subtract:" << a << "-" << b << "==" << diff;

    /* Multiplication */
    a = -42;
    b = -17;
    PRINT << "Multiplication:" << a << "*" << b << "==" << b * a;

    /* Division */
    a = "315414563456347657352375";
    b = 24524; // (if the number can be represented as a int64_t then you don't need the quotes)
    auto [q, r] = a / b;
    PRINT << "Division:" << a << "/" << b << "==" << q << "r" << r;

    /* Reuse q and r for divistion */
    std::tie(q, r) = b / 16;
    PRINT << "Division2:" << b << "/" << 16 << "==" << q << "r" << r;

    /* Exponentials, can be -ve exp too that finds multiplicitave inverse if it exists */
    QBigNum512 base, exp, mod;
    base = 3;
    exp = -7;
    mod = 13;
    QBigNum512 result = base.powMod(exp, mod);
    PRINT << "Exponential:" << base << "**" << exp << "%" << mod << "==" << result;

    /* Very big exponential */
    a = "0x3487256897436529873956827367839582364987234957826348756293874562938475";
    b = "0x2165154112a132a1a32a1320120b2a3156b46b4b894984bf84afb84984f484abf8f4b9a84b84d5d4b6db4a654";
    mod = 7566487;
    PRINT << "Exponential2:" << "really big number mod 23452345 ==" << a.powMod(b, mod);

    /* Mod function % */
    base = 3014054041;
    exp = -7210215437;
    mod = 13121;
    result = (base.powMod(exp, mod) * base.powMod(-exp, mod)) % mod;
    PRINT << "Mod:" << "((" << base << "**" << exp << "%" << mod << ") * (" <<base << "**" << -exp << "%" << mod <<")) %" << mod << "==" << result;

    /* Printng as a hex number */
    a = "123456789984763544324356475869465734653643687468235623456125123875128754182765312";
    PRINT << "Hex:" << a.toDecimalString() << "==" << a.toHexString();

    /* Decrement */
    a = 123;
    a--;
    PRINT << "Decrement:" << "123-- ==" << a;

    /* Increment */
    a = 456;
    a++;
    PRINT << "Increment:" << "456++ ==" << a;

    /* RShift */
    PRINT << "RShift:" << "123 << 1 ==" << (QBigNum512(123) << 1);

    /* LShift */
    PRINT << "LShift:" << "456 >> 1 ==" << (QBigNum512(456) >> 1);

    PRINT << "gcd:" << "gcd(1465041960, 423234344) ==" << QBigNum512::gcd(1465041960, 423234344);

    /* etc. */

    PRINT << "\n";

    return 0;
}
