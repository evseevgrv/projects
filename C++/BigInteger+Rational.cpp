#include <iostream>
#include <string>
#include <vector>

class BigInteger {
 private:
  static const int64_t k_big = 1000000000;
  int sign_ = 0;  // 1: positive, 0: null, -1: negative
  std::vector<int64_t> data_;

 public:
  BigInteger() = default;
  BigInteger(const BigInteger& a) = default;
  BigInteger(int64_t a) {
    if (a < 0) {
      sign_ = -1;
      a *= -1;
    } else if (a > 0) {
      sign_ = 1;
    } else {
      sign_ = 0;
      data_ = {0};
    }
    while (a != 0) {
      data_.push_back(a % k_big);
      a /= k_big;
    }
  }
  BigInteger(const std::string& str) {
    sign_ = 1;
    int64_t index = 0;
    if (str[0] == '-') {
      sign_ = -1;
      ++index;
    }
    if (str[index] == '0') {
      sign_ = 0;
      data_ = {0};
      return;
    }
    int64_t amount = (str.length() - index) / 9;
    int64_t cur = 0;
    for (int64_t i = str.length() - 1; i > index; i -= 9) {
      cur = 0;
      if (i < 8) {
        break;
      }
      for (int64_t j = 8; j >= 0; --j) {
        cur *= 10;
        cur += str[i - j] - '0';
      }
      data_.push_back(cur);
    }
    if ((str.length() - index) % 9 != 0) {
      cur = 0;
      for (size_t i = index; i + amount * 9 < str.length(); ++i) {
        cur *= 10;
        cur += str[i] - '0';
      }
      data_.push_back(cur);
    }
  }
  BigInteger& operator=(const BigInteger& a) = default;
  std::string toString() const {
    std::string str = "";
    if (sign_ == -1) {
      str = "-";
    }
    for (size_t i = 0; i < data_.size(); ++i) {
      std::string cur = std::to_string(data_[data_.size() - 1 - i]);
      if (i > 0) {
        while (cur.length() < 9) {
          cur = '0' + cur;
        }
      }
      str += cur;
    }
    return str;
  }
  explicit operator bool() const { return (sign_ != 0); }
  size_t size() const { return data_.size(); }
  int sign() const { return sign_; }
  const int64_t& operator[](size_t pos) const { return data_[pos]; }
  BigInteger& operator+=(const BigInteger& b);
  BigInteger& operator-=(const BigInteger& b);
  BigInteger& operator*=(const BigInteger& b);
  BigInteger& operator/=(const BigInteger& b);
  BigInteger& operator%=(const BigInteger& b);
  BigInteger& operator++();
  BigInteger& operator--();
  BigInteger operator++(int);
  BigInteger operator--(int);
  BigInteger operator-() const;
};

BigInteger operator+(const BigInteger& a, const BigInteger& b) {
  BigInteger c(a);
  c += b;
  return c;
}
BigInteger operator-(const BigInteger& a, const BigInteger& b) {
  BigInteger c(a);
  c -= b;
  return c;
}
BigInteger operator*(const BigInteger& a, const BigInteger& b) {
  BigInteger c(a);
  c *= b;
  return c;
}
BigInteger operator/(const BigInteger& a, const BigInteger& b) {
  BigInteger c(a);
  c /= b;
  return c;
}
BigInteger operator%(const BigInteger& a, const BigInteger& b) {
  BigInteger c(a);
  c %= b;
  return c;
}

std::ostream& operator<<(std::ostream& os, const BigInteger& a) {
  os << a.toString();
  return os;
}
std::istream& operator>>(std::istream& is, BigInteger& a) {
  std::string str;
  is >> str;
  a = BigInteger(str);
  return is;
}

bool operator==(const BigInteger& a, const BigInteger& b) {
  if ((a.size() != b.size()) || (a.sign() != b.sign())) {
    return false;
  }
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}
bool operator!=(const BigInteger& a, const BigInteger& b) { return !(a == b); }
bool operator<(const BigInteger& a, const BigInteger& b) {
  if (a == b) {
    return false;
  }
  if (a.sign() > b.sign()) {
    return false;
  }
  if (a.sign() < b.sign()) {
    return true;
  }
  if (a.size() > b.size()) {
    if (a.sign() != -1) {
      return false;
    } else {
      return true;
    }
  }
  if (a.size() < b.size()) {
    if (a.sign() != -1) {
      return true;
    } else {
      return false;
    }
  }
  bool flag = false;
  for (int64_t i = a.size() - 1; i >= 0; --i) {
    if ((a[i] > b[i]) && (!flag)) {
      return (a.sign() == -1);
    }
    if (a[i] < b[i]) {
      flag = true;
    }
  }
  if (a.sign() == -1) {
    return (!flag);
  }
  return flag;
}
bool operator>(const BigInteger& a, const BigInteger& b) { return (b < a); }
bool operator>=(const BigInteger& a, const BigInteger& b) { return !(a < b); }
bool operator<=(const BigInteger& a, const BigInteger& b) { return !(b < a); }

