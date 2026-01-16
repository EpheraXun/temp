/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: pp_sat_tactic.
 * Creation Date:  2017.
 * Modification History:
*/

#ifndef PP_SAT_TACTIC_H
#define PP_SAT_TACTIC_H

#include"params.h"
class ast_manager;
class tactic;

tactic * mk_pp_sat_tactic(ast_manager & m, params_ref const & p = params_ref());

tactic * mk_pp_sat_preprocessor_tactic(ast_manager & m, params_ref const & p = params_ref());

/*
  ADD_TACTIC('pp-sat', '(try to) solve goal using a SAT solver.', 'mk_pp_sat_tactic(m, p)')
  ADD_TACTIC('pp-sat-preprocess', 'Apply SAT solver preprocessing procedures (bounded resolution, Boolean constant propagation, 2-SAT, subsumption, subsumption resolution).', 'mk_pp_sat_preprocessor_tactic(m, p)')
*/


#endif //PP_SAT_TACTIC_H
