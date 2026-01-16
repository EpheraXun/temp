/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: pp_qfaufbv_tactic
 * Creation Date:  2017.
 * Modification History:
*/

#ifndef PP_QFAUFBV_TACTIC_H_
#define PP_QFAUFBV_TACTIC_H_

#include"params.h"
class ast_manager;
class tactic;

tactic * mk_pp_qfaufbv_tactic(ast_manager & m, params_ref const & p = params_ref());

tactic * mk_pp_qfaufbv_light_tactic(ast_manager & m, params_ref const & p = params_ref());

tactic * mk_pp_qfaufbv_layered_tactic(ast_manager & m, params_ref const & p = params_ref());

/*
  ADD_TACTIC("pp_qfaufbv",  "pintpoint strategy for solving QF_AUFBV problems.", "mk_pp_qfaufbv_tactic(m, p)")

  ADD_TACTIC("pp_qfaufbv_light",  "pintpoint strategy(light) for solving QF_AUFBV problems.", "mk_pp_qfaufbv_light_tactic(m, p)")

  ADD_TACTIC("pp_qfaufbv_layered",  "pintpoint strategy(alyered) for solving QF_AUFBV problems.", "mk_pp_qfaufbv_layered_tactic(m, p)")
*/

tactic * mk_pp_qfaufbv_preamble(ast_manager& m, params_ref const& p);

tactic * mk_pp_qfaufbv_tactic(ast_manager & m, params_ref const & p, tactic* sat, tactic* smt);

tactic * mk_pp_qfaufbv_light_tactic(ast_manager & m, params_ref const & p, tactic* sat, tactic* smt);

tactic * mk_pp_qfaufbv_layered_tactic(ast_manager & m, params_ref const & p, tactic* sat);

#endif
