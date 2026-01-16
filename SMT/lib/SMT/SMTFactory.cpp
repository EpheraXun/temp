/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: The context of all smt utilities
 * Creation Date:
 * Modification History:
 */

#include <llvm/Support/Debug.h>
#include <llvm/Support/CommandLine.h>
#include "Platform/OS/PerformanceGuard.h"

#include "SMT/SMTFactory.h"
#include "SMT/SMTConfigure.h"

#define DEBUG_TYPE "smt-fctry"

static llvm::cl::opt<bool> EnableSatProbing("enable-sat-probing", llvm::cl::init(false),
       llvm::cl::desc("Enable sat probing during global simplification of sat, default false"));

static llvm::cl::opt<bool> EnableSatPreSimplify("enable-sat-pre-simplify", llvm::cl::init(false),
       llvm::cl::desc("Enable pre_simplification before sat burst search, default false"));

SMTFactory::SMTFactory() :
        TempSMTVaraibleIndex(0) {

}

SMTExprVec SMTFactory::translate(const SMTExprVec & Exprs) {
    if (Exprs.empty())
        return this->createEmptySMTExprVec();

    std::lock_guard<ThreadTimerLock> L(Exprs.getSMTFactory().getFactoryLock());

    std::shared_ptr<z3::expr_vector> Vec(new z3::expr_vector(z3::expr_vector(Z3Ctx, Z3_ast_vector_translate(Exprs.ExprVec->ctx(), *Exprs.ExprVec, Z3Ctx))));
    SMTExprVec Ret(this, Vec);
    return Ret;
}

SMTExpr SMTFactory::translate(const SMTExpr & Expr) {
    std::lock_guard<ThreadTimerLock> Lock(Expr.getSMTFactory().getFactoryLock());

    if (Expr.isTrue())
        return this->createBoolVal(true);
    else if (Expr.isFalse())
        return this->createBoolVal(false);

    z3::expr RetExpr = z3::expr(Z3Ctx, Z3_translate(Expr.Expr.ctx(), Expr.Expr, Z3Ctx));
    SMTExpr Ret(this, RetExpr);
    return Ret;
}

std::pair<SMTExprVec, bool> SMTFactory::rename(const SMTExprVec& Exprs, const std::string& RenamingSuffix,
        std::unordered_map<std::string, SMTExpr>& Mapping, SMTRenamingAdvisor* Advisor) {

    SMTExprVec ResultExprVec = this->createEmptySMTExprVec();
    bool ConstraintPruned = false; // the constraint is pruned?

    for (unsigned ExprIdx = 0; ExprIdx < Exprs.size(); ExprIdx++) {
        SMTExpr RetExpr = Exprs[ExprIdx];

        std::unordered_map<std::string, SMTExpr> LocalMapping;

        auto It = ExprRenamingCache.find(RetExpr);
        if (It != ExprRenamingCache.end()) {
            auto & Cache = ExprRenamingCache.at(RetExpr);
            if (Cache.WillBePruned) {
                ConstraintPruned = true;
                RetExpr = Cache.AfterBeingPruned;

                if (RetExpr.isTrue()) {
                    continue;
                } else {
                    LocalMapping = Cache.SymbolMapping;
                }
            } else {
                LocalMapping = Cache.SymbolMapping;
            }
        } else {
            SMTExprVec ToPrune = this->createEmptySMTExprVec();
            std::map<SMTExpr, bool, SMTExprComparator> Visited;

            bool AllPruned = visit(RetExpr, LocalMapping, ToPrune, Visited, Advisor);
            if (AllPruned) {
                RenamingUtility RU { true, createBoolVal(true), createEmptySMTExprVec(), LocalMapping };
                ExprRenamingCache.insert(std::make_pair(RetExpr, RU));

                ConstraintPruned = true;
                continue;
            } else if (ToPrune.size()) {
                SMTExprVec TrueVec = this->createBoolSMTExprVec(true, ToPrune.size());
                assert(ToPrune.size() == TrueVec.size());
                SMTExpr AfterSubstitution = RetExpr.substitute(ToPrune, TrueVec);
                RenamingUtility RU { true, AfterSubstitution, ToPrune, LocalMapping };
                ExprRenamingCache.insert(std::make_pair(RetExpr, RU));

                ConstraintPruned = true;
                RetExpr = AfterSubstitution;
            } else {
                RenamingUtility RU { false, createBoolVal(true), createEmptySMTExprVec(), LocalMapping };
                ExprRenamingCache.insert(std::make_pair(RetExpr, RU));
            }
        }

        // renaming
        assert(RenamingSuffix.find(' ') == std::string::npos);
        if (RenamingSuffix != "") {
            // Utility for replacement
            SMTExprVec From = createEmptySMTExprVec(), To = createEmptySMTExprVec();

            // Mapping.string + suffix --> new string + Mapping.expr.sort -> new expr
            for (auto& It : LocalMapping) {
                std::string OldSymbol = It.first;
                SMTExpr OldExpr = It.second;

                if (!Advisor || Advisor->rename(OldExpr)) {
                    std::string NewSymbol = OldSymbol + RenamingSuffix;
                    SMTExpr NewExpr(this, z3::expr(Z3Ctx, Z3_mk_const(Z3Ctx, Z3_mk_string_symbol(Z3Ctx, NewSymbol.c_str()), OldExpr.Expr.get_sort())));
                    Mapping.insert(std::pair<std::string, SMTExpr>(OldSymbol, NewExpr));

                    From.push_back(OldExpr);
                    To.push_back(NewExpr);
                } else {
                    Mapping.insert(std::pair<std::string, SMTExpr>(OldSymbol, OldExpr));
                }
            }

            if (From.size()) {
                assert(From.size() == To.size());
                RetExpr = RetExpr.substitute(From, To);
            }
        }

        ResultExprVec.push_back(RetExpr);
    }

    return std::make_pair(ResultExprVec, ConstraintPruned);
}

