/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: pp_inc_bv_solver
 * Creation Date:  2017.
 * Modification History:
*/

#include "solver.h"
#include "tactical.h"
#include "sat_solver.h"
#include "tactic2solver.h"
#include "aig_tactic.h"
#include "propagate_values_tactic.h"
#include "max_bv_sharing_tactic.h"
#include "card2bv_tactic.h"
#include "bit_blaster_tactic.h"
#include "simplify_tactic.h"
#include "goal2sat.h"
#include "ast_pp.h"
#include "model_smt2_pp.h"
#include "filter_model_converter.h"
#include "bit_blaster_model_converter.h"
#include "ast_translation.h"
#include "ast_util.h"
#include "solve_eqs_tactic.h"
#include "elim_uncnstr_tactic.h"
#include "bv_size_reduction_tactic.h"


// incremental QF_BV solver.
class pp_inc_bv_solver : public solver {
    ast_manager&    m;
    sat::solver     m_solver;
    goal2sat        m_goal2sat;
    params_ref      m_params;
    expr_ref_vector m_fmls;
    expr_ref_vector m_asmsf;
    unsigned_vector m_fmls_lim;
    unsigned_vector m_asms_lim;
    unsigned_vector m_fmls_head_lim;
    unsigned            m_fmls_head;
    expr_ref_vector     m_core;
    atom2bool_var       m_map;
    model_ref           m_model;
    scoped_ptr<bit_blaster_rewriter> m_bb_rewriter;
    tactic_ref          m_preprocess;
    unsigned            m_num_scopes;
    sat::literal_vector m_asms;
    goal_ref_buffer     m_subgoals;
    proof_converter_ref m_pc;
    model_converter_ref m_mc;
    model_converter_ref m_mc0;
    expr_dependency_ref m_dep_core;
    svector<double>     m_weights;
    std::string         m_unknown;


    typedef obj_map<expr, sat::literal> dep2asm_t;
public:
    pp_inc_bv_solver(ast_manager& m, params_ref const& p):
            m(m), m_solver(p, m.limit(), 0),
            m_params(p),
            m_fmls(m),
            m_asmsf(m),
            m_fmls_head(0),
            m_core(m),
            m_map(m),
            m_num_scopes(0),
            m_dep_core(m),
            m_unknown("no reason given") {
        m_params.set_bool("elim_vars", false);
        m_solver.updt_params(m_params);
        init_preprocess();
    }

    virtual ~pp_inc_bv_solver() {}

    virtual solver* translate(ast_manager& dst_m, params_ref const& p) {
        ast_translation tr(m, dst_m);
        if (m_num_scopes > 0) {
            throw default_exception("Cannot translate sat solver at non-base level");
        }
        pp_inc_bv_solver* result = alloc(pp_inc_bv_solver, dst_m, p);
        expr_ref fml(dst_m);
        for (unsigned i = 0; i < m_fmls.size(); ++i) {
            fml = tr(m_fmls[i].get());
            result->m_fmls.push_back(fml);
        }
        for (unsigned i = 0; i < m_asmsf.size(); ++i) {
            fml = tr(m_asmsf[i].get());
            result->m_asmsf.push_back(fml);
        }
        return result;
    }

    virtual void set_progress_callback(progress_callback * callback) {}


    void display_weighted(std::ostream& out, unsigned sz, expr * const * assumptions, unsigned const* weights) {
        NOT_IMPLEMENTED_YET();
    }

    virtual lbool check_sat(unsigned sz, expr * const * assumptions) {
        m_solver.pop_to_base_level();
        dep2asm_t dep2asm;
        m_model = 0;
        lbool r = internalize_formulas();
        if (r != l_true) return r;
        r = internalize_assumptions(sz, assumptions, dep2asm);
        if (r != l_true) return r;

        try {
            r = m_solver.check(m_asms.size(), m_asms.c_ptr());
        } catch (sat::solver_exception &ex) {
            return l_undef;
        }


        return r;
    }

