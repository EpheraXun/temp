/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: SMT expressions
 * Creation Date:
 * Modification History:
 */

#include <vector>
#include <set>
#include <functional>
#include <iostream>
#include <fstream>
#include <llvm/Support/Debug.h>
#include <llvm/Support/CommandLine.h>
#include "SMT/SMTExpr.h"
#include "SMT/SMTFactory.h"

static llvm::cl::opt<std::string> DumpingOptsDst("dump-opt-dst", llvm::cl::init(""),
        llvm::cl::desc("Dump the symbolic optimization constraints in dst"));

static llvm::cl::opt<std::string> HelpString("help-string", llvm::cl::init(""),
        llvm::cl::desc("Dump the symbolic optimization constraints in dst"));
SMTExpr::SMTExpr(SMTFactory* F, z3::expr Z3Expr) : SMTObject(F),
        Expr(Z3Expr) {
}

SMTExpr::SMTExpr(SMTExpr const & E) : SMTObject(E),
        Expr(E.Expr) {
}

SMTExpr& SMTExpr::operator=(const SMTExpr& E) {
    SMTObject::operator =(E);
    if (this != &E) {
        this->Expr = E.Expr;
    }
    return *this;
}

SMTExpr SMTExpr::substitute(SMTExprVec& From, SMTExprVec& To) {
    assert(From.size() == To.size());
    if (From.empty()) {
        return *this;
    }
    return SMTExpr(&getSMTFactory(), Expr.substitute(*From.ExprVec, *To.ExprVec));
}

SMTExpr SMTExpr::localSimplify() {
    // TODO: customize parameters for `simplify()` method
    return SMTExpr(&getSMTFactory(), Expr.simplify());
}

SMTExpr SMTExpr::ctxSimplify() {
    // apply contextual simplification rules
    z3::goal G(Expr.ctx());
    G.add(Expr);
    z3::tactic CS = z3::tactic(Expr.ctx(), "ctx-simplify");
    return SMTExpr(&getSMTFactory(), CS.apply(G)[0].as_expr());
}


SMTExpr SMTExpr::doConstantPropagation() {
    // TODO: figure out whether CP will eliminate variables.
    z3::goal G(Expr.ctx());
    G.add(Expr);
    z3::tactic CP = z3::tactic(Expr.ctx(), "propagate-values");
    return SMTExpr(&getSMTFactory(), CP.apply(G)[0].as_expr());
}

SMTExpr SMTExpr::elimValidOrUnsatExpr() {
    z3::context& Ctx = Expr.ctx();
    z3::solver S1(Ctx); S1.add(Expr);
    z3::solver S2(Ctx); S2.add(!Expr);
    if (S1.check() == z3::unsat) {
        return SMTExpr(&getSMTFactory(), Ctx.bool_val(false));
    } else if (S2.check() == z3::unsat) {
        return SMTExpr(&getSMTFactory(), Ctx.bool_val(true));
    } else {
        return *this;
    }
}

SMTExpr SMTExpr::forgetVars() {
    // TODO: design new qe procedures
    // TODO: set timeout for each query
    if (!Expr.is_quantifier()) {
        // If Expr is not a quantified formula, then do not apply QE.
        return *this;
    } else {
        z3::context& Ctx = Expr.ctx();
        z3::tactic QE = z3::tactic(Ctx, "qe2");
        z3::goal G(Ctx);
        G.add(Expr);
        return SMTExpr(&getSMTFactory(), QE.apply(G)[0].as_expr());
    }
}

SMTExpr SMTExpr::forgetVar(SMTExpr& Var) {
    // Expr is not quantified, and we need to `forget' Var
    z3::context& Ctx = Expr.ctx();
    z3::tactic QE = z3::tactic(Ctx, "qe2");
    z3::goal G(Ctx);
    G.add(z3::exists(Var.Expr, Expr));
    return SMTExpr(&getSMTFactory(), QE.apply(G)[0].as_expr());
}

SMTExpr SMTExpr::forgetVars(SMTExprVec& Vars) {
    // Expr is not quantified, and we need to `forget' Vars
    z3::context& Ctx = Expr.ctx();
    z3::tactic QE = z3::tactic(Ctx, "qe2");
    z3::goal G(Ctx);
    G.add(z3::exists(*Vars.ExprVec, Expr));
    return SMTExpr(&getSMTFactory(), QE.apply(G)[0].as_expr());
}

bool SMTExpr::getExprVars(SMTExprVec& Vars) {
    try {
        auto& ctx = Expr.ctx();
        auto compare_func = [](const z3::expr& a, const z3::expr& b) {
            Z3_symbol sym_a = a.decl().name();
            Z3_symbol sym_b = b.decl().name();
            return sym_a < sym_b;
        };
        std::set<z3::expr, decltype(compare_func)> syms(compare_func);
        std::function<void(const z3::expr&)> recur = [&recur, &syms, &ctx](
                const z3::expr& e) {
            assert(Z3_is_app(ctx, e));
            auto app = Z3_to_app(ctx, e);
            unsigned n_args = Z3_get_app_num_args(ctx, app);

            auto fdecl = Z3_get_app_decl(ctx, app);
            if (n_args == 0 && Z3_get_decl_kind(ctx, fdecl) == Z3_OP_UNINTERPRETED)
                syms.insert(e);

            for (unsigned i = 0; i < n_args; ++i) {
                z3::expr arg(ctx, Z3_get_app_arg(ctx, app, i));
                recur(arg);
            }
        };
        recur(Expr);
        for (auto& i : syms) {
            Vars.push_back(SMTExpr(&getSMTFactory(), i));
        }
    } catch (z3::exception & ex) {
        return false;
    }
    return true;
}

bool SMTExpr::soundAbstractInterval(SMTExpr& Query, SMTExprVec& Res) {
    z3::context& ctx = Expr.ctx();
    z3::tactic get_bound = z3::tactic(ctx, "simplify") &
            z3::tactic(ctx, "propagate-values") & z3::tactic(ctx, "bv-bounds-collect");
    z3::goal g(ctx); g.add(Expr);
    z3::apply_result simple_bound = get_bound.apply(g);

    bool lower_success = false, upper_success = false;

    if (simple_bound.size() >= 1) {
        z3::goal bound = simple_bound[0];
        for (unsigned i = 0; i < bound.size(); i++) {
            if (bound[i].decl().decl_kind() == Z3_OP_SLEQ || bound[i].decl().decl_kind() == Z3_OP_ULEQ) {
                if (bound[i].arg(1).decl().name() == Query.Expr.decl().name()) {
                    // k < = x, k is a safe lower bound
                    lower_success = true;
                    Res.push_back(SMTExpr(&getSMTFactory(), bound[i].arg(1)));
                    break;
                }
            }
        }
        if (!lower_success) Res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));

        for (unsigned i = 0; i < bound.size(); i++) {
            if (bound[i].decl().decl_kind() == Z3_OP_SLEQ || bound[i].decl().decl_kind() == Z3_OP_ULEQ) {
                if (bound[i].arg(0).decl().name() == Query.Expr.decl().name()) {
                    // x <= k, k is a safe upper bound
                    upper_success = true;
                    Res.push_back(SMTExpr(&getSMTFactory(), bound[i].arg(1)));
                    break;
                }
            }
        }
        if (!upper_success) Res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
    } else {
        Res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
        Res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
    }
    return true;
}


