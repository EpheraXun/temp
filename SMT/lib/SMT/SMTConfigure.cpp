/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: global smt configurations
 * Creation Date:
 * Modification History:
 */

#include <llvm/Support/CommandLine.h>
#include "z3++.h"
#include "SMT/SMTConfigure.h"

int SMTConfigure::Timeout;
static llvm::cl::opt<int, true> SolverTimeOut(
        "solver-timeout",
        llvm::cl::desc("Set the timeout (ms) of the smt solver. The default value is 10000ms (i.e. 10s)."),
        llvm::cl::location(SMTConfigure::Timeout), llvm::cl::init(10000)
);

std::string SMTConfigure::Tactic;
static llvm::cl::opt<std::string, true> IncTactic("set-inc-tactic", llvm::cl::location(SMTConfigure::Tactic),llvm::cl::init("pp_qfbv_light_tactic"),
       llvm::cl::desc("Set the tactic for creating the incremental solver. "
               "Candidates are smt_tactic, qfbv_tactic, pp_qfbv_tactic, pp_inc_bv_solver and pp_qfbv_light_tactic. Default: pp_qfbv_tactic")
);


void SMTConfigure::init(int T) {
    if (T != -1 && SolverTimeOut.getNumOccurrences() == 0) {
        Timeout = T;
    }

    std::string& Tactic = IncTactic.getValue();
    if (Tactic == "pp_qfbv_light_tactic") z3::set_param("inc_qfbv", 4);
    else if (Tactic == "pp_qfbv_tactic") z3::set_param("inc_qfbv", 2);
    else if (Tactic == "smt_tactic") z3::set_param("inc_qfbv", 0);
    else if (Tactic == "pp_inc_bv_solver") z3::set_param("inc_qfbv", 3);
    else if (Tactic == "qfbv_tactic") z3::set_param("inc_qfbv", 1);
    else z3::set_param("inc_qfbv", 4); // Default changes to pp_qfbv_light_tactic
}
