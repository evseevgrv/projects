#include <stddef.h>

#include <stdexcept>

template <typename T, bool IsConst>
class DequeIterator;

template <typename T>
class Deque {
 private:
  template <typename, bool>
  friend class DequeIterator;
  static const int64_t kCell_size_ = 32;  // elements in one cell
  T** deque_;
  int64_t size_ = 0;
  int64_t capacity_ = 0;     // amount of cells
  int64_t start_ = 0;        // cell
  int64_t start_point_ = 0;  // index in cell

  void Swap(Deque& another) {
    std::swap(capacity_, another.capacity_);
    std::swap(size_, another.size_);
    std::swap(start_, another.start_);
    std::swap(start_point_, another.start_point_);
    std::swap(deque_, another.deque_);
  }

 public:
  using iterator = DequeIterator<T, false>;
  using const_iterator = DequeIterator<T, true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using reverse_const_iterator = std::reverse_iterator<const_iterator>;
  Deque() : deque_(new T*[1]), capacity_(3) {
    for (int i = 0; i < capacity_; ++i) {
      deque_[i] = reinterpret_cast<T*>(new char[sizeof(T) * kCell_size_]);
    }
  }
  Deque(const int64_t count)
      : deque_(new T*[3 * count / kCell_size_ + 3]),
        size_(count),
        capacity_(3 * count / kCell_size_ + 3) {
    for (int64_t i = 0; i < capacity_; ++i) {
      deque_[i] = reinterpret_cast<T*>(new char[sizeof(T) * kCell_size_]);
    }
    int64_t builded = 0;
    try {
      for (int64_t i = 0; i < capacity_; ++i) {
        for (int64_t j = 0; j < kCell_size_; ++j) {
          ++builded;
          new (deque_[i] + j) T();
        }
      }
    } catch (...) {
      for (int64_t j = 1; j < builded; ++j) {
        (deque_[start_ + (start_point_ + j) / kCell_size_] +
         (start_point_ + j) % kCell_size_)
            ->~T();
      }
      for (int64_t j = 0; j < capacity_; ++j) {
        delete[] reinterpret_cast<char*>(deque_[j]);
      }
      delete[] deque_;
      throw;
    }
  }

  Deque(int count, const T& value)
      : deque_(new T*[3 * count / kCell_size_ + 3]),
        size_(count),
        capacity_(3 * count / kCell_size_ + 3) {
    try {
      for (int64_t i = 0; i < capacity_; ++i) {
        deque_[i] = reinterpret_cast<T*>(new char[sizeof(T) * kCell_size_]);
      }
      for (int64_t i = 0; i < size_; ++i) {
        new (deque_[i / kCell_size_] + i % kCell_size_) T(value);
      }
    } catch (...) {
      for (int64_t i = 0; i < capacity_; ++i) {
        delete[] reinterpret_cast<char*>(deque_[i]);
      }
      delete[] deque_;
      throw;
    }
  }

  Deque(const Deque& another) {
    T** new_deque_ = new T*[another.capacity_];
    int64_t i = 0;
    int64_t new_capacity_ = another.capacity_;
    int64_t new_start_ = another.start_;
    int64_t new_start_point_ = another.start_point_;
    size_ = another.size_;
    capacity_ = new_capacity_;
    start_ = new_start_;
    start_point_ = new_start_point_;
    try {
      for (int64_t j = 0; j < new_capacity_; ++j) {
        new_deque_[j] = reinterpret_cast<T*>(new char[sizeof(T) * kCell_size_]);
      }
      for (; i < size_; ++i) {
        new (new_deque_[new_start_ + i / kCell_size_] + i % kCell_size_)
            T(*(another.deque_[another.start_ +
                               (another.start_point_ + i) / kCell_size_] +
                (another.start_point_ + i) % kCell_size_));
      }
    } catch (...) {
      for (int64_t j = 0; j < i; ++j) {
        (new_deque_[new_start_ + j / kCell_size_] + j % kCell_size_)->~T();
      }
      for (int64_t j = 0; j < new_capacity_; ++j) {
        delete[] reinterpret_cast<char*>(new_deque_[j]);
      }
      delete[] new_deque_;
      throw;
    }
    deque_ = new_deque_;
  }