// dump the symbolic optimization constraints
// Queries is the set of objectives to optimize.
void SMTExpr::dumpOptimizeObjectives(SMTExprVec& Queries) {
    z3::context& Ctx = Expr.ctx();
    z3::optimize Opt(Ctx);
    z3::params Param(Ctx);
    Param.set("priority", Ctx.str_symbol("box"));
    Opt.set(Param);
    Opt.add(Expr);

    for (unsigned i = 0; i < Queries.size(); i++) {
        Opt.minimize(Queries[i].Expr);
        Opt.maximize(Queries[i].Expr);
    }

    if (DumpingOptsDst.getNumOccurrences()) {
        // output the constraints to a temp file in the dst
        std::string DstFileName = DumpingOptsDst.getValue();
        DstFileName.append("/case");
        DstFileName.append(std::to_string(clock()));
        DstFileName.append(".opt.smt2");

        std::ofstream DstFile;
        DstFile.open(DstFileName);

        if (DstFile.is_open()) {
            DstFile << Opt << "\n";
            DstFile.close();
        } else {
            std::cerr << "File cannot be opened: " << DstFileName << "\n";
        }
    }
}

void SMTExpr::bestAbstractInterval(SMTExpr& Query, SMTExprVec& Res, std::string method, int Timeout) {
    if (method == "opt") {
        bestAbstractIntervalWithOPT(Query, Res, Timeout);
    } else if (method == "qsmt") {
        bestAbstractIntervalWithQSMT(Query, Res, Timeout);
    } else if (method == "lin_search") {
        bestAbstractIntervalWithLinearSearch(Query, Res, Timeout);
    } else {
        bestAbstractIntervalWithOPT(Query, Res, Timeout);
    }
}


void SMTExpr::bestAbstractIntervalWithLinearSearch(SMTExpr& Query, SMTExprVec& Res, int Timeout) {
    z3::context& ctx = Expr.ctx();
    // for "fix" over-flow problem in signed bit-vec
    z3::expr PreCond = Expr && Query.Expr >= 0;
    //Expr = Expr && Query.Expr >= 0;
    z3::params p(ctx);
    p.set("timeout", (unsigned)Timeout);
    z3::solver sol(ctx);
    sol.add(PreCond);     sol.set(p);
    z3::solver sol_two(ctx);
    sol_two.add(PreCond); sol_two.set(p);

    // find lower bound
    try {
        z3::expr lower = ctx.bv_val(0, Query.Expr.get_sort().bv_size());
        while (sol_two.check() == z3::sat) {
            z3::model m = sol_two.get_model();
            lower = m.eval(Query.Expr);
            sol_two.add(Query.Expr < lower);
        }
        Res.push_back(SMTExpr(&getSMTFactory(), lower));
    } catch(z3::exception &ex) {
        Res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
    }

    // find upper bound
    try {
        z3::expr upper = ctx.bv_val(0, Query.Expr.get_sort().bv_size());
        while (sol.check() == z3::sat) {
            z3::model m = sol.get_model();
            upper = m.eval(Query.Expr);
            sol.add(Query.Expr > upper);
        }
        Res.push_back(SMTExpr(&getSMTFactory(), upper));
    } catch(z3::exception &ex) {
        Res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
    }
}

void SMTExpr::bestAbstractIntervalWithOPT(SMTExpr& Query, SMTExprVec& Res, int Timeout) {
    z3::context& Ctx = Expr.ctx();
    z3::expr PreCond = Expr && Query.Expr >= 0;
    //Expr = Expr && Query.Expr >= 0;    // for "fix" over-flow problem in signed bit-vec
    z3::params Param(Ctx);
    Param.set("priority", Ctx.str_symbol("pareto"));
    z3::set_param("smt.timeout", Timeout);
    //TODO: it seems we cannot set timeout directly to opt
    //p.set("timeout", Timeout);
    z3::optimize UpperFinder(Ctx);
    z3::optimize LowerFinder(Ctx);
    UpperFinder.set(Param); LowerFinder.set(Param);
    UpperFinder.add(PreCond);

    z3::optimize::handle UpperGoal = UpperFinder.maximize(Query.Expr);
    LowerFinder.add(PreCond);
    z3::optimize::handle LowerGoal = LowerFinder.minimize(Query.Expr);

    // lower bound
    try {
        if (LowerFinder.check() == z3::sat) {
            Res.push_back(SMTExpr(&getSMTFactory(), LowerFinder.upper(LowerGoal)));
        } else {
            Res.push_back(SMTExpr(&getSMTFactory(), Ctx.bool_val(false)));
        }
    } catch(z3::exception &Ex) {
        Res.push_back(SMTExpr(&getSMTFactory(), Ctx.bool_val(false)));
    }

    // upper bound
    try {
        if (UpperFinder.check() == z3::sat) {
            Res.push_back(SMTExpr(&getSMTFactory(), UpperFinder.lower(UpperGoal)));
        } else {
            Res.push_back(SMTExpr(&getSMTFactory(), Ctx.bool_val(false)));
        }
    } catch(z3::exception &Ex) {
        Res.push_back(SMTExpr(&getSMTFactory(), Ctx.bool_val(false)));
    }
}


