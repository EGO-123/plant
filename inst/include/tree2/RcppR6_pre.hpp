// Generated by RcppR6 (0.1): do not edit by hand
#ifndef _TREE2_RCPPR6_PRE_HPP_
#define _TREE2_RCPPR6_PRE_HPP_

#include <RcppCommon.h>


namespace tree2 {
namespace RcppR6 {
template <typename T> class RcppR6;
}
}

namespace ode { namespace test { class Lorenz; } }


namespace Rcpp {
template <typename T> SEXP wrap(const tree2::RcppR6::RcppR6<T>&);
namespace traits {
template <typename T> class Exporter<tree2::RcppR6::RcppR6<T> >;
}

template <> SEXP wrap(const ode::test::Lorenz&);
template <> ode::test::Lorenz as(SEXP);

template <> SEXP wrap(const ode::OdeSystem<ode::test::Lorenz>&);
template <> ode::OdeSystem<ode::test::Lorenz> as(SEXP);
}

#endif
