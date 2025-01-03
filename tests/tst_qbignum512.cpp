#include <QtTest>
#include <gmp.h>
#include <QElapsedTimer>
#include "../qbignum.hpp"

class TestQBigNum512 : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultConstructor();
    void testConstructorWithValue();
    void testConstructorWithHex();
    void testAssignment();
    void testToHex();
    void testIncrement();
    void testDecrement();
    void testCompareAbs();
    void testAddition();
    void testAdditionByScalar();
    void testSubtraction();
    void testSubtractionByScalar();
    void testMultiplicationByScalar();
    void testMultiplication();
    void testLeftShift();
    void testRightShift();
    void testShiftLeft();
    void testComparisonOperators();
    void testToDecimal();
    void testFromDecimal();
    void testDevision();
    void testModulo();
    void testPowMod();
    void testInverseMod();
    void testDivisionWithGMP();
    void testDivisionSpeedWithGMP();
    void testGCD();
    void testMillerRabin();
    void testTonelli();
};

void TestQBigNum512::testLeftShift()
{
    // Test left shift by 4 bits
    QBigNum512 num1(1);
    num1 <<= 4;
    QCOMPARE(num1, QBigNum512(16)); // 1 << 4 = 16

    // Test left shift by 65 bits
    num1 = QBigNum512(1);
    num1 <<= 65;
    QBigNum512 expectedLeftShift65 = QBigNum512(0);
    expectedLeftShift65 += QBigNum512(1) << 65; // Build expected value dynamically
    QCOMPARE(num1, expectedLeftShift65);

    // Test left shift to the highest bit
    num1 = QBigNum512(1);
    num1 <<= QBigNum512::NUM_BITS - 1;
    QBigNum512 expectedLeftMaxShift = QBigNum512(1) << (QBigNum512::NUM_BITS - 1); // Build expected value
    QCOMPARE(num1, expectedLeftMaxShift);

    // Test shifting zero value
    QBigNum512 num3(0);
    num3 <<= 128;
    QCOMPARE(num3, QBigNum512(0)); // Still 0
}

void TestQBigNum512::testRightShift()
{
    // Test right shift by 4 bits
    QBigNum512 num2(16);
    num2 >>= 4;
    QCOMPARE(num2, QBigNum512(1)); // 16 >> 4 = 1

    // Test right shift by 65 bits after left shifting
    num2 = QBigNum512(1);
    num2 <<= 65;
    num2 >>= 65;
    QCOMPARE(num2, QBigNum512(1)); // Should return to original value

    // Test right shift from highest bit
    num2 = QBigNum512(1);
    num2 <<= QBigNum512::NUM_BITS - 1;
    num2 >>= QBigNum512::NUM_BITS - 1;
    QCOMPARE(num2, QBigNum512(1)); // Should return to 1

    // Test shifting zero value
    QBigNum512 num3(0);
    num3 >>= 128;
    QCOMPARE(num3, QBigNum512(0)); // Still 0
}

void TestQBigNum512::testDefaultConstructor()
{
    QBigNum512 num;
    QVERIFY(num == QBigNum512(0));
}

void TestQBigNum512::testConstructorWithValue()
{
    QBigNum512 num(42);
    QVERIFY(num == QBigNum512::fromHex("0x2A")); // Hex for 42
}

void TestQBigNum512::testConstructorWithHex()
{
    QBigNum512 num = QBigNum512::fromHex("0x123456789ABCDEF");
    QVERIFY(num == QBigNum512::fromHex("0x123456789ABCDEF"));
}

void TestQBigNum512::testAssignment()
{
    QBigNum512 num;
    num = 218347618;
    QVERIFY(num == QBigNum512(218347618));

    num = "5434567897765443456789876756432456789897675453345678976545342345678";
}

void TestQBigNum512::testToHex()
{
    QBigNum512 num = QBigNum512::fromHex("0x1234564756474536543645863475636566745575478635648653685856789ABCDEF");
    QVERIFY(num.toHexString() == QString("0x1234564756474536543645863475636566745575478635648653685856789ABCDEF").toLower());
}

void TestQBigNum512::testToDecimal()
{

    QBigNum512 num2 = QBigNum512::fromHex("0x48572348752970c143c5000bc716a65dd7efc1ecb9c55bc22395");
    QVERIFY(num2.toDecimalString() == "116246512175194222185115013272264321190673739172561462956794773");
    num2 = "-0x48572348752970c143c5000bc716a65dd7efc1ecb9c55bc22395";
    QVERIFY(num2.toDecimalString() == "-116246512175194222185115013272264321190673739172561462956794773");

    QVERIFY_THROWS_EXCEPTION(std::overflow_error, QBigNum512("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed4ced3decc5434ae3ea4b3e9b943e643640b0de"));

}

void TestQBigNum512::testFromDecimal()
{
    QBigNum512 num1 = QBigNum512::fromDecimal("1162465121751942221851150"
                                            "1327226432119067373917256"
                                            "1462956794773");
    QVERIFY(num1.toDecimalString() == "1162465121751942221851150132722"
                                      "64321190673739172561462956794773");

    num1 = "56484513215498465132132154326134325613452574634546755316732"
           "156237";
    QCOMPARE(num1.toDecimalString(), "564845132154984651321321543261343"
                                     "25613452574634546755316732156237");

    num1 = "-2342564845132154984651321321543261343256134525746345467553"
           "16732156237";
    QCOMPARE(num1.toDecimalString(), "-23425648451321549846513213215432"
                                     "613432561345257463454675531673215"
                                     "6237");

    QVERIFY_THROWS_EXCEPTION(std::overflow_error, QBigNum512("456666666666666666666666456677852384757849873289674295687349568734098567340862387456203784598374598374598237459827349587239485732948572398475329847529384572039845"));
    QVERIFY_THROWS_EXCEPTION(std::overflow_error, QBigNum512("-456666666666666666666666456677852384757849873289674295687349568734098567340862387456203784598374598374598237459827349587239485732948572398475329847529384572039845"));
}

