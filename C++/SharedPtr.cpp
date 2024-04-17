#include <iostream>
#include <memory>
#include <type_traits>

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename U, typename... Args>
SharedPtr<U> makeShared(Args&&... args);

template <typename U, typename Alloc, typename... Args>
SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);

template <typename T>
class EnableSharedFromThis;

struct BaseControlBlock {
  int shared_count;
  int weak_count;
  BaseControlBlock() : shared_count(1), weak_count(0) {}
  virtual void useDeleter() = 0;
  virtual void useDeleterForAll() = 0;
  virtual void* Object() = 0;
  virtual ~BaseControlBlock() = default;
  void IncreaseShared() { ++shared_count; }
  void IncreaseWeak() { ++weak_count; }
  void DecreaseShared() {
    --shared_count;
    if (shared_count == 0) {
      useDeleter();
      if (weak_count == 0) {
        useDeleterForAll();
      }
    }
  }
  void DecreaseWeak() {
    --weak_count;
    if (shared_count == 0 && weak_count == 0) {
      useDeleterForAll();
    }
  }
};

template <typename T, typename Deleter = std::default_delete<T>,
          typename Alloc = std::allocator<T>>
struct ControlBlockRegular : public BaseControlBlock {
  using AllocTraits = std::allocator_traits<Alloc>;
  using BlockAlloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
  using BlockTraits = typename std::allocator_traits<
      Alloc>::template rebind_traits<ControlBlockRegular<T, Deleter, Alloc>>;
  Deleter deleter;
  Alloc alloc;
  T* object;
  ControlBlockRegular(T* ptr) : object(ptr) {}
  ControlBlockRegular(Deleter deleter, Alloc alloc, T* object)
      : deleter(deleter), alloc(alloc), object(object) {}
  virtual void useDeleter() override { deleter(object); }
  virtual void useDeleterForAll() override {
    BlockAlloc block_alloc = alloc;
    BlockTraits::deallocate(block_alloc, this, 1);
  }

  virtual void* Object() override { return object; }
};

template <typename T, typename Alloc = std::allocator<T>>
struct ControlBlockMakeShared : public BaseControlBlock {
  using AllocTraits = std::allocator_traits<Alloc>;
  using BlockAlloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
  using BlockTraits = typename std::allocator_traits<
      Alloc>::template rebind_traits<ControlBlockMakeShared<T, Alloc>>;
  Alloc alloc;
  char object[sizeof(T)];

  template <typename... Args>
  ControlBlockMakeShared(Alloc alloc, Args&&... args) : alloc(alloc) {
    AllocTraits::construct(alloc, reinterpret_cast<T*>(object),
                           std::forward<Args>(args)...);
  }
  virtual void useDeleter() override {
    AllocTraits::destroy(alloc, reinterpret_cast<T*>(object));
  }
  virtual void useDeleterForAll() override {
    BlockAlloc block_alloc = alloc;
    BlockTraits::deallocate(block_alloc, this, 1);
  }
  virtual void* Object() override { return object; }
};

template <typename T>
class SharedPtr {
 private:
  template <typename>
  friend class WeakPtr;

  template <typename>
  friend class SharedPtr;

  template <typename U, typename... Args>
  friend SharedPtr<U> makeShared(Args&&... args);

