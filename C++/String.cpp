#include <algorithm>
#include <cstring>
#include <iostream>

class String {
 private:
  size_t size_ = 0;
  size_t capacity_ = 0;
  char* elements = nullptr;

 public:
  String(){};

  String(char symb) : size_(1), capacity_(1), elements(new char[2]) {
    memset(elements, symb, 1);
    elements[1] = '\0';
  }

  String(size_t n, char symb)
      : size_(n), capacity_(n), elements(new char[n + 1]) {
    std::memset(elements, symb, n);
    elements[n] = '\0';
  }

  String(const String& str) {
    size_ = str.size_;
    capacity_ = str.size_;
    elements = new char[size_ + 1];
    std::copy(str.elements, str.elements + str.size_, elements);
    elements[size_] = '\0';
  }

  String(const char* str) {
    size_t length = strlen(str);
    elements = new char[length + 1];
    std::copy(str + 0, str + length, elements);
    size_ = length;
    capacity_ = length;
    elements[size_] = '\0';
  }

  String(char* el, size_t sz)
      : size_(sz), capacity_(sz), elements(new char[sz + 1]) {
    std::copy(el, el + sz, elements);
    elements[size_] = '\0';
  }
  String& operator=(const String& other) {
    String tmp(other);
    std::swap(elements, tmp.elements);
    std::swap(capacity_, tmp.capacity_);
    std::swap(size_, tmp.size_);
    return *this;
  }

  size_t length() const { return size_; }
  size_t capacity() const { return capacity_; }
  size_t size() const { return size_; }
  char* str() { return elements; }
  char& operator[](size_t index) { return elements[index]; }
  const char& operator[](size_t index) const { return elements[index]; }

  void push_back(char symb) {
    if (size_ == capacity_) {
      capacity_ = 2 * capacity_ + 1;
      char* new_string = new char[capacity_ + 1];
      std::copy(elements + 0, elements + size_, new_string);
      delete[] elements;
      elements = new_string;
    }
    elements[size_++] = symb;
    elements[size_] = '\0';
  }

  void pop_back() { elements[size_--] = '\0'; }

  char& front() { return elements[0]; }
  const char& front() const { return elements[0]; }
  char& back() { return elements[size_ - 1]; }
  const char& back() const { return elements[size_ - 1]; }

  String& operator+=(const char symb) {
    push_back(symb);
    return *this;
  }
  String& operator+=(const String& str) {
    if (capacity_ < str.size_ + size_) {
      capacity_ = std::max(str.size_ + size_ + 1, 2 * capacity_ + 1);
      char* new_string = new char[capacity_ + 1];
      std::copy(elements, elements + size_, new_string);
      delete[] elements;
      elements = new_string;
    }
    std::copy(str.elements, str.elements + str.size_, elements + size_);
    size_ += str.size_;
    elements[size_] = '\0';
    return *this;
  }
  size_t find(const String& str) const {
    bool flag = false;
    size_t index = 0;
    for (size_t i = 0; i < size_; ++i) {
      if (i + str.size_ > size_) {
        return length();
      }
      if (elements[i] == str[0]) {
        index = i;
        flag = true;
        for (size_t j = i + 1; j < i + str.size_; ++j) {
          if (j >= size_ || elements[j] != str[j - i]) {
            flag = false;
            break;
          }
        }
        if (flag) return index;
      }
    }
    if (flag) return index;
    return length();
  }
  size_t rfind(const String& str) const {
    bool flag = false;
    if (size_ == 0) {
      return length();
    }
    size_t index = size_ - 1;
    size_t i = size_;
    do {
      --i;
      if (elements[i] == str[str.size_ - 1]) {
        index = i;
        flag = true;
        if (i < str.size_ - 1) {
          flag = false;
          break;
        }
        for (size_t j = 1; j < str.size_; ++j) {
          if (elements[i - j] != str[str.size_ - 1 - j]) {
            flag = false;
            break;
          }
        }
        if (flag) {
          return index - str.size_ + 1;
        }
      }
    } while (i != 0);
    if (flag) {
      return index - str.size_ + 1;
    }
    return length();
  }
  String substr(size_t start, size_t count) const {
    return String(elements + start, count);
  }

  bool empty() { return size_ == 0; }
  void clear() {
    size_ = 0;
    elements[0] = '\0';
  }
  void shrink_to_fit() {
    char* new_elements = new char[size_ + 1];
    std::copy(elements + 0, elements + size_, new_elements);
    delete[] elements;
    capacity_ = size_;
    elements = new_elements;
  }

  char* data() { return elements + 0; }
  const char* data() const { return elements + 0; }
  ~String() { delete[] elements; }
  friend std::ostream& operator<<(std::ostream& os, const String& a);
};

bool operator==(const String& a, const String& b) {
  if (a.length() != b.length()) {
    return false;
  }
  size_t number = a.length();
  for (size_t i = 0; i < number; ++i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}
bool operator!=(const String& str_a, const String& str_b) {
  return !(str_a == str_b);
}
bool operator<(const String& str_a, const String& str_b) {
  for (size_t i = 0; i < str_a.length(); ++i) {
    if ((i >= str_b.length() && i < str_a.length()) || str_a[i] > str_b[i]) {
      return false;
    }
  }
  return true;
}
bool operator>=(const String& str_a, const String& str_b) {
  return !(str_a < str_b);
}
bool operator>(const String& str_a, const String& str_b) {
  return str_b < str_a;
}
bool operator<=(const String& str_a, const String& str_b) {
  return !(str_b < str_a);
}
String operator+(const String& str_a, const String& str_b) {
  String new_string(str_a);
  new_string += str_b;
  return new_string;
}
std::ostream& operator<<(std::ostream& os, const String& a) {
  os << a.elements;
  return os;
}
std::istream& operator>>(std::istream& os, String& a) {
  char symb;
  String b;
  while (os.get(symb) && !(std::isspace(symb))) {
    b.push_back(symb);
  }
  a = b;
  return os;
}