    virtual void push() {
        // TODO: do we need call internalize_formula() after every push?
        // What about other forms of incremental solving?
        // internalize_formulas();  // DO NOT internalize while push
        m_solver.user_push();
        ++m_num_scopes;
        m_fmls_lim.push_back(m_fmls.size());
        m_asms_lim.push_back(m_asmsf.size());
        m_fmls_head_lim.push_back(m_fmls_head);
        if (m_bb_rewriter) m_bb_rewriter->push();
        m_map.push();
    }
    virtual void pop(unsigned n) {
        if (n > m_num_scopes) {   // allow inc_sat_solver to
            n = m_num_scopes;     // take over for another solver.
        }
        if (m_bb_rewriter) m_bb_rewriter->pop(n);
        m_map.pop(n);
        SASSERT(n <= m_num_scopes);
        m_solver.user_pop(n);
        m_num_scopes -= n;
        while (n > 0) {
            m_fmls_head = m_fmls_head_lim.back();
            m_fmls.resize(m_fmls_lim.back());
            m_fmls_lim.pop_back();
            m_fmls_head_lim.pop_back();
            m_asmsf.resize(m_asms_lim.back());
            m_asms_lim.pop_back();
            --n;
        }
    }
    virtual unsigned get_scope_level() const {
        return m_num_scopes;
    }
    virtual void assert_expr(expr * t, expr * a) {
        if (a) {
            m_asmsf.push_back(a);
            assert_expr(m.mk_implies(a, t));
        }
        else {
            assert_expr(t);
        }
    }
    virtual ast_manager& get_manager() const { return m; }

    // TODO: If t is complex enough, what about `m_fmls.push_tack(t.simplify()`?
    virtual void assert_expr(expr * t) {
        TRACE("sat", tout << mk_pp(t, m) << "\n";);
        m_fmls.push_back(t);
    }
    virtual void set_produce_models(bool f) {}
    virtual void collect_param_descrs(param_descrs & r) {
        goal2sat::collect_param_descrs(r);
        sat::solver::collect_param_descrs(r);
    }
    virtual void updt_params(params_ref const & p) {
        m_params = p;
        m_params.set_bool("elim_vars", false);
        m_solver.updt_params(m_params);
    }
    virtual void collect_statistics(statistics & st) const {
        if (m_preprocess) m_preprocess->collect_statistics(st);
        m_solver.collect_statistics(st);
    }
    virtual void get_unsat_core(ptr_vector<expr> & r) {
        NOT_IMPLEMENTED_YET();
    }
    virtual void get_model(model_ref & mdl) {
        if (!m_model.get()) {
            extract_model();
        }
        mdl = m_model;
    }
    virtual proof * get_proof() {
        UNREACHABLE();
        return 0;
    }

    virtual lbool get_consequences_core(expr_ref_vector const& assumptions, expr_ref_vector const& vars, expr_ref_vector& conseq) {
        NOT_IMPLEMENTED_YET();
    }

    virtual std::string reason_unknown() const {
        return m_unknown;
    }
    virtual void set_reason_unknown(char const* msg) {
        m_unknown = msg;
    }
    virtual void get_labels(svector<symbol> & r) {
    }
    virtual unsigned get_num_assertions() const {
        return m_fmls.size();
    }
    virtual expr * get_assertion(unsigned idx) const {
        return m_fmls[idx];
    }
    virtual unsigned get_num_assumptions() const {
        return m_asmsf.size();
    }
    virtual expr * get_assumption(unsigned idx) const {
        return m_asmsf[idx];
    }

    static tactic * main_pp(tactic* t) {
        params_ref p;
        p.set_bool("elim_and", true);
        p.set_bool("push_ite_bv", true);
        p.set_bool("blast_distinct", true);
        return using_params(t, p);
    }

    static tactic * mk_pp_inc_bv_preamble(ast_manager& m, params_ref const& p) {
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
        simp2_p.set_bool("elim_and", true); // pp test

        /*
        params_ref hoist_p;
        hoist_p.set_bool("hoist_mul", true);
        hoist_p.set_bool("som", false); */

#if 0
        return

                and_then(
                        mk_simplify_tactic(m),
                        mk_propagate_values_tactic(m),
                        using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                        mk_elim_uncnstr_tactic(m),
                        if_no_proofs(if_no_unsat_cores(mk_bv_size_reduction_tactic(m))),

                        using_params(mk_simplify_tactic(m), simp2_p),

                        mk_max_bv_sharing_tactic(m)
                        //if_no_proofs(if_no_unsat_cores(mk_ackermannize_bv_tactic(m,p)))
                );
#else
        return
            // Same with pp_qfbv_light
                and_then(
                        mk_simplify_tactic(m),
                        mk_propagate_values_tactic(m),
                        using_params(mk_solve_eqs_tactic(m), solve_eq_p),
                        mk_elim_uncnstr_tactic(m),
                        using_params(mk_simplify_tactic(m), simp2_p),
                        mk_max_bv_sharing_tactic(m)

                );
#endif

    }