  Deque& operator=(const Deque& another) {
    Deque temp(another);
    Swap(temp);
    return *this;
  }

  size_t size() const { return size_; }

  T& operator[](int64_t index) {
    return *(deque_[start_ + (index + start_point_) / kCell_size_] +
             (index + start_point_) % kCell_size_);
  }

  const T& operator[](int64_t index) const {
    return *(deque_[start_ + (index + start_point_) / kCell_size_] +
             (index + start_point_) % kCell_size_);
  }

  T& at(int64_t index) {
    if (index >= size_ || index < 0) {
      throw std::out_of_range("deque out of range");
    }
    return *(deque_[start_ + (index + start_point_) / kCell_size_] +
             (index + start_point_) % kCell_size_);
  }
  const T& at(int64_t index) const {
    if (index >= size_ || index < 0) {
      throw std::out_of_range("deque out of range");
    }
    return *(deque_[start_ + (index + start_point_) / kCell_size_] +
             (index + start_point_) % kCell_size_);
  }

  void push_back(const T& value) {
    if (start_ * kCell_size_ + start_point_ + size_ >=
        capacity_ * kCell_size_) {
      T** new_deque_ = new T*[capacity_ * 3 + 3];
      int64_t new_capacity_ = capacity_ * 3 + 3;
      int64_t new_start_ = capacity_;
      try {
        for (int64_t j = 0; j < new_capacity_; ++j) {
          new_deque_[j] =
              reinterpret_cast<T*>(new char[sizeof(T) * kCell_size_]);
        }
        std::copy(deque_ + start_, deque_ + start_ + size_ / kCell_size_,
                  new_deque_ + new_start_);
        *(new_deque_[new_start_ + size_ / kCell_size_] + size_ % kCell_size_) =
            T(value);
      } catch (...) {
        delete[] new_deque_;
        throw;
      }
      delete[] deque_;
      deque_ = new_deque_;
      capacity_ = new_capacity_;
      start_ = new_start_;
    } else {
      new (deque_[start_ + (start_point_ + size_) / kCell_size_] +
           (start_point_ + size_) % kCell_size_) T(value);
    }
    ++size_;
  };

  void pop_back() {
    --size_;
    (deque_[start_ + (size_ + start_point_) / kCell_size_] +
     (size_ + start_point_) % kCell_size_)
        ->~T();
  };

  void push_front(const T& value) {
    if (start_ == 0 && start_point_ == 0) {
      T** new_deque_ = new T*[capacity_ * 3];
      int64_t new_capacity_ = capacity_ * 3;
      int64_t new_start_ = capacity_;
      int64_t new_start_point_ = kCell_size_ - 1;
      try {
        for (int64_t j = 0; j < new_capacity_; ++j) {
          new_deque_[j] =
              reinterpret_cast<T*>(new char[sizeof(T) * kCell_size_]);
        }
        new (new_deque_[new_start_] + new_start_point_) T(value);
        std::copy(deque_, deque_ + capacity_, new_deque_ + capacity_ + 1);
      } catch (...) {
        delete[] new_deque_;
        throw;
      }
      delete[] deque_;
      deque_ = new_deque_;
      capacity_ = new_capacity_;
      start_ = new_start_;
      start_point_ = new_start_point_;
    } else {
      --start_point_;
      if (start_point_ < 0) {
        start_point_ = kCell_size_ - 1;
        --start_;
      }
      new (deque_[start_] + start_point_) T(value);
    }
    ++size_;
  };
  void pop_front() {
    --size_;
    (deque_[start_] + start_point_)->~T();
    ++start_point_;
    if (start_point_ >= kCell_size_) {
      start_point_ = 0;
      ++start_;
    }
  };

