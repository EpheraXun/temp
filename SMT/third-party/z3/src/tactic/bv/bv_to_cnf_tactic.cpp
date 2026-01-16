/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: QF_BV to DIMACS CNF
 * Creation Date:  26/06/2017.
 * Modification History:
*/
#include"tactical.h"
#include"simplify_tactic.h"
#include"propagate_values_tactic.h"
#include"solve_eqs_tactic.h"
#include"elim_uncnstr_tactic.h"
#include"bit_blaster_tactic.h"
#include"max_bv_sharing_tactic.h"
#include"tseitin_cnf_tactic.h"
#include"bv_size_reduction_tactic.h"
#include"../aig/aig_tactic.h"
#include<iostream>
#include<fstream>

#define MEMLIMIT 300


static tactic * mk_bv_basic_simplify(ast_manager& m, params_ref const& p) {
    params_ref solve_eq_p;
    solve_eq_p.set_uint("solve_eqs_max_occs", 2);
    return
            and_then(
                mk_simplify_tactic(m),
                mk_propagate_values_tactic(m),
                using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                mk_elim_uncnstr_tactic(m),
                if_no_proofs(if_no_unsat_cores(mk_bv_size_reduction_tactic(m))),
                using_params(mk_solve_eqs_tactic(m), solve_eq_p)  // pinpoint: this can handle some "fast sat" cases
                );
}

static tactic * mk_bv_to_cnf_preamble(ast_manager& m, params_ref const& p) {
    params_ref solve_eq_p;
    solve_eq_p.set_uint("solve_eqs_max_occs", 2);
    params_ref simp2_p = p;
    simp2_p.set_bool("som", true);
    simp2_p.set_bool("pull_cheap_ite", true);
    simp2_p.set_bool("push_ite_bv", false);
    simp2_p.set_bool("local_ctx", true);
    simp2_p.set_uint("local_ctx_limit", 10000000);
    simp2_p.set_bool("flat", true);
    simp2_p.set_bool("hoist_mul", false);
    return
            and_then(
                mk_simplify_tactic(m),
                mk_propagate_values_tactic(m),
                using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                mk_elim_uncnstr_tactic(m),
                using_params(mk_simplify_tactic(m), simp2_p),
                mk_max_bv_sharing_tactic(m)
                );
}


static tactic * mk_bv_to_cnf_preamble_enhanced(ast_manager& m, params_ref const& p) {
    params_ref solve_eq_p;
    // conservative guassian elimination.
    solve_eq_p.set_uint("solve_eqs_max_occs", 2);

    params_ref simp2_p = p;
    simp2_p.set_bool("som", true);
    simp2_p.set_bool("pull_cheap_ite", true);
    simp2_p.set_bool("push_ite_bv", false);
    simp2_p.set_bool("local_ctx", true);
    simp2_p.set_uint("local_ctx_limit", 10000000);
    simp2_p.set_bool("flat", true);      // required by som
    simp2_p.set_bool("hoist_mul", false); // required by som

    //params_ref hoist_p;
    //hoist_p.set_bool("hoist_mul", true);
    //hoist_p.set_bool("som", false);

    return
            and_then(
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
                //using_params(mk_simplify_tactic(m), hoist_p),
                mk_max_bv_sharing_tactic(m)
                );
}

static tactic * main_pp(tactic* t) {
    params_ref p;
    p.set_bool("elim_and", true);
    p.set_bool("push_ite_bv", true);
    p.set_bool("blast_distinct", true);
    return using_params(t, p);
}


// Interface
tactic * mk_bv_fast_check_tactic(ast_manager& m, params_ref const & p) {
    tactic* st = mk_bv_basic_simplify(m, p);
    st->updt_params(p);
    return st;
}

tactic * mk_bv_to_cnf_tactic(ast_manager& m, params_ref const & p) {
    tactic* preamble_st = mk_bv_to_cnf_preamble(m, p);
    tactic * st = and_then(main_pp(and_then(preamble_st, mk_bit_blaster_tactic(m))), mk_tseitin_cnf_tactic(m, p));
    st->updt_params(p);
    return st;
}

tactic * mk_bv_to_cnf_enhanced_tactic(ast_manager& m, params_ref const & p) {
	// more powerful preprocessing steps
    params_ref local_ctx_p = p;
    local_ctx_p.set_bool("local_ctx", true);

    params_ref big_aig_p;
    big_aig_p.set_bool("aig_per_assertion", false);

    tactic* preamble_st = mk_bv_to_cnf_preamble_enhanced(m, p);
    tactic * st = and_then(main_pp(and_then(preamble_st, mk_bit_blaster_tactic(m),
                                             when(mk_lt(mk_memory_probe(), mk_const_probe(MEMLIMIT)),
                                               and_then(using_params(and_then(mk_simplify_tactic(m), mk_solve_eqs_tactic(m)), local_ctx_p),
                		                               using_params(mk_aig_tactic(),big_aig_p))))),
    		               mk_tseitin_cnf_tactic(m, p));
    st->updt_params(p);
    return st;
}

class cnf_output_tactic : public tactic {

    ast_manager&  m;
    params_ref    m_params;

public:

    cnf_output_tactic(ast_manager& m, params_ref const& p):
        m(m), m_params(p) {}

    virtual tactic * translate(ast_manager & m) {
        return alloc(cnf_output_tactic, m, m_params);
    }

    virtual void updt_params(params_ref const & p) {
    }

    virtual void collect_param_descrs(param_descrs & r) {
    }

    virtual void operator()(goal_ref const & g,
                            goal_ref_buffer & result,
                            model_converter_ref & mc,
                            proof_converter_ref & pc,
                            expr_dependency_ref & core) {
        g->display_dimacs(std::cout);
    }

    virtual void cleanup() {
    }

};

// Interface
tactic * mk_cnf_output_tactic(ast_manager & m, params_ref const & p) {
    return alloc(cnf_output_tactic, m, p);
}





