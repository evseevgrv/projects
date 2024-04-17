#include <memory>
#include <type_traits>

template <size_t N>
struct StackStorage {
  char pool[N];
  size_t shift;

  StackStorage() : shift(0) {}

  ~StackStorage() {}

  StackStorage(const StackStorage& other) = delete;
  StackStorage& operator=(const StackStorage& other) = delete;
};

template <typename T, size_t N>
struct StackAllocator {
  using value_type = T;
  using type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  StackStorage<N>* storage_;

  StackAllocator(StackStorage<N>& main_storage) : storage_(&main_storage) {}

  template <typename U>
  StackAllocator(const StackAllocator<U, N>& other)
      : storage_(other.storage_) {}

  template <typename U>
  StackAllocator& operator=(const StackAllocator<U, N>& other) {
    StackAllocator<U, N> temp(other);
    std::swap(storage_, temp.storage_);
    return *this;
  }

  ~StackAllocator() = default;

  T* allocate(size_t count) {
    size_t space = N - storage_->shift;
    void* current = static_cast<void*>(storage_->pool + storage_->shift);
    void* prev = current;
    std::align(alignof(T), sizeof(T), current, space);
    storage_->shift += count * sizeof(T) +
                       (static_cast<char*>(current) - static_cast<char*>(prev));
    return static_cast<T*>(current);
  }

  void deallocate(T* ptr, size_t) { std::ignore = ptr; }

  template <typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };
};

template <typename T>
struct BaseNode;

template <typename T>
struct Node;

template <typename T>
struct BaseNode {
  BaseNode<T>* prev;
  BaseNode<T>* next;

  BaseNode() : prev(this), next(this) {}
  BaseNode(BaseNode<T>* prev, BaseNode<T>* next) : prev(prev), next(next) {}
  ~BaseNode() {}

  T& ValueReference() { return reinterpret_cast<Node<T>*>(this)->value; }
  T* ValuePointer() { return &(reinterpret_cast<Node<T>*>(this)->value); }
};

template <typename T>
struct Node : private BaseNode<T> {
  T value;
  Node() : BaseNode<T>(nullptr, nullptr) {}
  Node(const T& value) : BaseNode<T>(nullptr, nullptr), value(value) {}
  ~Node() {}
};

template <typename T, bool IsConst>
struct NodeIterator {
  using Type = typename std::conditional<IsConst, const T, T>::type;
  using difference_type = int64_t;
  using iterator_category = std::bidirectional_iterator_tag;
  using pointer = Type*;
  using reference = Type&;
  using value_type = Type;

  BaseNode<T>* node;

  NodeIterator(const BaseNode<T>* node)
      : node(const_cast<BaseNode<T>*>(node)) {}

  NodeIterator(const BaseNode<T>& node)
      : node(const_cast<BaseNode<T>*>(&node)) {}

  NodeIterator(const NodeIterator<T, false>& other) : node(other.node) {}

  ~NodeIterator() {}

  NodeIterator& operator++() {
    node = node->next;
    return *this;
  }

  NodeIterator operator++(int) {
    auto temp = *this;
    node = node->next;
    return temp;
  }

  NodeIterator& operator--() {
    node = node->prev;
    return *this;
  }

  NodeIterator operator--(int) {
    auto temp = *this;
    node = node->prev;
    return temp;
  }

  reference operator*() { return node->ValueReference(); }
  pointer operator->() { return node->ValuePointer(); }

  const T* operator*() const { return *node; }

  const T& operator->() const { return node; }
};

template <typename T, bool IsConst>
bool operator==(NodeIterator<T, IsConst> first,
                NodeIterator<T, IsConst> second) {
  return (first.node == second.node);
}

template <typename T, bool IsConst>
bool operator!=(NodeIterator<T, IsConst> first,
                NodeIterator<T, IsConst> second) {
  return !(first == second);
}

template <typename T, typename Alloc = std::allocator<T>>
class List {
 private:
  using NodeAllocator =
      typename std::allocator_traits<Alloc>::template rebind_alloc<Node<T>>;
  using NodeAllocator_traits =
      typename std::allocator_traits<Alloc>::template rebind_traits<Node<T>>;

  size_t size_ = 0;
  BaseNode<T> fakeNode;

 public:
  using iterator = NodeIterator<T, false>;
  using const_iterator = NodeIterator<T, true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  [[no_unique_address]] NodeAllocator node_alloc;

  List() {}

  List(const size_t count) {
    for (size_t i = 0; i < count; ++i) {
      Node<T>* ptr = NodeAllocator_traits::allocate(node_alloc, 1);
      try {
        NodeAllocator_traits::construct(node_alloc, ptr);
      } catch (...) {
        NodeAllocator_traits::deallocate(node_alloc, ptr, 1);
        this->~List();
        throw;
      }
      BaseNode<T>* base = reinterpret_cast<BaseNode<T>*>(ptr);
      base->next = begin().node->next;
      base->prev = begin().node;
      begin().node->next = base;
      base->next->prev = base;
      ++size_;
    }
  }