  iterator begin() { return iterator(*this); }

  const_iterator begin() const { return const_iterator(*this); }

  const_iterator cbegin() const { return const_iterator(*this); }

  iterator end() {
    iterator it(*this);
    it += size_;
    return it;
  }
  const_iterator end() const {
    const_iterator it(*this);
    it += size_;
    return it;
  }
  const_iterator cend() {
    const_iterator it(*this);
    it += size_;
    return it;
  }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_const_iterator rbegin() const {
    return reverse_const_iterator(end());
  }
  reverse_const_iterator crbegin() const {
    return reverse_const_iterator(end());
  }

  reverse_iterator rend() { return reverse_iterator(begin()); }
  reverse_const_iterator rend() const {
    return reverse_const_iterator(begin());
  }
  reverse_const_iterator crend() const {
    return reverse_const_iterator(begin());
  }

  void insert(iterator it, const T& value) {
    iterator current = end();
    push_back(value);
    while (current != it) {
      std::swap(*current, *(current - 1));
      --current;
    }
  }

  void erase(iterator it) {
    int64_t shift = 0;
    iterator begin(*this);
    for (auto i = begin; i < end(); ++i) {
      if (i == it) {
        shift = 1;
      }
      auto index = i - begin;
      if (index != size_ - 1) {
        *(deque_[start_ + (start_point_ + index) / kCell_size_] +
          (start_point_ + index) % kCell_size_) =
            *(deque_[start_ + (start_point_ + index + shift) / kCell_size_] +
              (start_point_ + index + shift) % kCell_size_);
      }
    }
    (deque_[start_ + (start_point_ + size_ - 1) / kCell_size_] +
     (start_point_ + size_ - 1) % kCell_size_)
        ->~T();
    --size_;
  }

  ~Deque() {
    for (int64_t j = 0; j < size_; ++j) {
      (deque_[start_ + (start_point_ + j) / kCell_size_] +
       (start_point_ + j) % kCell_size_)
          ->~T();
    }
    for (int64_t j = 0; j < capacity_; ++j) {
      delete[] reinterpret_cast<char*>(deque_[j]);
    }
    delete[] deque_;
  };
};

template <typename T, bool IsConst>
class DequeIterator {
 private:
  static const int64_t kCell_size_ = 32;
  T** start_ptr_;
  T* start_point_ptr_;

 public:
  T** cell_ptr_;
  T* point_ptr_;
  using Type = typename std::conditional<IsConst, const T, T>::type;
  using difference_type = int64_t;
  using iterator_category = std::random_access_iterator_tag;
  using pointer = Type*;
  using reference = Type&;
  using value_type = Type;
  DequeIterator(const Deque<T>& deque)
      : start_ptr_(deque.deque_ + deque.start_),
        start_point_ptr_(*(deque.deque_ + deque.start_) + deque.start_point_),
        cell_ptr_(deque.deque_ + deque.start_),
        point_ptr_(*(deque.deque_ + deque.start_) + deque.start_point_) {}

  DequeIterator(const DequeIterator<T, false>& other)
      : start_ptr_(other.start_ptr_),
        start_point_ptr_(other.start_point_ptr_),
        cell_ptr_(other.cell_ptr_),
        point_ptr_(other.point_ptr_) {}

  ~DequeIterator() {}

  DequeIterator& operator++() {
    ++point_ptr_;
    int64_t gap = point_ptr_ - *cell_ptr_;
    if (gap >= kCell_size_) {
      ++cell_ptr_;
      point_ptr_ = *cell_ptr_;
    }
    return *this;
  }