void SMTExpr::bestAbstractIntervalWithQSMT(SMTExpr& query, SMTExprVec& res, int Timeout) {
    z3::context& ctx = Expr.ctx();
    z3::expr PreCond = Expr && query.Expr >= 0;
    //Expr = Expr && query.Expr >= 0;    // for "fix" over-flow problem in signed bit-vec
    z3::params p(ctx);
    p.set("timeout", (unsigned)Timeout);
    z3::solver sol_min = z3::tactic(ctx, "ufbv").mk_solver();
    sol_min.set(p);
    z3::solver sol_max = z3::tactic(ctx, "ufbv").mk_solver();
    sol_max.set(p);

    // find min
    z3::expr query_min = ctx.bv_const("rot_min", query.Expr.get_sort().bv_size());
    Z3_ast from[] = { query.Expr };
    Z3_ast to[] = { query_min };
    z3::expr repl_min(ctx);
    repl_min = z3::to_expr(ctx, Z3_substitute(ctx, PreCond, 1, from, to));

    z3::expr qsmt_min = PreCond && z3::forall(query_min, z3::implies(repl_min, query_min >= query.Expr));
    sol_min.add(qsmt_min);
    try {
        if (sol_min.check() == z3::sat) {
            z3::model m = sol_min.get_model();
            z3::expr lower = m.eval(query.Expr);
            res.push_back(SMTExpr(&getSMTFactory(), lower));
        } else {
            res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
        }
    } catch(z3::exception &Ex) {
        res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
    }

    // find max
    z3::expr query_max = ctx.bv_const("rot_max", query.Expr.get_sort().bv_size());
    Z3_ast from_x[] = { query.Expr };
    Z3_ast to_x[] = { query_max };
    z3::expr repl_max(ctx);
    repl_max = z3::to_expr(ctx, Z3_substitute(ctx, PreCond, 1, from_x, to_x));
    z3::expr qsmt_max = PreCond && z3::forall(query_max, z3::implies(repl_max, query_max <= query.Expr));
    sol_max.add(qsmt_max);
    try {
        if (sol_max.check() == z3::sat) {
            z3::model m = sol_max.get_model();
            z3::expr upper = m.eval(query.Expr);
            res.push_back(SMTExpr(&getSMTFactory(), upper));
        } else {
            res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
        }
    } catch(z3::exception &Ex) {
        res.push_back(SMTExpr(&getSMTFactory(), ctx.bool_val(false)));
    }
}


SMTExpr SMTExpr::dilligSimplify(SMTExpr Expr2Simplify, z3::solver& Solver4Simplify, z3::context& Z3Ctx) {
    if (!Expr2Simplify.isLogicAnd() && !Expr2Simplify.isLogicOr()) {
        // A leaf
        Solver4Simplify.push();
        Solver4Simplify.add(Expr2Simplify.Expr);
        if (Solver4Simplify.check() == z3::check_result::unsat) {
            Solver4Simplify.pop();
            return SMTExpr(&getSMTFactory(), Z3Ctx.bool_val(false));
        }
        Solver4Simplify.pop();
        Solver4Simplify.push();
        Solver4Simplify.add(!Expr2Simplify.Expr);
        if (Solver4Simplify.check() == z3::check_result::unsat) {
            Solver4Simplify.pop();
            return SMTExpr(&getSMTFactory(), Z3Ctx.bool_val(true));
        }
        Solver4Simplify.pop();
        return Expr2Simplify;
    } else {
        // A connective (AND or OR)
        assert(Expr2Simplify.isLogicAnd() || Expr2Simplify.isLogicOr());

        std::vector<SMTExpr> C;
        std::set<SMTExpr, SMTExprComparator> CSet;
        for (unsigned I = 0, E = Expr2Simplify.numArgs(); I < E; I++) {
            if (Expr2Simplify.getArg(I).isTrue()) {
                if (Expr2Simplify.isLogicAnd()) {
                    continue;
                } else if (Expr2Simplify.isLogicOr()) {
                    return SMTExpr(&getSMTFactory(), Z3Ctx.bool_val(true));
                }
            } else if (Expr2Simplify.getArg(I).isFalse()) {
                if (Expr2Simplify.isLogicAnd()) {
                    return SMTExpr(&getSMTFactory(), Z3Ctx.bool_val(false));
                } else if (Expr2Simplify.isLogicOr()) {
                    continue;
                }
            }

            if (!CSet.count(Expr2Simplify.getArg(I))) {
                C.push_back(Expr2Simplify.getArg(I));
                CSet.insert(Expr2Simplify.getArg(I));
            }
        }

        bool ReachFixedPoint = false;
        while (!ReachFixedPoint) {
            ReachFixedPoint = true;

            for (size_t I = 0, E = C.size(); I < E; ++I) {
                Z3_ast* Args = new Z3_ast[C.size() - 1];
                for (size_t J = 0; J < C.size() - 1; J++) {
                    SMTExpr *Candidate = nullptr;
                    if (J < I) {
                        Candidate = &C[J];
                    } else {
                        assert(J + 1 < C.size());
                        Candidate = &C[J + 1];
                    }

                    if (Expr2Simplify.isLogicOr()) {
                        Args[J] = !(*Candidate).Expr;
                    } else {
                        Args[J] = (*Candidate).Expr;
                    }
                }
                SMTExpr Alpha(&getSMTFactory(), to_expr(Z3Ctx, Z3_mk_and(Z3Ctx, C.size() - 1, Args)));
                delete[] Args;

                SMTExpr& Ci = C[I];

                Solver4Simplify.push();
                Solver4Simplify.add(Alpha.Expr);
                SMTExpr NewCi = dilligSimplify(Ci, Solver4Simplify, Z3Ctx);
                Solver4Simplify.pop();

                if (!z3::eq(Ci.Expr, NewCi.Expr)) {
                    if (ReachFixedPoint)
                        ReachFixedPoint = false;
                    C[I] = NewCi;
                }

                if (NewCi.isTrue() && Expr2Simplify.isLogicOr()) {
                    return SMTExpr(&getSMTFactory(), Z3Ctx.bool_val(true));
                } else if (NewCi.isFalse() && Expr2Simplify.isLogicAnd()) {
                    return SMTExpr(&getSMTFactory(), Z3Ctx.bool_val(false));
                }
            }

            // FIXME
            ReachFixedPoint = true;
        }

        if (Expr2Simplify.isLogicAnd()) {
            Z3_ast* Args = new Z3_ast[C.size()];
            size_t J = 0;
            for (size_t I = 0; I < C.size(); I++) {
                if (C[I].isTrue()) {
                    continue;
                }
                Args[J++] = C[I].Expr;
            }

            if (J == 1) {
                SMTExpr Ret(&getSMTFactory(), to_expr(Z3Ctx, Args[0]));
                delete[] Args;
                return Ret;
            }

            if (J == 0) {
                delete[] Args;
                return SMTExpr(&getSMTFactory(), Z3Ctx.bool_val(true));
            }

            SMTExpr Ret(&getSMTFactory(), to_expr(Z3Ctx, Z3_mk_and(Z3Ctx, J, Args)));
            delete[] Args;

            return Ret;
        } else {
            // is logic OR
            Z3_ast* Args = new Z3_ast[C.size()];
            size_t J = 0;
            for (size_t I = 0; I < C.size(); I++) {
                if (C[I].isFalse()) {
                    continue;
                }
                Args[J++] = C[I].Expr;
            }

            if (J == 1) {
                SMTExpr Ret(&getSMTFactory(), to_expr(Z3Ctx, Args[0]));
                delete[] Args;
                return Ret;
            }

            if (J == 0) {
                delete[] Args;
                return SMTExpr(&getSMTFactory(), Z3Ctx.bool_val(false));
            }

            SMTExpr Ret(&getSMTFactory(), to_expr(Z3Ctx, Z3_mk_or(Z3Ctx, J, Args)));
            delete[] Args;

            return Ret;
        }
    }
}