  template <typename U, typename Alloc, typename... Args>
  friend SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);
  BaseControlBlock* cb;

  SharedPtr(const WeakPtr<T>& other) : cb(other.cb) {
    if (cb != nullptr) {
      cb->IncreaseShared();
    }
    SharedFromThis();
  }

 public:
  SharedPtr() : cb(nullptr) { SharedFromThis(); }

  SharedPtr(T* ptr) : cb(new ControlBlockRegular<T>(ptr)) {
    // cb = new ControlBlockRegular(reinterpret_cast<T*>(ptr));
    SharedFromThis();
  }

  SharedPtr(BaseControlBlock* cb) : cb(cb) { SharedFromThis(); }

  SharedPtr(const SharedPtr& other) : cb(other.cb) {
    if (cb != nullptr) {
      cb->IncreaseShared();
    }
    SharedFromThis();
  }

  SharedPtr(SharedPtr&& other) {
    cb = other.cb;
    other.cb = nullptr;
    SharedFromThis();
  }

  template <typename U>
  SharedPtr(const SharedPtr<U>& other) : cb(other.cb) {
    if (cb != nullptr) {
      cb->IncreaseShared();
    }
    SharedFromThis();
  }

  template <typename U>
  SharedPtr(SharedPtr<U>&& other) {
    cb = other.cb;
    other.cb = nullptr;
    SharedFromThis();
  }

  SharedPtr<T>& operator=(const SharedPtr<T>& other) {
    SharedPtr<T> copy(other);
    swap(copy);
    return *this;
  }

  template <typename U>
  SharedPtr<T>& operator=(const SharedPtr<U>& other) {
    SharedPtr<T> copy(other);
    swap(copy);
    return *this;
  }

  SharedPtr<T>& operator=(SharedPtr<T>&& other) {
    SharedPtr<T> copy(std::move(other));
    std::swap(cb, copy.cb);
    return *this;
  }

  template <typename U>
  SharedPtr<T> operator=(SharedPtr<U>&& other) {
    SharedPtr<T> copy(std::move(other));
    swap(copy);
    return *this;
  }

  template <typename U, typename Deleter, typename Alloc>
  SharedPtr(U* ptr, Deleter deleter, Alloc alloc) {
    using BlockAlloc = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
    using BlockTraits = typename std::allocator_traits<
        Alloc>::template rebind_traits<ControlBlockRegular<T, Deleter, Alloc>>;
    BlockAlloc block_alloc = alloc;
    cb = BlockTraits::allocate(block_alloc, 1);
    new (reinterpret_cast<ControlBlockRegular<T, Deleter, Alloc>*>(cb))
        ControlBlockRegular<T, Deleter, Alloc>(deleter, alloc,
                                               reinterpret_cast<T*>(ptr));
    SharedFromThis();
  }

  template <typename U, typename Deleter>
  SharedPtr(U* ptr, Deleter deleter)
      : SharedPtr(ptr, deleter, std::allocator<T>()) {}

  ~SharedPtr() {
    if (cb != nullptr) {
      cb->DecreaseShared();
    }
  }

  int use_count() const { return cb->shared_count; }
  void reset() {
    if (cb != nullptr) {
      cb->DecreaseShared();
    }
    cb = nullptr;
  }

  template <typename U>
  void reset(U* ptr) {
    if (cb != nullptr) {
      cb->DecreaseShared();
    }
    cb = new ControlBlockRegular(reinterpret_cast<T*>(ptr));
  }

  T* get() const {
    if (cb == nullptr) {
      return nullptr;
    }
    return reinterpret_cast<T*>(cb->Object());
  }

  T& operator*() const { return *get(); }
  T* operator->() const { return get(); }

  void swap(SharedPtr& other) { std::swap(cb, other.cb); }

  void SharedFromThis() {
    if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
      get()->wptr = *this;
    }
  }
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
  return SharedPtr<T>(
      allocateShared<T>(std::allocator<T>(), std::forward<Args>(args)...));
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
  using BlockAlloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
  using BlockTraits = typename std::allocator_traits<
      Alloc>::template rebind_traits<ControlBlockMakeShared<T, Alloc>>;
  BlockAlloc block_alloc = alloc;
  ControlBlockMakeShared<T, Alloc>* cb = BlockTraits::allocate(block_alloc, 1);
  new (cb) ControlBlockMakeShared<T, Alloc>(alloc, std::forward<Args>(args)...);
  return SharedPtr<T>(cb);
}

template <typename T>
class WeakPtr {
 private:
  template <typename>
  friend class SharedPtr;
  template <typename>
  friend class WeakPtr;
  template <typename>
  friend class EnableSharedFromThis;
  BaseControlBlock* cb = nullptr;

 public:
  WeakPtr() {}

  template <typename U>
  WeakPtr(const SharedPtr<U>& other) : cb(other.cb) {
    if (cb != nullptr) {
      cb->IncreaseWeak();
    }
  }

  WeakPtr(const WeakPtr& other) : cb(other.cb) {
    if (cb != nullptr) {
      cb->IncreaseWeak();
    }
  }

  WeakPtr<T>& operator=(const WeakPtr<T>& other) {
    WeakPtr<T> copy(other);
    swap(copy);
    return *this;
  }

  WeakPtr(WeakPtr&& other) {
    cb = other.cb;
    other.cb = nullptr;
  }

  WeakPtr<T>& operator=(WeakPtr<T>&& other) {
    WeakPtr<T> copy(std::move(other));
    swap(copy);
    return *this;
  }

  template <typename U>
  WeakPtr(const WeakPtr<U>& other) : cb(other.cb) {
    if (cb != nullptr) {
      cb->IncreaseWeak();
    }
  }

  template <typename U>
  WeakPtr<T>& operator=(const WeakPtr<U>& other) {
    WeakPtr<T> copy(other);
    swap(copy);
    return *this;
  }

  template <typename U>
  WeakPtr(WeakPtr<U>&& other) {
    cb = other.cb;
    other.cb = nullptr;
  }

  template <typename U>
  WeakPtr<T>& operator=(WeakPtr<U>&& other) {
    WeakPtr<T> copy(std::move(other));
    std::swap(cb, other.cb);
    return *this;
  }

  T* get() const {
    if (cb == nullptr) {
      return nullptr;
    }
    return reinterpret_cast<T*>(cb->Object());
  }

  T& operator*() { return *get(); }

  T* operator->() { return get(); }

  int use_count() const { return cb->shared_count; }

  bool expired() const {
    if (cb == nullptr) {
      return true;
    }
    return cb->shared_count == 0;
  }

  SharedPtr<T> lock() const {
    if (expired()) {
      return SharedPtr<T>();
    }
    return SharedPtr<T>(*this);
  }

  void swap(WeakPtr& other) { std::swap(cb, other.cb); }

  ~WeakPtr() {
    if (cb != nullptr) {
      cb->DecreaseWeak();
    }
  }
};

template <typename T>
class EnableSharedFromThis {
  template <typename U>
  friend class SharedPtr;
  template <typename U>
  friend class WeakPtr;

 private:
  WeakPtr<T> wptr;

 public:
  SharedPtr<T> shared_from_this() const {
    if (wptr.cb == nullptr) {
      throw std::bad_weak_ptr();
    }
    return wptr.lock();
  }
};
