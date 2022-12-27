#ifndef CONSTEXPR_STD_HPP
#define CONSTEXPR_STD_HPP

#include <string>

constexpr int stoi(const std::string& str)
{
    // TODO: implement with from_chars with g++13
    int ret = 0;

    if (str.length() == 0) return ret;

    bool negative = str[0] == '-';
    auto start_it = negative ? str.begin() + 1 : str.begin();

    for (auto num = start_it; num != str.end(); num++)
    {
        ret *= 10;
        ret += *num - '0';
    }

    if (negative) ret *= -1;

    return ret;
}

constexpr long stol(const std::string& str)
{
    long ret = 0;

    if (str.length() == 0) return ret;

    bool negative = str[0] == '-';
    auto start_it = negative ? str.begin() + 1 : str.begin();

    for (auto num = start_it; num != str.end(); num++)
    {
        ret *= 10;
        ret += *num - '0';
    }

    if (negative) ret *= -1;

    return ret;
}

constexpr double stod(const std::string& str)
{
    double ret = 0;

    if (str.length() == 0) return ret;

    bool negative = str[0] == '-';
    auto start_it = negative ? str.begin() + 1 : str.begin();

    bool full_part = true;
    for (auto num = start_it; num != str.end(); num++)
    {
        if (*num == '.')
        {
            full_part = false;
            continue;
        }
        if (full_part)
        {
            ret *= 10;
            ret += *num - '0';
        }
        else
        {
            static unsigned counter = 10;
            ret += (*num - '0') / counter;
            counter *= 10;
        }
    }

    if (negative) ret *= -1;

    return ret;
}

#endif  // CONSTEXPR_STD_HPP
