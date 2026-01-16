/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: SMT solver
 * Creation Date:
 * Modification History:
 */

#ifndef SMT_SMTSOLVER_H
#define SMT_SMTSOLVER_H

#include <vector>
#include <llvm/Support/Debug.h>

#include "z3++.h"
#include "SMTObject.h"

#ifdef __linux__
#include "Support/TCPClient.h"
#endif //__linux__

class SMTFactory;
class SMTModel;
class SMTExpr;
class SMTExprVec;
class MessageQueue;

class SMTSolver : public SMTObject {
public:
    enum SMTResultType {
        SMTRT_Unsat,
        SMTRT_Sat,
        SMTRT_Unknown,
        SMTRT_Uncheck
    };

private:
    z3::solver Solver;

    z3::model ModelCache;

    unsigned checkCount;

    SMTSolver(SMTFactory* F, z3::solver& Z3Solver, z3::model& Z3Model);

public:
    virtual ~SMTSolver();

    SMTSolver(const SMTSolver& Solver);

    SMTSolver& operator=(const SMTSolver& Solver);

    void add(SMTExpr);

    void addAll(SMTExprVec);

    void addAll(const std::vector<SMTExpr>& EVec);

    // set a specific Timeout for current query.
    // in some cases, we may want to spent more/less time than the default timeout on a query
    virtual SMTResultType check(unsigned Timeout=0);

    // check sat/unsat with fast, incomplete theory-level decision procedures
    virtual SMTResultType fastCheck(unsigned Timeout=100);

    /*
     * Given a formula G which is the constraint in the solver,
     * and a set of predicates in FVec (F1, F2, ..., Fn)
     * Decide the satisfibilies of G \land F1, ..., G \land Fn, resp.
     */
    virtual bool checkN2NQueryWithUnderAppro(SMTExprVec& FVec, std::vector<SMTSolver::SMTResultType>& ResVec);

    virtual bool overApproCheckMisc(SMTExpr& G, SMTExprVec& FVec, std::vector<SMTSolver::SMTResultType>& Labels);

    virtual bool checkN2NQueryWithOverAppro(SMTExprVec& FVec, std::vector<SMTSolver::SMTResultType>& ResVec);

    // instead of using the global timeout option, setting timeout explicitly for a SMTSolver
    void setTimeout(unsigned Timeout);

    SMTModel getSMTModel();

    SMTExprVec assertions();

    virtual void reset();

    virtual void push();

    virtual void pop(unsigned N = 1);

    unsigned getNumScopes();

    bool operator<(const SMTSolver& Solver) const;

    friend std::ostream & operator<<(std::ostream & O, SMTSolver& Solver) {
        O << Solver.Solver.to_smt2() << "\n";
        return O;
    }

    friend llvm::raw_ostream & operator<<(llvm::raw_ostream & O, SMTSolver& Solver) {
        O << Solver.Solver.to_smt2() << "\n";
        return O;
    }

    friend class SMTModel;
    friend class SMTFactory;

private:
#ifdef __linux__
    /// The followings are used when smtd is enabled
    /// @{
    class SMTDMessageQueues {
    public:
        /// User ID for communication
        long UserID = 0;

        /// This field is to pass command to smtd's master
        std::shared_ptr<MessageQueue> CommandMSQ;
        /// This field is for other communication with smtd's master
        std::shared_ptr<MessageQueue> CommunicateMSQ;
        /// This field is for communication with one of the smtd's slaves
        std::shared_ptr<MessageQueue> WorkerMSQ;

        ~SMTDMessageQueues();
    };

    std::shared_ptr<SMTDMessageQueues> Channels;
#endif // __linux__
    /// reconnect to smtd
    void reconnect();
    /// @}


#ifdef __linux__
    /// The following are used when smtd is enable

    // tcp client used to communicate with AI server
    TCPClient tcp;
    // check smt constraints by ml
    int checkByMachineLearning();
    //get the features of assertions in the solver
    void getBvFeatures(std::vector<double>& Features);

#endif // __linux__
};

#endif // SMT_SMTSOLVER_H