SMTExpr SMTExpr::dilligSimplify() {
    z3::context& Ctx = Expr.ctx();
    z3::solver Solver4Sim(Ctx);
    Solver4Sim.add(Ctx.bool_val(true));

    SMTExpr AftSim = dilligSimplify(*this, Solver4Sim, Ctx);
    return AftSim;
}

unsigned SMTExpr::size(std::map<SMTExpr, unsigned, SMTExprComparator>& SizeCache) {
    if (!this->isLogicAnd() && !this->isLogicOr()) {
        if (isLogicNot())
            return this->getArg(0).size(SizeCache);
        else
            return 1;
    } else {
        if (SizeCache.count(*this))
            return 0;

        unsigned Size = 0;
        for (unsigned Index = 0, NumArgs = numArgs(); Index < NumArgs; Index++) {
            Size += this->getArg(Index).size(SizeCache);
        }
        SizeCache.insert(std::make_pair(*this, Size));
        return Size;
    }
}

SMTExpr SMTExpr::getQuantifierBody() const {
    return SMTExpr(&getSMTFactory(), Expr.body());
}

SMTExpr SMTExpr::getArg(unsigned I) const {
    return SMTExpr(&getSMTFactory(), Expr.arg(I));
}

SMTExpr SMTExpr::bv12bool() {
    auto* Factory = &getSMTFactory();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_bv());
        unsigned bvSize = Expr.get_sort().array_range().bv_size();
        assert(bvSize == 1);
        auto func = (Expr.ctx().bv_val("0", 1) == Expr.ctx().bv_val("0", 1)).decl();

        z3::expr const_bv1 = const_array(Expr.get_sort().array_domain(), Expr.ctx().bv_val("1", 1));
        Z3_ast mapargs[2] = { Expr, const_bv1 };

        return SMTExpr(Factory, z3::expr(Expr.ctx(), Z3_mk_map(Expr.ctx(), func, 2, mapargs)));
    } else {
        assert(Expr.is_bv() && Expr.get_sort().bv_size() == 1);
        return SMTExpr(Factory, Expr == Expr.ctx().bv_val(1, 1));
    }
}

SMTExpr SMTExpr::bool2bv1() {
    auto* Factory = &getSMTFactory();
    z3::context& ctx = Expr.ctx();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_bool());
        auto func = ite(ctx.bool_val(false), ctx.bv_val(1, 1), ctx.bv_val(0, 1)).decl();

        z3::expr const_bv0 = const_array(Expr.get_sort().array_domain(), ctx.bv_val(0, 1));
        z3::expr const_bv1 = const_array(Expr.get_sort().array_domain(), ctx.bv_val(1, 1));

        Z3_ast mapargs[3] = { Expr, const_bv0, const_bv1 };

        z3::expr bvret(ctx, Z3_mk_map(ctx, func, 3, mapargs));
        return SMTExpr(Factory, bvret);
    } else {
        assert(Expr.is_bool());
        return SMTExpr(Factory, ite(Expr, ctx.bv_val(1, 1), ctx.bv_val(0, 1)));
    }
}

SMTExpr SMTExpr::real2int() {
    auto* Factory = &getSMTFactory();
    z3::context & ctx = Expr.ctx();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_real());
        auto func = z3::expr(ctx, Z3_mk_real2int(ctx, ctx.real_val("0.0"))).decl();

        Z3_ast mapargs[1] = { Expr };
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 1, mapargs)));
    } else {
        assert(Expr.is_real());
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_real2int(ctx, Expr)));
    }
}

SMTExpr SMTExpr::int2real() {
    auto* Factory = &getSMTFactory();
    z3::context & ctx = Expr.ctx();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_int());

        auto func = to_real(Expr).decl();
        Z3_ast mapargs[1] = { Expr };
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 1, mapargs)));
    } else {
        assert(Expr.is_int());
        return SMTExpr(Factory, to_real(Expr));
    }
}

SMTExpr SMTExpr::int2bv(unsigned sz) {
    auto* Factory = &getSMTFactory();
    z3::context & ctx = Expr.ctx();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_int());
        auto func = z3::expr(ctx, Z3_mk_int2bv(ctx, sz, ctx.int_val("0"))).decl();

        Z3_ast mapargs[1] = { Expr };
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 1, mapargs)));
    } else {
        assert(Expr.is_int());
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_int2bv(ctx, sz, Expr)));
    }
}

SMTExpr SMTExpr::bv2int(bool isSigned) {
    auto* Factory = &getSMTFactory();
    z3::context & ctx = Expr.ctx();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_bv());
        unsigned bvSize = Expr.get_sort().array_range().bv_size();
        auto func = z3::expr(ctx, Z3_mk_bv2int(ctx, ctx.bv_val("0", bvSize), isSigned)).decl();

        Z3_ast mapargs[1] = { Expr };
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 1, mapargs)));
    } else {
        assert(Expr.is_bv());
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_bv2int(ctx, Expr, isSigned)));
    }
}

#define BINARY_OPERATION(X) \
SMTExpr SMTExpr::basic_##X(SMTExpr &b) { \
    z3::context& ctx = Expr.ctx(); \
    if (Expr.is_array() && b.Expr.is_array()) { \
        Z3_ast const mapargs[2] = { Expr, b.Expr };\
        z3::func_decl func(ctx);\
        if (Expr.get_sort().array_range().is_bv()) {\
            unsigned bvSize = Expr.get_sort().array_range().bv_size();\
            func = z3::expr(ctx, Z3_mk_bv##X(ctx, ctx.bv_val("0", bvSize), ctx.bv_val("0", bvSize))).decl();\
        } else {\
            assert(false && "not bit vectors"); \
        }\
        return SMTExpr(&getSMTFactory(), z3::expr(ctx, Z3_mk_map(ctx, func, 2, mapargs)));\
    } else {\
        assert(isBitVector() && b.isBitVector()); \
        return SMTExpr(&getSMTFactory(), z3::expr(ctx, Z3_mk_bv##X(ctx, Expr, b.Expr)));\
    }\
}\

BINARY_OPERATION(add)
BINARY_OPERATION(sub)
BINARY_OPERATION(mul)
BINARY_OPERATION(udiv)
BINARY_OPERATION(sdiv)
BINARY_OPERATION(urem)
BINARY_OPERATION(srem)
BINARY_OPERATION(shl)
BINARY_OPERATION(ashr)
BINARY_OPERATION(lshr)
BINARY_OPERATION(and)
BINARY_OPERATION(or)
BINARY_OPERATION(xor)

BINARY_OPERATION(ugt)
BINARY_OPERATION(uge)
BINARY_OPERATION(sgt)
BINARY_OPERATION(sge)
BINARY_OPERATION(ult)
BINARY_OPERATION(ule)
BINARY_OPERATION(sle)
BINARY_OPERATION(slt)

SMTExpr SMTExpr::basic_concat(SMTExpr &b) {
    auto* Factory = &getSMTFactory();
    z3::context& ctx = Expr.ctx();
    if (Expr.is_array() && b.Expr.is_array()) {
        Z3_ast const mapargs[2] = { Expr, b.Expr };
        z3::func_decl func(ctx);
        if (Expr.get_sort().array_range().is_bv()) {
            unsigned bvSize = Expr.get_sort().array_range().bv_size();
            func = z3::expr(ctx, Z3_mk_concat(ctx, ctx.bv_val("0", bvSize), ctx.bv_val("0", bvSize))).decl();
        } else {
            assert(false);
        }
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 2, mapargs)));
    } else {
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_concat(ctx, Expr, b.Expr)));
    }
}