  List(const size_t count, const T& value) : size_(count) {
    try {
      for (size_t i = 0; i < count; ++i) {
        push_back(value);
      }
    } catch (...) {
      this->~List();
      throw;
    }
  }

  List(Alloc alloc) : node_alloc(alloc) {}

  List(const size_t count, Alloc alloc) : node_alloc(alloc) {
    for (size_t i = 0; i < count; ++i) {
      Node<T>* ptr = NodeAllocator_traits::allocate(node_alloc, 1);
      try {
        NodeAllocator_traits::construct(node_alloc, ptr);
      } catch (...) {
        NodeAllocator_traits::deallocate(node_alloc, ptr, 1);
        this->~List();
        throw;
      }
      BaseNode<T>* base = reinterpret_cast<BaseNode<T>*>(ptr);
      base->next = begin().node->next;
      base->prev = begin().node;
      begin().node->next = base;
      base->next->prev = base;
      ++size_;
    }
  }

  List(const size_t count, const T& value, const Alloc& alloc)
      : size_(count), node_alloc(alloc) {
    try {
      for (size_t i = 0; i < count; ++i) {
        push_back(value);
      }
    } catch (...) {
      this->~List();
      throw;
    }
  }

  NodeAllocator get_allocator() { return node_alloc; }

  ~List() {
    BaseNode<T>* base = fakeNode.next;
    for (size_t i = 0; i < size_; ++i) {
      BaseNode<T>* next = base->next;
      NodeAllocator_traits::destroy(node_alloc,
                                    reinterpret_cast<Node<T>*>(base));
      NodeAllocator_traits::deallocate(node_alloc,
                                       reinterpret_cast<Node<T>*>(base), 1);
      base = next;
    }
  }

  List(const List& other)
      : node_alloc(NodeAllocator_traits::select_on_container_copy_construction(
            other.node_alloc)) {
    try {
      for (const auto& i : other) {
        push_back(i);
      }
    } catch (...) {
      this->~List();
      throw;
    }
  }
  List(const List<T, Alloc>& other, Alloc node_alloc) : node_alloc(node_alloc) {
    try {
      for (const auto& i : other) {
        push_back(i);
      }
    } catch (...) {
      this->~List();
      throw;
    }
  }
  List<T, Alloc>& operator=(const List<T, Alloc>& other) {
    if constexpr (NodeAllocator_traits::propagate_on_container_copy_assignment::
                      value) {
      List<T, Alloc> copy(other, other.node_alloc);
      std::swap(size_, copy.size_);
      std::swap(node_alloc, copy.node_alloc);
      std::swap(fakeNode, copy.fakeNode);
    } else {
      List<T, Alloc> copy(other);
      std::swap(size_, copy.size_);
      std::swap(node_alloc, copy.node_alloc);
      std::swap(fakeNode, copy.fakeNode);
    }
    return *this;
  }

  size_t size() const { return size_; }

  void push_back(const T& value) { insert(end(), value); }

  void push_front(const T& value) { insert(begin(), value); }

  void pop_back() { erase(--end()); }

  void pop_front() { erase(begin()); }

  void insert(const_iterator it, const T& value) {
    Node<T>* ptr = NodeAllocator_traits::allocate(node_alloc, 1);
    try {
      NodeAllocator_traits::construct(node_alloc, ptr, value);
    } catch (...) {
      NodeAllocator_traits::deallocate(node_alloc, ptr, 1);
      throw;
    }
    BaseNode<T>* base = reinterpret_cast<BaseNode<T>*>(ptr);
    base->prev = it.node->prev;
    base->next = it.node;
    base->prev->next = base;
    base->next->prev = base;
    ++size_;
  }

  void erase(const_iterator it) {
    it.node->prev->next = it.node->next;
    it.node->next->prev = it.node->prev;
    BaseNode<T>* base = it.node;
    NodeAllocator_traits::destroy(node_alloc, reinterpret_cast<Node<T>*>(base));
    NodeAllocator_traits::deallocate(node_alloc,
                                     reinterpret_cast<Node<T>*>(base), 1);
    --size_;
  }

  iterator begin() { return iterator(fakeNode.next); }
  const_iterator begin() const { return const_iterator(fakeNode.next); }
  const_iterator cbegin() const { return const_iterator(fakeNode.next); }

  iterator end() { return iterator(fakeNode); }
  const_iterator end() const { return const_iterator(fakeNode); }
  const_iterator cend() { return const_iterator(fakeNode); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(end());
  }

  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(begin());
  }
};