BigInteger& BigInteger::operator+=(const BigInteger& b) {
  if (sign_ == 1 && b.sign_ == -1) {
    BigInteger c(b);
    c.sign_ = 1;
    *this -= c;
    return *this;
  }
  if (sign_ == -1 && b.sign_ == 1) {
    BigInteger c(b);
    sign_ = 1;
    c -= *this;
    *this = c;
    return *this;
  }
  if (sign_ == 0) {
    *this = b;
    return *this;
  }
  if (b.sign_ == 0) {
    return *this;
  }
  size_t i = 0;
  int64_t cur = 0;
  int64_t extra = 0;
  while (i < data_.size() && i < b.size()) {
    cur = data_[i] + b[i] + extra;
    extra = cur / k_big;
    data_[i] = cur % k_big;
    ++i;
  }
  while (i < b.size()) {
    cur = b[i] + extra;
    extra = cur / k_big;
    data_.push_back(cur % k_big);
    ++i;
  }
  while (i < data_.size()) {
    cur = data_[i] + extra;
    extra = cur / k_big;
    data_[i] = cur % k_big;
    ++i;
  }
  while (extra != 0) {
    data_.push_back(extra % k_big);
    extra /= k_big;
  }
  return *this;
}
BigInteger& BigInteger::operator-=(const BigInteger& b) {
  if (b.sign_ == 0) {
    return *this;
  }
  if (sign_ == 0) {
    *this = -b;
    return *this;
  }
  if (sign_ == 1 && b.sign_ == -1) {
    BigInteger c(-b);
    *this += c;
    return *this;
  }
  if (sign_ == -1 && b.sign_ == 1) {
    sign_ = 1;
    *this += b;
    sign_ = -1;
    return *this;
  }
  if (sign_ == -1 && b.sign_ == -1) {
    sign_ = 1;
    BigInteger c(-b);
    c -= *this;
    *this = c;
    return *this;
  }
  BigInteger k(b);
  if (*this < k) {
    k -= *this;
    *this = -k;
    return *this;
  }
  if (*this == b) {
    *this = 0;
    return *this;
  }
  std::vector<int64_t> ans;
  for (size_t i = 0; i < data_.size(); ++i) {
    if (i >= b.size()) {
      ans.push_back(data_[i]);
      continue;
    }
    if (data_[i] < b[i]) {
      int64_t additional = 1;
      while (data_[i + additional] == 0) {
        data_[i + additional] = k_big - 1;
        ++additional;
      }
      --data_[i + additional];
      data_[i] += k_big;
    }
    ans.push_back(data_[i] - b[i]);
  }
  while (ans[ans.size() - 1] == 0) {
    ans.pop_back();
    if (ans.size() == 0) {
      *this = 0;
      return *this;
    }
  }
  data_ = ans;
  return *this;
}
BigInteger& BigInteger::operator*=(const BigInteger& b) {
  sign_ *= b.sign();
  std::vector<int64_t> ans = {0};
  size_t index = 0;
  for (size_t i = 0; i < b.size(); ++i) {
    for (size_t j = 0; j < data_.size(); ++j) {
      index = i + j;
      while (index > ans.size() - 1) {
        ans.push_back(0);
      }
      int64_t cur = b[i] * data_[j];
      ans[index] += cur;
      if (ans[index] >= k_big) {
        while (index + 2 > ans.size()) {
          ans.push_back(0);
        }
        ans[index + 1] += ans[index] / k_big;
        ans[index] %= k_big;
      }
    }
  }
  while (ans[index] >= k_big) {
    ans.push_back(ans[index] / k_big);
    ans[index] %= k_big;
    ++index;
  }
  while (ans[ans.size() - 1] == 0) {
    ans.pop_back();
    if (ans.size() == 0) {
      break;
    }
  }
  if (ans.size() == 0) {
    *this = 0;
    return *this;
  }
  data_ = ans;
  return *this;
}
BigInteger& BigInteger::operator/=(const BigInteger& c) {
  BigInteger b(c);
  b.sign_ = 1;
  int new_sign = sign_ * c.sign();
  sign_ = 1;
  BigInteger ans(0);
  BigInteger cur(0);
  int64_t index = data_.size() - 1;
  while (index >= 0) {
    cur *= k_big;
    cur += data_[index];
    int64_t left = 0;
    int64_t right = k_big;
    bool flag = true;
    while (left < right) {
      int64_t middle = (left + right) / 2;
      if (middle == left) {
        break;
      }
      if (b * middle < cur) {
        left = middle;
      } else if (b * middle == cur) {
        ans *= k_big;
        ans += middle;
        cur = 0;
        flag = false;
        break;
      } else {
        right = middle;
      }
    }
    if (flag) {
      ans *= k_big;
      ans += left;
      cur -= b * left;
    }
    --index;
  }
  *this = ans;
  if (ans != 0) {
    sign_ = new_sign;
  }
  return *this;
}
BigInteger& BigInteger::operator%=(const BigInteger& b) {
  BigInteger c = *this / b;
  *this -= b * c;
  return *this;
}
BigInteger& BigInteger::operator++() {
  *this += 1;
  return *this;
}
BigInteger& BigInteger::operator--() {
  *this -= 1;
  return *this;
}
BigInteger BigInteger::operator++(int) {
  BigInteger a(*this);
  *this += 1;
  return a;
}
BigInteger BigInteger::operator--(int) {
  BigInteger a(*this);
  *this -= 1;
  return a;
}
BigInteger BigInteger::operator-() const {
  BigInteger c(*this);
  c.sign_ *= -1;
  return c;
}

