/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: pp_qfbv_layered_tactic
 * Creation Date:  2017.
 * Modification History:
*/

#ifndef PP_QFBV_LAYERED_TACTIC_H_
#define PP_QFBV_LAYERED_TACTIC_H_
#include"params.h"
class ast_manager;
class tactic;

tactic * mk_pp_qfbv_layer_one_tactic(ast_manager & m, params_ref const & p = params_ref());
tactic * mk_pp_qfbv_layer_two_tactic(ast_manager & m, params_ref const & p = params_ref());
tactic * mk_pp_qfbv_layer_three_tactic(ast_manager & m, params_ref const & p = params_ref());
tactic * mk_pp_qfbv_layer_four_tactic(ast_manager & m, params_ref const & p = params_ref());
tactic * mk_pp_qfbv_layer_five_tactic(ast_manager & m, params_ref const & p = params_ref());
tactic * mk_pp_qfbv_layer_six_tactic(ast_manager & m, params_ref const & p = params_ref());
tactic * mk_pp_qfbv_layer_seven_tactic(ast_manager & m, params_ref const & p = params_ref());

/*
  ADD_TACTIC("pp_qfbv_layer_one",  "pintpoint strategy(layer one) for solving QF_BV problems.", "mk_pp_qfbv_layer_one_tactic(m, p)")

  ADD_TACTIC("pp_qfbv_layer_two",  "pintpoint strategy(layer two) for solving QF_BV problems.", "mk_pp_qfbv_layer_two_tactic(m, p)")

  ADD_TACTIC("pp_qfbv_layer_three", "pintpoint strategy(layer three) for solving QF_BV problems.", "mk_pp_qfbv_layer_three_tactic(m, p)")

  ADD_TACTIC("pp_qfbv_layer_four",  "pintpoint strategy(layer four) for solving QF_BV problems.", "mk_pp_qfbv_layer_four_tactic(m, p)")

  ADD_TACTIC("pp_qfbv_layer_five",  "pintpoint strategy(layer five) for solving QF_BV problems.", "mk_pp_qfbv_layer_five_tactic(m, p)")

  ADD_TACTIC("pp_qfbv_layer_six",  "pintpoint strategy(layer six) for solving QF_BV problems.", "mk_pp_qfbv_layer_six_tactic(m, p)")

  ADD_TACTIC("pp_qfbv_layer_seven",  "pintpoint strategy(layer seven) for solving QF_BV problems.", "mk_pp_qfbv_layer_seven_tactic(m, p)")
*/


tactic * mk_pp_qfbv_layer_one_tactic(ast_manager & m, params_ref const & p, tactic* sat);
tactic * mk_pp_qfbv_layer_two_tactic(ast_manager & m, params_ref const & p, tactic* sat);
tactic * mk_pp_qfbv_layer_three_tactic(ast_manager & m, params_ref const & p, tactic* sat);
tactic * mk_pp_qfbv_layer_four_tactic(ast_manager & m, params_ref const & p, tactic* sat);
tactic * mk_pp_qfbv_layer_five_tactic(ast_manager & m, params_ref const & p, tactic* sat);
tactic * mk_pp_qfbv_layer_six_tactic(ast_manager & m, params_ref const & p, tactic* sat);
tactic * mk_pp_qfbv_layer_seven_tactic(ast_manager & m, params_ref const & p, tactic* sat);

#endif
