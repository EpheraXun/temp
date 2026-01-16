/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: The slave of smtd
 * Creation Date: 04/2017
 * Modification History:
 *  Lai Wei: make it as an independent class
 */

#include <llvm/Support/Debug.h>

#include "Support/MessageQueue.h"
#include "SMT/SMTFactory.h"
#include "Slave.h"

#define DEBUG_TYPE "smtd-slave"

using namespace llvm;

void Slave::listen() {
    SMTFactory Factory;
    SMTSolver Solver = Factory.createSMTSolver();
    std::string Message;
    while (true) {
        if (!Incremental) {
            if (-1 == SlaveMSQ->recvMessage(Message, 1)) {
                perror("Slave fails to recv: ");
                abort();
            }
            SMTExpr Expr = Factory.parseSMTLib2String(Message);
            Solver.add(Expr);
            auto Result = Solver.check();
            if (-1 == SlaveMSQ->sendMessage(std::to_string((int) Result), 2)) {
                perror("Slave fails to send: ");
                abort();
            }
            Solver.reset();
        } else {
            // TODO incremental methods should have more complex protocol
        }
    }
}