BigInteger operator"" _bi(unsigned long long a) { return BigInteger(a); }

class Rational {
 private:
  int sign_ = 0;
  BigInteger numerator_ = 0;
  BigInteger denominator_ = 1;

 public:
  Rational() = default;
  Rational(const int& a) {
    if (a > 0) {
      sign_ = 1;
      numerator_ = a;
    } else if (a < 0) {
      sign_ = -1;
      numerator_ = -a;
    } else {
      numerator_ = 0;
    }
    denominator_ = 1;
  }
  Rational(const Rational& other) = default;
  Rational(const BigInteger& a, const BigInteger& b = 1) {
    if (a < 0) {
      sign_ = -1;
      numerator_ = -a;
    } else if (a > 0) {
      sign_ = 1;
      numerator_ = a;
    } else {
      sign_ = 0;
      numerator_ = 0;
    }
    if (b < 0) {
      sign_ *= -1;
      denominator_ = -b;
    } else {
      denominator_ = b;
    }
  }
  Rational& operator=(const Rational& other) = default;
  int sign() const { return sign_; }
  BigInteger numerator() const { return numerator_; }
  BigInteger denominator() const { return denominator_; }
  void CheckSign();
  void Simplify();
  std::string toString();
  Rational& operator+=(const Rational& b);
  Rational& operator-=(const Rational& b);
  Rational& operator*=(const Rational& b);
  Rational& operator/=(const Rational& b);
  Rational operator-() const {
    Rational c(*this);
    c *= -1;
    return c;
  }
  std::string asDecimal(size_t precision = 0) {
    CheckSign();
    Simplify();
    std::string ans = "";
    if (sign_ == -1) {
      ans += '-';
    }
    ans += (numerator_ / denominator_).toString();
    if (precision == 0) {
      return ans;
    }
    ans += '.';
    BigInteger temp(numerator_ % denominator_);
    for (size_t i = 0; i + 1 < precision; ++i) {
      temp *= 10;
      ans += (temp / denominator_).toString();
      temp %= denominator_;
    }
    temp *= 10;
    BigInteger res(temp / denominator_);
    temp %= denominator_;
    temp *= 10;
    BigInteger next = temp / denominator_;
    if (next >= 5) {
      ++res;
    }
    ans += res.toString();
    return ans;
  }
  explicit operator double() { return (std::stod(asDecimal(10))); }
};
void Rational::CheckSign() {
  if (numerator_ == 0) {
    *this = 0;
    return;
  }
  if (sign_ == 0) {
    sign_ = numerator_.sign() * denominator_.sign();
  } else {
    sign_ *= numerator_.sign() * denominator_.sign();
  }
  if (numerator_ < 0) {
    numerator_ *= -1;
  }
  if (denominator_ < 0) {
    denominator_ *= -1;
  }
}
void Rational::Simplify() {
  BigInteger a(numerator_);
  BigInteger b(denominator_);
  BigInteger r;
  while (true) {
    if (a > b) {
      a %= b;
    } else {
      b %= a;
    }
    if (a == 0) {
      r = b;
      break;
    }
    if (b == 0) {
      r = a;
      break;
    }
  }
  numerator_ /= r;
  denominator_ /= r;
}

