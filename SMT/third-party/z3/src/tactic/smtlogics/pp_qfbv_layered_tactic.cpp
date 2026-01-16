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
#include"pp_qfbv_tactic.h"
#include"pp_qfbv_light_tactic.h"

#define MEMLIMIT 300


// TODO
//  choice of main_p
//

// Layer one: no linear reasoning
tactic * mk_pp_qfbv_layer_one_tactic(ast_manager& m, params_ref const & p, tactic* sat) {
    params_ref main_p;
    main_p.set_bool("elim_and", true);
    main_p.set_bool("push_ite_bv", true);
    main_p.set_bool("blast_distinct", true);

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
                                    mk_propagate_values_tactic(m),
                                    //mk_solve_eqs_tactic(m),
                                    //using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                                    //mk_elim_uncnstr_tactic(m),
                                    //mk_bv_size_reduction_tactic(m),
                                    using_params(mk_simplify_tactic(m), simp2_p),
                                    mk_max_bv_sharing_tactic(m)
    );

    tactic * st = using_params(and_then(preamble_st, mk_bit_blaster_tactic(m), sat), main_p);
    st->updt_params(p);
    return st;
}

tactic * mk_pp_qfbv_layer_two_tactic(ast_manager& m, params_ref const & p, tactic* sat) {
    params_ref main_p;
    main_p.set_bool("elim_and", true);
    main_p.set_bool("push_ite_bv", true);
    main_p.set_bool("blast_distinct", true);

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
                                    mk_propagate_values_tactic(m),
                                    //mk_solve_eqs_tactic(m),
                                    using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                                    mk_elim_uncnstr_tactic(m),
                                    mk_bv_size_reduction_tactic(m),
                                    using_params(mk_simplify_tactic(m), simp2_p),
                                    mk_max_bv_sharing_tactic(m)
    );

    tactic * st = using_params(and_then(preamble_st, mk_bit_blaster_tactic(m), sat), main_p);
    st->updt_params(p);
    return st;
}


// Layer three: same as pp_qfbv_light(conservative linear reasoning)
tactic * mk_pp_qfbv_layer_three_tactic(ast_manager& m, params_ref const & p, tactic* sat) {
    params_ref main_p;
    main_p.set_bool("elim_and", true);
    main_p.set_bool("push_ite_bv", true);
    main_p.set_bool("blast_distinct", true);

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


    tactic * preamble_st = and_then(mk_simplify_tactic(m),
                                    mk_propagate_values_tactic(m),
                                    //mk_solve_eqs_tactic(m),
                                    using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                                    mk_elim_uncnstr_tactic(m),
                                    //mk_bv_size_reduction_tactic(m),
                                    using_params(mk_simplify_tactic(m), simp2_p),
                                    mk_max_bv_sharing_tactic(m)
    );

    tactic * st = using_params(and_then(preamble_st, mk_bit_blaster_tactic(m), sat), main_p);
    st->updt_params(p);
    return st;
}


// Layer four:
tactic * mk_pp_qfbv_layer_four_tactic(ast_manager& m, params_ref const & p, tactic* sat) {
    params_ref main_p;
    main_p.set_bool("elim_and", true);
    main_p.set_bool("push_ite_bv", true);
    main_p.set_bool("blast_distinct", true);

    //params_ref solve_eq_p;
    // conservative guassian elimination.
    //solve_eq_p.set_uint("solve_eqs_max_occs", 2);

    params_ref simp2_p = p;
    simp2_p.set_bool("som", true);
    simp2_p.set_bool("pull_cheap_ite", true);
    simp2_p.set_bool("push_ite_bv", false);
    simp2_p.set_bool("local_ctx", true);
    simp2_p.set_uint("local_ctx_limit", 10000000);
    simp2_p.set_bool("flat", true); // required by som
    simp2_p.set_bool("hoist_mul", false); // required by som


    tactic * preamble_st = and_then(mk_simplify_tactic(m),
                                    mk_propagate_values_tactic(m),
                                    mk_solve_eqs_tactic(m),
                                    //using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                                    mk_elim_uncnstr_tactic(m),
                                    //mk_bv_size_reduction_tactic(m),
                                    using_params(mk_simplify_tactic(m), simp2_p),
                                    mk_max_bv_sharing_tactic(m)
    );

    tactic * st = using_params(and_then(preamble_st, mk_bit_blaster_tactic(m), sat), main_p);
    st->updt_params(p);
    return st;
}

