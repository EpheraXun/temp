/**
 * Authors: Qingkai & Andy
 */

#include "SMT/SMTModel.h"
#include "SMT/SMTFactory.h"

SMTModel::SMTModel(SMTFactory* Factory, z3::model Z3Model) : SMTObject(Factory),
        Z3Model(Z3Model) {
}

SMTModel::SMTModel(SMTModel const & Model) : SMTObject(Model),
        Z3Model(Model.Z3Model) {
}

SMTModel::~SMTModel() {
}

SMTModel& SMTModel::operator=(const SMTModel& Model) {
    SMTObject::operator =(Model);
    if (this != &Model) {
        this->Z3Model = Model.Z3Model;
    }
    return *this;
}

unsigned SMTModel::size() {
    return Z3Model.size();
}

std::pair<std::string, std::string> SMTModel::getModelDbgInfo(int Index) {
    auto Element = Z3Model[Index];

    std::pair<std::string, std::string> Ret = {"", ""};

    if (Element.name().kind() == Z3_STRING_SYMBOL) {
        z3::expr E = Z3Model.get_const_interp(Element);
        std::ostringstream ExprOS;
        ExprOS << E;

        std::ostringstream TypeOS;
        TypeOS << E.get_sort();

        Ret.first  = ExprOS.str();
        Ret.second = TypeOS.str();
    }
    return Ret;
}


SMTExpr SMTModel::Eval(SMTExpr const & E, bool ModelCompletion) const {
    return SMTExpr(&E.getSMTFactory(), Z3Model.eval(E.Expr, ModelCompletion));
}

std::pair<int, bool> SMTModel::getIntValue(const SMTExpr & E, bool ModelCompletion) const {
    std::pair<int, bool> Ret(0, false);
    try {
        Ret.first = Z3Model.eval(E.Expr, ModelCompletion).get_numeral_int();
    } catch (z3::exception &Ex) {
        return Ret;
    }
    Ret.second = true;
    return Ret;
}

std::pair<bool, bool> SMTModel::getBoolValue(const SMTExpr & E, bool ModelCompletion) const {
    // E can be a variable of bool type or a predicate.
    std::pair<bool, bool> Ret(false, false);
    if (!E.Expr.is_bool()) return Ret;

    Z3_lbool Tmp = Z3_L_UNDEF;
    try {
        Tmp = Z3Model.eval(E.Expr, ModelCompletion).bool_value();
    } catch (z3::exception &Ex) {
    }

    if (Tmp == Z3_L_FALSE) {
        Ret.second = true;
    } else if (Tmp == Z3_L_TRUE) {
        Ret.first = true;
        Ret.second = true;
    }
    return Ret;
}