SMTExpr SMTExpr::basic_eq(SMTExpr &b) {
    z3::context& ctx = Expr.ctx();
    if (Expr.is_array() && b.Expr.is_array()) {
        Z3_ast const mapargs[2] = { Expr, b.Expr };
        z3::func_decl func(ctx);
        if (Expr.get_sort().array_range().is_bv()) {
            unsigned bvSize = Expr.get_sort().array_range().bv_size();
            func = (ctx.bv_val("0", bvSize) == ctx.bv_val("0", bvSize)).decl();
        } else {
            func = (ctx.real_val("0.0") == ctx.real_val("0.0")).decl();
        }
        auto* Factory = &getSMTFactory();
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 2, mapargs)));
    } else {
        return *this == b;
    }
}

SMTExpr SMTExpr::basic_ne(SMTExpr &b) {
    z3::context& ctx = Expr.ctx();
    if (Expr.is_array() && b.Expr.is_array()) {
        Z3_ast const mapargs[2] = { Expr, b.Expr };
        z3::func_decl func(ctx);
        if (Expr.get_sort().array_range().is_bv()) {
            unsigned bvSize = Expr.get_sort().array_range().bv_size();
            func = (ctx.bv_val("0", bvSize) != ctx.bv_val("0", bvSize)).decl();
        } else {
            func = (ctx.real_val("0.0") != ctx.real_val("0.0")).decl();
        }
        auto* Factory = &getSMTFactory();
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 2, mapargs)));
    } else {
        return *this != b;
    }
}

SMTExpr SMTExpr::basic_extract(unsigned high, unsigned low) {
    auto* Factory = &getSMTFactory();
    z3::context& ctx = Expr.ctx();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_bv());
        unsigned bvSize = Expr.get_sort().array_range().bv_size();
        auto func = z3::expr(ctx, Z3_mk_extract(ctx, high, low, ctx.bv_val("10", bvSize))).decl();
        Z3_ast mapargs[1] = { Expr };
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 1, mapargs)));
    } else {
        assert(Expr.is_bv());
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_extract(ctx, high, low, Expr)));
    }
}

SMTExpr SMTExpr::basic_sext(unsigned sz) const {
    auto* Factory = &getSMTFactory();
    z3::context& ctx = Expr.ctx();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_bv());
        unsigned bvSize = Expr.get_sort().array_range().bv_size();
        auto func = z3::expr(ctx, Z3_mk_sign_ext(ctx, sz, ctx.bv_val("10", bvSize))).decl();
        Z3_ast mapargs[1] = { Expr };
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 1, mapargs)));
    } else {
        assert(Expr.is_bv());
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_sign_ext(ctx, sz, Expr)));
    }
}

SMTExpr SMTExpr::basic_zext(unsigned sz) const {
    auto* Factory = &getSMTFactory();
    z3::context& ctx = Expr.ctx();
    if (Expr.is_array()) {
        assert(Expr.get_sort().array_range().is_bv());
        unsigned bvSize = Expr.get_sort().array_range().bv_size();
        auto func = z3::expr(ctx, Z3_mk_zero_ext(ctx, sz, ctx.bv_val("10", bvSize))).decl();
        Z3_ast mapargs[1] = { Expr };
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 1, mapargs)));
    } else {
        assert(Expr.is_bv());
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_zero_ext(ctx, sz, Expr)));
    }
}

SMTExpr SMTExpr::basic_ite(SMTExpr& TBValue, SMTExpr& FBValue) {
    auto* Factory = &getSMTFactory();
    z3::context& ctx = Expr.ctx();
    z3::expr& condition = Expr;
    if (condition.is_array()) {
        assert(TBValue.Expr.is_array() && FBValue.Expr.is_array());

        Z3_ast mapargs[3] = { condition, TBValue.Expr, FBValue.Expr };
        z3::func_decl func(ctx);
        if (TBValue.Expr.get_sort().array_range().is_bv()) {
            assert(FBValue.Expr.get_sort().array_range().is_bv());
            unsigned bvSize = TBValue.Expr.get_sort().array_range().bv_size();
            func = ite(ctx.bool_val(true), ctx.bv_val(1, bvSize), ctx.bv_val(0, bvSize)).decl();
        } else {
            assert(TBValue.Expr.get_sort().array_range().is_real());
            assert(FBValue.Expr.get_sort().array_range().is_real());
            func = ite(ctx.bool_val(true), ctx.real_val("0.0"), ctx.real_val("0.0")).decl();
        }

        z3::expr mapped(ctx, Z3_mk_map(ctx, func, 3, mapargs));
        return SMTExpr(Factory, z3::expr(ctx, Z3_mk_map(ctx, func, 3, mapargs)));
    } else {
        return SMTExpr(Factory, ite(condition, TBValue.Expr, FBValue.Expr));
    }
}

SMTExpr SMTExpr::array_elmt(unsigned ElementNum, unsigned Index) {
    assert(ElementNum > Index);
    unsigned TotalSize = getBitVecSize();
    unsigned EachSize = TotalSize / ElementNum;
    unsigned High = TotalSize - Index * EachSize - 1;
    unsigned Low = TotalSize - (Index + 1) * EachSize;
    return basic_extract(High, Low);
}

#define ARRAY_CMP_OPERATION(X) \
SMTExpr SMTExpr::array_##X(SMTExpr &BvAsArray, unsigned ElmtNum) { \
    assert(ElmtNum > 0); \
    if (ElmtNum == 1) { \
        return basic_##X(BvAsArray).bool2bv1(); \
    } else { \
        SMTExpr Op2 = BvAsArray.array_elmt(ElmtNum, 0); \
        SMTExpr Ret = array_elmt(ElmtNum, 0).basic_##X(Op2).bool2bv1(); \
        for (unsigned I = 1; I < ElmtNum; I++) { \
            Op2 = BvAsArray.array_elmt(ElmtNum, I); \
            SMTExpr NextRet = array_elmt(ElmtNum, I).basic_##X(Op2).bool2bv1(); \
            Ret = Ret.basic_concat(NextRet); \
        } \
        return Ret; \
    } \
} \

