#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename T>
struct DefaultEdge : std::pair<T, T> {
  DefaultEdge(const T& first, const T& second)
      : std::pair<T, T>(first, second) {}
  using BaseClass = std::pair<T, T>;
  const T& Start() const { return BaseClass::first; }
  const T& Finish() const { return BaseClass::second; }
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
struct Visitor;

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
class AbstractGraph {
 protected:
  int64_t vertices_number_ = 0;
  int64_t edges_number_ = 0;

  struct Iterator {
    const AbstractGraph<Vertex, Edge>* graph = nullptr;
    Vertex current_parent;
    Vertex current_child;
    size_t index = 0;
    Vertex operator*() const { return current_child; }
    Iterator& operator++() {
      graph->Increase(*this);
      return *this;
    }
    bool operator==(const Iterator& other) const {
      return (graph == other.graph) &&
             (current_parent == other.current_parent) && (index == other.index);
    }
    bool operator!=(const Iterator& other) const {
      return !(operator==(other));
    }
    Iterator(const AbstractGraph<Vertex, Edge>& graph, const Vertex& parent,
             const Vertex& child, int64_t index = 0)
        : graph(&graph),
          current_parent(parent),
          current_child(child),
          index(index) {}
    Iterator Begin() { return graph->NeighboursIt(current_parent); }
    Iterator End() { return graph->EndIt(current_parent); }
    ~Iterator() = default;
  };

 public:
  using VertexType = Vertex;
  using EdgeType = Edge;

  explicit AbstractGraph(int64_t vertices_num, int64_t edges_num = 0)
      : vertices_number_(vertices_num), edges_number_(edges_num) {}

  virtual int64_t GetVerticesNumber() const = 0;
  virtual int64_t GetEdgesNumber() const = 0;

  virtual std::vector<Vertex> GetNeighbours(const Vertex& vertex) = 0;
  virtual void Increase(Iterator& iterator) const = 0;
  virtual Iterator NeighboursIt(VertexType v) const = 0;
  virtual Iterator EndIt(VertexType v) const = 0;
  virtual void Accept(Visitor<Vertex, Edge>& visitor) const = 0;
  virtual ~AbstractGraph() = default;
};

template <typename Vertex, typename Edge>
struct Visitor {
  virtual void Visit(const AbstractGraph<Vertex, Edge>* graph) = 0;
  virtual ~Visitor() = default;
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
class AdjacencyListGraph : public AbstractGraph<Vertex, Edge> {
 private:
  std::unordered_map<Vertex, std::vector<Vertex>> list_;

 public:
  using VertexType = Vertex;
  using EdgeType = Edge;
  using Iterator = typename AbstractGraph<Vertex, Edge>::Iterator;
  explicit AdjacencyListGraph(int64_t vertices_num,
                              const std::vector<Edge>& edges)
      : AbstractGraph<Vertex, Edge>(vertices_num, edges.size()) {
    for (const auto& edge : edges) {
      list_[edge.Start()].push_back(edge.Finish());
      list_[edge.Finish()].push_back(edge.Start());
    }
  }

  int64_t GetVerticesNumber() const override { return list_.size(); }
  int64_t GetEdgesNumber() const override {
    int64_t sz = 0;
    for (size_t i = 0; i < list_.size(); ++i) {
      sz += list_.at(i).size();
    }
    sz /= 2;
    return sz;
  }

  AdjacencyListGraph(int64_t number, int64_t number_of_pairs)
      : AbstractGraph<Vertex, Edge>(number, number_of_pairs) {
    for (int64_t i = 0; i < number; ++i) {
      list_[i] = std::vector<Vertex>(0);
    }
    int64_t first;
    int64_t second;
    for (int64_t i = 0; i < number_of_pairs; ++i) {
      std::cin >> first >> second;
      --first;
      --second;
      if (first == second) {
        continue;
      }
      list_[first].push_back(second);
      list_[second].push_back(first);
    }
  }

  std::vector<Vertex> GetNeighbours(const Vertex& vertex) final {
    return list_.at(vertex);
  }

  void Increase(Iterator& iterator) const override {
    ++iterator.index;
    if (iterator.index != list_.at(iterator.current_parent).size()) {
      iterator.current_child =
          list_.at(iterator.current_parent)[iterator.index];
    }
  }
  Iterator EndIt(VertexType v) const override {
    return Iterator(*this, v, v, list_.at(v).size());
  }
  Iterator NeighboursIt(VertexType v) const override {
    if (list_.at(v).empty()) {
      return Iterator(*this, v, v, 0);
    }
    return Iterator(*this, v, list_.at(v)[0], 0);
  }
  void Accept(Visitor<Vertex, Edge>& visitor) const override {
    visitor.Visit(this);
  }
  ~AdjacencyListGraph() override = default;
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
class MatrixGraph : public AbstractGraph<Vertex, Edge> {
 private:
  std::vector<Vertex, std::vector<int64_t>> matrix_;

