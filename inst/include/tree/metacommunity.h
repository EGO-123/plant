// -*-c++-*-
#ifndef TREE_METACOMMUNITY_H_
#define TREE_METACOMMUNITY_H_

#include <Rcpp.h>
#include "patch.h"
#include "ode_solver.h"

namespace model {

class MetacommunityBase : public ode::OdeTarget {
public:
  virtual ~MetacommunityBase();
  virtual size_t size() const = 0;
  virtual double get_time() const = 0;
  virtual void step() = 0;
  virtual void step_deterministic() = 0;
  virtual void step_stochastic() = 0;
  virtual std::vector<int> births() = 0;
  virtual void deaths() = 0;
  virtual void add_seeds(std::vector<int> seeds) = 0;
  virtual Rcpp::List r_get_patches() const = 0;
  virtual void r_add_seedlings(Rcpp::IntegerMatrix seeds) = 0;
  virtual Rcpp::IntegerMatrix r_disperse(std::vector<int> seeds) const = 0;
  virtual Rcpp::IntegerMatrix r_n_individuals() const = 0;
  virtual void reset() = 0;
  virtual void r_step() = 0;
  virtual void r_step_stochastic() = 0;
  virtual Rcpp::List r_height() const = 0;
  virtual void r_set_height(Rcpp::List x) = 0;
};

template <class Individual> 
class Metacommunity : public MetacommunityBase {
public:
  Metacommunity(Parameters p);
  Metacommunity(Parameters *p);
  
  // * Simple interrogation:
  size_t size() const;
  double get_time() const;

  // * Main simulation control
  void step();
  void step_deterministic();
  void step_stochastic();

  // * Lower level parts of the main simulation
  std::vector<int> births();
  void deaths();
  void add_seeds(std::vector<int> seeds);

  // * ODE interface
  size_t ode_size() const;
  ode::iterator_const set_ode_values(double time_, ode::iterator_const it);
  ode::iterator       ode_values(ode::iterator it) const;
  ode::iterator       ode_rates(ode::iterator it)  const;

  // * R interface
  Patch<Individual> r_at(size_t idx) const;
  Rcpp::List r_get_patches() const;
  void r_add_seedlings(Rcpp::IntegerMatrix seeds);
  Rcpp::IntegerMatrix r_disperse(std::vector<int> seeds) const;
  Rcpp::IntegerMatrix r_n_individuals() const;
  void reset();
  void r_step();
  void r_step_stochastic();
  Rcpp::List r_height() const;
  void r_set_height(Rcpp::List x);

private:
  void initialise();
  size_t n_species() const {return parameters->size(); }
  std::vector< std::vector<int> > disperse(std::vector<int> seeds) const;
  
  Parameters::ptr parameters;
  std::vector< Patch<Individual> > patches;
  double time;
  ode::Solver< Metacommunity > ode_solver;