ARRAY_CMP_OPERATION(sgt)
ARRAY_CMP_OPERATION(sge)
ARRAY_CMP_OPERATION(ugt)
ARRAY_CMP_OPERATION(uge)
ARRAY_CMP_OPERATION(slt)
ARRAY_CMP_OPERATION(sle)
ARRAY_CMP_OPERATION(ule)
ARRAY_CMP_OPERATION(ult)
ARRAY_CMP_OPERATION(eq)
ARRAY_CMP_OPERATION(ne)

#define ARRAY_BIN_OPERATION(X) \
SMTExpr SMTExpr::array_##X(SMTExpr &BvAsArray, unsigned ElmtNum) { \
    assert(ElmtNum > 0); \
    if (ElmtNum == 1) { \
        return basic_##X(BvAsArray); \
    } else { \
        SMTExpr Op2 = BvAsArray.array_elmt(ElmtNum, 0); \
        SMTExpr Ret = array_elmt(ElmtNum, 0).basic_##X(Op2); \
        for (unsigned I = 1; I < ElmtNum; I++) { \
            Op2 = BvAsArray.array_elmt(ElmtNum, I); \
            SMTExpr Next = array_elmt(ElmtNum, I).basic_##X(Op2); \
            Ret = Ret.basic_concat(Next); \
        } \
        return Ret; \
    } \
} \

ARRAY_BIN_OPERATION(add)
ARRAY_BIN_OPERATION(sub)
ARRAY_BIN_OPERATION(mul)
ARRAY_BIN_OPERATION(udiv)
ARRAY_BIN_OPERATION(sdiv)
ARRAY_BIN_OPERATION(urem)
ARRAY_BIN_OPERATION(srem)
ARRAY_BIN_OPERATION(shl)
ARRAY_BIN_OPERATION(ashr)
ARRAY_BIN_OPERATION(lshr)
ARRAY_BIN_OPERATION(and)
ARRAY_BIN_OPERATION(or)
ARRAY_BIN_OPERATION(xor)

SMTExpr SMTExpr::array_ite(SMTExpr& TBValue, SMTExpr& FBValue, unsigned ElmtNum) {
    assert(ElmtNum > 0);
    if (ElmtNum == 1) {
        return this->bv12bool().basic_ite(TBValue, FBValue);
    } else {
        SMTExpr T = TBValue.array_elmt(ElmtNum, 0);
        SMTExpr F = FBValue.array_elmt(ElmtNum, 0);
        SMTExpr Ret = array_elmt(ElmtNum, 0).bv12bool().basic_ite(T, F);

        for (unsigned Index = 1; Index < ElmtNum; Index++) {
            T = TBValue.array_elmt(ElmtNum, Index);
            F = FBValue.array_elmt(ElmtNum, Index);
            SMTExpr Next = array_elmt(ElmtNum, Index).bv12bool().basic_ite(T, F);
            Ret = Ret.basic_concat(Next);
        }
        return Ret;
    }
}

#define ARRAY_EXT_OPERATION(X) \
SMTExpr SMTExpr::array_##X(unsigned Sz, unsigned ElmtNum) { \
    assert(ElmtNum > 0); \
    if (ElmtNum == 1) { \
        return basic_##X(Sz); \
    } else { \
        unsigned ElmtExtSz = Sz / ElmtNum; \
        SMTExpr Ret = array_elmt(ElmtNum, 0).basic_##X(ElmtExtSz); \
        for (unsigned I = 1; I < ElmtNum; I++) { \
            SMTExpr Next = array_elmt(ElmtNum, I).basic_##X(ElmtExtSz); \
            Ret = Ret.basic_concat(Next); \
        } \
        return Ret; \
    } \
}

ARRAY_EXT_OPERATION(zext)
ARRAY_EXT_OPERATION(sext)

SMTExpr SMTExpr::array_trunc(unsigned Sz, unsigned ElementNum) {
    assert(ElementNum > 0);
    if (ElementNum == 1) {
        return basic_extract(getBitVecSize() - Sz - 1, 0);
    } else {
        unsigned ElmtSz = getBitVecSize() / ElementNum;
        unsigned ElmtTruncSz = Sz / ElementNum;

        assert(ElmtSz > ElmtTruncSz);

        unsigned High = ElmtSz - ElmtTruncSz - 1;
        SMTExpr Ret = array_elmt(ElementNum, 0).basic_extract(High, 0);
        for (unsigned I = 1; I < ElementNum; I++) {
            SMTExpr Next = array_elmt(ElementNum, I).basic_extract(High, 0);
            Ret = Ret.basic_concat(Next);
        }
        return Ret;
    }
}

SMTExpr operator!(SMTExpr const & E) {
    if (E.isLogicNot()) {
        assert(E.numArgs() == 1);
        return E.getArg(0);
    } else if (E.isTrue()) {
        return E.getSMTFactory().createBoolVal(false);
    } else if (E.isFalse()) {
        return E.getSMTFactory().createBoolVal(true);
    } else {
        return SMTExpr(&E.getSMTFactory(), !E.Expr);
    }
}

SMTExpr operator||(SMTExpr const & A, SMTExpr const & B) {
    if (A.isFalse() || B.isTrue()) {
        return B;
    } else if (A.isTrue() || B.isFalse()) {
        return A;
    }

    return SMTExpr(&A.getSMTFactory(), A.Expr || B.Expr);
}

SMTExpr operator||(SMTExpr const & A, bool B) {
    return A || A.getSMTFactory().createBoolVal(B);
}

SMTExpr operator||(bool A, SMTExpr const & B) {
    return B || A;
}

SMTExpr operator&&(SMTExpr const & A, SMTExpr const & B) {
    if (A.isTrue() || B.isFalse()) {
        return B;
    } else if (B.isTrue() || A.isFalse()) {
        return A;
    }

    return SMTExpr(&A.getSMTFactory(), A.Expr && B.Expr);
}

SMTExpr operator&&(SMTExpr const & A, bool B) {
    return A && A.getSMTFactory().createBoolVal(B);
}

SMTExpr operator&&(bool A, SMTExpr const & B) {
    return B && A;
}

#define UNARY_OPERATION_EXPR(X) \
SMTExpr operator X(SMTExpr const & A) { \
    return SMTExpr(&A.getSMTFactory(), X(A.Expr)); \
}

UNARY_OPERATION_EXPR(-)
UNARY_OPERATION_EXPR(~)