void TestQBigNum512::testIncrement()
{
    QBigNum512 num(0);

    // Test pre-increment
    QCOMPARE(++num, QBigNum512(1));  // Increment to 1
    QCOMPARE(++num, QBigNum512(2));  // Increment to 2

    // Test post-increment
    num = QBigNum512(2);
    QCOMPARE(num++, QBigNum512(2));  // Return the current value before increment
    QCOMPARE(num, QBigNum512(3));    // Incremented value after post-increment

    // Test large number increment
    QBigNum512 largeNum = QBigNum512("999999999999999999999999999999999");
    QCOMPARE(++largeNum, QBigNum512("1000000000000000000000000000000000"));

    QBigNum512 nonNegNum(-1);
    QCOMPARE(++nonNegNum, 0);

    // Test large number increment
    largeNum = QBigNum512::fromHex("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                                  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                                  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    largeNum++;
    QCOMPARE(++largeNum, "-0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                         "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                         "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
}

void TestQBigNum512::testDecrement()
{
    return;  // Test disabled for now

    QBigNum512 num(3);

    // Test pre-decrement
    QCOMPARE(--num, QBigNum512(2));  // Decrement to 2
    QCOMPARE(--num, QBigNum512(1));  // Decrement to 1

    // Test post-decrement
    num = QBigNum512(1);
    QCOMPARE(num--, QBigNum512(1));  // Return the current value before decrement
    QCOMPARE(num, QBigNum512(0));    // Decremented value after post-decrement

    // Test decrementing a large number
    QBigNum512 largeNum = QBigNum512("1000000000000000000000000000000000");
    QCOMPARE(--largeNum, QBigNum512("999999999999999999999999999999999"));

    // Test decrementing to negative values (if supported)
    QBigNum512 negNum(0);
    QCOMPARE(--negNum, QBigNum512("-1"));  // Result should handle negatives

    // Test large number decrement
    largeNum = QBigNum512::fromHex(
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        );
    QCOMPARE(
        --largeNum,
        QBigNum512::fromHex(
            "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            "FFFFFFFFFFFFFFFFFFE"
            )
        );
}

void TestQBigNum512::testCompareAbs()
{
    // Case 1: Absolute values are equal
    QBigNum512 num1("123456789012345678901234567890");
    QBigNum512 num2("-123456789012345678901234567890");
    QCOMPARE(num1.compareAbs(num2), 0);  // Equal absolute values

    // Case 2: num1 has a larger absolute value
    QBigNum512 num3("123456789012345678901234567891");
    QBigNum512 num4("-123456789012345678901234567890");
    QCOMPARE(num3.compareAbs(num4), 1);  // num3 has a larger absolute value

    // Case 3: num2 has a larger absolute value
    QBigNum512 num5("-123456789012345678901234567890");
    QBigNum512 num6("123456789012345678901234567891");
    QCOMPARE(num5.compareAbs(num6), -1);  // num6 has a larger absolute value

    // Case 4: One number is zero
    QBigNum512 num7("0");
    QBigNum512 num8("-987654321098765432109876543210");
    QCOMPARE(num7.compareAbs(num8), -1);  // Zero is smaller than any non-zero absolute value

    // Case 5: Both numbers are zero
    QBigNum512 num9("0");
    QBigNum512 num10("0");
    QCOMPARE(num9.compareAbs(num10), 0);  // Equal absolute values

    // Case 6: Large numbers comparison
    QBigNum512 num11("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE");
    QBigNum512 num12("-0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    QCOMPARE(num11.compareAbs(num12), -1);  // num12 has larger absolute value

    // Case 7: Compare identical numbers
    QBigNum512 num13("-56789");
    QBigNum512 num14("-56789");
    QCOMPARE(num13.compareAbs(num14), 0);  // Identical absolute values
}

void TestQBigNum512::testAddition()
{
    QBigNum512 num1(42);
    QBigNum512 num2(58);
    QBigNum512 result = num1 + 58;
    QVERIFY(result == QBigNum512(100));

    num1 += 58;
    QVERIFY(num1 == QBigNum512(100));

    QBigNum512 a = QBigNum512::fromHex("48572348752983745687134683cbc2738cb126587b61258172b7");
    QBigNum512 b = QBigNum512::fromHex("12b312c2133abcb51c15b4c1646bc19bc9bf4f22");
    result = a + b;
    QVERIFY(result == QBigNum512::fromHex("0x4857234875299627694926814080de8941728ac43cfcef40c1d9"));

    result += a;
    QVERIFY(result == QBigNum512::fromHex("0x90ae4690ea53199bbfd039c7c44ca0fcce23b11cb85e14c23490"));

    QBigNum512 m = QBigNum512("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    QBigNum512 m2 = QBigNum512("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    result = m + m2;
    QVERIFY(result == QBigNum512("0x1fffffffffffffffffffffffffffffffe"));

    a = "1231654654654541321564654654131";
    b = "-1231565165465121321321";
    result = a + b;
    QCOMPARE(a + b, "1231654653422976156099533332810");
    QCOMPARE(b + a, "1231654653422976156099533332810");

    a = "-1";
    b = "0";
    QCOMPARE(a + b, QBigNum512("-1"));

    a = "0";
    b = "-1";
    QCOMPARE(a + b, QBigNum512("-1"));

    a = "-1231654654654541321564654654131";
    b = "1231565165465121321321";
    QCOMPARE(a + b, "-1231654653422976156099533332810");
    QCOMPARE(b + a, "-1231654653422976156099533332810");

    a = "-1231654654654541321564654654131";
    b = "-1231565165465121321321";
    QCOMPARE(a + b, "-1231654655886106487029775975452");
    QCOMPARE(b + a, "-1231654655886106487029775975452");
}

void TestQBigNum512::testAdditionByScalar()
{
    // Test 1: Adding a small positive scalar
    QBigNum512 num1(5);
    num1 += 3;
    QCOMPARE(num1, QBigNum512(8));

    // Test 2: Adding a small negative scalar
    QBigNum512 num2(10);
    num2 += -4;
    QVERIFY(num2 == QBigNum512(6));

    // Test 3: Adding a scalar that causes carry
    QBigNum512 num3 = QBigNum512::fromHex("0xFFFFFFFFFFFFFFFF");
    num3 += 1;
    QVERIFY(num3 == QBigNum512::fromHex("0x10000000000000000"));

    // Test 4: Adding maximum positive scalar
    QBigNum512 num4(0);
    num4 += static_cast<int64_t>(0x7FFFFFFFFFFFFFFF); // Largest positive int64_t
    QVERIFY(num4 == QBigNum512::fromHex("0x7FFFFFFFFFFFFFFF"));

    // Test 5: Adding maximum negative scalar
    QBigNum512 num5(0);
    num5 += static_cast<int64_t>(-0x7FFFFFFFFFFFFFFF); // Largest negative int64_t
    QVERIFY(num5 == QBigNum512::fromHex("-0x7FFFFFFFFFFFFFFF"));

    // Test 6: Adding scalar to a large number
    QBigNum512 num6 = QBigNum512::fromHex("0x1234567890ABCDEF1234567890ABCDEF");
    num6 += 0x12345;
    QCOMPARE(num6, QBigNum512("24197857200151252728969465429440131380"));

    // Test 7: Adding zero
    QBigNum512 num7 = QBigNum512::fromHex("0x1234567890ABCDEF");
    num7 += 0;
    QVERIFY(num7 == QBigNum512::fromHex("0x1234567890ABCDEF"));

    // Test 9: Adding to itself
    QBigNum512 num9(50);
    num9 += 50;
    QVERIFY(num9 == QBigNum512(100));
}

void TestQBigNum512::testSubtraction()
{
    QBigNum512 num1(100);
    QBigNum512 result = num1 - 42;
    QVERIFY(result == QBigNum512(58));

    num1 -= 42;
    QVERIFY(num1 == QBigNum512(58));

    result = QBigNum512::fromHex("0x1fffffffffffffffffffffffffffffff") -
             QBigNum512::fromHex("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    QVERIFY(result == QBigNum512("-297747071055821155530452781502797185024"));

    QBigNum512 a = QBigNum512::fromHex("48572348752983745687134683cbc2738cb126587b61258172b7");
    QBigNum512 b = QBigNum512::fromHex("12b312c2133abcb51c15b4c1646bc19bc9bf4f22");
    result = a - b;
    QVERIFY(result == QBigNum512::fromHex("0x48572348752970c143c5000bc716a65dd7efc1ecb9c55bc22395"));

    result -= a;
    QVERIFY(result == QBigNum512("-106755301326852359904736178990178685327392067362"));

    QVERIFY(
        QBigNum512::fromHex("0x100000000000000000000000000000000") -
            QBigNum512::fromHex("0x0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF") == QBigNum512(1)
        );

    a = "1231654654654541321564654654131";
    b = "-1231565165465121321321";
    QVERIFY(a - b == "1231654655886106487029775975452");
    QVERIFY(b - a == "-1231654655886106487029775975452");

    a = "-1231654654654541321564654654131";
    b = "1231565165465121321321";
    QVERIFY(a - b == "-1231654655886106487029775975452");
    QVERIFY(b - a == "1231654655886106487029775975452");
}

void TestQBigNum512::testSubtractionByScalar()
{
    // Test 1: Subtracting zero (no change expected)
    QBigNum512 num1 = QBigNum512::fromHex("0x123456789ABCDEF");
    QBigNum512 original = num1;
    num1 -= 0;
    QCOMPARE(num1, original); // Subtracting zero should result in no change

    // Test 2: Subtracting a small scalar
    num1 = QBigNum512::fromHex("0x123456789ABCDEF");
    num1 -= 5;
    QCOMPARE(num1, QBigNum512::fromHex("0x123456789ABCDEA"));

    // Test 3: Subtracting a scalar resulting in a negative BigNum
    num1 = QBigNum512::fromHex("0x5");
    num1 -= 10; // Result should be -5 in two's complement
    // QVERIFY(num1.isNegative()); // Check if marked negative
    QCOMPARE(num1, QBigNum512("-5")); // -5 in two's complement

    // Test 4: Subtracting causing borrow across words
    num1 = QBigNum512::fromHex("0x10000000000000000");
    num1 -= 1;
    QCOMPARE(num1, QBigNum512::fromHex("0xFFFFFFFFFFFFFFFF"));

    // Test 5: Subtracting a large scalar from a smaller BigNum
    num1 = QBigNum512::fromHex("0x123");
    num1 -= 0x456;
    QVERIFY(num1.isNegative());
    QCOMPARE(num1, "-819");

    // Test 6: Subtracting a scalar equal to the BigNum
    num1 = QBigNum512::fromHex("0xABCDEF");
    num1 -= 0xABCDEF;
    QCOMPARE(num1, QBigNum512(0)); // Should result in zero

    // Test 7: Large BigNum subtraction
    num1 = QBigNum512::fromHex("0x123456789ABCDEF123456789ABCDEF123456789ABCDEF123456789ABCDEF");
    num1 -= 42;
    QCOMPARE(num1, QBigNum512::fromHex("0x123456789abcdef123456789abcdef123456789abcdef123456789abcdc5"));

    // Test 8:
    num1 = QBigNum512::fromHex("0x2");
    QCOMPARE(num1-5, QBigNum512(-3));
}

void TestQBigNum512::testMultiplicationByScalar()
{
    QBigNum512 num(5);
    QBigNum512 result = num * 3;
    QVERIFY(result == QBigNum512(15));
    result = num * 0x7FFFFFFFFFFFFFFF;
    QCOMPARE(result, "46116860184273879035");
    QBigNum512 num2 = QBigNum512::fromHex("0x786871234564756474536549889abfbfa"
                                        "f9876745575478635648653685856789ABCDEF");
    result = num2 * 348726387552;
    QVERIFY(result ==
            QBigNum512::fromHex("0x26306d92ceaf0fc5bc5d2394992dd4d3c5a44f88a95"
                               "4273e34d5146c8b5fdd1424cfafc3f37faa6a0"));

    num = QBigNum512::fromHex("4756823abab978456923874568734657823495873249857"
                             "2349587230570234785");
    num *= 1625476235476ULL;
    QVERIFY(num==QBigNum512::fromHex("0x6976996e3b35aa863ee1e7baf54e2b15756b4fa1"
                                      "86774e6b3811537aa5be96546b826b623a24"));

    QVERIFY(num * 0 == QBigNum512(0));
    QVERIFY(num * 1 == num);
}

void TestQBigNum512::testMultiplication()
{
    QBigNum512 a,b;
    a = "1231654654654541321564654654131";
    b = "-1231565165465121321321";
    QVERIFY(a*b == "-1516862968555507041076172169562208670475462971027051");
    QVERIFY(b*a == "-1516862968555507041076172169562208670475462971027051");

    a = "-1231654654654541321564654654131";
    b = "1231565165465121321321";
    QVERIFY(a*b == "-1516862968555507041076172169562208670475462971027051");
    QVERIFY(b*a == "-1516862968555507041076172169562208670475462971027051");

    a = "-1231654654654541321564654654131";
    b = "-1231565165465121321321";
    QVERIFY(a*b == "1516862968555507041076172169562208670475462971027051");
    QVERIFY(b*a == "1516862968555507041076172169562208670475462971027051");

    a = "1231654654654541321564654654131";
    b = "1231565165465121321321";
    QVERIFY(a*b == "1516862968555507041076172169562208670475462971027051");
    QVERIFY(b*a == "1516862968555507041076172169562208670475462971027051");
}

void TestQBigNum512::testShiftLeft()
{
    QBigNum512 num(1);
    QBigNum512 result = num << 4;
    QVERIFY(result == QBigNum512(16));

    result = num << 64;
    QVERIFY(result == QBigNum512::fromHex("0x10000000000000000"));

    QBigNum512 num2 = QBigNum512::fromHex("0xab875647564654a64a564547aa4"
                                        "a754a754aa75a4754754a753463a6436436ef0");
    result = num2 << 13;
    QVERIFY(result==QBigNum512::fromHex("0x1570eac8eac8ca94c94ac8a8f549"
                                         "4ea94ea954eb48ea8ea94ea68c74c86c86dde0000"));

    num2 <<= 13;
    QVERIFY(num2==QBigNum512::fromHex("0x1570eac8eac8ca94c94ac8a8f5494e"
                                       "a94ea954eb48ea8ea94ea68c74c86c86dde0000"));

    num = QBigNum512(1);
    num <<= 64;
    QVERIFY(num == QBigNum512::fromHex("0x10000000000000000"));

    num = QBigNum512::fromHex("0xab875647564654a64a564547aa4a754a754aa7"
                             "5a4754754a753463a6436436ef0");
    QVERIFY(num << 13 == QBigNum512::fromHex("0x1570eac8eac8ca94c94ac8a"
                                            "8f5494ea94ea954eb48ea8ea94ea68c74c86c86dde0000"));

    num = QBigNum512(1);
    QVERIFY(num << 4 == QBigNum512(16));
}

void TestQBigNum512::testComparisonOperators()
{
    QBigNum512 num1(100);
    QBigNum512 num2(42);

    QVERIFY(num1 > num2);
    QVERIFY(num1 >= num2);
    QVERIFY(num2 < num1);
    QVERIFY(num2 <= num1);
    QVERIFY(num1 != num2);
    QVERIFY(!(num1 == num2));

    QBigNum512 num3(100);
    QVERIFY(num1 == num3);

    QBigNum512 num4 = QBigNum512::fromHex("0xab875647564654a64a564547aa"
                                        "4a754a754aa75a4754754a753463a6436436EF0");
    QBigNum512 num5 = QBigNum512::fromHex("0xFEDC33BA546546469876543210");

    QVERIFY(num4 > num5);
    QVERIFY(num4 >= num5);
    QVERIFY(num4 >= num4);
    QVERIFY(num5 < num4);
    QVERIFY(num5 <= num4);
    QVERIFY(num4 <= num4);
    QVERIFY(num4 != num5);
    QVERIFY(!(num4 == num5));
    QVERIFY(num4 == num4);

    QVERIFY(QBigNum512("-1")<QBigNum512(1));
    QVERIFY(QBigNum512(1)>QBigNum512("-1"));
    QVERIFY(QBigNum512("-10000000")<QBigNum512("-1"));
    QVERIFY(QBigNum512("-1123")>QBigNum512("-10000000"));
    QVERIFY(QBigNum512("-1123")==QBigNum512("-1123"));

    QVERIFY(QBigNum512(-1)<QBigNum512(1));
    QVERIFY(QBigNum512(1)>QBigNum512(-1));
    QVERIFY(QBigNum512(-1)==QBigNum512(-1));

    num1 = -1;
    num2 = -3;
    QVERIFY(num2<num1);

    QVERIFY(QBigNum512(-3)<QBigNum512(3));

    num1 = 3;
    num2 = -3;

    QVERIFY(num2<num1);

    QVERIFY(num2<3);
    QVERIFY(num2>-5);
    QVERIFY(num2<=0);
    QVERIFY(num2<0);
    QVERIFY(QBigNum512(0)>=0);
    QVERIFY(QBigNum512(0)<=0);
    QVERIFY(QBigNum512(0)==0);
}

void TestQBigNum512::testDevision()
{
    QBigNum512 num1 = QBigNum512::fromHex("0xab874623448275123456789ABCDEF0123456789ABCDEF0");
    QBigNum512 num2 = QBigNum512::fromHex("0xFEDCBA9876543210");
    QPair<QBigNum512, QBigNum512> result = num1 / num2;
    QVERIFY(result.first == QBigNum512::fromHex("0xac4b4e736cd0c105aa8ce81c53a7e1"));
    QVERIFY(result.second == QBigNum512::fromHex("0x11cd3001b2e46ee0"));

    QBigNum512 num3 = QBigNum512::fromHex("32432FEDCBA98767875766546543210");
    result = num1 / num3;
    QVERIFY(result.first == QBigNum512("0x369a3c6323f18aec"));
    QVERIFY(result.second == QBigNum512("0x2aaaa581b9a6829fa38c4e711121830"));

    QBigNum512 qresult = num1;
    qresult /= num3;
    QVERIFY(qresult == QBigNum512("0x369a3c6323f18aec"));

    QBigNum512 num4(10);
    QBigNum512 num5(0);
    QVERIFY_THROWS_EXCEPTION(std::overflow_error, num4 / num5);

    QBigNum512 a,m;
    a = 27;
    m = 100;
    auto [q, r] = a / m;
    QCOMPARE(q, 0);
    QCOMPARE(r, 27);

    a = "36087504667311563868020454554782215476044902";
    m = "24673491392418750477366968840498960461215257940068241046415031762"
        "349950108412952187466858534273310853259172293954753124287697568"
        "4812138381256758496";
    std::tie(q, r) = a / m;
    QCOMPARE(q, 0);
    QCOMPARE(r, "36087504667311563868020454554782215476044902");

    a = 100;
    m = 27;
    std::tie(q, r) = a / m;
    QCOMPARE(q, 3);
    QCOMPARE(r, 19);

    a = -100;
    m = -27;
    std::tie(q, r) = a / m;
    QCOMPARE(q, 3);
    QCOMPARE(r, -19);

    a = -100;
    m = 27;
    std::tie(q, r) = a / m;
    QCOMPARE(q, -4);
    QCOMPARE(r, 8);

    a = 100;
    m = -27;
    std::tie(q, r) = a / m;
    QCOMPARE(q, -4);
    QCOMPARE(r, -8);

    a = 0;
    m = -27;
    std::tie(q, r) = a / m;
    QCOMPARE(q, 0);
    QCOMPARE(r, 0);

    a = 100;
    m = 50;
    std::tie(q, r) = a / m;
    QCOMPARE(q, 2);
    QCOMPARE(r, 0);

    a = -100;
    m = -50;
    std::tie(q, r) = a / m;
    QCOMPARE(q, 2);
    QCOMPARE(r, 0);

    a = -100;
    m = 50;
    std::tie(q, r) = a / m;
    QCOMPARE(q, -2);
    QCOMPARE(r, 0);

    a = 100;
    m = -50;
    std::tie(q, r) = a / m;
    QCOMPARE(q, -2);
    QCOMPARE(r, 0);

    a = "-6703903964971298549787012499102923063739682910296196688861780721860882015036740530601620165006951513494783666455657658827321391804455137936060906508738223";
    m = "-1622847881280160990320250296461570294404344900795395418626939411"
        "763813730776057";
    std::tie(q, r) = a / m;
    QCOMPARE(a, q * m + r);

    for (int k = 0; k < 10000; k++)
    {
        uint16_t n_nbits = QRandomGenerator::global()->generate();
        while (n_nbits <= 0 || n_nbits >= 512)
        {
            n_nbits = QRandomGenerator::global()->generate();
        }
        uint16_t d_nbits = QRandomGenerator::global()->generate();
        while (d_nbits <= 0 || d_nbits >= 512)
        {
            d_nbits = QRandomGenerator::global()->generate();
        }
        QBigNum512 a = QBigNum512::randomize(n_nbits, QRandomGenerator::global()->generate() & 1);
        QBigNum512 m = QBigNum512::randomize(d_nbits, QRandomGenerator::global()->generate() & 1);

        if (m == 0)
        {
            continue;
        }

        auto [q, r] = a / m;

        if (a != q * m + r)
        {
            qDebug().noquote().nospace() << "a=" << a << "\nq=" << q << "\nm=" << m << "\nr=" << r << "\nresult=divmod(a,m)";
        }

        QCOMPARE(a, q * m + r);
    }
}

void TestQBigNum512::testModulo()
{
    QBigNum512 a = QBigNum512::fromDecimal("8187839724595137590217291752483");
    QBigNum512 p = QBigNum512::fromDecimal("114871136315397");
    a %= p;
    QVERIFY(a == QBigNum512::fromDecimal("107238895119914"));

    QBigNum512 num1(100);
    QBigNum512 num2(42);
    QBigNum512 result = num1 % num2;
    QCOMPARE(result, QBigNum512(16));

    QBigNum512 num3 = QBigNum512::fromHex("0xab875647564654a64a564547aa4a754a"
                                        "754aa75a4754754a753463a6436436EF0");
    QBigNum512 num4 = QBigNum512::fromHex("0xFEDC33BA546546469876543210");
    result = num3 % num4;
    QVERIFY(result == QBigNum512::fromHex("0x75fd268eec128a2881e8ea4cd0"));

    result = num4 % num3;
    QVERIFY(result == QBigNum512::fromHex("0xFEDC33BA546546469876543210"));

    result = QBigNum512(0) % num3;
    QVERIFY(result == QBigNum512::fromHex("0"));

    QVERIFY_THROWS_EXCEPTION(std::overflow_error, num4 % QBigNum512(0));

    a = -100;
    QBigNum512 m = 27;
    auto [q, r] = a / m;
    QCOMPARE(q, -4);
    QCOMPARE(r, 8);

    a = -3;
    m = 7;
    std::tie(q, r) = a / m;
    result = q * m + r;
    QCOMPARE(result, -3);
    QCOMPARE(q, -1);
    QCOMPARE(r, 4);

    for (int k = 0; k < 1000; k++)
    {
        uint16_t n_nbits = QRandomGenerator::global()->generate();
        while (n_nbits <= 0 || n_nbits >= 512)
        {
            n_nbits = QRandomGenerator::global()->generate();
        }
        uint16_t d_nbits = QRandomGenerator::global()->generate();
        while (d_nbits <= 0 || d_nbits >= 512)
        {
            d_nbits = QRandomGenerator::global()->generate();
        }
        QBigNum512 a = QBigNum512::randomize(n_nbits, QRandomGenerator::global()->generate() & 1);
        QBigNum512 m = QBigNum512::randomize(d_nbits, QRandomGenerator::global()->generate() & 1);

        if (m == 0)
        {
            continue;
        }

        auto [q, r] = a / m;
        auto result = a % m;

        QCOMPARE(result, r);
    }
}

void TestQBigNum512::testDivisionWithGMP()
{
    // GMP variables
    mpz_t gmp_q, gmp_r, gmp_a, gmp_b;
    mpz_inits(gmp_q, gmp_r, gmp_a, gmp_b, nullptr);

    // Test cases
    QVector<QPair<QString, QString>> testCases = {
                                                  {"100", "5"},
                                                  {"12345678901234567890", "1"},
                                                  {"5", "10"},
                                                  {"-100", "7"},
                                                  {"0", "123"},
                                                  {"987654321", "12345"},
                                                  };

    for (const auto& [aStr, bStr] : testCases)
    {
        // Set GMP values
        mpz_set_str(gmp_a, aStr.toStdString().c_str(), 10);
        mpz_set_str(gmp_b, bStr.toStdString().c_str(), 10);

        // Perform GMP division
        mpz_fdiv_qr(gmp_q, gmp_r, gmp_a, gmp_b);

        // Perform BigNum division
        QBigNum512 a(aStr);
        QBigNum512 b(bStr);
        auto [q, r] = a / b;

        // Compare results
        QCOMPARE(q.toDecimalString(), QString(mpz_get_str(nullptr, 10, gmp_q)));
        QCOMPARE(r.toDecimalString(), QString(mpz_get_str(nullptr, 10, gmp_r)));
    }

    // Clear GMP variables
    mpz_clears(gmp_q, gmp_r, gmp_a, gmp_b, nullptr);
}

void TestQBigNum512::testDivisionSpeedWithGMP()
{
    // GMP variables
    mpz_t gmp_a, gmp_b, gmp_q, gmp_r;
    mpz_inits(gmp_a, gmp_b, gmp_q, gmp_r, nullptr);

    // Test input
    QString bigNumber1 = "123456789012345678901234586789765425423748656978694"
                         "5678901234567845687980653456780987765649012345678901234567890";
    QString bigNumber2 = "98765432109876543210987654321098765432109876543210";

    // Set GMP values
    mpz_set_str(gmp_a, bigNumber1.toStdString().c_str(), 10);
    mpz_set_str(gmp_b, bigNumber2.toStdString().c_str(), 10);

    // BigNum objects
    QBigNum512 bigNum1(bigNumber1);
    QBigNum512 bigNum2(bigNumber2);

    // Number of iterations for the test
    constexpr int iterations = 10000;
    constexpr uint32_t seed = 123456;
    QRandomGenerator generator(seed);

    // Loading time
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < iterations; ++i)
    {
        uint16_t n_nbits = generator.generate();
        while (n_nbits > 512)
        {
            n_nbits = generator.generate();
        }
        uint16_t d_nbits = generator.generate();
        while (d_nbits > 512)
        {
            d_nbits = generator.generate();
        }
        QBigNum512 ta = QBigNum512::randomize(n_nbits, generator.generate() & 1);
        QBigNum512 tb = QBigNum512::randomize(d_nbits, generator.generate() & 1);
        if (tb == 0)
        {
            continue;
        }
    }
    generator.seed(seed);
    qint64 loadingTime = timer.elapsed();
    qDebug() << "Loading time for" << iterations << "iterations:" << loadingTime << "ms";

    // GMP division speed test
    timer.restart();
    for (int i = 0; i < iterations; ++i)
    {

        uint16_t n_nbits = generator.generate();
        while (n_nbits > 512)
        {
            n_nbits = generator.generate();
        }
        uint16_t d_nbits = generator.generate();
        while (d_nbits > 512)
        {
            d_nbits = generator.generate();
        }
        QBigNum512 ta = QBigNum512::randomize(n_nbits, generator.generate() & 1);
        QBigNum512 tb = QBigNum512::randomize(d_nbits, generator.generate() & 1);
        if (tb == 0)
        {
            continue;
        }

        // Set GMP values
        mpz_set_str(gmp_a, ta.toDecimalString().toStdString().c_str(), 10);
        mpz_set_str(gmp_b, tb.toDecimalString().toStdString().c_str(), 10);

        mpz_fdiv_qr(gmp_q, gmp_r, gmp_a, gmp_b);
    }
    generator.seed(seed);
    qint64 gmpTime = timer.elapsed();
    qDebug() << "GMP division time for" << iterations << "iterations:" << gmpTime - loadingTime << "ms";

    // BigNum division speed test
    timer.restart();
    for (int i = 0; i < iterations; ++i)
    {
        uint16_t n_nbits = generator.generate();
        while (n_nbits > 512)
        {
            n_nbits = generator.generate();
        }
        uint16_t d_nbits = generator.generate();
        while (d_nbits > 512)
        {
            d_nbits = generator.generate();
        }
        QBigNum512 ta = QBigNum512::randomize(n_nbits, generator.generate() & 1);
        QBigNum512 tb = QBigNum512::randomize(d_nbits, generator.generate() & 1);
        if (tb == 0)
        {
            continue;
        }

        /* Set bignum values */
        QBigNum512 a(ta.toDecimalString());
        QBigNum512 b(tb.toDecimalString());

        auto [q, r] = a / b;
    }
    generator.seed(seed);
    qint64 bigNumTime = timer.elapsed();
    qDebug() << "BigNum division time for" << iterations << "iterations:" << bigNumTime - loadingTime << "ms";

    // Cleanup GMP variables
    mpz_clears(gmp_a, gmp_b, gmp_q, gmp_r, nullptr);
}

void TestQBigNum512::testPowMod()
{
    QBigNum512 base("2");
    QBigNum512 exp("10");
    QBigNum512 mod("1000");

    QCOMPARE(QBigNum512::powMod(base, exp, mod), "24"); // 2^10 % 1000 = 24
    QCOMPARE(QBigNum512::powMod(QBigNum512("-43523452"), QBigNum512("23562456"), QBigNum512("34534")), "11038");
    QCOMPARE(QBigNum512::powMod(QBigNum512("-43523452"), QBigNum512("0"), QBigNum512("412")), "1");
    QCOMPARE(QBigNum512::powMod(QBigNum512("43523452"), QBigNum512("0"), QBigNum512("412")), "1");
    QCOMPARE(QBigNum512::powMod(QBigNum512("43523452"), QBigNum512("123"), QBigNum512("-412")), "-172");
    QCOMPARE(QBigNum512::powMod(QBigNum512("43523452"), QBigNum512("123"), QBigNum512("-412")), "-172");
    QCOMPARE(QBigNum512::powMod(QBigNum512("-43523452"), QBigNum512("123"), QBigNum512("412")), "172");
    QCOMPARE(QBigNum512::powMod(QBigNum512("-43523452"), QBigNum512("123"), QBigNum512("-412")), "-240");

    base = "15548325492384758723457862387456028374568723464";
    exp = "123215647465412132165465123546513521232168546432453";
    mod = "213452134523452345234532";
    QCOMPARE(base.powMod(exp, mod), "46552951319514750044964");

    /* -ve powers */

    QCOMPARE(QBigNum512::powMod(QBigNum512("4"), QBigNum512("-3"), QBigNum512("13")), "12");
    QCOMPARE(QBigNum512::powMod("462458624980567298467943",
                               "-325629856298576981762098467",
                               "145682346723870562038756023563"), "137096311132785955879795219808");
    QCOMPARE(QBigNum512::powMod("4", "-3", "-13"), "-1");


}

void TestQBigNum512::testInverseMod()
{
    // Test 1: Basic case
    {
        QBigNum512 a("3");
        QBigNum512 mod("7");
        QCOMPARE(a.inverseMod(mod), QBigNum512("5")); // 3 * 5 % 7 == 1
    }

    // Test 2: No inverse (GCD != 1)
    {
        QBigNum512 a("6");
        QBigNum512 mod("9");
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, a.inverseMod(mod)); // GCD(6, 9) != 1
    }

    // Test 3: Large numbers
    {
        QBigNum512 a("123456789");
        QBigNum512 mod("1000000007");
        QCOMPARE(a.inverseMod(mod), QBigNum512("18633540")); // Verified using other libraries
    }

    // Test 4: Negative number
    {
        QBigNum512 a("-3");
        QBigNum512 mod("7");
        QCOMPARE(a.inverseMod(mod), QBigNum512("2")); // -3 mod 7 == 4; 4 * 2 % 7 == 1
    }

    // Test 5: Modulus of 1
    {
        QBigNum512 a("42");
        QBigNum512 mod("1");
        QCOMPARE(a.inverseMod(mod), QBigNum512("0")); // Any number mod 1 is 0
    }

    // Test 6: Self-inverse
    {
        QBigNum512 a("1");
        QBigNum512 mod("1000003");
        QCOMPARE(a.inverseMod(mod), QBigNum512("1")); // 1 is always its own inverse
    }

    // Test 7: Edge case - modulus cannot be zero
    {
        QBigNum512 a("10");
        QBigNum512 mod("0");
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, a.inverseMod(mod)); // Modulus cannot be zero
    }

    QCOMPARE(QBigNum512("4").inverseMod(13), 10);
    QCOMPARE(QBigNum512("4").inverseMod(-13), -3);
}

void TestQBigNum512::testGCD()
{
    QCOMPARE(QBigNum512::gcd(23422, 234234), 14);
    QCOMPARE(QBigNum512::gcd(23422, -234234), 14);
    QCOMPARE(QBigNum512::gcd(-23422, 234234), 14);
    QCOMPARE(QBigNum512::gcd(-23422, -234234), 14);
    QCOMPARE(QBigNum512::gcd_slow(23422, 234234), 14);
    QCOMPARE(QBigNum512::gcd_slow(23422, -234234), 14);
    QCOMPARE(QBigNum512::gcd_slow(-23422, 234234), 14);
    QCOMPARE(QBigNum512::gcd_slow(-23422, -234234), 14);

    QCOMPARE(QBigNum512::gcd(23423, 234234), 1);
    QCOMPARE(QBigNum512::gcd("-2342452345728345782364578236452",
                             "23423523745982374695872364534252333224"), 4);

    // Number of iterations for the test
    constexpr int iterations = 30000;
    constexpr uint32_t seed = 123456;
    QRandomGenerator generator(seed);

    QElapsedTimer timer;
    timer.start();
    for (int k = 0; k < iterations; k++)
    {
        uint16_t n_nbits = QRandomGenerator::global()->generate();
        while (n_nbits <= 0 || n_nbits >= 512)
        {
            n_nbits = QRandomGenerator::global()->generate();
        }
        uint16_t d_nbits = QRandomGenerator::global()->generate();
        while (d_nbits <= 0 || d_nbits >= 512)
        {
            d_nbits = QRandomGenerator::global()->generate();
        }
        QBigNum512 a = QBigNum512::randomize(n_nbits, QRandomGenerator::global()->generate() & 1);
        QBigNum512 b = QBigNum512::randomize(d_nbits, QRandomGenerator::global()->generate() & 1);
        QCOMPARE(QBigNum512::gcd(a, b), QBigNum512::gcd_slow(a, b));
    }

    generator.seed(seed);
    timer.restart();
    for (int k = 0; k < iterations; k++)
    {
        uint16_t n_nbits = QRandomGenerator::global()->generate();
        while (n_nbits <= 0 || n_nbits >= 512)
        {
            n_nbits = QRandomGenerator::global()->generate();
        }
        uint16_t d_nbits = QRandomGenerator::global()->generate();
        while (d_nbits <= 0 || d_nbits >= 512)
        {
            d_nbits = QRandomGenerator::global()->generate();
        }
        QBigNum512 a = QBigNum512::randomize(n_nbits, QRandomGenerator::global()->generate() & 1);
        QBigNum512 b = QBigNum512::randomize(d_nbits, QRandomGenerator::global()->generate() & 1);

        auto result = QBigNum512::gcd_slow(a, b);
    }
    generator.seed(seed);
    qint64 elapsed = timer.elapsed();
    qDebug() << "gcd_slow" << iterations << "iterations:" << elapsed << "ms";

    timer.restart();
    for (int k = 0; k < iterations; k++)
    {
        uint16_t n_nbits = QRandomGenerator::global()->generate();
        while (n_nbits <= 0 || n_nbits >= 512)
        {
            n_nbits = QRandomGenerator::global()->generate();
        }
        uint16_t d_nbits = QRandomGenerator::global()->generate();
        while (d_nbits <= 0 || d_nbits >= 512)
        {
            d_nbits = QRandomGenerator::global()->generate();
        }
        QBigNum512 a = QBigNum512::randomize(n_nbits, QRandomGenerator::global()->generate() & 1);
        QBigNum512 b = QBigNum512::randomize(d_nbits, QRandomGenerator::global()->generate() & 1);

        auto result = QBigNum512::gcd(a, b);
    }
    generator.seed(seed);
    elapsed = timer.elapsed();
    qDebug() <<  "gcd" << iterations << "iterations:" << elapsed << "ms";
}

void TestQBigNum512::testMillerRabin()
{
    // Number of iterations for the test
    constexpr int iterations = 100;
    constexpr uint32_t seed = 123456;
    QRandomGenerator generator(seed);
    QElapsedTimer timer;
    timer.start();
    int maxNbits = 0;
    for (int k = 0; k < iterations; k++)
    {
        QBigNum512 a, p;
        do
        {
            uint16_t d_nbits = QRandomGenerator::global()->generate();
            while (d_nbits <= 0 || d_nbits >= 512)
            {
                d_nbits = QRandomGenerator::global()->generate();
            }
            p = QBigNum512::randomize(d_nbits, false);
            if (p.bitLength() > maxNbits)
            {
                maxNbits = p.bitLength();
            }
        } while (!QBigNum512::millerRabin(p));
    }
    qDebug() << "found" << iterations << "random primes of length upto" << maxNbits << "bits in" << timer.elapsed() << "ms";
}

void TestQBigNum512::testTonelli()
{
    // Number of iterations for the test
    constexpr int iterations = 20;
    constexpr uint32_t seed = 123456;
    QRandomGenerator generator(seed);
    QElapsedTimer timer;
    timer.start();
    for (int k = 0; k < iterations; k++)
    {
        QBigNum512 a, p;
        do
        {
            uint16_t n_nbits = QRandomGenerator::global()->generate();
            while (n_nbits <= 1 || n_nbits >= 512)
            {
                n_nbits = QRandomGenerator::global()->generate();
            }
            uint16_t d_nbits = QRandomGenerator::global()->generate();
            while (d_nbits <= 1 || d_nbits >= 512)
            {
                d_nbits = QRandomGenerator::global()->generate();
            }
            a = QBigNum512::randomize(n_nbits, false);
            p = QBigNum512::randomize(d_nbits, false);

        } while (p == 2 ||
                 p % 2 == 0 ||
                 QBigNum512::gcd(a, p) != 1 ||
                 QBigNum512::legendre(a, p) != 1 ||
                 !QBigNum512::millerRabin(p));

        /* p will most likly be an odd prime so we can use tonelli */
        auto result = QBigNum512::tonelli(a, p);
        QCOMPARE(QBigNum512::mulMod(result, result, p), a % p);
    }
    qDebug() << "found" << iterations << "random quadratic residuals for" << iterations << "random primes in" << timer.elapsed() << "ms";
}

QTEST_MAIN(TestQBigNum512)
#include "tst_qbignum512.moc"
