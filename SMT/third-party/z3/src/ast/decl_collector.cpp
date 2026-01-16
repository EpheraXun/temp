/*++
Copyright (c) 2011 Microsoft Corporation

Module Name:

    smt_decl_collector.cpp

Abstract:

    Collect uninterpreted func_delcs and sorts.
    This class was originally in ast_smt_pp.h

Author:

    Leonardo (leonardo) 2011-10-04

Revision History:

--*/
#include"decl_collector.h"

// pinpoint {
#include<sstream>
#include"bv_decl_plugin.h"
#include"arith_decl_plugin.h"
#include"warning.h"
#include"ast_pp.h"
#include"ast_smt2_pp.h"
#include"ast.h"
// }

void decl_collector::visit_sort(sort * n) {
    family_id fid = n->get_family_id();
    if (m().is_uninterp(n))
        m_sorts.push_back(n);
    if (fid == m_dt_fid)
        m_sorts.push_back(n);
}

bool decl_collector::is_bool(sort * s) {
    return m().is_bool(s);
}

void decl_collector::visit_func(func_decl * n) {
    family_id fid = n->get_family_id();
    if (fid == null_family_id) {
        if (m_sep_preds && is_bool(n->get_range()))
            m_preds.push_back(n);
        else
            m_decls.push_back(n);
    }        
}

decl_collector::decl_collector(ast_manager & m, bool preds):
    m_manager(m),
    m_sep_preds(preds) {
    m_basic_fid = m_manager.get_basic_family_id();
    m_dt_fid    = m_manager.mk_family_id("datatype");
}

void decl_collector::visit(ast* n) {
    ptr_vector<ast> todo;
    todo.push_back(n);
    while (!todo.empty()) {
        n = todo.back();
        todo.pop_back();
        if (!m_visited.is_marked(n)) {
            m_visited.mark(n, true);                
            switch(n->get_kind()) {
            case AST_APP: {
                app * a = to_app(n);
                for (unsigned i = 0; i < a->get_num_args(); ++i) {
                    todo.push_back(a->get_arg(i));
                }
                todo.push_back(a->get_decl());
                break;
            }                    
            case AST_QUANTIFIER: {
                quantifier * q = to_quantifier(n);
                unsigned num_decls = q->get_num_decls();
                for (unsigned i = 0; i < num_decls; ++i) {
                    todo.push_back(q->get_decl_sort(i));
                }
                todo.push_back(q->get_expr());
                for (unsigned i = 0; i < q->get_num_patterns(); ++i) {
                    todo.push_back(q->get_pattern(i));
                }
                break;
            }
            case AST_SORT: 
                visit_sort(to_sort(n));
                break;
            case AST_FUNC_DECL: {
                func_decl * d = to_func_decl(n);
                for (unsigned i = 0; i < d->get_arity(); ++i) {
                    todo.push_back(d->get_domain(i));
                }
                todo.push_back(d->get_range());
                visit_func(d);
                break;
            }
            case AST_VAR:
                break;
            default:
                UNREACHABLE();
            }
        }
    }
}

