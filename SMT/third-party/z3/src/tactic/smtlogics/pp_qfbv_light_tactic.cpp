/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: pp_qfbv_light_tactic
 * Creation Date:  2017.
 * Modification History:
*/

#include"tactical.h"
#include"simplify_tactic.h"
#include"propagate_values_tactic.h"
#include"solve_eqs_tactic.h"
#include"elim_uncnstr_tactic.h"
#include"smt_tactic.h"
#include"bit_blaster_tactic.h"
#include"max_bv_sharing_tactic.h"
#include"bv_size_reduction_tactic.h"
#include"aig_tactic.h"
#include"sat_tactic.h"
#include"pp_sat_tactic.h"

static tactic * mk_pp_qfbv_light_preamble(ast_manager& m, params_ref const& p) {

    params_ref solve_eq_p;
    // conservative guassian elimination.
    solve_eq_p.set_uint("solve_eqs_max_occs", 2);

    params_ref simp2_p = p;
    simp2_p.set_bool("som", true);
    simp2_p.set_bool("pull_cheap_ite", true);
    simp2_p.set_bool("push_ite_bv", false);
    simp2_p.set_bool("local_ctx", true);
    simp2_p.set_uint("local_ctx_limit", 10000000);
    simp2_p.set_bool("flat", true); // required by som
    simp2_p.set_bool("hoist_mul", false); // required by som

    return
            and_then(
                mk_simplify_tactic(m),
                mk_propagate_values_tactic(m),
                using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                mk_elim_uncnstr_tactic(m),
                //mk_bv_size_reduction_tactic(m),  // get bv_size_reduction back
                //using_params(mk_solve_eqs_tactic(m), solve_eq_p),  // pinpoint: this can handle some "fast sat" cases
                using_params(mk_simplify_tactic(m), simp2_p),
                mk_max_bv_sharing_tactic(m)
                );
}

static tactic * main_p(tactic* t) {
    params_ref p;
    p.set_bool("elim_and", true);
    p.set_bool("push_ite_bv", true);
    p.set_bool("blast_distinct", true);
    return using_params(t, p);
}


tactic * mk_pp_qfbv_light_tactic(ast_manager& m, params_ref const & p, tactic* sat) {
    tactic* preamble_st = mk_pp_qfbv_light_preamble(m, p);
    tactic * st = main_p(and_then(preamble_st,mk_bit_blaster_tactic(m),sat));
    st->updt_params(p);
    return st;
}


tactic * mk_pp_sat_light_tactic(ast_manager& m, params_ref const & p, tactic* sat) {

	params_ref main_p;
    main_p.set_bool("elim_and", true);
    main_p.set_bool("push_ite_bv", true);
    //main_p.set_bool("blast_distinct", true);

    /*
    params_ref simp2_p = p;
    simp2_p.set_bool("som", true);
    simp2_p.set_bool("pull_cheap_ite", true);
    simp2_p.set_bool("push_ite_bv", false);
    simp2_p.set_bool("local_ctx", true);
    simp2_p.set_uint("local_ctx_limit", 10000000);
    simp2_p.set_bool("flat", true); // required by som
    simp2_p.set_bool("hoist_mul", false); // required by som

    params_ref solve_eq_p;
    // conservative guassian elimination.
    solve_eq_p.set_uint("solve_eqs_max_occs", 2);

    tactic * preamble_st = and_then(mk_simplify_tactic(m),
                                    //mk_propagate_values_tactic(m),
                                    //mk_solve_eqs_tactic(m),
                                    //using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                                    //mk_elim_uncnstr_tactic(m),
                                    //mk_bv_size_reduction_tactic(m),
                                    //using_params(mk_simplify_tactic(m), simp2_p),
                                    mk_max_bv_sharing_tactic(m)
    );
    */

    //tactic * st = using_params(and_then(mk_simplify_tactic(m), mk_bit_blaster_tactic(m), sat), main_p);
    tactic * st = using_params(and_then(mk_simplify_tactic(m), sat), main_p);
    st->updt_params(p);
    return st;
}


// Interface
tactic * mk_pp_sat_light_tactic(ast_manager & m, params_ref const & p) {

    tactic * new_sat = cond(mk_produce_proofs_probe(),
                            and_then(mk_simplify_tactic(m), mk_smt_tactic()),
                            mk_sat_tactic(m));
    return mk_pp_sat_light_tactic(m, p, new_sat);
}

tactic * mk_pp_qfbv_light_tactic(ast_manager & m, params_ref const & p) {

    tactic * new_sat = cond(mk_produce_proofs_probe(),
                            and_then(mk_simplify_tactic(m), mk_smt_tactic()),
                            mk_sat_tactic(m));

    return mk_pp_qfbv_light_tactic(m, p, new_sat);

}