    void init_preprocess() {
        if (m_preprocess) {
            m_preprocess->reset();
        }
        if (!m_bb_rewriter) {
            m_bb_rewriter = alloc(bit_blaster_rewriter, m, m_params);
        }


#if 0
        params_ref simp2_p = m_params;
        simp2_p.set_bool("som", true);
        simp2_p.set_bool("pull_cheap_ite", true);
        simp2_p.set_bool("push_ite_bv", false);
        simp2_p.set_bool("local_ctx", true);
        simp2_p.set_uint("local_ctx_limit", 10000000);
        simp2_p.set_bool("flat", true); // required by som
        simp2_p.set_bool("hoist_mul", false); // required by som
        simp2_p.set_bool("elim_and", true);
        m_preprocess =
            and_then(mk_card2bv_tactic(m, m_params),
                     using_params(mk_simplify_tactic(m), simp2_p),
                     mk_max_bv_sharing_tactic(m),
                     mk_bit_blaster_tactic(m, m_bb_rewriter.get()),
                     //mk_aig_tactic(),
                     //mk_propagate_values_tactic(m, simp2_p),
                     using_params(mk_simplify_tactic(m), simp2_p));


#else
        tactic* pp_inc_bv_preamble = mk_pp_inc_bv_preamble(m, m_params);
        m_preprocess =
                main_pp(and_then(pp_inc_bv_preamble,
                                mk_bit_blaster_tactic(m, m_bb_rewriter.get())
                        /*when(mk_lt(mk_memory_probe(), mk_const_probe(MEMLIMIT)),
                             using_params(and_then(mk_simplify_tactic(m),
                                                            mk_solve_eqs_tactic(m)),
                                                   local_ctx_p)))*/
                ));
        m_preprocess->updt_params(m_params);

#endif

        while (m_bb_rewriter->get_num_scopes() < m_num_scopes) {
            m_bb_rewriter->push();
        }
        m_preprocess->reset();
    }


private:
    // TODO: understand goal2sat better
    lbool internalize_goal(goal_ref& g, dep2asm_t& dep2asm) {
        m_mc.reset();
        m_pc.reset();
        m_dep_core.reset();
        m_subgoals.reset();
        init_preprocess();
        SASSERT(g->models_enabled());
        SASSERT(!g->proofs_enabled());
        TRACE("sat", g->display(tout););
        try {
            (*m_preprocess)(g, m_subgoals, m_mc, m_pc, m_dep_core);
        }
        catch (tactic_exception & ex) {
            //IF_VERBOSE(0, verbose_stream() << "exception in tactic " << ex.msg() << "\n";);
            TRACE("sat", tout << "exception: " << ex.msg() << "\n";);
            m_preprocess = 0;
            m_bb_rewriter = 0;
            return l_undef;
        }
        if (m_subgoals.size() != 1) {
            IF_VERBOSE(0, verbose_stream() << "size of subgoals is not 1, it is: " << m_subgoals.size() << "\n";);
            return l_undef;
        }
        g = m_subgoals[0];
        expr_ref_vector atoms(m);
        TRACE("sat", g->display_with_dependencies(tout););
        try {
            m_goal2sat(*g, m_params, m_solver, m_map, dep2asm, true);
            //std::cout << m_solver.num_vars() << std::endl;
            m_goal2sat.get_interpreted_atoms(atoms);
            if (!atoms.empty()) {
                std::stringstream strm;
                strm << "interpreted atoms sent to SAT solver " << atoms;
                TRACE("sat", tout << strm.str() << "\n";);
                IF_VERBOSE(1, verbose_stream() << strm.str() << "\n";);
                set_reason_unknown(strm.str().c_str());
                return l_undef;
            }
        }
        catch (tactic_exception & ex) {
            return l_undef;
        }

        return l_true;
    }

    lbool internalize_assumptions(unsigned sz, expr* const* asms, dep2asm_t& dep2asm) {
        if (sz == 0 && get_num_assumptions() == 0) {
            m_asms.shrink(0);
            return l_true;
        }
        goal_ref g = alloc(goal, m, true, true); // models and cores are enabled.
        for (unsigned i = 0; i < sz; ++i) {
            g->assert_expr(asms[i], m.mk_leaf(asms[i]));
        }
        for (unsigned i = 0; i < get_num_assumptions(); ++i) {
            g->assert_expr(get_assumption(i), m.mk_leaf(get_assumption(i)));
        }
        lbool res = internalize_goal(g, dep2asm);
        if (res == l_true) {
            extract_assumptions(sz, asms, dep2asm);
        }
        return res;
    }

