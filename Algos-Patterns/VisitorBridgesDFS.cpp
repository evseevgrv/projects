#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <set>
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
  int vertices_number_ = 0;
  int edges_number_ = 0;

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
             const Vertex& child, int index = 0)
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

  explicit AbstractGraph(int vertices_num, int edges_num = 0)
      : vertices_number_(vertices_num), edges_number_(edges_num) {}

  virtual int GetVerticesNumber() const = 0;
  virtual int GetEdgesNumber() const = 0;

  virtual std::vector<Vertex> GetNeighbours(const Vertex& vertex) = 0;
  virtual void Increase(Iterator& iterator) const = 0;
  virtual Iterator NeighboursIt(VertexType v) const = 0;
  virtual Iterator EndIt(VertexType v) const = 0;
  virtual void Accept(Visitor<Vertex, Edge>& visitor) const = 0;
  virtual std::pair<int, int> GetEdge(
      const std::pair<Vertex, Vertex> edge) const = 0;
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
  std::map<std::pair<Vertex, Vertex>, std::pair<int, int>> number_;

 public:
  using VertexType = Vertex;
  using EdgeType = Edge;
  using Iterator = typename AbstractGraph<Vertex, Edge>::Iterator;
  explicit AdjacencyListGraph(int vertices_num, const std::vector<Edge>& edges)
      : AbstractGraph<Vertex, Edge>(vertices_num, edges.size()) {
    for (const auto& edge : edges) {
      list_[edge.Start()].push_back(edge.Finish());
      list_[edge.Finish()].push_back(edge.Start());
    }
  }

  int GetVerticesNumber() const override { return list_.size(); }
  int GetEdgesNumber() const override {
    int sz = 0;
    for (size_t i = 0; i < list_.size(); ++i) {
      sz += list_.at(i).size();
    }
    sz /= 2;
    return sz;
  }

  AdjacencyListGraph(int count, int number_of_pairs)
      : AbstractGraph<Vertex, Edge>(count, number_of_pairs) {
    for (int i = 0; i < count; ++i) {
      list_[i] = std::vector<Vertex>(0);
    }
    int first;
    int second;
    for (int i = 0; i < number_of_pairs; ++i) {
      std::cin >> first >> second;
      if (first != second) {
        --first;
        --second;
        list_[first].push_back(second);
        list_[second].push_back(first);
        if (number_.find({first, second}) != number_.end()) {
          number_[{first, second}].first = i;
          ++number_[{first, second}].second;
          number_[{second, first}].first = i;
          ++number_[{second, first}].second;
        } else {
          number_[{first, second}] = {i, 1};
          number_[{second, first}] = {i, 1};
        }
      }
    }
  }

  std::vector<Vertex> GetNeighbours(const Vertex& vertex) final {
    return list_.at(vertex);
  }

  virtual std::pair<int, int> GetEdge(
      std::pair<Vertex, Vertex> edge) const override {
    return number_.at(edge);
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
  std::vector<Vertex, std::vector<int>> matrix_;

 public:
  using VertexType = Vertex;
  using EdgeType = Edge;
  using Iterator = typename AbstractGraph<Vertex, Edge>::Iterator;
  explicit MatrixGraph(int vertices_num, const std::vector<Edge>& edges)
      : AbstractGraph<Vertex, Edge>(vertices_num, edges.size()),
        matrix_(vertices_num, std::vector<int>(vertices_num, 0)) {
    for (const auto& edge : edges) {
      matrix_[edge.Start()][edge.Finish()] = 1;
      matrix_[edge.Finish()][edge.Start()] = 1;
    }
  }
  int GetVerticesNumber() const override { return matrix_.size(); }
  int GetEdgesNumber() const override {
    int sz = 0;
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
    for (int i = 0; i < GetVerticesNumber(); ++i) {
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
    int count = 0;
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
  std::unordered_map<Vertex, int> cost;
  std::vector<Vertex> parent;
  Vertex start;
  Vertex finish;
  VisitorBFS(Vertex start, Vertex finish, int number)
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

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
struct VisitorDFS : Visitor<Vertex, Edge> {
  const AbstractGraph<Vertex, Edge>* graph;
  std::unordered_map<Vertex, bool> used;
  VisitorDFS() = default;
  VisitorDFS(const int& number) : used(number, false) {}
  virtual ~VisitorDFS() = default;
  virtual void ExamineVertex(Vertex& vertex, const int& value) = 0;
  virtual void CompareVertex(Vertex vertex, Vertex other) = 0;
  virtual void MatchVertex(Vertex vertex, Vertex other) = 0;
  virtual void Visit(const AbstractGraph<Vertex, Edge>* graph) override {
    this->graph = graph;
    int value = 0;
    for (Vertex ver = 0; ver < static_cast<Vertex>(graph->GetVerticesNumber());
         ++ver) {
      if (!used[ver]) {
        DiscoverVertex(ver, value);
      }
    }
  }
  virtual void DiscoverVertex(Vertex vertex, int& value, Vertex parent = -1) {
    used[vertex] = true;
    ++value;
    ExamineVertex(vertex, value);
    for (auto next = graph->NeighboursIt(vertex); next != graph->EndIt(vertex);
         ++next) {
      if (*next == parent) {
        continue;
      }
      if (!used[*next]) {
        DiscoverVertex(*next, value, vertex);
      }
      CompareVertex(vertex, *next);
    }
  }
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
struct BridgesDFS : VisitorDFS<Vertex, Edge> {
 private:
  using MainDFS = VisitorDFS<Vertex, Edge>;
  std::vector<int> tin;
  std::vector<int> ret;
  std::set<int> bridges;

 public:
  BridgesDFS(int number) : tin(number, 1e9), ret(number, 1e9) {}
  ~BridgesDFS() override = default;
  virtual void ExamineVertex(Vertex& vertex, const int& value) override {
    tin[vertex] = value;
    ret[vertex] = value;
  }
  virtual void MatchVertex(Vertex vertex, Vertex other) override {
    ret[vertex] = std::min(ret[vertex], ret[other]);
  }
  virtual void CompareVertex(Vertex vertex, Vertex other) override {
    ret[vertex] = std::min(ret[vertex], ret[other]);
    auto edge = MainDFS::graph->GetEdge({vertex, other});
    if (ret[other] > tin[vertex] && edge.second == 1) {
      bridges.insert(edge.first + 1);
    }
  }
  std::set<int> Path() { return bridges; }
};

class Task {
 private:
  int number_;
  int pairs_;

 public:
  Task() {}

  void Solve() {
    std::cin >> number_ >> pairs_;
    AdjacencyListGraph<int, DefaultEdge<int>> graph(number_, pairs_);
    BridgesDFS<int> visitor(number_);
    graph.Accept(visitor);
    std::set<int> ans = visitor.Path();
    std::cout << ans.size() << '\n';
    for (auto elem : ans) {
      std::cout << elem << ' ';
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
