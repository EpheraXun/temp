/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: bv_under_approximation
 *     Solve QF_BV with under-approximation
 *      - zero-extend abstraction(TACAS 07)
 *      - sign-extend abstraction(TACAS 07)
 *      - segment-partition abstraction
 *      - randomized-search abstraction
 *      - value-restriction abstraction
 *      - ...?
 *     Instead of applying iterative refinement, call another complete solver if returning unsat.
 *
 * Creation Date:  2017.
 * Modification History:
*/

#include"bv_under_approximation.h"
#include"tactical.h"
#include"bv_decl_plugin.h"
#include"expr_replacer.h"
#include"extension_model_converter.h"
#include"filter_model_converter.h"
#include"ast_smt2_pp.h"


class bv_under_approximation_tactic : public tactic {
    struct imp;
    imp *      m_imp;
public:
    bv_under_approximation_tactic(ast_manager & m);

    virtual tactic * translate(ast_manager & m) {
        return alloc(bv_under_approximation_tactic, m);
    }

    virtual ~bv_under_approximation_tactic();

    virtual void operator()(goal_ref const & g, goal_ref_buffer & result, model_converter_ref & mc, proof_converter_ref & pc, expr_dependency_ref & core);

    virtual void cleanup();
};

tactic * mk_bv_under_approximation_tactic(ast_manager & m, params_ref const & p) {
    return clean(alloc(bv_under_approximation_tactic, m));
}

struct bv_under_approximation_tactic::imp {
    typedef rational numeral;

    ast_manager &                  m;
    bv_util                        m_util;
    expr_substitution              m_subst;

    imp(ast_manager & _m):
        m(_m),
        m_util(m),
        m_subst(m){
    }

    class get_uninterp_proc {
        imp& m_imp;
        ptr_vector<app> m_vars;
        bool m_in_supported_fragment;
    public:
        get_uninterp_proc(imp& s): m_imp(s), m_in_supported_fragment(true) {}
        ptr_vector<app> const& vars() { return m_vars; }
        void operator()(var * n) {
            m_in_supported_fragment = false;
        }
        void operator()(app* n) {
            bv_util& a = m_imp.m_util;
            ast_manager& m = a.get_manager();
            if (a.is_bv(n) &&
                is_uninterp_const(n)) {
                m_vars.push_back(n);
            }
            else if (m.is_bool(n) && is_uninterp_const(n)) {

            }
        }
        void operator()(quantifier* q) {
            m_in_supported_fragment = false;
        }
        bool is_supported() const { return m_in_supported_fragment; }
    };

    bool collect_vars(goal const & g) {
        get_uninterp_proc fe_var(*this);
        for_each_expr_at(fe_var, g);
        //std::cout << "num of vars: " << fe_var.vars().size() << std::endl;
        /*for (unsigned i = 0; i < fe_var.vars().size(); ++i) {
        	std::cout << fe_var.vars()[i]->get_size() << std::endl;
        }*/
        return fe_var.is_supported() && !fe_var.vars().empty();
    }

    void substitute_vars(goal & g) {
        scoped_ptr<expr_replacer> er = mk_default_expr_replacer(m);
        er->set_substitution(&m_subst);
        expr_ref r(m);
        for (unsigned i = 0; i < g.size(); ++i) {
            (*er)(g.form(i), r);
            g.update(i, r);
        }
    }

    // Given a w-bit BitVec P, use a s-bit(s <= w) BitVec U to encode it, and extend U with zero to w-bit.
    void zero_extend_abstraction(goal & g) {
        get_uninterp_proc fe_var(*this);
        for_each_expr_at(fe_var, g);
        //std::cout << fe_var.vars().size() << std::endl;
        for (unsigned i = 0; i < fe_var.vars().size(); ++i) {
            app* v = fe_var.vars()[i];
            unsigned bv_sz = v->get_size();
            if (bv_sz == 32) {
                app * w = m.mk_fresh_const(0, m_util.mk_sort(8));
                app * zero_extended_w = m_util.mk_zero_extend(24, w);
                m_subst.insert(v, zero_extended_w);
            } else if (bv_sz == 64) {
                app * w = m.mk_fresh_const(0, m_util.mk_sort(16));
                app * zero_extended_w = m_util.mk_zero_extend(48, w);
                m_subst.insert(v, zero_extended_w);
            }
        }
        substitute_vars(g);
        m_subst.reset();
    }


    void value_restriction_abstraction(goal & g) {
        get_uninterp_proc fe_var(*this);
        for_each_expr_at(fe_var, g);
        for (unsigned i = 0; i < fe_var.vars().size(); ++i) {
            app* v = fe_var.vars()[i];
            if (v->get_size() == 32) {
                app * new_restriction = m_util.mk_sle(v, m_util.mk_numeral(10000, 32));
                g.assert_expr(new_restriction);
            }
        }
    }

    void sign_extend_abstraction(goal & g) {
    }

    void segment_partition_abstraction(goal & g) {
    }

    void randomized_search_abstraction(goal & g) {
    }

    void checkpoint() {
        if (m.canceled())
            throw tactic_exception(m.limit().get_cancel_msg());
    }

    void operator()(goal & g, model_converter_ref & mc) {
    	zero_extend_abstraction(g);
    	//value_restriction_abstraction(g);
    }
};

bv_under_approximation_tactic::bv_under_approximation_tactic(ast_manager & m) {
    m_imp = alloc(imp, m);
}

bv_under_approximation_tactic::~bv_under_approximation_tactic() {
    dealloc(m_imp);
}

void bv_under_approximation_tactic::operator()(goal_ref const & g,
                                          goal_ref_buffer & result,
                                          model_converter_ref & mc,
                                          proof_converter_ref & pc,
                                          expr_dependency_ref & core) {
    SASSERT(g->is_well_sorted());
    fail_if_proof_generation("bv-under-approximation", g);
    fail_if_unsat_core_generation("bv-under-approximation", g);
    mc = 0; pc = 0; core = 0; result.reset();
    m_imp->operator()(*(g.get()), mc);
    g->inc_depth();
    result.push_back(g.get());
    SASSERT(g->is_well_sorted());
}


void bv_under_approximation_tactic::cleanup() {
    imp * d = alloc(imp, m_imp->m);
    std::swap(d, m_imp);
    dealloc(d);
}