 public:
  using VertexType = Vertex;
  using EdgeType = Edge;
  using Iterator = typename AbstractGraph<Vertex, Edge>::Iterator;
  explicit MatrixGraph(int64_t vertices_num, const std::vector<Edge>& edges)
      : AbstractGraph<Vertex, Edge>(vertices_num, edges.size()),
        matrix_(vertices_num, std::vector<int64_t>(vertices_num, 0)) {
    for (const auto& edge : edges) {
      matrix_[edge.Start()][edge.Finish()] = 1;
      matrix_[edge.Finish()][edge.Start()] = 1;
    }
  }
  int64_t GetVerticesNumber() const override { return matrix_.size(); }
  int64_t GetEdgesNumber() const override {
    int64_t sz = 0;
    for (size_t i = 0; i < matrix_.size(); ++i) {
      for (size_t j = 0; j < matrix_[i].size(); ++j) {
        if (matrix_[i][j] == 1) {
          ++sz;
        }
      }
    }
    sz /= 2;
    return sz;
  }
  std::vector<Vertex> GetNeighbours(const Vertex& vertex) final {
    std::vector<Vertex> neighbours;
    for (int64_t i = 0; i < GetVerticesNumber(); ++i) {
      if (matrix_[vertex][i] != 0) {
        neighbours.push_back(i);
      }
    }
    return neighbours;
  }
  void Increase(Iterator& iterator) const override {
    ++iterator.index;
    while (iterator.index != GetVerticesNumber() &&
           !(matrix_[iterator.current_parent][iterator.index])) {
      ++iterator.index;
    }
    iterator.current_child = iterator.index;
  }
  Iterator EndIt(VertexType v) const override {
    return Iterator(*this, v, v, GetVerticesNumber());
  }
  Iterator NeighboursIt(VertexType v) const override {
    int64_t count = 0;
    while (!(matrix_[v][count]) && count < GetVerticesNumber()) {
      ++count;
    }
    return Iterator(*this, v, count, count);
  }
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
struct VisitorBFS : Visitor<Vertex, Edge> {
  const AbstractGraph<Vertex, Edge>* graph;
  std::unordered_map<Vertex, bool> used;
  std::unordered_map<Vertex, int64_t> cost;
  std::vector<Vertex> parent;
  Vertex start;
  Vertex finish;
  VisitorBFS(Vertex start, Vertex finish, int64_t number)
      : parent(number, -1), start(start), finish(finish) {}
  virtual void Visit(const AbstractGraph<Vertex, Edge>* graph) override {
    this->graph = graph;
    used[start] = true;
    cost[start] = 0;
    std::queue<Vertex> queue;
    queue.push(start);
    while (!queue.empty()) {
      auto current = queue.front();
      queue.pop();
      if (current == finish) {
        break;
      }
      for (auto next = graph->NeighboursIt(current);
           next != graph->EndIt(current); ++next) {
        if (!used[*next]) {
          used[*next] = true;
          cost[*next] = cost[current] + 1;
          parent[*next] = current;
          queue.push(*next);
        }
      }
    }
  }
  std::vector<Vertex> Path() {
    std::vector<Vertex> ans;
    if (!used[finish]) {
      return ans;
    }
    auto current = finish;
    while (current != start) {
      ans.push_back(current + 1);
      current = parent[current];
    }
    ans.push_back(start + 1);
    std::reverse(ans.begin(), ans.end());
    return ans;
  }
  ~VisitorBFS() override = default;
};

class Task {
 private:
  int64_t number_;
  int64_t pairs_;
  int64_t start_;
  int64_t finish_;
  int64_t first_;
  int64_t second_;

 public:
  Task() {}

  void Solve() {
    std::cin >> number_ >> pairs_ >> start_ >> finish_;
    --start_;
    --finish_;
    AdjacencyListGraph<int64_t, DefaultEdge<int64_t>> graph(number_, pairs_);
    VisitorBFS<int64_t> visitor(start_, finish_, number_);
    graph.Accept(visitor);
    std::vector<int64_t> ans = visitor.Path();
    if (ans.empty()) {
      std::cout << -1;
    } else {
      std::cout << ans.size() - 1 << '\n';
      for (size_t i = 0; i < ans.size(); ++i) {
        std::cout << ans[i] << ' ';
      }
    }
  }
  ~Task() = default;
};

int main() {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  Task task;
  task.Solve();
  return 0;
}
