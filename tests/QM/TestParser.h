#pragma once

#include <QtTest>

namespace OpenSR
{
namespace QM
{

class TestParser : public QObject
{
    Q_OBJECT

private slots:
    void testIntegerArithmetic_data();
    void testIntegerArithmetic();

    void testFloatArithmetic_data();
    void testFloatArithmetic();

    void testParameters();

    void testRange_data();
    void testRange();

    void testDegenerateRangeDoesNotCrash();
    void testMalformedExpressionDoesNotCrash_data();
    void testMalformedExpressionDoesNotCrash();
};

} // namespace QM
} // namespace OpenSR
