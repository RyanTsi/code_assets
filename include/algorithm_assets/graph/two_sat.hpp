#pragma once
#include <algorithm_assets/graph/strong_components.hpp>
#include <utility>

namespace algorithm_assets {
class TwoSat {
public:
  struct Literal { std::uint32_t variable; bool value; };
  struct Result { bool satisfiable; std::vector<std::uint8_t> assignment; };
  explicit TwoSat(std::size_t variables) : variables_(variables) {
    if (variables > (std::numeric_limits<std::uint32_t>::max()-1)/2)
      throw std::length_error("Too many Boolean variables");
  }
  void add_clause(Literal a, Literal b) {
    validate(a); validate(b); clauses_.push_back({a,b});
  }
  void force(Literal a) { add_clause(a,a); }
  void implies(Literal a, Literal b) { add_clause({a.variable,!a.value}, b); }
  [[nodiscard]] Result solve() const {
    std::vector<UnweightedCsrGraph::Edge> edges;
    if (clauses_.size() > edges.max_size()/2) throw std::length_error("Too many clauses");
    edges.reserve(2*clauses_.size());
    for (auto [a,b] : clauses_) {
      const auto x=encode(a), y=encode(b);
      edges.push_back({x^1U,y}); edges.push_back({y^1U,x});
    }
    UnweightedCsrGraph graph(2*variables_,edges);
    auto components=strong_components(graph);
    Result result{true,std::vector<std::uint8_t>(variables_)};
    for (std::size_t i=0;i<variables_;++i) {
      if (components.component[2*i]==components.component[2*i+1]) return {false,{}};
      result.assignment[i]=components.component[2*i+1]<components.component[2*i];
    }
    return result;
  }
  [[nodiscard]] std::size_t variable_count() const noexcept { return variables_; }
private:
  struct Clause { Literal first, second; };
  void validate(Literal a) const {
    if (a.variable>=variables_) throw std::out_of_range("Invalid literal variable");
  }
  static std::uint32_t encode(Literal a) noexcept { return 2*a.variable+static_cast<unsigned>(a.value); }
  std::size_t variables_;
  std::vector<Clause> clauses_;
};
} // namespace algorithm_assets
