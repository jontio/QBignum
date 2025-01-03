#include "qbignum.hpp"

DEFINE_USING_NAMESPACE_QBIGNUM(512);
#define PRINT qDebug().noquote()

int main()
{
    BigNum a, b, sum, diff;
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

    /* Just q for divistion */
    PRINT << "Division3: floor(563456 / 5675) ==" << div("563456", "5675");

    /* Exponentials, can be -ve exp too that finds multiplicitave inverse if it exists */
    BigNum base, exp, mod;
    base = 3;
    exp = -7;
    mod = 13;
    BigNum result = powMod(base, exp, mod);
    PRINT << "Exponential:" << base << "**" << exp << "%" << mod << "==" << result;

    /* Very big exponential */
    a = "0x3487256897436529873956827367839582364987234957826348756293874562938475";
    b = "0x2165154112a132a1a32a1320120b2a3156b46b4b894984bf84afb84984f484abf8f4b9a84b84d5d4b6db4a654";
    mod = 7566487;
    PRINT << "Exponential2:" << "really big number mod 23452345 ==" << powMod(a, b, mod);

    /* Mod function % */
    base = 3014054041;
    exp = -7210215437;
    mod = 13121;
    result = (powMod(base, exp, mod) * powMod(base, -exp, mod)) % mod;
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
    PRINT << "RShift:" << "123 << 1 ==" << (BigNum(123) << 1);

    /* LShift */
    PRINT << "LShift:" << "456 >> 1 ==" << (BigNum(456) >> 1);

    /* gcd */
    PRINT << "gcd:" << "gcd(1465041960, 423234344) ==" << gcd(1465041960, 423234344);

    /* abs */
    PRINT << "abs:" << "abs(-876978698769876457862349857629305) ==" << abs("-876978698769876457862349857629305");

    /* legendre1 */
    a = "562756987249856793486";
    BigNum p("67586567573");
    QBigNum<1024> h;
    h=p;

    result = legendre(a, p);
    if (result == p - 1)
    {
        PRINT << "legendre1:" << "legendre(" << a << "," << p <<") ==" << result << " (quadratic residule doesn't exist)";
    }
    else if (result == 1)
    {
        PRINT << "legendre1:" << "legendre(" << a << "," << p <<") ==" << result << " (quadratic residule exists)";
    }
    else if (result != 0)
    {
        PRINT << "legendre1:" << "legendre(" << a << "," << p <<") ==" << result << " (67586567573 is not a prime)";
    }
    else
    {
        PRINT << "legendre1:" << "legendre(" << a << "," << p <<") ==" << result;
    }

    /* legendre2 */
    p = 67586567603;
    result = legendre(a, p);
    if (result == -1 % p)
    {
        PRINT << "legendre2:" << "legendre(" << a << "," << p <<") ==" << result << " (quadratic residule doesn't exist)";
    }
    else if (result == 1)
    {
        PRINT << "legendre2:" << "legendre(" << a << "," << p <<") ==" << result << " (quadratic residule exists)";
    }
    else if (result != 0)
    {
        PRINT << "legendre2:" << "legendre(" << a << "," << p <<") ==" << result << " (67586567573 is not a prime)";
    }
    else
    {
        PRINT << "legendre2:" << "legendre(" << a << "," << p <<") ==" << result;
    }

    /* legendre3 */
    a = 1000000009;
    result = legendre(a, p);
    if (result == -1 % p)
    {
        PRINT << "legendre3:" << "legendre(" << a << "," << p <<") ==" << result << " (quadratic residule doesn't exist)";
    }
    else if (result == 1)
    {
        PRINT << "legendre3:" << "legendre(" << a << "," << p <<") ==" << result << " (quadratic residule exists)";
    }
    else if (result != 0)
    {
        PRINT << "legendre3:" << "legendre(" << a << "," << p <<") ==" << result << " (67586567573 is not a prime)";
    }
    else
    {
        PRINT << "legendre2:" << "legendre(" << a << "," << p <<") ==" << result;
    }

    /* legendre4 */
    a = 2 * p;
    result = legendre(a, p);
    PRINT << "legendre4:" << "legendre(" << a << "," << p <<") ==" << result;

    /* tonelli1 */
    a = 1000000009;
    p = 67586567603;
    result = tonelli(a, p);
    PRINT << "tonelli1:" << "tonelli(" << a << "," << p <<") ==" << result << " i.e. (" << result << "*" << result << ") %" << p << "==" << (result * result) % p;

    /* tonelli2 */
    a = 3456;
    p = 1000000009;
    result = tonelli(a, p);
    PRINT << "tonelli2:" << "tonelli(" << a << "," << p <<") ==" << result << " i.e. (" << result << "*" << result << ") %" << p << "==" << (result * result) % p;

    /* Look for a prime */
    p = "5468726578264911111111111111158248756245456245624222222222222225625625634534534525624567240";
    bool p_is_prime = false;
    for(uint k = 0; !BigNum::millerRabin(p) && k < 10000; k++, p++)
    {
        //
    }
    if(BigNum::millerRabin(p))
    {
        p_is_prime = true;
        PRINT << "prime1:" << p << "is probably prime";
    }
    else
    {
        PRINT << "prime1: failed to find a prime";
    }

    /* Find square root (quadratic residual) given a prime mod number */
    if (p_is_prime)
    {
        a = "6666666666666666666666666666666666666666666666";
        result = tonelli(a, p);
        PRINT << "      : (" << result << "*" << result << ") %" << p << "==" << mulMod(result, result, p);
    }

    PRINT << "\n";

    return 0;
}