bool SMTFactory::visit(SMTExpr& Expr2Visit, std::unordered_map<std::string, SMTExpr>& Mapping, SMTExprVec& ToPruneVec,
        std::map<SMTExpr, bool, SMTExprComparator>& Visited, SMTRenamingAdvisor* Advisor) {
    assert(Expr2Visit.isApp() && "Must be an application-only constraints.");

    auto* timer =  ThreadTimerManage::get_thread_timer();
    if(timer && timer->isTimeOut())
        return true;

    if (Visited.count(Expr2Visit)) {
        return Visited[Expr2Visit];
    } else {
        auto It = ExprRenamingCache.find(Expr2Visit);
        if (It != ExprRenamingCache.end()) {
            auto & Cache = ExprRenamingCache.at(Expr2Visit);
            Mapping.insert(Cache.SymbolMapping.begin(), Cache.SymbolMapping.end());

            if (Cache.WillBePruned) {
                if (Cache.AfterBeingPruned.isTrue()) {
                    Visited[Expr2Visit] = true;
                    return true;
                } else {
                    ToPruneVec.mergeWithAnd(Cache.ToPrune);
                    Visited[Expr2Visit] = false;
                    return false;
                }
            } else {
                Visited[Expr2Visit] = false;
                return false;
            }
        }

        unsigned NumArgs = Expr2Visit.numArgs();
        std::vector<bool> Arg2Prune;
        bool All2Prune = true;
        bool One2Prune = false;
        for (unsigned Idx = 0; Idx < NumArgs; Idx++) {
            SMTExpr Arg = Expr2Visit.getArg(Idx);
            bool WillPrune = visit(Arg, Mapping, ToPruneVec, Visited, Advisor);
            Arg2Prune.push_back(WillPrune);
            if (!WillPrune && All2Prune) {
                All2Prune = false;
            } else if (WillPrune && !One2Prune) {
                One2Prune = true;
            }
        }

        if (Expr2Visit.isConst() && !Expr2Visit.isNumeral()) {
            if (Advisor && Advisor->prune(Expr2Visit)) {
                Visited[Expr2Visit] = true;
                return true;
            } else {
                // If the node do not need to prune, we record it
                if (!Expr2Visit.isTrue() && !Expr2Visit.isFalse()) {
                    std::string Symbol = Expr2Visit.getSymbol();
                    assert(Symbol != "true" && Symbol != "false");
                    Mapping.insert(std::pair<std::string, SMTExpr>(Symbol, Expr2Visit));
                }
            }
        } else if (Expr2Visit.isLogicAnd()) {
            if (All2Prune) {
                Visited[Expr2Visit] = true;
                return true;
            } else {
                // recording the expr to prune
                for (unsigned Idx = 0; Idx < NumArgs; Idx++) {
                    if (Arg2Prune[Idx]) {
                        ToPruneVec.push_back(Expr2Visit.getArg(Idx));
                    }
                }
            }
        } else {
            if (One2Prune) {
                Visited[Expr2Visit] = true;
                return true;
            }
        }

        Visited[Expr2Visit] = false;
        return false;
    }
}

