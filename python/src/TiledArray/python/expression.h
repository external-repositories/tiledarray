/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2020  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TA_PYTHON_EXPRESSION_H
#define TA_PYTHON_EXPRESSION_H

#include "python.h"

#include <tiledarray.h>
#include <vector>
#include <string>

namespace TiledArray {
namespace python {
namespace expression {

  template<class Array>
  struct Expression {

    struct Term {
      std::shared_ptr<Array> array;
      std::string index;
      double factor = 1;
      auto evaluate() const {
        const auto &array = *this->array;
        return factor*(array(index));
      }
    };

    explicit Expression(std::vector<Term> terms)
      : terms(terms)
    {
    }

    Expression add(const Expression &e) const {
      auto r = this->terms;
      for (const auto &t : e.terms) {
        r.push_back({t.array, t.index, t.factor});
      }
      return Expression{r};
    }

    Expression sub(const Expression &e) const {
      auto r = this->terms;
      for (const auto &t :  e.terms) {
        r.push_back({t.array, t.index, -t.factor});
      }
      return Expression{r};
    }

    Expression mul(double f) const {
      auto r = this->terms;
      for (auto &t :  r) {
        t.factor *= f;
      }
      return Expression{r};
    }

    Expression div(double f) const {
      auto r = this->terms;
      for (auto &t :  r) {
        t.factor /= f;
      }
      return Expression{r};
    }

    template<size_t ... Idx>
    auto operator[](std::integer_sequence<size_t,Idx...>) const {
      return (terms.at(Idx).evaluate() + ...);
    }

  public:
    const std::vector<Term> terms;

  };

  template<size_t N, class Array, size_t ... Idx>
  auto index(const Expression<Array> &e, std::integer_sequence<size_t,Idx...> idx) {
    using Index = std::variant< std::make_integer_sequence<size_t,1+Idx>... >;
    if (N == e.terms.size()) {
      return Index(std::make_integer_sequence<size_t,N>());
    }
    if constexpr (N < TA_PYTHON_MAX_EXPRESSION) {
      return index<N+1>(e, idx);
    }
    throw std::domain_error(
      "Expression exceeds TA_PYTHON_MAX_EXPRESSION=" + std::to_string(TA_PYTHON_MAX_EXPRESSION)
    );
  }

  template<class Array>
  auto index(const Expression<Array> &e) {
    return index<1>(e, std::make_integer_sequence<size_t,TA_PYTHON_MAX_EXPRESSION>());
  }

  template<class F, class Array>
  auto evaluate(F &&f, const Expression<Array> &a) {
    auto visitor = [&](auto &&A) {
      return f(a[A]);
    };
    return std::visit(visitor, index(a));
  }

  template<class F, class Array>
  auto evaluate(F &&f, const Expression<Array> &a, const Expression<Array> &b) {
    auto visitor = [&](auto &&A, auto &&B) {
      return f(a[A], b[B]);
    };
    return std::visit(visitor, index(a), index(b));
  }

#define TA_PYTHON_EXPRESSION_REDUCE(EXPRESSION, OP)                     \
  [](const EXPRESSION &e) {                                             \
    auto op = [](auto &&e) { return e.OP(); };                          \
    return evaluate(op, e).get();                                       \
  }

#define TA_PYTHON_EXPRESSION_REDUCE2(EXPRESSION, OP)                    \
  [](const EXPRESSION &a, const EXPRESSION &b) {                        \
    auto op = [](auto &&a, auto &&b) { return a.OP(b); };               \
    return evaluate(op, a, b).get();                                    \
  }

  template<class Array>
  inline Expression<Array> getitem(std::shared_ptr<Array> array, std::string idx) {
    return Expression<Array>({{array, idx}});
  }

  template<class Array>
  inline void setitem(Array &array, std::string idx, const Expression<Array> &e) {
    auto op = [&](auto &&e) {
      array(idx) = e;
    };
    evaluate(op, e);
  }

  template<class Array>
  void make_array_expression_class(py::module m, const char *name) {
    using Expression = Expression<Array>;
    py::class_<Expression>(m, name)
      .def("__add__", &Expression::add)
      .def("__sub__", &Expression::sub)
      .def("__mul__", &Expression::mul)
      .def("__rmul__", &Expression::mul)
      .def("__truediv__", &Expression::div)
      .def("min", TA_PYTHON_EXPRESSION_REDUCE(Expression ,min))
      .def("max", TA_PYTHON_EXPRESSION_REDUCE(Expression, max))
      .def("norm", TA_PYTHON_EXPRESSION_REDUCE(Expression, norm))
      .def("dot", TA_PYTHON_EXPRESSION_REDUCE2(Expression, dot))
      ;
  }

  inline void __init__(py::module m) {
    make_array_expression_class< TArray<double> >(m, "Expression");
    make_array_expression_class< TSpArray<double> >(m, "SparseExpression");
  }

}
}
}

#endif // TA_PYTHON_EXPRESSION_H
