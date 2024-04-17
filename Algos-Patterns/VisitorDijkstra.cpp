// 87439403
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

template <typename T>
struct WeightedEdge : DefaultEdge<T> {
 private:
  int64_t weight_;

 public:
  WeightedEdge(const T& first, const T& second, int64_t weight)
      : DefaultEdge<T>(first, second), weight_(weight) {}
  const int64_t& Weight() const { return weight_; }
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

  virtual void Increase(Iterator& iterator) const = 0;
  virtual Iterator NeighboursIt(VertexType ver) const = 0;
  virtual Iterator EndIt(VertexType ver) const = 0;
  virtual void Accept(Visitor<Vertex, Edge>& visitor) const = 0;
  virtual std::vector<Edge> GetEdges(Vertex vertex) const = 0;
  virtual ~AbstractGraph() = default;
};

template <typename Vertex, typename Edge>
class Visitor {
 public:
  virtual void Visit(const AbstractGraph<Vertex, Edge>* graph) = 0;
  virtual ~Visitor() = default;
  virtual void SetIndex(int64_t index) = 0;
  virtual int64_t GetValue(int64_t index) = 0;
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
class AdjacencyListGraph : public AbstractGraph<Vertex, Edge> {
 private:
  std::unordered_map<Vertex, std::vector<Edge>> list_;
  std::vector<int64_t> start_;
  //   std::map<std::pair<Vertex, Vertex>, std::pair<int, int>> number_;

 public:
  using VertexType = Vertex;
  using EdgeType = Edge;
  using Iterator = typename AbstractGraph<Vertex, Edge>::Iterator;
  explicit AdjacencyListGraph(int vertices_num, const std::vector<Edge>& edges)
      : AbstractGraph<Vertex, Edge>(vertices_num, edges.size()) {
    for (const auto& edge : edges) {
      list_[edge.Start()].push_back(edge);
      list_[edge.Finish()].push_back(edge);
    }
  }

  int GetVerticesNumber() const override { return list_.size(); }
  int GetEdgesNumber() const override {
    int number_of_edges = 0;
    for (size_t i = 0; i < list_.size(); ++i) {
      number_of_edges += list_.at(i).size();
    }
    number_of_edges /= 2;
    return number_of_edges;
  }

  AdjacencyListGraph(int64_t count, Visitor<Vertex, Edge>& visitor)
      : AbstractGraph<Vertex, Edge>(count), start_(count) {
    int64_t number_of_vertexes;
    int64_t number_of_pairs;
    Vertex first;
    Vertex second;
    int64_t weight;
    int64_t start;
    for (int64_t i = 0; i < count; ++i) {
      std::cin >> number_of_vertexes >> number_of_pairs;
      list_.clear();
      for (int64_t j = 0; j < number_of_pairs; ++j) {
        std::cin >> first >> second >> weight;
        list_[first].emplace_back(first, second, weight);
        list_[second].emplace_back(second, first, weight);
      }
      std::cin >> start_[i];
      visitor.SetIndex(start_[i]);
      Accept(visitor);
      for (int64_t i = 0; i < number_of_vertexes; ++i) {
        std::cout << visitor.GetValue(i) << ' ';
      }
      std::cout << '\n';
    }
  }

  virtual std::vector<Edge> GetEdges(Vertex vertex) const override {
    return list_.at(vertex);
  }

  void Increase(Iterator& iterator) const override {
    ++iterator.index;
    if (iterator.index != list_.at(iterator.current_parent).size()) {
      iterator.current_child =
          list_.at(iterator.current_parent)[iterator.index].Finish();
    }
  }
  Iterator EndIt(VertexType ver) const override {
    return Iterator(*this, ver, ver, list_.at(ver).size());
  }
  Iterator NeighboursIt(VertexType ver) const override {
    if (list_.at(ver).empty()) {
      return Iterator(*this, ver, ver, 0);
    }
    return Iterator(*this, ver, list_.at(ver)[0].Finish(), 0);
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
    int number_of_edges = 0;
    for (size_t i = 0; i < matrix_.size(); ++i) {
      for (size_t j = 0; j < matrix_[i].size(); ++j) {
        if (matrix_[i][j] == 1) {
          ++number_of_edges;
        }
      }
    }
    number_of_edges /= 2;
    return number_of_edges;
  }

  void Increase(Iterator& iterator) const override {
    ++iterator.index;
    while (iterator.index != GetVerticesNumber() &&
           !(matrix_[iterator.current_parent][iterator.index])) {
      ++iterator.index;
    }
    iterator.current_child = iterator.index;
  }
  Iterator EndIt(VertexType ver) const override {
    return Iterator(*this, ver, ver, GetVerticesNumber());
  }
  Iterator NeighboursIt(VertexType ver) const override {
    int count = 0;
    while (!(matrix_[ver][count]) && count < GetVerticesNumber()) {
      ++count;
    }
    return Iterator(*this, ver, count, count);
  }
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
class VisitorBFS : public Visitor<Vertex, Edge> {
 private:
  std::unordered_map<Vertex, bool> used_;
  std::unordered_map<Vertex, int> cost_;
  std::vector<Vertex> parent_;
  Vertex start_;
  Vertex finish_;

