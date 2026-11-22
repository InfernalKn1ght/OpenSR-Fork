#include "TestParser.h"

#include "OpenSR/QM/Parser.h"

namespace OpenSR
{
namespace QM
{

void TestParser::testIntegerArithmetic_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<float>("expected");

    QTest::newRow("addition") << "3+4" << 7.0f;
    QTest::newRow("subtraction") << "10-3" << 7.0f;
    QTest::newRow("multiplication") << "6*7" << 42.0f;
    QTest::newRow("division") << "9/2" << 4.5f;
    QTest::newRow("modulo") << "9mod2" << 1.0f;
    QTest::newRow("precedence") << "1+2*3" << 7.0f;
    QTest::newRow("parentheses") << "(1+2)*3" << 9.0f;
    QTest::newRow("integer division") << "7div2" << 3.0f;
}

void TestParser::testIntegerArithmetic()
{
    QFETCH(QString, expression);
    QFETCH(float, expected);

    QCOMPARE(eval(expression, {}), expected);
}

void TestParser::testFloatArithmetic_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<float>("expected");

    // Regression: fractional literals used to be parsed through QString::toInt()
    // and silently truncated to 0 (e.g. "2.5*2" evaluated to 0).
    QTest::newRow("fraction times integer") << "2.5*2" << 5.0f;
    QTest::newRow("fraction plus integer") << "1.5+1" << 2.5f;
    QTest::newRow("single float token") << "[1.5]" << 1.5f;
    QTest::newRow("negative fraction") << "-1.5+0.5" << -1.0f;
}

void TestParser::testFloatArithmetic()
{
    QFETCH(QString, expression);
    QFETCH(float, expected);

    QCOMPARE(eval(expression, {}), expected);
}

void TestParser::testParameters()
{
    QMap<uint32_t, float> parameters;
    parameters.insert(1, 10.0f);
    parameters.insert(2, 4.0f);

    QCOMPARE(eval("[p1]+[p2]", parameters), 14.0f);
    QCOMPARE(eval("[p1]*[p2]", parameters), 40.0f);
}

void TestParser::testRange_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<float>("min");
    QTest::addColumn<float>("max");

    QTest::newRow("single") << "[1h5]" << 1.0f << 5.0f;
    QTest::newRow("zero based") << "[0h3]" << 0.0f << 3.0f;
    QTest::newRow("negative") << "[-2h2]" << -2.0f << 2.0f;
    QTest::newRow("list") << "[1;2;3]" << 1.0f << 3.0f;
}

void TestParser::testRange()
{
    QFETCH(QString, expression);
    QFETCH(float, min);
    QFETCH(float, max);

    for (int i = 0; i < 200; ++i)
    {
        const float value = eval(expression, {});
        QVERIFY2(value >= min && value <= max,
                 qPrintable(QString("value %1 out of [%2, %3]").arg(value).arg(min).arg(max)));
    }
}

void TestParser::testDegenerateRangeDoesNotCrash()
{
    // A reversed range used to compute `rand() % 0` and abort with SIGFPE.
    for (int i = 0; i < 100; ++i)
    {
        QCOMPARE(eval("[2h1]", {}), 2.0f);
    }
}

void TestParser::testMalformedExpressionDoesNotCrash_data()
{
    QTest::addColumn<QString>("expression");

    QTest::newRow("unknown range suffix") << "[5x]";
    QTest::newRow("trailing operator") << "1 +";
    QTest::newRow("range with words") << "[5 to 2]";
    QTest::newRow("unclosed bracket") << "[5";
    QTest::newRow("empty expression") << "";
}

void TestParser::testMalformedExpressionDoesNotCrash()
{
    QFETCH(QString, expression);

    // Malformed input must not crash; the exact result is unspecified.
    eval(expression, {});
}

} // namespace QM
} // namespace OpenSR

QTEST_APPLESS_MAIN(OpenSR::QM::TestParser)
