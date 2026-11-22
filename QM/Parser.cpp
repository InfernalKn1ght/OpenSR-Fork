/*
    OpenSR - opensource multi-genre game based upon "Space Rangers 2: Dominators"
    Copyright (C) 2013 - 2015 Kosyak <ObKo@mail.ru>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "OpenSR/QM/Parser.h"

#include <QDebug>
#include <QStack>
#include <cmath>

namespace OpenSR
{
namespace QM
{
namespace
{
const uint8_t PRECEDENCES[23] = {0, 0, 0, 0, 6, 6, 8, 7, 8, 3, 3, 3, 3, 3, 3, 1, 2, 9, 4, 5, 0, 0, 8};

// Bounds-checked character access. QString::operator[] asserts when the index
// equals the size, which happens routinely while tokenizing a truncated
// expression such as "[5".
inline QChar charAt(const QString &s, qsizetype i)
{
    return (i >= 0 && i < s.size()) ? s.at(i) : QChar();
}

// Sample a random integer in [from, to] inclusive. Defends against a
// reversed or degenerate range (to < from) which would otherwise yield a
// zero/negative modulus and crash with SIGFPE on `rand() % 0`.
float randomInRange(float from, float to)
{
    double span = static_cast<double>(to) - static_cast<double>(from) + 1.0;
    if (span <= 0.0)
    {
        return from;
    }

    uint32_t count = static_cast<uint32_t>(span);
    if (count == 0)
    {
        count = 1;
    }

    return from + static_cast<float>(rand() % count);
}
} // namespace

Token::Token() : type(TOKEN_NONE), value{}
{

    value.from = 0.0f;
    value.to = 0.0f;
    value.number = 0.0f;
}

Token::Token(float number) : type(TOKEN_NUMBER), value{}
{
    value.number = number;
}

Token::Token(float from, float to) : type(TOKEN_RANGE), value{}
{
    value.from = from;
    value.to = to;
}

Token Token::apply(const Token &a, const Token &b, const QMap<uint32_t, float> &parameters) const
{
    float av{}, bv{};

    if (a.type == TOKEN_RANGE)
    {
        av = randomInRange(a.value.from, a.value.to);
    }
    else if (a.type == TOKEN_LIST)
    {
        av = a.list.at(rand() % a.list.size());
    }
    else if (a.type == TOKEN_PARAMETER)
    {
        av = parameters.value(a.value.id);
    }
    else
    {
        av = a.value.number;
    }

    if (b.type == TOKEN_RANGE)
    {
        bv = randomInRange(b.value.from, b.value.to);
    }
    else if (b.type == TOKEN_LIST)
    {
        bv = b.list.at(rand() % b.list.size());
    }
    else if (b.type == TOKEN_PARAMETER)
    {
        bv = parameters.value(b.value.id);
    }
    else
    {
        bv = b.value.number;
    }

    const auto iAv = static_cast<int32_t>(av);
    const auto iBv = static_cast<int32_t>(bv);

    switch (type)
    {
    case TOKEN_NUMBER:
    case TOKEN_RANGE:
    case TOKEN_PARAMETER:
    case TOKEN_OPEN_PAR:
    case TOKEN_CLOSE_PAR:
    case TOKEN_NONE:
    case TOKEN_LIST:
        return *this;
    case TOKEN_OP_PLUS:
        return Token(av + bv);
    case TOKEN_OP_MINUS:
        return Token(av - bv);
    case TOKEN_OP_DIV:
        return Token(av / bv);
    case TOKEN_OP_INT_DIV:
        if (iBv == 0)
        {
            return Token(0.0f);
        }
        return Token(static_cast<float>(iAv / iBv));
    case TOKEN_OP_MULTIPLY:
        return Token(av * bv);
    case TOKEN_OP_MOD:
        return Token(static_cast<float>(fmod(av, bv)));
    case TOKEN_EQUAL:
        return Token(av == bv);
    case TOKEN_MORE:
        return Token(av > bv);
    case TOKEN_LESS:
        return Token(av < bv);
    case TOKEN_MORE_EQUAL:
        return Token(av >= bv);
    case TOKEN_LESS_EQUAL:
        return Token(av <= bv);
    case TOKEN_NOT_EQUAL:
        return Token(av != bv);
    case TOKEN_OR:
        return Token(static_cast<bool>(av) || static_cast<bool>(bv));
    case TOKEN_AND:
        return Token(static_cast<bool>(av) && static_cast<bool>(bv));
    case TOKEN_NOT:
        return Token(!static_cast<bool>(av));
    case TOKEN_TO:
        // FIXME: List?
        if ((a.type == TOKEN_RANGE) && (b.type == TOKEN_RANGE))
        {
            float min = std::min(a.value.from, b.value.from);
            float max = std::max(a.value.to, b.value.to);
            return Token(min, max);
        }
        else if ((a.type == TOKEN_RANGE) && (b.type != TOKEN_RANGE))
        {
            float min = std::min(a.value.from, bv);
            float max = std::max(a.value.to, bv);
            return Token(min, max);
        }
        else if ((a.type != TOKEN_RANGE) && (b.type == TOKEN_RANGE))
        {
            float min = std::min(av, b.value.from);
            float max = std::max(av, b.value.to);
            return Token(min, max);
        }
        else
        {
            float min = std::min(av, bv);
            float max = std::max(av, bv);
            return Token(min, max);
        }
    case TOKEN_IN:
        if ((a.type == TOKEN_RANGE) && (b.type == TOKEN_RANGE))
        {
            return Token((a.value.from >= b.value.from) && (a.value.to <= b.value.to));
        }
        else if ((a.type == TOKEN_RANGE) && (b.type != TOKEN_RANGE))
        {
            return Token((a.value.from <= bv) && (a.value.to >= bv));
        }
        else if ((a.type != TOKEN_RANGE) && (b.type == TOKEN_RANGE))
        {
            return Token((av >= b.value.from) && (av <= b.value.to));
        }
        else
        {
            return Token(av == bv);
        }
    default:
        return *this;
    }
}

int32_t getInt(int &pos, const QString &exp)
{
    const qsizetype start = pos;

    // A '-' is only a sign here; consuming it greedily (as before) turned
    // "10-3" into the single invalid token "10-3" which converts to 0.
    if (pos < exp.size() && exp[pos] == '-')
    {
        pos++;
    }

    while (pos < exp.size() && exp[pos].isDigit())
    {
        pos++;
    }

    return exp.sliced(start, pos - start).toInt();
}

float getFloat(int &pos, const QString &exp)
{
    const qsizetype start = pos;
    bool wasPoint = false;

    if (pos < exp.size() && exp[pos] == '-')
    {
        pos++;
    }

    while (pos < exp.size() && (exp[pos].isDigit() || (exp[pos] == '.' && !wasPoint)))
    {
        if (exp[pos] == '.')
        {
            wasPoint = true;
        }
        pos++;
    }

    // Parse the token as a floating point value: integer literals stay
    // exact and fractional literals like "2.5" are no longer truncated to 0.
    double res = exp.sliced(start, pos - start).toDouble();
    return static_cast<float>(res);
}

QList<Token> tokenize(const QString &expression)
{
    int pos = 0;
    Token prev;
    QList<Token> result;

    QStack<Token> opStack;

    QString exp = expression;
    exp.replace(',', '.');

    while (pos < exp.length())
    {
        while ((pos < exp.length()) && (charAt(exp, pos).isSpace()))
        {
            pos++;
        }
        if (pos == exp.length())
        {
            break;
        }

        Token t;
        // FIXME: Ugly unary minus detection.
        if ((charAt(exp, pos).isDigit()) ||
            ((charAt(exp, pos) == '-') && (prev.type != Token::TOKEN_NUMBER) && (prev.type != Token::TOKEN_RANGE) &&
             (prev.type != Token::TOKEN_PARAMETER) && (prev.type != Token::TOKEN_LIST) &&
             (prev.type != Token::TOKEN_CLOSE_PAR)))
        {
            t = Token(getFloat(pos, exp));
        }
        else if (charAt(exp, pos) == '-')
        {
            t.type = Token::TOKEN_OP_MINUS;
            pos++;
        }
        else if (charAt(exp, pos) == '+')
        {
            t.type = Token::TOKEN_OP_PLUS;
            pos++;
        }
        else if (charAt(exp, pos) == '*')
        {
            t.type = Token::TOKEN_OP_MULTIPLY;
            pos++;
        }
        else if (charAt(exp, pos) == '/')
        {
            t.type = Token::TOKEN_OP_DIV;
            pos++;
        }
        else if (charAt(exp, pos) == '(')
        {
            t.type = Token::TOKEN_OPEN_PAR;
            pos++;
        }
        else if (charAt(exp, pos) == ')')
        {
            t.type = Token::TOKEN_CLOSE_PAR;
            pos++;
        }
        else if (charAt(exp, pos) == '=')
        {
            t.type = Token::TOKEN_EQUAL;
            pos++;
        }
        else if (charAt(exp, pos) == '>')
        {
            if (charAt(exp, pos + 1) == '=')
            {
                t.type = Token::TOKEN_MORE_EQUAL;
                pos += 2;
            }
            else
            {
                t.type = Token::TOKEN_MORE;
                pos++;
            }
        }
        else if (charAt(exp, pos) == '<')
        {
            if (charAt(exp, pos + 1) == '=')
            {
                t.type = Token::TOKEN_LESS_EQUAL;
                pos += 2;
            }
            else if (charAt(exp, pos + 1) == '>')
            {
                t.type = Token::TOKEN_NOT_EQUAL;
                pos += 2;
            }
            else
            {
                t.type = Token::TOKEN_LESS;
                pos++;
            }
        }
        else if (charAt(exp, pos) == '[')
        {
            if (charAt(exp, pos + 1) == 'p')
            {
                pos += 2;
                t.type = Token::TOKEN_PARAMETER;
                t.value.id = (uint32_t)getInt(pos, exp);
                if (charAt(exp, pos) != ']')
                {
                    qWarning() << "Unclosed parameter token in \"" << exp << "\"";
                    return QList<Token>();
                }
                pos++;
            }
            else if ((charAt(exp, pos + 1).isDigit()) || ((charAt(exp, pos + 1) == '-')))
            {
                pos++;
                float v1{}, v2{};
                v1 = getFloat(pos, exp);
                if (charAt(exp, pos) == ']')
                {
                    t = Token(v1);
                    pos++;
                }
                else if ((charAt(exp, pos) == '.') || (charAt(exp, pos) == 'h'))
                {
                    pos++;
                    v2 = getFloat(pos, exp);
                    if (charAt(exp, pos) != ']')
                    {
                        qWarning() << "Invalid range token in \"" << exp << "\" at pos = " << pos;
                        return QList<Token>();
                    }
                    t = Token(v1, v2);
                    pos++;
                }
                else if (charAt(exp, pos) == ';')
                {
                    t.type = Token::TOKEN_LIST;
                    t.list.push_back(v1);
                    while (charAt(exp, pos) == ';')
                    {
                        pos++;
                        t.list.push_back(getFloat(pos, exp));
                    }
                    if (charAt(exp, pos) != ']')
                    {
                        qWarning() << "Unclosed list token in \"" << exp << "\"";
                        return QList<Token>();
                    }
                    pos++;
                }
                else
                {
                    qWarning() << "Invalid range token in \"" << exp << "\" at pos = " << pos;
                    return QList<Token>();
                }
            }
            else
            {
                qWarning() << "Invalid token in \"" << exp << "\"";
                return QList<Token>();
            }
        }
        else if (exp.mid(pos, 3) == "mod")
        {
            t.type = Token::TOKEN_OP_MOD;
            pos += 3;
        }
        else if (exp.mid(pos, 3) == "div")
        {
            t.type = Token::TOKEN_OP_INT_DIV;
            pos += 3;
        }
        else if (exp.mid(pos, 3) == "and")
        {
            t.type = Token::TOKEN_AND;
            pos += 3;
        }
        else if (exp.mid(pos, 3) == "not")
        {
            t.type = Token::TOKEN_NOT;
            pos += 3;
        }
        else if (exp.mid(pos, 2) == "or")
        {
            t.type = Token::TOKEN_OR;
            pos += 2;
        }
        else if (exp.mid(pos, 2) == "to")
        {
            t.type = Token::TOKEN_TO;
            pos += 2;
        }
        else if (exp.mid(pos, 2) == "in")
        {
            t.type = Token::TOKEN_IN;
            pos += 2;
        }
        else
        {
            qWarning() << "Invalid token in \"" << exp << "\" at pos = " << pos;
            return QList<Token>();
        }

        if ((t.type == Token::TOKEN_NUMBER) || (t.type == Token::TOKEN_RANGE) || (t.type == Token::TOKEN_PARAMETER) ||
            (t.type == Token::TOKEN_LIST))
        {
            result.append(t);
        }
        else if (t.type == Token::TOKEN_OPEN_PAR)
        {
            opStack.push(t);
        }
        else if (t.type == Token::TOKEN_CLOSE_PAR)
        {
            Token t2;
            if (opStack.empty())
            {
                qWarning() << "Parenthesis error in \"" << exp << "\"";
                return QList<Token>();
            }
            t2 = opStack.top();
            opStack.pop();
            if (t2.type != Token::TOKEN_OPEN_PAR)
            {
                result.append(t2);
            }

            while (t2.type != Token::TOKEN_OPEN_PAR && !opStack.empty())
            {
                t2 = opStack.top();
                opStack.pop();
                if (t2.type != Token::TOKEN_OPEN_PAR)
                {
                    result.append(t2);
                }
            }
        }
        else
        {
            if (!opStack.empty())
            {
                Token o2 = opStack.top();
                while ((!opStack.empty()) && (o2.type != Token::TOKEN_OPEN_PAR) &&
                       (o2.type != Token::TOKEN_CLOSE_PAR) && (PRECEDENCES[t.type] <= PRECEDENCES[o2.type]))
                {
                    result.append(o2);
                    opStack.pop();
                    if (!opStack.empty())
                    {
                        o2 = opStack.top();
                    }
                }
            }
            opStack.push(t);
        }

        prev = t;
    }
    while (!opStack.empty())
    {
        Token t = opStack.top();
        opStack.pop();
        if (t.type == Token::TOKEN_OPEN_PAR)
        {
            qWarning() << "Parenthesis error in \"" << exp << "\"";
            return QList<Token>();
        }
        result.append(t);
    }
    return result;
}

float eval(const QList<Token> &exp, const QMap<uint32_t, float> &parameters)
{
    if (exp.empty())
    {
        return 0;
    }

    /*QString dump;
    for(const Token& t: exp)
    {
        switch(t.type)
        {
        case Token::TOKEN_NUMBER:
            dump += "N";
            break;
        case Token::TOKEN_RANGE:
            dump += "R";
            break;
        case Token::TOKEN_OPEN_PAR:
            dump += "(";
            break;
        case Token::TOKEN_CLOSE_PAR:
            dump += ")";
            break;
        case Token::TOKEN_OP_PLUS:
            dump += "+";
            break;
        case Token::TOKEN_OP_MINUS:
            dump += "-";
            break;
        case Token::TOKEN_OP_DIV:
            dump += "/";
            break;
        case Token::TOKEN_OP_MULTIPLY:
            dump += "*";
            break;
        case Token::TOKEN_OP_MOD:
            dump += "%";
            break;
        case Token::TOKEN_EQUAL:
            dump += "=";
            break;
        case Token::TOKEN_MORE:
            dump += ">";
            break;
        case Token::TOKEN_LESS:
            dump += "<";
            break;
        case Token::TOKEN_MORE_EQUAL:
            dump += ">=";
            break;
        case Token::TOKEN_LESS_EQUAL:
            dump += "<=";
            break;
        case Token::TOKEN_NOT_EQUAL:
            dump += "!=";
            break;
        case Token::TOKEN_OR:
            dump += "||";
            break;
        case Token::TOKEN_AND:
            dump += "&&";
            break;
        case Token::TOKEN_NOT:
            dump += "!";
            break;
        case Token::TOKEN_TO:
            dump += "T";
            break;
        case Token::TOKEN_IN:
            dump += "I";
            break;
        case Token::TOKEN_PARAMETER:
            dump += "P";
            break;
        case Token::TOKEN_LIST:
            dump += "L";
            break;
        case Token::TOKEN_OP_INT_DIV:
            dump += "i/";
            break;
        }
    }
    qDebug() << dump;*/

    QStack<Token> opStack;
    for (const Token &t : exp)
    {
        if ((t.type == Token::TOKEN_NUMBER) || (t.type == Token::TOKEN_RANGE) || (t.type == Token::TOKEN_PARAMETER) ||
            (t.type == Token::TOKEN_LIST))
        {
            opStack.push(t);
        }
        else
        {
            // Malformed expression: an operator without enough operands.
            if (opStack.size() < 2)
            {
                qWarning() << "Malformed expression: not enough operands for operator.";
                return 0;
            }
            Token b = opStack.top();
            opStack.pop();
            Token a = opStack.top();
            opStack.pop();
            opStack.push(t.apply(a, b, parameters));
        }
    }

    if (opStack.isEmpty())
    {
        return 0;
    }

    Token r = opStack.top();

    if (r.type == Token::TOKEN_PARAMETER)
    {
        return parameters.value(r.value.id);
    }
    else if (r.type == Token::TOKEN_RANGE)
    {
        return randomInRange(r.value.from, r.value.to);
    }
    else if (r.type == Token::TOKEN_LIST)
    {
        return r.list.value(rand() % r.list.count());
    }
    else
    {
        return r.value.number;
    }
}

float eval(const QString &exp, const QMap<uint32_t, float> &parameters)
{
    return eval(tokenize(exp), parameters);
}

} // namespace QM
} // namespace OpenSR