#define BINARY_OPERATION_EXPR_EXPR(X) \
SMTExpr operator X(SMTExpr const & A, SMTExpr const & B) { \
    if(A.isBitVector()) {\
        auto a_sz = A.getBitVecSize();\
        auto b_sz = B.getBitVecSize();\
        if(a_sz > b_sz) {\
            return A X B.basic_zext(a_sz-b_sz);\
        }\
        if(a_sz < b_sz) {\
            return A.basic_zext(b_sz-a_sz) X B;\
        }\
    }\
    assert(A.isSameSort(B)); \
    return SMTExpr(&A.getSMTFactory(), A.Expr X B.Expr); \
}

BINARY_OPERATION_EXPR_EXPR(|)
BINARY_OPERATION_EXPR_EXPR(^)
BINARY_OPERATION_EXPR_EXPR(&)
BINARY_OPERATION_EXPR_EXPR(>)
BINARY_OPERATION_EXPR_EXPR(<)
BINARY_OPERATION_EXPR_EXPR(>=)
BINARY_OPERATION_EXPR_EXPR(<=)
BINARY_OPERATION_EXPR_EXPR(!=)
BINARY_OPERATION_EXPR_EXPR(==)
BINARY_OPERATION_EXPR_EXPR(+)
BINARY_OPERATION_EXPR_EXPR(-)
BINARY_OPERATION_EXPR_EXPR(*)
BINARY_OPERATION_EXPR_EXPR(/)

#define BINARY_OPERATION_EXPR_INT(X) \
SMTExpr operator X(SMTExpr const & A, int B) { \
    return SMTExpr(&A.getSMTFactory(), A.Expr X B); \
}

BINARY_OPERATION_EXPR_INT(|)
BINARY_OPERATION_EXPR_INT(^)
BINARY_OPERATION_EXPR_INT(&)
BINARY_OPERATION_EXPR_INT(>)
BINARY_OPERATION_EXPR_INT(<)
BINARY_OPERATION_EXPR_INT(>=)
BINARY_OPERATION_EXPR_INT(<=)
BINARY_OPERATION_EXPR_INT(!=)
BINARY_OPERATION_EXPR_INT(==)
BINARY_OPERATION_EXPR_INT(+)
BINARY_OPERATION_EXPR_INT(-)
BINARY_OPERATION_EXPR_INT(*)
BINARY_OPERATION_EXPR_INT(/)

#define BINARY_OPERATION_INT_EXPR(X) \
SMTExpr operator X(int A, SMTExpr const & B) { \
    return SMTExpr(&B.getSMTFactory(), A X B.Expr); \
}

BINARY_OPERATION_INT_EXPR(|)
BINARY_OPERATION_INT_EXPR(^)
BINARY_OPERATION_INT_EXPR(&)
BINARY_OPERATION_INT_EXPR(>)
BINARY_OPERATION_INT_EXPR(<)
BINARY_OPERATION_INT_EXPR(>=)
BINARY_OPERATION_INT_EXPR(<=)
BINARY_OPERATION_INT_EXPR(!=)
BINARY_OPERATION_INT_EXPR(==)
BINARY_OPERATION_INT_EXPR(+)
BINARY_OPERATION_INT_EXPR(-)
BINARY_OPERATION_INT_EXPR(*)
BINARY_OPERATION_INT_EXPR(/)

llvm::raw_ostream& operator<<(llvm::raw_ostream& Out, SMTExpr E) {
    z3::expr& Z3Expr = E.Expr;
    Out << Z3_ast_to_string(Z3Expr.ctx(), Z3Expr);
    return Out;
}

std::ostream & operator<<(std::ostream& Out, SMTExpr const & N) {
    Out << N.Expr;
    return Out;
}

bool SMTExpr::SMTExprToStream(std::string& ExprStr) {
    try {
        z3::solver Sol(Expr.ctx());
        Sol.add(Expr);
        ExprStr = Sol.to_smt2();
        // The following function does not work. We cannot recover expr
        // from the generated string because it looses type info.
        //ExprStr = Z3_ast_to_string(Expr.ctx(), Expr);
        return true;
    } catch (z3::exception& Ex) {
        return false;
    }
}

bool SMTExpr::SMTExprToStream(std::ostream& Out) {
    try {
        z3::solver Sol(Expr.ctx());
        Sol.add(Expr);
        // TODO: in fact we don't need call to_smt2() here.
        Out << Sol.to_smt2();
        return true;
    } catch (z3::exception& Ex) {
        return false;
    }
}

bool SMTExpr::SMTExprToStream(llvm::raw_fd_ostream& Out) {
    try {
        z3::solver Sol(Expr.ctx());
        Sol.add(Expr);
        // TODO: in fact we don't need call to_smt2() here.
        Out << Sol.to_smt2();
        return true;
    } catch (z3::exception& Ex) {
        return false;
    }
}

bool SMTExpr::isLogicalEquivTo(SMTExpr const & E) {
    // TODO: set timeout for each query
    z3::context& Ctx = Expr.ctx();
    z3::solver S1(Ctx); S1.add(!z3::implies(Expr, E.Expr));
    z3::solver S2(Ctx); S2.add(!z3::implies(E.Expr, Expr));
    if (S1.check() == z3::unsat && S2.check() == z3::unsat) {
        return true;
    } else {
        return false;
    }
}

/*==-- SMTExprVec --==*/

SMTExprVec::SMTExprVec(SMTFactory* F, std::shared_ptr<z3::expr_vector> Vec) : SMTObject(F),
        ExprVec(Vec) {
}

SMTExprVec::SMTExprVec(const SMTExprVec& Vec) : SMTObject(Vec),
        ExprVec(Vec.ExprVec) {
}

SMTExprVec& SMTExprVec::operator=(const SMTExprVec& Vec) {
    SMTObject::operator =(Vec);
    if (this != &Vec) {
        this->ExprVec = Vec.ExprVec;
    }
    return *this;
}

unsigned SMTExprVec::size() const {
    if (ExprVec.get() == nullptr) {
        return 0;
    }
    return ExprVec->size();
}

void SMTExprVec::push_back(SMTExpr E, bool Enforce) {
    if (E.isTrue() && !Enforce) {
        return;
    }
    if (ExprVec.get() == nullptr) {
        ExprVec = std::make_shared<z3::expr_vector>(E.Expr.ctx());
    }
    ExprVec->push_back(E.Expr);
}

SMTExpr SMTExprVec::operator[](unsigned I) const {
    assert(I < size());
    return SMTExpr(&getSMTFactory(), (*ExprVec)[I]);
}

bool SMTExprVec::empty() const {
    return size() == 0;
}