Rational operator+(const Rational& a, const Rational& b) {
  Rational c(a);
  c += b;
  return c;
}
Rational operator-(const Rational& a, const Rational& b) {
  Rational c(a);
  c -= b;
  return c;
}
Rational operator*(const Rational& a, const Rational& b) {
  Rational c(a);
  c *= b;
  return c;
}
Rational operator/(const Rational& a, const Rational& b) {
  Rational c(a);
  c /= b;
  return c;
}

std::string Rational::toString() {
  CheckSign();
  Simplify();
  std::string str = "";
  if (sign_ == -1) {
    str = "-";
  }
  str += numerator_.toString();
  if (denominator_ != 1) {
    str += '/';
    str += denominator_.toString();
  }
  return str;
}

bool operator==(const Rational& a, const Rational& b) {
  return ((a.sign() == b.sign()) && ((a.numerator() * b.denominator()) ==
                                     (a.denominator() * b.numerator())));
}
bool operator!=(const Rational& a, const Rational& b) { return !(a == b); }
bool operator<(const Rational& a, const Rational& b) {
  if (a == b) {
    return false;
  }
  if (a.sign() < b.sign()) {
    return true;
  }
  if (a.sign() > b.sign()) {
    return false;
  }
  BigInteger r = a.numerator() * b.denominator();
  BigInteger q = b.numerator() * a.denominator();
  if (a.sign() == -1) {
    return (r > q);
  } else {
    return (r < q);
  }
}
bool operator>(const Rational& a, const Rational& b) { return (b < a); }
bool operator>=(const Rational& a, const Rational& b) { return !(a < b); }
bool operator<=(const Rational& a, const Rational& b) { return !(b < a); }

Rational& Rational::operator+=(const Rational& summand) {
  if ((sign_ == 1) && (summand.sign_ == -1)) {
    Rational c(summand);
    c *= -1;
    *this -= c;
    return *this;
  }
  if ((sign_ == -1) && (summand.sign_ == 1)) {
    Rational c(summand);
    sign_ = 1;
    c -= *this;
    *this = c;
    return *this;
  }
  if (sign_ == 0) {
    *this = summand;
    return *this;
  }
  numerator_ *= summand.denominator_;
  numerator_ += summand.numerator_ * denominator_;
  denominator_ *= summand.denominator_;
  CheckSign();
  return *this;
}
Rational& Rational::operator-=(const Rational& subtrahend) {
  if ((sign_ == 1) && (subtrahend.sign_ == -1)) {
    Rational c(subtrahend);
    c *= -1;
    *this += c;
    return *this;
  }
  if ((sign_ == -1) && (subtrahend.sign_ == 1)) {
    sign_ = 1;
    *this += subtrahend;
    sign_ = -1;
    return *this;
  }
  if ((sign_ == -1) && (subtrahend.sign_ == -1)) {
    Rational c(-subtrahend);
    sign_ = 1;
    c -= *this;
    *this = c;
    return *this;
  }
  if (sign_ == 0) {
    *this = subtrahend;
    *this *= -1;
    return *this;
  }
  numerator_ *= subtrahend.denominator_;
  numerator_ -= subtrahend.numerator_ * denominator_;
  denominator_ *= subtrahend.denominator_;
  CheckSign();
  return *this;
}
Rational& Rational::operator*=(const Rational& multiplier) {
  if (multiplier == 0) {
    *this = 0;
    return *this;
  }
  if (*this == 0) {
    return *this;
  }
  sign_ *= multiplier.sign_;
  numerator_ *= multiplier.numerator_;
  denominator_ *= multiplier.denominator_;
  CheckSign();
  return *this;
}
Rational& Rational::operator/=(const Rational& divider) {
  sign_ *= divider.sign();
  numerator_ *= divider.denominator_;
  denominator_ *= divider.numerator_;
  CheckSign();
  return *this;
}