tactic * mk_pp_qfbv_layer_five_tactic(ast_manager& m, params_ref const & p, tactic* sat) {
    params_ref main_p;
    main_p.set_bool("elim_and", true);
    main_p.set_bool("push_ite_bv", true);
    main_p.set_bool("blast_distinct", true);

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

    params_ref hoist_p;
    hoist_p.set_bool("hoist_mul", true);
    hoist_p.set_bool("som", false);

    tactic * preamble_st = and_then(
            mk_simplify_tactic(m),
            mk_propagate_values_tactic(m),
            using_params(mk_solve_eqs_tactic(m), solve_eq_p),
            mk_elim_uncnstr_tactic(m),
            if_no_proofs(if_no_unsat_cores(mk_bv_size_reduction_tactic(m))),
            using_params(mk_solve_eqs_tactic(m), solve_eq_p),  // pinpoint: this can handle some "fast sat" cases
            using_params(mk_simplify_tactic(m), simp2_p),
            //
            // Z3 can solve a couple of extra benchmarks by using hoist_mul
            // but the timeout in SMT-COMP is too small.
            // Moreover, it impacted negatively some easy benchmarks.
            // We should decide later, if we keep it or not.
            //
            using_params(mk_simplify_tactic(m), hoist_p),
            mk_max_bv_sharing_tactic(m)
    );

    tactic * st = using_params(and_then(preamble_st, mk_bit_blaster_tactic(m), sat), main_p);
    st->updt_params(p);
    return st;
}

// Layer six, same as pp_qfbv
tactic * mk_pp_qfbv_layer_six_tactic(ast_manager& m, params_ref const & p, tactic* sat) {
    params_ref main_p;
    main_p.set_bool("elim_and", true);
    main_p.set_bool("push_ite_bv", true);
    main_p.set_bool("blast_distinct", true);

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

    params_ref local_ctx_p = p;
    local_ctx_p.set_bool("local_ctx", true);


    params_ref big_aig_p;
    big_aig_p.set_bool("aig_per_assertion", false);


    tactic * preamble_st = and_then(mk_simplify_tactic(m),
                                    mk_propagate_values_tactic(m),
                                    //mk_solve_eqs_tactic(m),
                                    using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                                    mk_elim_uncnstr_tactic(m),
                                    mk_bv_size_reduction_tactic(m),
                                    using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                                    using_params(mk_simplify_tactic(m), simp2_p),
                                    mk_max_bv_sharing_tactic(m)
    );

    tactic * st = using_params(and_then(
            preamble_st,
            mk_bit_blaster_tactic(m),
            when(mk_lt(mk_memory_probe(), mk_const_probe(MEMLIMIT)),
                 and_then(using_params(and_then(mk_simplify_tactic(m),
                                                mk_solve_eqs_tactic(m)),
                                       local_ctx_p),
                          using_params(mk_aig_tactic(),
                                       big_aig_p))),
            sat), main_p);
    st->updt_params(p);
    return st;
}


// Interface
tactic * mk_pp_qfbv_layer_one_tactic(ast_manager & m, params_ref const & p) {

    tactic * new_sat = cond(mk_produce_proofs_probe(),
                            and_then(mk_simplify_tactic(m), mk_smt_tactic()),
                            mk_sat_tactic(m));
    return mk_pp_qfbv_layer_one_tactic(m, p, new_sat);

}

tactic * mk_pp_qfbv_layer_two_tactic(ast_manager & m, params_ref const & p) {
    tactic * new_sat = cond(mk_produce_proofs_probe(),
                            and_then(mk_simplify_tactic(m), mk_smt_tactic()),
                            mk_sat_tactic(m));
    return mk_pp_qfbv_layer_two_tactic(m, p, new_sat);

}

tactic * mk_pp_qfbv_layer_three_tactic(ast_manager & m, params_ref const & p) {
    tactic * new_sat = cond(mk_produce_proofs_probe(),
                            and_then(mk_simplify_tactic(m), mk_smt_tactic()),
                            mk_pp_sat_tactic(m));
    return mk_pp_qfbv_layer_three_tactic(m, p, new_sat);

}

tactic * mk_pp_qfbv_layer_four_tactic(ast_manager & m, params_ref const & p) {
    tactic * new_sat = cond(mk_produce_proofs_probe(),
                            and_then(mk_simplify_tactic(m), mk_smt_tactic()),
                            mk_sat_tactic(m));
    return mk_pp_qfbv_layer_four_tactic(m, p, new_sat);

}

tactic * mk_pp_qfbv_layer_five_tactic(ast_manager & m, params_ref const & p) {
    tactic * new_sat = cond(mk_produce_proofs_probe(),
                            and_then(mk_simplify_tactic(m), mk_smt_tactic()),
                            mk_sat_tactic(m));
    return mk_pp_qfbv_layer_five_tactic(m, p, new_sat);

}

tactic * mk_pp_qfbv_layer_six_tactic(ast_manager & m, params_ref const & p) {
    tactic * new_sat = cond(mk_produce_proofs_probe(),
                            and_then(mk_simplify_tactic(m), mk_smt_tactic()),
                            mk_sat_tactic(m));
    return mk_pp_qfbv_layer_six_tactic(m, p, new_sat);

}

tactic * mk_pp_qfbv_layer_seven_tactic(ast_manager & m, params_ref const & p) {
    //return or_else(try_for(mk_pp_qfbv_light_tactic(m, p), 7000),
               //     mk_pp_qfbv_tactic(m, p));

    return or_else(try_for(mk_pp_qfbv_layer_three_tactic(m, p), 15000),
                   mk_pp_qfbv_layer_six_tactic(m, p));

}