/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: pp_qfbv_light_tactic
 * Creation Date:  2017.
 * Modification History:
*/

#ifndef PP_QFBV_LIGHT_TACTIC_H_
#define PP_QFBV_LIGHT_TACTIC_H_
#include"params.h"
class ast_manager;
class tactic;


tactic * mk_pp_sat_light_tactic(ast_manager & m, params_ref const & p = params_ref());

tactic * mk_pp_qfbv_light_tactic(ast_manager & m, params_ref const & p = params_ref());

/*
  ADD_TACTIC("pp_sat_light",  "pinpoint strategy for solving SAT problems.", "mk_pp_sat_light_tactic(m, p)")

  ADD_TACTIC("pp_qfbv_light",  "pinpoint strategy(lightweight) for solving QF_BV problems.", "mk_pp_qfbv_light_tactic(m, p)")
*/

tactic * mk_pp_qfbv_light_preamble(ast_manager& m, params_ref const& p);

tactic * mk_pp_sat_light_tactic(ast_manager & m, params_ref const & p, tactic* sat);

tactic * mk_pp_qfbv_light_tactic(ast_manager & m, params_ref const & p, tactic* sat);

#endif