SMTSolver SMTFactory::createSMTSolver() {
    std::string& Tactic = SMTConfigure::Tactic;
    z3::solver Ret(Z3Ctx);
    // If Tactic == qfbv_tactic or pp_qfbv_tactic,
    // only use the result of the incremental solver.
    // That is, when the incremental solver returns unknown,
    // just return unknown.
    //std::string& Tactic = IncTactic.getValue();
    if (Tactic == "pp_qfbv_tactic" || Tactic == "pp_qfbv_light_tactic" || Tactic == "pp_inc_bv_solver" || Tactic == "qfbv_tactic") {
        z3::params Z3Params(Z3Ctx);
        Z3Params.set("solver2-unknown", 0u);
        if (Tactic == "pp_qfbv_light_tactic") {
            if (!EnableSatProbing.getValue()) {
                // do not apply failed literal probing during sat solving.
                Z3Params.set("enable_probing", false);
            }
            if (!EnableSatPreSimplify.getValue()) {
                // do not apply pre_simplify during sat burst search.
                Z3Params.set("enable_pre_simplify", false);
            }
            Z3Params.set("burst_search", (unsigned)150);    // num. of search before first global simplifications, z3 default 100
            Z3Params.set("restart.initial", (unsigned)150); // num. of conflicts before first restart, z3 default 100
        }
        Ret.set(Z3Params);
    }
    // a trick for initializing the ModelCache(In version 4.6 we can construct a model without the solver)
    Ret.check();
    z3::model Z3Model = Ret.get_model();
    return SMTSolver(this, Ret, Z3Model);
}

SMTSolver SMTFactory::createSMTSolverWithTactic(const std::string& Tactic) {
    if (Tactic.empty()) {
        z3::solver Ret(Z3Ctx);
        Ret.check();
        z3::model Z3Model = Ret.get_model();
        return SMTSolver(this, Ret, Z3Model);
    } else {
        z3::solver Ret = z3::tactic(Z3Ctx, Tactic.c_str()).mk_solver();
        Ret.check();
        z3::model Z3Model = Ret.get_model();
        return SMTSolver(this, Ret, Z3Model);
    }
}

SMTExprVec SMTFactory::createBoolSMTExprVec(bool Content, size_t Size) {
    SMTExprVec Ret = createEmptySMTExprVec();
    for (unsigned Index = 0; Index < Size; Index++) {
        Ret.push_back(createBoolVal(Content), true);
    }
    return Ret;
}

SMTExprVec SMTFactory::createSMTExprVec(const std::vector<SMTExpr>& ExprVec) {
    SMTExprVec Ret = createEmptySMTExprVec();
    for (size_t Index = 0; Index < ExprVec.size(); Index++) {
        const SMTExpr& Ex = ExprVec[Index];
        assert(&Ret.getSMTFactory() == &Ex.getSMTFactory() && "Contexts are not compatible!");
        Ret.push_back(Ex);
    }
    return Ret;
}

SMTExpr SMTFactory::createSMTExprFromStream(std::string& ExprStr) {
    // TODO: the created SMTExpr looses type info of variables.
    //
    return SMTExpr(this, Z3Ctx.parse_string(ExprStr.c_str()));
}

SMTExpr SMTFactory::parseSMTLib2File(const std::string& FileName) {
    Z3_ast Ast = Z3_parse_smtlib2_file(Z3Ctx, FileName.c_str(), 0, 0, 0, 0, 0, 0);
    z3::expr Whole(Z3Ctx, Ast);
    return SMTExpr(this, Whole);
}