void decl_collector::visit_bv(ast* n, std::vector<double>& tmp) {
    ptr_vector<ast> todo;
    todo.push_back(n);
    while (!todo.empty()) {
        n = todo.back();
        todo.pop_back();
        if (!m_visited.is_marked(n)) {
            m_visited.mark(n, true);
            switch(n->get_kind()) {
                case AST_APP: {
                    app * a = to_app(n);
                    for (unsigned i = 0; i < a->get_num_args(); ++i) {
                        todo.push_back(a->get_arg(i));
                    }
                    todo.push_back(a->get_decl());
                    decl_kind kind_a = a->get_decl()->get_decl_kind();
#if 0
                    if (kind_a == OP_AND)              tmp[0]++;  // and         0
                    else if (kind_a == OP_EQ)          tmp[1]++;  // =
                    else if (kind_a == OP_TRUE)        tmp[2]++;  // true   2
                    //else if (kind_a ==  )        tmp[3]++;      // kinds   3
                    else if (kind_a == OP_DISTINCT)    tmp[4]++;  // distinct   4
                    else if (kind_a == OP_BLSHR)       tmp[5]++;  // bvlshr   5
                    else if (kind_a == OP_ZERO_EXT)    tmp[6]++;  // zero_extend  6
                    else if (kind_a == OP_EXTRACT)     tmp[7]++;  // extract   7
                    else if (kind_a == OP_SGEQ)        tmp[8]++;  // bvsge   8
                    else if (kind_a == OP_BXOR)        tmp[9]++;  // bvxor   9
                    else if (kind_a == OP_BAND)        tmp[10]++; // bvand   10
                    else if (kind_a == OP_NOT)         tmp[11]++; // not      11
                    else if (kind_a == OP_SIGN_EXT)    tmp[12]++; // sign_extend  12
                    else if (kind_a == OP_OR)          tmp[13]++; // or    13
                    else if (kind_a == OP_ITE)         tmp[14]++; // if     14 (iff?)
                    else if (kind_a == OP_UGT)         tmp[15]++; // bvugt   15
                    else if (kind_a == OP_BOR)         tmp[16]++; // bvor     16
                    else if (kind_a == OP_BMUL)        tmp[17]++; // bvmul   17
                    else if (kind_a == OP_BSUB)        tmp[18]++; // bvsub   18
                    else if (kind_a == OP_ULT)         tmp[19]++; // bvult  19
                    else if (kind_a == OP_BADD)        tmp[20]++; // bvadd   20
                    else if (kind_a == OP_BSHL)        tmp[21]++; // bvshl   21
                    else if (kind_a == OP_ULEQ)        tmp[22]++; // bvule   22
                    else if (kind_a == OP_UGEQ)        tmp[23]++; // bvuge   23
                    else if (kind_a == OP_SGT)         tmp[24]++; // bvsgt    24
                    else if (kind_a == OP_SLEQ)        tmp[25]++; // bvsle   25
                    else if (kind_a == OP_SLT)         tmp[26]++; // bvslt   26
                    else if (kind_a == OP_BSDIV)       tmp[27]++; // bvsdiv 27
                    else if (kind_a == OP_BUDIV)       tmp[28]++; // bvudiv  28
                    else if (kind_a == OP_BUREM)       tmp[29]++; // bvurem   29
                    else if (kind_a == OP_BASHR)       tmp[30]++; // bvashr    30
                    else if (kind_a == OP_BSREM)       tmp[31]++; // bvsrem   31
                    break;

#else
                    switch (kind_a) {
                    case OP_AND:      tmp[0]++;  break;  // and 0
                    case OP_EQ:       tmp[1]++;  break;  // =  1
                    case OP_TRUE:     tmp[2]++;  break;  // True 2
                    //kind_a ==  )    tmp[3]++;      // kinds   3
                    case OP_DISTINCT: tmp[4]++;  break;  // distinct
                    case OP_BLSHR:    tmp[5]++;  break;  // bvlshr   5
                    case OP_ZERO_EXT: tmp[6]++;  break;  // zero_extend 6
                    case OP_EXTRACT:  tmp[7]++;  break;
                    case OP_SGEQ:     tmp[8]++;  break;
                    case OP_BXOR:     tmp[9]++;  break;
                    case OP_BAND:     tmp[10]++; break;
                    case OP_NOT:      tmp[11]++; break;
                    case OP_SIGN_EXT: tmp[12]++; break;
                    case OP_OR:       tmp[13]++; break; // or    13
                    case OP_ITE:      tmp[14]++; break; // if    14 (iff?)
                    case OP_UGT:      tmp[15]++; break; // bvugt 15
                    case OP_BOR:      tmp[16]++; break; // bvor  16
                    case OP_BMUL:     tmp[17]++; break; // bvmul 17
                    case OP_BSUB:     tmp[18]++; break; // bvsub 18
                    case OP_ULT:      tmp[19]++; break; // bvult 19
                    case OP_BADD:     tmp[20]++; break; // bvadd 20
                    case OP_BSHL:     tmp[21]++; break; // bvshl 21
                    case OP_ULEQ:     tmp[22]++; break; // bvule 22
                    case OP_UGEQ:     tmp[23]++; break; // bvuge 23
                    case OP_SGT:      tmp[24]++; break; // bvsgt 24
                    case OP_SLEQ:     tmp[25]++; break; // bvsle 25
                    case OP_SLT:      tmp[26]++; break; // bvslt 26
                    case OP_BSDIV:    tmp[27]++; break; // bvsdiv 27
                    case OP_BUDIV:    tmp[28]++; break; // bvudiv 28
                    case OP_BUREM:    tmp[29]++; break; // bvurem 29
                    case OP_BASHR:    tmp[30]++; break; // bvashr 30
                    case OP_BSREM:    tmp[31]++; break; // bvsrem 31
                    default: break;
                    }
                    break;
#endif
                }
                case AST_QUANTIFIER: {
                    quantifier * q = to_quantifier(n);
                    unsigned num_decls = q->get_num_decls();
                    for (unsigned i = 0; i < num_decls; ++i) {
                        todo.push_back(q->get_decl_sort(i));
                    }
                    todo.push_back(q->get_expr());
                    for (unsigned i = 0; i < q->get_num_patterns(); ++i) {
                        todo.push_back(q->get_pattern(i));
                    }
                    break;
                }
                case AST_SORT:
                    visit_sort(to_sort(n));
                    break;
                case AST_FUNC_DECL: {
                    func_decl * d = to_func_decl(n);
                    //if (d->get_decl_kind() == OP_BADD) res++;
                    for (unsigned i = 0; i < d->get_arity(); ++i) {
                        todo.push_back(d->get_domain(i));
                    }
                    todo.push_back(d->get_range());
                    visit_func(d);
                    break;
                }
                case AST_VAR: {
                    break;
                }
                default:
                    UNREACHABLE();
            }
        }
    }

}



// }