  DequeIterator operator++(int) {
    auto temp = *this;
    ++point_ptr_;
    int64_t gap = point_ptr_ - *cell_ptr_;
    if (gap >= kCell_size_) {
      ++cell_ptr_;
      point_ptr_ = *cell_ptr_;
    }
    return temp;
  }

  DequeIterator& operator--() {
    --point_ptr_;
    int64_t gap = point_ptr_ - *cell_ptr_;
    if (gap < 0) {
      --cell_ptr_;
      point_ptr_ = *cell_ptr_ + kCell_size_ - 1;
    }
    return *this;
  }

  DequeIterator operator--(int) {
    auto temp = this;
    --point_ptr_;
    int64_t gap = point_ptr_ - *cell_ptr_;
    if (gap < 0) {
      --cell_ptr_;
      point_ptr_ = *cell_ptr_ + kCell_size_ - 1;
    }
    return temp;
  }

  DequeIterator& operator+=(const int64_t& value) {
    if (value == 0) {
      return *this;
    }
    int64_t shift = kCell_size_ * (cell_ptr_ - start_ptr_) +
                    (point_ptr_ - *cell_ptr_) -
                    (start_point_ptr_ - *start_ptr_);
    shift += value;
    cell_ptr_ =
        start_ptr_ + (shift + (start_point_ptr_ - *start_ptr_)) / kCell_size_;

    point_ptr_ =
        *cell_ptr_ + (shift + (start_point_ptr_ - *start_ptr_)) % kCell_size_;
    return *this;
  }

  DequeIterator& operator-=(const int64_t& value) { return *this += -value; }

  reference operator*() { return *point_ptr_; }

  pointer operator->() { return point_ptr_; }
};

template <typename T, bool IsConst>
bool operator==(const DequeIterator<T, IsConst>& first,
                const DequeIterator<T, IsConst>& second) {
  return (first.cell_ptr_ == second.cell_ptr_) &&
         (first.point_ptr_ == second.point_ptr_);
}

template <typename T, bool IsConst>
bool operator!=(const DequeIterator<T, IsConst>& first,
                const DequeIterator<T, IsConst>& second) {
  return !(first == second);
}

template <typename T, bool IsConst>
bool operator<(const DequeIterator<T, IsConst>& first,
               const DequeIterator<T, IsConst>& second) {
  return (first.cell_ptr_ < second.cell_ptr_) ||
         ((first.cell_ptr_ == second.cell_ptr_) &&
          (first.point_ptr_ < second.point_ptr_));
}

template <typename T, bool IsConst>
bool operator>(const DequeIterator<T, IsConst>& first,
               const DequeIterator<T, IsConst>& second) {
  return (second < first);
}

template <typename T, bool IsConst>
bool operator<=(const DequeIterator<T, IsConst>& first,
                const DequeIterator<T, IsConst>& second) {
  return (first < second) || (first == second);
}

template <typename T, bool IsConst>
bool operator>=(const DequeIterator<T, IsConst>& first,
                const DequeIterator<T, IsConst>& second) {
  return (first > second) || (first == second);
}

template <typename T, bool IsConst>
DequeIterator<T, IsConst> operator+(const DequeIterator<T, IsConst>& iterator,
                                    const int64_t value) {
  DequeIterator<T, IsConst> temp(iterator);
  temp += value;
  return temp;
}

template <typename T, bool IsConst>
DequeIterator<T, IsConst> operator-(const DequeIterator<T, IsConst>& iterator,
                                    const int64_t value) {
  DequeIterator<T, IsConst> temp(iterator);
  if (value < 0) {
    temp += value;
  } else {
    temp += -value;
  }
  return temp;
}

template <typename T, bool IsConst>
int64_t operator-(const DequeIterator<T, IsConst>& first,
                  const DequeIterator<T, IsConst>& second) {
  return 32 * (first.cell_ptr_ - second.cell_ptr_) +
         (first.point_ptr_ - *first.cell_ptr_) -
         (second.point_ptr_ - *second.cell_ptr_);
}
