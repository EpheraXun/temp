/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi and Andy Zhou
 * File Description: SMT model
 * Creation Date:
 * Modification History:
 */

#ifndef SMT_SMTMODEL_H
#define SMT_SMTMODEL_H

#include <string>
#include <llvm/Support/Debug.h>

#include "z3++.h"
#include "z3.h"
#include "SMTObject.h"
#include "SMTExpr.h"

class SMTFactory;

class SMTModel : public SMTObject {
private:
    z3::model Z3Model;

    SMTModel(SMTFactory* F, z3::model Z3Model);

public:

    SMTModel(SMTModel const & M);

    virtual ~SMTModel();

    SMTModel& operator=(const SMTModel& M);

    unsigned size();

    std::pair<std::string, std::string> getModelDbgInfo(int Index);

    friend class SMTSolver;

    SMTExpr Eval(const SMTExpr & E, bool ModelCompletion = false) const;

    std::pair<int, bool> getIntValue(const SMTExpr & E, bool ModelCompletion = false) const;

    std::pair<bool, bool> getBoolValue(const SMTExpr & E, bool ModelCompletion = false) const;

    friend std::ostream & operator<<(std::ostream & Out, SMTModel& M) {
        Out << Z3_model_to_string(M.Z3Model.ctx(), M.Z3Model) << "\n";
        return Out;
    }

    friend llvm::raw_ostream & operator<<(llvm::raw_ostream & Out, SMTModel& M) {
        Out << Z3_model_to_string(M.Z3Model.ctx(), M.Z3Model) << "\n";
        return Out;
    }

};

#endif