SMTExprVec SMTExprVec::copy() {
    if (size() == 0) {
        // std::shared_ptr<z3::expr_vector> Ret(nullptr);
        assert(!ExprVec.get());
        return *this;
    }

    std::shared_ptr<z3::expr_vector> Ret = std::make_shared<z3::expr_vector>(ExprVec->ctx());
    Ret->resize(ExprVec->size());
    for (unsigned Idx = 0; Idx < ExprVec->size(); Idx++) {
        Z3_ast_vector_set(ExprVec->ctx(), *Ret, Idx, (*ExprVec)[Idx]);
    }
    auto* Factory = &getSMTFactory();
    return SMTExprVec(Factory, Ret);
}

/// *this = *this && v2
void SMTExprVec::mergeWithAnd(const SMTExprVec& Vec) {
    if (Vec.size() == 0)
        return;

    if (ExprVec.get() == nullptr)
        ExprVec = std::make_shared<z3::expr_vector>(Vec.ExprVec->ctx());

    for (size_t I = 0; I < Vec.size(); I++) {
        ExprVec->push_back((*Vec.ExprVec)[I]);
    }
}

SMTExprVec SMTExprVec::merge(SMTExprVec Vec1, SMTExprVec Vec2) {
    if (Vec1.size() < Vec2.size()) {
        Vec2.mergeWithAnd(Vec1);
        return Vec2;
    } else {
        Vec1.mergeWithAnd(Vec2);
        return Vec1;
    }
}

/// *this = *this || v2
void SMTExprVec::mergeWithOr(const SMTExprVec& Vec) {
    SMTExprVec Ret = getSMTFactory().createEmptySMTExprVec();
    if (Vec.empty() || empty()) {
        Ret.push_back(getSMTFactory().createBoolVal(true));
        *this = Ret;
        return;
    }

    SMTExpr E1 = toAndExpr();
    SMTExpr E2 = Vec.toAndExpr();
    Ret.push_back(E1 || E2);
    *this = Ret;
    return;
}

SMTExpr SMTExprVec::toAndExpr() const {
    if (empty()) {
        return getSMTFactory().createBoolVal(true);
    }

    z3::expr t = ExprVec->ctx().bool_val(true), f = ExprVec->ctx().bool_val(false);

    Z3_ast* Args = new Z3_ast[ExprVec->size()];
    unsigned ActualSize = 0, Index = 0;
    for (unsigned I = 0, E = ExprVec->size(); I < E; I++) {
        z3::expr e = (*ExprVec)[I];
        if (z3::eq(e, t)) {
            continue;
        } else if (z3::eq(e, f)) {
            delete[] Args;
            return getSMTFactory().createBoolVal(false);
        }
        Args[ActualSize++] = (*ExprVec)[I];
        Index = I;
    }

    if (ActualSize == 1) {
        delete[] Args;
        return SMTExpr(&getSMTFactory(), (*ExprVec)[Index]);
    }

    SMTExpr Ret(&getSMTFactory(), to_expr(ExprVec->ctx(), Z3_mk_and(ExprVec->ctx(), ActualSize, Args)));
    delete[] Args;

    return Ret;
}

SMTExpr SMTExprVec::toOrExpr() const {
    if (empty()) {
        return getSMTFactory().createBoolVal(true);
    }

    z3::expr t = ExprVec->ctx().bool_val(true), f = ExprVec->ctx().bool_val(false);

    Z3_ast* Args = new Z3_ast[ExprVec->size()];
    unsigned ActualSize = 0, Index = 0;
    for (unsigned I = 0, E = ExprVec->size(); I < E; I++) {
        z3::expr e = (*ExprVec)[I];
        if (z3::eq(e, f)) {
            continue;
        } else if (z3::eq(e, t)) {
            delete[] Args;
            return SMTExpr(&getSMTFactory(), t);
        }
        Args[ActualSize++] = (*ExprVec)[I];
        Index = I;
    }

    if (ActualSize == 1) {
        delete[] Args;
        return SMTExpr(&getSMTFactory(), (*ExprVec)[Index]);
    }

    SMTExpr Ret(&getSMTFactory(), to_expr(ExprVec->ctx(), Z3_mk_or(ExprVec->ctx(), ActualSize, Args)));
    delete[] Args;
    return Ret;
}

unsigned SMTExprVec::constraintSize() const {
    unsigned Ret = 0;
    std::map<SMTExpr, unsigned, SMTExprComparator> Cache;
    for (unsigned I = 0; I < this->size(); I++) {
        Ret += (*this)[I].size(Cache);
    }
    return Ret;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& Out, SMTExprVec Vec) {
    if (Vec.ExprVec.get() == nullptr) {
        Out << "(empty vector)";
        return Out;
    }
    Out << Z3_ast_vector_to_string(Vec.ExprVec->ctx(), *Vec.ExprVec);
    return Out;
}

std::ostream & operator<<(std::ostream& Out, SMTExprVec Vec) {
    if (Vec.ExprVec.get() == nullptr) {
        Out << "(empty vector)";
        return Out;
    }
    Out << *Vec.ExprVec;
    return Out;
}

bool SMTExprVec::SMTExprVecToStream(std::string& ExprStr) {
    try {
        SMTExpr SingleExpr = this->toAndExpr();
        z3::solver Sol(SingleExpr.Expr.ctx());
        Sol.add(SingleExpr.Expr);
        ExprStr = Sol.to_smt2();
        return true;
    } catch (z3::exception& Ex) {
        return false;
    }
}

bool SMTExprVec::SMTExprVecToStream(std::ostream& Out) {
    try {
        SMTExpr SingleExpr = this->toAndExpr();
        z3::solver Sol(SingleExpr.Expr.ctx());
        Sol.add(SingleExpr.Expr);
        // TODO: in fact we don't need call to_smt2() here.
        Out << Sol.to_smt2();
        return true;
    } catch (z3::exception& Ex) {
        return false;
    }
}

bool SMTExprVec::SMTExprVecToStream(llvm::raw_ostream& Out) {
    try {
        SMTExpr SingleExpr = this->toAndExpr();
        z3::solver Sol(SingleExpr.Expr.ctx());
        Sol.add(SingleExpr.Expr);
        // TODO: in fact we don't need call to_smt2() here.
        Out << Sol.to_smt2();
        return true;
    } catch (z3::exception& Ex) {
        return false;
    }
}

SMTExprVec SMTExprVec::diff(const SMTExprVec& Vars) {
    SMTExprVec Ret = Vars.getSMTFactory().createEmptySMTExprVec();
    try {
        for (unsigned I = 0; I < (*this).size(); I++) {
            bool isDiff = true;
            Z3_symbol SymI = (*this)[I].Expr.decl().name();
            for (unsigned J = 0; J < Vars.size(); J++) {
                if (SymI == Vars[J].Expr.decl().name()) {
                    isDiff = false;
                }
            }
            if (isDiff) {
                Ret.push_back((*this)[I]);
            }
        }
    } catch (z3::exception & Ex) {
        std::cout << Ex.msg() << std::endl;
        return Ret;
    }

    return Ret;
}