 public:
  const AbstractGraph<Vertex, Edge>* graph;
  VisitorBFS(Vertex start, Vertex finish, int number)
      : parent_(number, -1), start_(start), finish_(finish) {}
  virtual void Visit(const AbstractGraph<Vertex, Edge>* graph) override {
    this->graph = graph;
    used_[start_] = true;
    cost_[start_] = 0;
    std::queue<Vertex> queue;
    queue.push(start_);
    while (!queue.empty()) {
      auto current = queue.front();
      queue.pop();
      if (current == finish_) {
        break;
      }
      for (auto next = graph->NeighboursIt(current);
           next != graph->EndIt(current); ++next) {
        if (!used_[*next]) {
          used_[*next] = true;
          cost_[*next] = cost_[current] + 1;
          parent_[*next] = current;
          queue.push(*next);
        }
      }
    }
  }
  std::vector<Vertex> Path() {
    std::vector<Vertex> ans;
    if (!used_[finish_]) {
      return ans;
    }
    auto current = finish_;
    while (current != start_) {
      ans.push_back(current + 1);
      current = parent_[current];
    }
    ans.push_back(start_ + 1);
    std::reverse(ans.begin(), ans.end());
    return ans;
  }
  ~VisitorBFS() override = default;
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
class VisitorDijkstra : public Visitor<Vertex, Edge> {
 private:
  std::unordered_map<Vertex, int64_t> cost;

 public:
  const AbstractGraph<Vertex, Edge>* graph;
  VisitorDijkstra() = default;
  VisitorDijkstra(int64_t number) {}
  ~VisitorDijkstra() override = default;
  Vertex index_ = 0;

  virtual void Visit(const AbstractGraph<Vertex, Edge>* graph) override {
    this->graph = graph;
    ProcessBlock();
  }

  void SetIndex(Vertex index) { index_ = index; }

  int64_t GetValue(Vertex index) override { return cost[index]; }

  void ProcessBlock() {
    std::set<std::pair<int64_t, Vertex>> queue;
    cost.clear();
    queue.insert({0, index_});
    cost[index_] = 0;
    while (!queue.empty()) {
      Vertex current = queue.begin()->second;
      queue.erase(queue.begin());
      for (auto edge : graph->GetEdges(current)) {
        if (cost.find(edge.Finish()) == cost.end()) {
          cost[edge.Finish()] = 2009000999;
        }
        if (cost.find(current) == cost.end()) {
          cost[current] = 2009000999;
        }
        if (cost[edge.Finish()] > cost[current] + edge.Weight()) {
          queue.erase({cost[edge.Finish()], edge.Finish()});
          cost[edge.Finish()] = cost[current] + edge.Weight();
          queue.insert({cost[edge.Finish()], edge.Finish()});
        }
      }
    }
  }
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
class VisitorDFS : public Visitor<Vertex, Edge> {
 private:
  std::unordered_map<Vertex, bool> used_;
  virtual void DiscoverVertex(Vertex vertex, int& value, Vertex parent = -1) {
    used_[vertex] = true;
    ++value;
    ExamineVertex(vertex, value);
    for (auto next = graph->NeighboursIt(vertex); next != graph->EndIt(vertex);
         ++next) {
      if (*next == parent) {
        continue;
      }
      if (!used_[*next]) {
        DiscoverVertex(*next, value, vertex);
        CompareVertex(vertex, *next);
      } else {
        MatchVertex(vertex, *next);
      }
    }
  }
  virtual void ExamineVertex(Vertex& vertex, const int& value) = 0;
  virtual void CompareVertex(Vertex vertex, Vertex other) = 0;
  virtual void MatchVertex(Vertex vertex, Vertex other) = 0;

 public:
  const AbstractGraph<Vertex, Edge>* graph;
  VisitorDFS() = default;
  VisitorDFS(const int& number) : used_(number, false) {}
  virtual ~VisitorDFS() = default;
  virtual void Visit(const AbstractGraph<Vertex, Edge>* graph) override {
    this->graph = graph;
    int value = 0;
    for (Vertex ver = 0; ver < static_cast<Vertex>(graph->GetVerticesNumber());
         ++ver) {
      if (!used_[ver]) {
        DiscoverVertex(ver, value);
      }
    }
  }
};

template <typename Vertex = int, typename Edge = DefaultEdge<Vertex>>
class BridgesDFS : public VisitorDFS<Vertex, Edge> {
 private:
  using MainDFS = VisitorDFS<Vertex, Edge>;
  const int kInf = 1e9;
  std::vector<int> tin_;
  std::vector<int> ret_;
  std::set<int> bridges_;
  virtual void ExamineVertex(Vertex& vertex, const int& value) override {
    tin_[vertex] = value;
    ret_[vertex] = value;
  }
  virtual void MatchVertex(Vertex vertex, Vertex other) override {
    ret_[vertex] = std::min(ret_[vertex], ret_[other]);
  }
  virtual void CompareVertex(Vertex vertex, Vertex other) override {
    ret_[vertex] = std::min(ret_[vertex], ret_[other]);
    auto edge = MainDFS::graph->GetEdge({vertex, other});
    if (ret_[other] > tin_[vertex] && edge.second == 1) {
      bridges_.insert(edge.first + 1);
    }
  }

 public:
  BridgesDFS(int number) : tin_(number, kInf), ret_(number, kInf) {}
  virtual ~BridgesDFS() override = default;

  std::set<int> Path() { return bridges_; }
};

class Task {
 private:
  int64_t number_;

 public:
  Task() {}

  void Solve() {
    std::cin >> number_;
    VisitorDijkstra<int64_t, WeightedEdge<int64_t>> visitor(number_);
    AdjacencyListGraph<int64_t, WeightedEdge<int64_t>> graph(number_, visitor);
    graph.Accept(visitor);
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