  typedef typename std::vector< Patch<Individual> >::iterator 
  patch_iterator;
  typedef typename std::vector< Patch<Individual> >::const_iterator
  patch_const_iterator;
};

template <class Individual>
Metacommunity<Individual>::Metacommunity(Parameters p)
  : parameters(p),
    time(0.0),
    ode_solver(this, parameters->control.ode_control) {
  initialise();
}

template <class Individual>
Metacommunity<Individual>::Metacommunity(Parameters *p)
  : parameters(p),
    time(0.0),
    ode_solver(this, parameters->control.ode_control) {
  initialise();
}

template <class Individual>
size_t Metacommunity<Individual>::size() const {
  return patches.size();
}

// TODO: Should this always just return ode_solver.get_time()?
template <class Individual>
double Metacommunity<Individual>::get_time() const {
  return time;
}

template <class Individual>
void Metacommunity<Individual>::step() {
  step_deterministic();
  step_stochastic();
}

template <class Individual>
void Metacommunity<Individual>::step_deterministic() {
  ode_solver.set_state_from_problem();
  ode_solver.step();
  time = ode_solver.get_time();
}

template <class Individual>
void Metacommunity<Individual>::step_stochastic() {
  deaths();
  add_seeds(births());
}

template <class Individual>
Rcpp::List Metacommunity<Individual>::r_height() const {
  Rcpp::List ret;
  for (patch_const_iterator patch = patches.begin();
       patch != patches.end(); ++patch)
    ret.push_back(Rcpp::wrap(patch->r_height()));
  return ret;
}

template <class Individual>
void Metacommunity<Individual>::r_set_height(Rcpp::List x) {
  util::check_length(static_cast<size_t>(x.size()), size());
  for (size_t i = 0; i < size(); ++i)
    patches[i].r_set_height(x[static_cast<int>(i)]);
}

template <class Individual>
std::vector<int> Metacommunity<Individual>::births() {
  std::vector<int> n(size(), 0);
  for (patch_iterator patch = patches.begin();
       patch != patches.end(); ++patch)
    n = util::sum(n, patch->births());
  return n;
}

template <class Individual>
void Metacommunity<Individual>::deaths() {
  for (patch_iterator patch = patches.begin();
       patch != patches.end(); ++patch)
    patch->deaths();
}

template <class Individual>
void Metacommunity<Individual>::add_seeds(std::vector<int> seeds) {
  std::vector< std::vector<int> > seeds_dispersed = disperse(seeds);
  for (size_t i = 0; i < size(); ++i)
    patches[i].add_seeds(seeds_dispersed[i]);
}

// Island model of dispersal, equivalent to equal-probability
// multinomial sampling.  NOTE: This will change considerably if
// moving to a spatial model of dispersal.
template <class Individual>
std::vector< std::vector<int> >
Metacommunity<Individual>::disperse(std::vector<int> seeds) const {
  std::vector< std::vector<int> > ret;
  for (size_t i = 0; i < size(); ++i) {
    const double p = 1.0/(size() - i);
    ret.push_back(util::rbinom_multiple(seeds.begin(), seeds.end(), p));
  }

  return ret;
}

// * ODE interace.
template <class Individual>
size_t Metacommunity<Individual>::ode_size() const {
  return ode::ode_size(patches.begin(), patches.end());
}

template <class Individual>
ode::iterator_const
Metacommunity<Individual>::set_ode_values(double time_,
					  ode::iterator_const it) {
  return ode::set_ode_values(patches.begin(), patches.end(), time_, it);
}

template <class Individual>
ode::iterator
Metacommunity<Individual>::ode_values(ode::iterator it) const {
  return ode::ode_values(patches.begin(), patches.end(), it);
}

template <class Individual>
ode::iterator
Metacommunity<Individual>::ode_rates(ode::iterator it) const {
  return ode::ode_rates(patches.begin(), patches.end(), it);
}

template <class Individual>
Patch<Individual> Metacommunity<Individual>::r_at(size_t idx) const {
  return patches.at(util::check_bounds_r(idx, size()));
}

template <class Individual>
Rcpp::List Metacommunity<Individual>::r_get_patches() const {
  Rcpp::List ret;
  for (patch_const_iterator patch = patches.begin();
       patch != patches.end(); ++patch)
    ret.push_back(Rcpp::wrap(*patch));
  return ret;
}

// Each column is a patch, each row a species.
template <class Individual>
void Metacommunity<Individual>::r_add_seedlings(Rcpp::IntegerMatrix seeds) {
  const size_t
    nr = static_cast<size_t>(seeds.nrow()),
    nc = static_cast<size_t>(seeds.ncol());

  util::check_dimensions(nr,          nc,
			 n_species(), size());
  std::vector< std::vector<int> > seeds_m = util::from_rcpp_matrix(seeds);
  for (size_t i = 0; i < size(); ++i)
    patches[i].add_seedlings(seeds_m[i]);
}

template <class Individual>
Rcpp::IntegerMatrix 
Metacommunity<Individual>::r_disperse(std::vector<int> seeds) const {
  util::check_length(seeds.size(), n_species());
  Rcpp::RNGScope scope;
  return util::to_rcpp_matrix(disperse(seeds));
}

template <class Individual>
Rcpp::IntegerMatrix Metacommunity<Individual>::r_n_individuals() const {
  std::vector< std::vector<int> > n;
  for (patch_const_iterator patch = patches.begin();
       patch != patches.end(); ++patch)
    n.push_back(patch->r_n_individuals());
  return util::to_rcpp_matrix(n);
}

template <class Individual>
void Metacommunity<Individual>::reset() {
  time = 0.0;
  for (patch_iterator patch = patches.begin();
       patch != patches.end(); ++patch)
    patch->reset();
  ode_solver.reset();
}

template <class Individual>
void Metacommunity<Individual>::r_step() {
  Rcpp::RNGScope scope;
  step();
}

template <class Individual>
void Metacommunity<Individual>::r_step_stochastic() {
  Rcpp::RNGScope scope;
  step_stochastic();
}

template <class Individual>
void Metacommunity<Individual>::initialise() {
  patches.clear(); // TODO: should not be needed? (see Patch::initialise)
  // All patches are identical on creation.
  Patch<Individual> p(parameters.get());
  patches.resize(parameters->n_patches, p);
  reset();
}

SEXP metacommunity(Rcpp::CppClass individual, Parameters p);

}

RCPP_EXPOSED_CLASS_NODECL(model::Metacommunity<model::Plant>)
RCPP_EXPOSED_CLASS_NODECL(model::Metacommunity<model::CohortDiscrete>)

#endif