    lbool internalize_formulas() {
        if (m_fmls_head == m_fmls.size()) {
            return l_true; // already processed
        }
        dep2asm_t dep2asm;
        goal_ref g = alloc(goal, m, true, false); // models, maybe cores are enabled
        for (unsigned i = m_fmls_head ; i < m_fmls.size(); ++i) {
            g->assert_expr(m_fmls[i].get());
        }
        lbool res = internalize_goal(g, dep2asm);
        if (res != l_undef) {
            m_fmls_head = m_fmls.size();
        }
        return res;
    }

    void extract_assumptions(unsigned sz, expr* const* asms, dep2asm_t& dep2asm) {
        m_asms.reset();
        unsigned j = 0;
        sat::literal lit;
        for (unsigned i = 0; i < sz; ++i) {
            if (dep2asm.find(asms[i], lit)) {
                SASSERT(lit.var() <= m_solver.num_vars());
                m_asms.push_back(lit);
                if (i != j && !m_weights.empty()) {
                    m_weights[j] = m_weights[i];
                }
                ++j;
            }
        }
        for (unsigned i = 0; i < get_num_assumptions(); ++i) {
            if (dep2asm.find(get_assumption(i), lit)) {
                SASSERT(lit.var() <= m_solver.num_vars());
                m_asms.push_back(lit);
            }
        }

        SASSERT(dep2asm.size() == m_asms.size());
    }

    void extract_asm2dep(dep2asm_t const& dep2asm, u_map<expr*>& asm2dep) {
        dep2asm_t::iterator it = dep2asm.begin(), end = dep2asm.end();
        for (; it != end; ++it) {
            expr* e = it->m_key;
            asm2dep.insert(it->m_value.index(), e);
        }
    }

    void extract_core(dep2asm_t& dep2asm) {
        NOT_IMPLEMENTED_YET();
    }

    void extract_model() {
        TRACE("sat", tout << "retrieve model " << (m_solver.model_is_current()?"present":"absent") << "\n";);
        if (!m_solver.model_is_current()) {
            m_model = 0;
            return;
        }
        sat::model const & ll_m = m_solver.get_model();
        model_ref md = alloc(model, m);
        atom2bool_var::iterator it  = m_map.begin();
        atom2bool_var::iterator end = m_map.end();
        for (; it != end; ++it) {
            expr * n   = it->m_key;
            if (is_app(n) && to_app(n)->get_num_args() > 0) {
                continue;
            }
            sat::bool_var v = it->m_value;
            switch (sat::value_at(v, ll_m)) {
                case l_true:
                    md->register_decl(to_app(n)->get_decl(), m.mk_true());
                    break;
                case l_false:
                    md->register_decl(to_app(n)->get_decl(), m.mk_false());
                    break;
                default:
                    break;
            }
        }
        m_model = md;

        if (m_bb_rewriter.get() && !m_bb_rewriter->const2bits().empty()) {
            m_mc0 = concat(m_mc0.get(), mk_bit_blaster_model_converter(m, m_bb_rewriter->const2bits()));
        }
        if (m_mc0) {
            (*m_mc0)(m_model);
        }
        SASSERT(m_model);

        DEBUG_CODE(
        for (unsigned i = 0; i < m_fmls.size(); ++i) {
            expr_ref tmp(m);
            if (m_model->eval(m_fmls[i].get(), tmp, true)) {
                CTRACE("sat", !m.is_true(tmp),
                       tout << "Evaluation failed: " << mk_pp(m_fmls[i].get(), m)
                            << " to " << tmp << "\n";
                model_smt2_pp(tout, m, *(m_model.get()), 0););
                SASSERT(m.is_true(tmp));
            }
        });
    }
};


solver* mk_pp_inc_bv_solver(ast_manager& m, params_ref const& p) {
    return alloc(pp_inc_bv_solver, m, p);
}


void pp_inc_bv_display(std::ostream& out, solver& _s, unsigned sz, expr*const* soft, rational const* _weights) {
    pp_inc_bv_solver& s = dynamic_cast<pp_inc_bv_solver&>(_s);
    vector<unsigned> weights;
    for (unsigned i = 0; _weights && i < sz; ++i) {
        if (!_weights[i].is_unsigned()) {
            throw default_exception("Cannot display weights that are not integers");
        }
        weights.push_back(_weights[i].get_unsigned());
    }
    s.display_weighted(out, sz, soft, weights.c_ptr());
}
