# Changelog

This is the changelog for the SMT submodule of Pinpoint. This follows [Keep a Changelog v1.0.0](http://keepachangelog.com/en/1.0.0/).

## From 2018-06-04

### Added 
* Added machine learning based timeout predication (PR#51)

## SMT for PP-CHECK 1.4 - 2018-1-31

### Added

* Addes support of quantifier eliminatio and symbolic abstraction(Interval abstraction)
* Added several procedures for solving N2N queries
* Added basic model cache support
* Added control to global SMT solving timeout  
* Added change the cmake structure and the code to fit Windows



###Fixed

* Fixed a bug in parameter settinggit add lib/SMT/SMTFactory.cpp 


## SMT for PP-CHECK 1.3.1 or older - 2017-9-30


###Added

* Added streaming APIs(e.g., create SMTExpr from a string)
* Added several logical simplification procedures for SMTExpr(e.g., elimValidOrUnsatExpr)
* Added better SMT Model support
* Added several options for controling the SAT engine used by pp_sat_tactic(e.g., disable SAT probing)
* Added options for local simplifications while adding a vector of constraints 
* Added several test cases for smtd
* Added the IncTactic option for choosing a tactic to create the incremental solver(based on the new option inc_qfbv we add for z3)


###Changed

* Use python2.7 to compile z3(the default Python might be Python3, with which we will fail to build z3)


###Fixed

* Fixed build if the default compiler is clang<3.7
* Fixed an NPD in SMTExpr 
* Fixed the makefile bug that cannot correctly link to static/dynamic libs of z3 on demand 