SMTExpr SMTFactory::parseSMTLib2String(const std::string& Raw) {
    Z3_ast Ast = Z3_parse_smtlib2_string(Z3Ctx, Raw.c_str(), 0, 0, 0, 0, 0, 0);
    z3::expr Whole(Z3Ctx, Ast);
    return SMTExpr(this, Whole);
}

SMTExpr SMTFactory::createEmptySMTExpr() {
    return SMTExpr(this, z3::expr(Z3Ctx));
}

SMTExprVec SMTFactory::createEmptySMTExprVec() {
    std::shared_ptr<z3::expr_vector> Vec(nullptr);
    return SMTExprVec(this, Vec);
}

SMTExpr SMTFactory::createRealConst(const std::string& Name) {
    return SMTExpr(this, Z3Ctx.real_const(Name.c_str()));
}

SMTExpr SMTFactory::createRealVal(const std::string& ValStr) {
    return SMTExpr(this, Z3Ctx.real_val(ValStr.c_str()));
}

SMTExpr SMTFactory::createBitVecConst(const std::string& Name, uint64_t Size) {
    return SMTExpr(this, Z3Ctx.bv_const(Name.c_str(), Size));
}

SMTExpr SMTFactory::createTemporaryBitVecConst(uint64_t Size) {
    std::string Symbol("temp_");
    Symbol.append(std::to_string(TempSMTVaraibleIndex++));
    return SMTExpr(this, Z3Ctx.bv_const(Symbol.c_str(), Size));
}

SMTExpr SMTFactory::createBitVecVal(const std::string& ValStr, uint64_t Size) {
    return SMTExpr(this, Z3Ctx.bv_val(ValStr.c_str(), Size));
}

SMTExpr SMTFactory::createBoolConst(const std::string& Name) {
    return SMTExpr(this, Z3Ctx.bool_const(Name.c_str()));
}

SMTExpr SMTFactory::createBitVecVal(uint64_t Val, uint64_t Size) {
    return SMTExpr(this, Z3Ctx.bv_val((__uint64 ) Val, Size));
}

SMTExpr SMTFactory::createIntVal(int Val) {
    return SMTExpr(this, Z3Ctx.int_val(Val));
}

SMTExpr SMTFactory::createSelect(SMTExpr& Vec, SMTExpr Index) {
    return SMTExpr(this, z3::expr(Z3Ctx, Z3_mk_select(Z3Ctx, Vec.Expr, Index.Expr)));
}

SMTExpr SMTFactory::createStore(SMTExpr& Vec, SMTExpr Index, SMTExpr& Val2Store) {
    return SMTExpr(this, z3::expr(Z3Ctx, Z3_mk_store(Z3Ctx, Vec.Expr, Index.Expr, Val2Store.Expr)));
}


SMTExpr SMTFactory::createIntRealArrayConstFromStringSymbol(const std::string& Name) {
    Z3_sort ArraySort = Z3Ctx.array_sort(Z3Ctx.int_sort(), Z3Ctx.real_sort());
    return SMTExpr(this, z3::expr(Z3Ctx, Z3_mk_const(Z3Ctx, Z3_mk_string_symbol(Z3Ctx, Name.c_str()), ArraySort)));
}

SMTExpr SMTFactory::createIntBvArrayConstFromStringSymbol(const std::string& Name, uint64_t Size) {
    Z3_sort ArraySort = Z3Ctx.array_sort(Z3Ctx.int_sort(), Z3Ctx.bv_sort(Size));
    return SMTExpr(this, z3::expr(Z3Ctx, Z3_mk_const(Z3Ctx, Z3_mk_string_symbol(Z3Ctx, Name.c_str()), ArraySort)));
}

SMTExpr SMTFactory::createIntDomainConstantArray(SMTExpr& ElmtExpr) {
    return SMTExpr(this, z3::expr(Z3Ctx, Z3_mk_const_array(Z3Ctx, Z3Ctx.int_sort(), ElmtExpr.Expr)));
}

SMTExpr SMTFactory::createBoolVal(bool B) {
    return SMTExpr(this, Z3Ctx.bool_val(B));
}
