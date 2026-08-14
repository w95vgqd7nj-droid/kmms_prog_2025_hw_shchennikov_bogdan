#include "long_number.h"

using biv::LongNumber;

LongNumber::LongNumber() {
    length = 1;
    sign = 1;
    numbers = new int[1];
    numbers[0] = 0;
}

LongNumber::LongNumber(int length, int sign) {
    this->length = length;
    this->sign = sign;
    numbers = new int[length];
    for (int i = 0; i < length; ++i) {
        numbers[i] = 0;
    }
}

LongNumber::LongNumber(const char* const str) {
    int start_index = 0;
    if (str[0] == '-') {
        sign = -1;
        start_index = 1;
    } else if (str[0] == '+') {
        sign = 1;
        start_index = 1;
    } else {
        sign = 1;
    }

    length = get_length(str);
    if (length == 0) {
        length = 1;
        sign = 1;
        numbers = new int[1]{0};
        return;
    }

    numbers = new int[length];
    for (int i = 0; i < length; ++i) {
        numbers[i] = str[start_index + i] - '0';
    }

    if (length == 1 && numbers[0] == 0) {
        sign = 1;
    }
}

LongNumber::LongNumber(const LongNumber& x) {
    length = x.length;
    sign = x.sign;
    numbers = new int[length];
    for (int i = 0; i < length; i++) {
        numbers[i] = x.numbers[i];
    }
}

LongNumber::LongNumber(LongNumber&& x) {
    length = x.length;
    sign = x.sign;
    numbers = x.numbers;

    x.length = 0;
    x.sign = 1;
    x.numbers = nullptr;
}

LongNumber::~LongNumber() {
    if (numbers != nullptr) {
        delete[] numbers;
        numbers = nullptr;
    }
}

LongNumber& LongNumber::operator = (const char* const str) {
    *this = LongNumber(str);
    return *this;
}

LongNumber& LongNumber::operator = (const LongNumber& x) {
    if (this == &x) return *this;

    delete[] numbers;

    length = x.length;
    sign = x.sign;
    numbers = new int[length];
    for (int i = 0; i < length; i++) {
        numbers[i] = x.numbers[i];
    }

    return *this;
}

LongNumber& LongNumber::operator = (LongNumber&& x) {
    if (this != &x) {
        delete[] numbers;

        numbers = x.numbers;
        length = x.length;
        sign = x.sign;

        x.numbers = nullptr;
        x.length = 0;
        x.sign = 1;
    }
    return *this;
}

bool LongNumber::operator == (const LongNumber& x) const {
    if (length != x.length || sign != x.sign) {
        return false;
    }
    for (int i = 0; i < length; i++) {
        if (numbers[i] != x.numbers[i]) {
            return false;
        }
    }
    return true;
}

bool LongNumber::operator != (const LongNumber& x) const {
    return !(*this == x);
}

bool LongNumber::operator > (const LongNumber& x) const {
    if (sign > x.sign) return true;
    if (sign < x.sign) return false;

    if (sign == 1) {
        if (length > x.length) return true;
        if (length < x.length) return false;

        for (int i = 0; i < length; i++) {
            if (numbers[i] > x.numbers[i]) return true;
            if (numbers[i] < x.numbers[i]) return false;
        }
    } else {
        if (length > x.length) return false;
        if (length < x.length) return true;

        for (int i = 0; i < length; i++) {
            if (numbers[i] > x.numbers[i]) return false;
            if (numbers[i] < x.numbers[i]) return true;
        }
    }

    return false;
}

bool LongNumber::operator < (const LongNumber& x) const {
    return !(*this > x) && !(*this == x);
}

LongNumber LongNumber::operator + (const LongNumber& x) const {
    if (sign != x.sign) {
        if (sign == -1) {
            LongNumber temp = *this;
            temp.sign = 1;
            return x - temp;
        } else {
            LongNumber temp = x;
            temp.sign = 1;
            return *this - temp;
        }
    }

    int max_len = (length > x.length ? length : x.length) + 1;
    LongNumber result(max_len, sign);

    int i = length - 1;
    int j = x.length - 1;
    int k = max_len - 1;
    int carry = 0;

    while (i >= 0 || j >= 0 || carry) {
        int sum = carry;
        if (i >= 0) sum += numbers[i--];
        if (j >= 0) sum += x.numbers[j--];

        result.numbers[k--] = sum % 10;
        carry = sum / 10;
    }

    int start = 0;
    while (start < max_len - 1 && result.numbers[start] == 0) {
        start++;
    }

    if (start > 0) {
        int new_len = max_len - start;
        LongNumber trimmed(new_len, sign);
        for (int idx = 0; idx < new_len; ++idx) {
            trimmed.numbers[idx] = result.numbers[start + idx];
        }
        return trimmed;
    }

    return result;
}

