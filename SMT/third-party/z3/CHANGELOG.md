# Changelog

This is the changelog for z3. This follows [Keep a Changelog v1.0.0](http://keepachangelog.com/en/1.0.0/).

## z3 for PP-CHECK 1.4 - 2018-1-31

### Added

* Added pp_sat_tactic: a tactic for solving purely SAT constraints
* Added C++ APIs for the constraint datalog engine(z3++.h)
* Added C++ API for converting DIMACS CNF in a goal object to std::string; APIs for getting expr types(z3++.h)
* Added bv_fast_check_tactic: a tactic as in-complete procedure for checking satisfiability/unsatisfiability fastly


### Changed

* When setting inc_qfbv to 4(Currently the default option),  use pp_qfbv_light_tactic to create both the incremental sovler and the non-incremental sovler



## z3 for PP-CHECK 1.3.1 or older - 2017-9-30


###Added

* Added C++ APIs for collecting features of a bit-vector constraint(use `solver.get_bv_features(std::vector<double>& feat)`)
* Added pp_qfbv_layered_tactic for experiments
* Added pp_qf_aufbv_tactic and pp_qf_aufbv_light_tactic for solving bit-vector array constraints(not fine-tuned)	
* Added several new options for the sat engine used by pp_qfbv_tactic
* Added bv_to_cnf_tactic, bv_to_cnf_enhanced_tactic and cnf_output_tactic for transforming a bit-vector constraint to a SAT constraint in DIMACS format
* Added pp_qfbv_approximation_tactic that applies under-approximation for solving bit-vector constraints(not used now)
* Added pp_inc_bv_solver for solving bit-vector constraints incrementally(not used now)
* Added a new option inc_qfbv for choosing a tactic to create the incremental solver(the default solver created by z3 is slow)
* Added pp_qfbv_tactic, pp_qfbv_light_tactic for solving bit-vector constraints



###Fxied

* Fixed utf-8 version string handling for python2. Resolved #787
* Fixed a potential memory leak bug in smt_strategic_solver.cpp(introduced by us..)
* Fixed double free bug in sat_asymm_branch.cpp
* Fixed iss939(double free) by disabling some Boolean simplifications
