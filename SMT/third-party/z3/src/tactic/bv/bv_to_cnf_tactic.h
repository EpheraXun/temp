/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: Transform QF_BV to DIMACS CNF format
 * Creation Date:  26/06/2017.
 * Modification History:
*/

#ifndef BV_TO_CNF_TACTIC_H_
#define BV_TO_CNF_TACTIC_H_

#include"params.h"
class ast_manager;
class tactic;

tactic * mk_bv_fast_check_tactic(ast_manager & m, params_ref const & p = params_ref());

tactic * mk_bv_to_cnf_tactic(ast_manager & m, params_ref const & p = params_ref());

tactic * mk_cnf_output_tactic(ast_manager & m, params_ref const & p = params_ref());

tactic * mk_bv_to_cnf_enhanced_tactic(ast_manager & m, params_ref const & p = params_ref());

/*
  ADD_TACTIC("bv-fast-check", "check satisfiability of bv with fast semi-decision procedures.", "mk_bv_fast_check_tactic(m, p)")

  ADD_TACTIC("bv-to-cnf", "convert bv into CNF.", "mk_bv_to_cnf_tactic(m, p)")

  ADD_TACTIC("bv-to-cnf-enhanced", "convert bv to CNF with more powerful preprocessing.", "mk_bv_to_cnf_enhanced_tactic(m, p)")

  ADD_TACTIC("cnf-output", "Output CNF.", "mk_cnf_output_tactic(m, p)")

*/

#endif /* BV_TO_CNF_TACTIC_H_ */