LongNumber LongNumber::operator - (const LongNumber& x) const {
    if (sign == -1 && x.sign == 1) {
        LongNumber a_abs = *this; a_abs.sign = 1;
        LongNumber res = a_abs + x; res.sign = -1;
        return res;
    }
    if (sign == 1 && x.sign == -1) {
        LongNumber b_abs = x; b_abs.sign = 1;
        return *this + b_abs;
    }
    if (sign == -1 && x.sign == -1) {
        LongNumber a_abs = *this; a_abs.sign = 1;
        LongNumber b_abs = x; b_abs.sign = 1;
        return b_abs - a_abs;
    }

    if (*this == x) {
        return LongNumber("0");
    }

    if (*this < x) {
        LongNumber res = x - *this;
        res.sign = -1;
        return res;
    }

    int max_len = length;
    LongNumber result(max_len, 1);

    int i = length - 1;
    int j = x.length - 1;
    int k = max_len - 1;
    int borrow = 0;

    while (i >= 0) {
        int diff = numbers[i] - borrow - (j >= 0 ? x.numbers[j] : 0);
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.numbers[k--] = diff;
        i--;
        j--;
    }

    int start = 0;
    while (start < max_len - 1 && result.numbers[start] == 0) {
        start++;
    }

    if (start > 0) {
        LongNumber trimmed(max_len - start, 1);
        for (int idx = 0; idx < trimmed.length; ++idx) {
            trimmed.numbers[idx] = result.numbers[start + idx];
        }
        return trimmed;
    }

    return result;
}

LongNumber LongNumber::operator * (const LongNumber& x) const {
    if ((length == 1 && numbers[0] == 0) || (x.length == 1 && x.numbers[0] == 0)) {
        return LongNumber("0");
    }

    int len_c = length + x.length;
    LongNumber result(len_c, sign == x.sign ? 1 : -1);

    for (int i = length - 1; i >= 0; i--) {
        for (int j = x.length - 1; j >= 0; j--) {
            int pos = i + j + 1;
            result.numbers[pos] += numbers[i] * x.numbers[j];
        }
    }

    for (int i = len_c - 1; i > 0; i--) {
        if (result.numbers[i] >= 10) {
            result.numbers[i - 1] += result.numbers[i] / 10;
            result.numbers[i] %= 10;
        }
    }

    int start = 0;
    while (start < len_c - 1 && result.numbers[start] == 0) {
        start++;
    }

    if (start > 0) {
        LongNumber trimmed(len_c - start, result.sign);
        for (int i = 0; i < trimmed.length; i++) {
            trimmed.numbers[i] = result.numbers[start + i];
        }
        return trimmed;
    }

    return result;
}

LongNumber LongNumber::operator / (const LongNumber& x) const {
    if (x.length == 1 && x.numbers[0] == 0) {
        std::cout << "Ошибка! Деление на ноль." << std::endl;
        return LongNumber("0");
    }

    LongNumber dividend = *this; dividend.sign = 1;
    LongNumber divisor = x; divisor.sign = 1;

    LongNumber remainder("0");
    LongNumber quotient(length, sign == x.sign ? 1 : -1);
    int q_idx = 0;

    for (int i = 0; i < dividend.length; ++i) {
        if (remainder.length == 1 && remainder.numbers[0] == 0) {
            char buf[2] = { (char)(dividend.numbers[i] + '0'), '\0' };
            remainder = LongNumber(buf);
        } else {
            LongNumber temp(remainder.length + 1, 1);
            for (int j = 0; j < remainder.length; ++j) temp.numbers[j] = remainder.numbers[j];
            temp.numbers[remainder.length] = dividend.numbers[i];

            remainder = temp;
        }

        int digit = 0;
        for (int d = 9; d >= 0; --d) {
            char buf[2] = { (char)(d + '0'), '\0' };
            LongNumber product = divisor * LongNumber(buf);
            if (product < remainder || product == remainder) {
                digit = d;
                remainder = remainder - product;
                break;
            }
        }
        quotient.numbers[q_idx++] = digit;
    }

    int start = 0;
    while (start < q_idx - 1 && quotient.numbers[start] == 0) start++;

    LongNumber final_q(q_idx - start, quotient.sign);
    for (int i = 0; i < final_q.length; ++i) {
        final_q.numbers[i] = quotient.numbers[start + i];
    }

    if (final_q.length == 1 && final_q.numbers[0] == 0) {
        final_q.sign = 1;
    }

    if (sign == -1 && !(remainder.length == 1 && remainder.numbers[0] == 0)) {
        if (x.sign == -1) {
            final_q = final_q + LongNumber("1");
        } else {
            final_q = final_q - LongNumber("1");
        }
    }

    return final_q;
}

LongNumber LongNumber::operator % (const LongNumber& x) const {
    if (x.length == 1 && x.numbers[0] == 0) {
        std::cout << "Ошибка! Деление на ноль при взятии остатка." << std::endl;
        return LongNumber("0");
    }
    return *this - (*this / x) * x;
}

bool LongNumber::is_negative() const noexcept {
    return sign == -1;
}

// ----------------------------------------------------------
// PRIVATE
// ----------------------------------------------------------
int LongNumber::get_length(const char* const str) const noexcept {
    int len = 0;
    int i = 0;

    if (str[0] == '-' || str[0] == '+') {
        i = 1;
    }

    while (str[i] != '\0') {
        len++;
        i++;
    }

    return len;
}

// ----------------------------------------------------------
// FRIENDLY
// ----------------------------------------------------------
namespace biv {
    std::ostream& operator << (std::ostream &os, const LongNumber& x) {
        if (x.sign == -1 && !(x.length == 1 && x.numbers[0] == 0)) {
            os << '-';
        }
        for (int i = 0; i < x.length; i++) {
            os << x.numbers[i];
        }
        return os;
    }
}
