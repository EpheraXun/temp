/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: SMT solver
 * Creation Date:
 * Modification History:
 * 1. add machine learning based prediction feature, client end. Yongchao WANG. 2018.5.29
 */

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>

#include "SMT/SMTSolver.h"
#include "SMT/SMTFactory.h"
#include "SMT/SMTExpr.h"
#include "SMT/SMTModel.h"
#include "SMT/SMTConfigure.h"
#include <exception>

#ifdef __linux__
#include "Support/MessageQueue.h"
#include "Support/TCPClient.h"
#endif // __linux__

#include <time.h>
#include <map>
#include <iostream>
#include <fstream>
#include <unistd.h> // fork()
#include <sys/wait.h> // wait()

#define DEBUG_TYPE "solver"
#define FEATURE_SIZE 32
#define LOCALHOST "127.0.0.1"

//following are define SMT operations used for features.
#define SMT_AND_OP 0
#define SMT_EQUAL_OP 1
#define SMT_TRUE_OP 2
#define SMT_KIND_OP 3

using namespace llvm;

static llvm::cl::opt<std::string> UsingSimplify("solver-simplify", llvm::cl::init(""),
        llvm::cl::desc("Using online simplification technique. Candidates are local and dillig."));

static llvm::cl::opt<std::string> DumpingConstraintsDst("dump-cnts-dst", llvm::cl::init(""),
        llvm::cl::desc("If solving time is larger than the time that -dump-cnts-timeout, the constraints will be output the destination."));

static llvm::cl::opt<int> DumpingConstraintsTimeout("dump-cnts-timeout",
        llvm::cl::desc("If solving time is too large (ms), the constraints will be output to the destination that -dump-cnts-dst set."));

static llvm::cl::opt<int> EnableSMTD("solver-enable-smtd", llvm::cl::init(0), llvm::cl::ValueRequired,
        llvm::cl::desc("Using smtd service"));

static llvm::cl::opt<bool> EnableSMTDIncremental("solver-enable-smtd-incremental", llvm::cl::init(false),
        llvm::cl::desc("Using incremental when smtd is enabled"));

static llvm::cl::opt<bool> EnableLocalSimplify("enable-local-simplify", llvm::cl::init(true),
        llvm::cl::desc("Enable local simplifications while adding a vector of constraints"));


static llvm::cl::opt<bool> EnableModelCache("enable-model-cache", llvm::cl::init(false),
        llvm::cl::desc("Enable satisfying model cache"));


static llvm::cl::opt<int> EnableMLPredicate("enable-ml-predicate", llvm::cl::init(0), llvm::cl::desc("Enable machine learning based timeout prediction, the value of this option is the port number provided by AIServer."));

static llvm::cl::opt<std::string> UsingMLModel("ml-model", llvm::cl::init("SVM"), llvm::cl::desc("Choose the machine learning models for prediction. Candidates are SVM and LR"));

// only for debugging (single-thread)
bool SMTSolvingTimeOut = false;

SMTSolver::SMTSolver(SMTFactory* F, z3::solver& Z3Solver, z3::model& Z3Model) : SMTObject(F),
        Solver(Z3Solver), ModelCache(Z3Model), checkCount(0) {
    if (SMTConfigure::Timeout > 0) {
        z3::params Z3Params(Z3Solver.ctx());
        Z3Params.set("timeout", (unsigned) SMTConfigure::Timeout);
        Z3Solver.set(Z3Params);
    }

#ifdef __linux__
    if (EnableSMTD.getNumOccurrences()) {
        Channels = std::make_shared<SMTDMessageQueues>();

        int MasterKey = EnableSMTD.getValue();
        Channels->CommandMSQ = std::make_shared<MessageQueue>(MasterKey);
        Channels->CommunicateMSQ = std::make_shared<MessageQueue>(++MasterKey);

        // Step 0: send id-request
        DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Try to connect to master.\n");
        if (-1 == Channels->CommandMSQ->sendMessage("requestid")) {
            llvm_unreachable("Fail to send open command!");
        }
        std::string UserIDStr;
        if (-1 == Channels->CommunicateMSQ->recvMessage(UserIDStr, 12)) {
            llvm_unreachable("Fail to recv user id!");
        }
        Channels->UserID = std::stol(UserIDStr);
        DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Connect to master and get user id: " << UserIDStr << ".\n");

        // Step 1: send open-request
        if (-1 == Channels->CommandMSQ->sendMessage(UserIDStr + ":open")) {
            llvm_unreachable("Fail to send open command!");
        }
        DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Request sended: " << UserIDStr << ":open\n");

        // Step 2: wait for worker info
        std::string SlaveIDStr;
        if (-1 == Channels->CommunicateMSQ->recvMessage(SlaveIDStr, Channels->UserID)) {
            llvm_unreachable("Fail to recv worker id!");
        }
        int SlaveID = std::stoi(SlaveIDStr);
        DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Receive worker Id: " << SlaveIDStr << "\n");

        // Step 3: confirmation
        if (-1 == Channels->CommunicateMSQ->sendMessage(std::to_string(Channels->UserID) + ":got", 11)) {
            llvm_unreachable("Fail to send got command!");
        }
        DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Confirmation sended\n");

        // Step 4: connect to server
        Channels->WorkerMSQ = std::make_shared<MessageQueue>(SlaveID);
        DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Connect to Slave\n");
    }

    // following is initilize the tcp client
    if(EnableMLPredicate.getNumOccurrences()){
        // get listening port
        int Port = EnableMLPredicate.getValue();
        assert(Port);
        // setup tcp config
        tcp.Setup(LOCALHOST, Port);
    }
#endif
}

#ifdef __linux__
SMTSolver::SMTSolver(const SMTSolver& Solver) : SMTObject(Solver), Solver(Solver.Solver),
        ModelCache(Solver.ModelCache), checkCount(Solver.checkCount), Channels(Solver.Channels) {
}

SMTSolver& SMTSolver::operator=(const SMTSolver& Solver) {
    SMTObject::operator =(Solver);
    if (this != &Solver) {
        this->Solver = Solver.Solver;
        this->ModelCache = Solver.ModelCache;
        this->checkCount = Solver.checkCount;
        this->Channels = Solver.Channels;
    }
    return *this;
}

SMTSolver::SMTDMessageQueues::~SMTDMessageQueues() {
    assert(CommandMSQ.get() && "CommandMSQ is not initialized in a channel!");
    CommandMSQ->sendMessage(std::to_string(UserID) + ":close");
}

#else
SMTSolver::SMTSolver(const SMTSolver& Solver) : SMTObject(Solver),
Solver(Solver.Solver), ModelCache(Solver.ModelCache) {
}

SMTSolver& SMTSolver::operator=(const SMTSolver& Solver) {
    SMTObject::operator =(Solver);
    if (this != &Solver) {
        this->Solver = Solver.Solver;
    }
    return *this;
}
#endif

SMTSolver::~SMTSolver() {
#ifdef __linux__
    // recycle
    tcp.Exit();
#endif
}

void SMTSolver::reconnect() {
#ifdef __linux__
    assert(EnableSMTD.getNumOccurrences() && "reconnect can be used only if --solver-enable-smtd is opened!");

    if (-1 == Channels->CommandMSQ->sendMessage(std::to_string(Channels->UserID) + ":reopen")) {
        llvm_unreachable("Fail to send open command!");
    }
    DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Request sended: " << Channels->UserID << ":open\n");
    std::string SlaveIdStr;
    if (-1 == Channels->CommunicateMSQ->recvMessage(SlaveIdStr, Channels->UserID)) {
        llvm_unreachable("Fail to recv worker id!");
    }
    DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Receive Slave Id: " << SlaveIdStr << "\n");
    if (-1 == Channels->CommunicateMSQ->sendMessage(std::to_string(Channels->UserID) + ":got", 11)) {
        llvm_unreachable("Fail to send got command!");
    }
    DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Confirmation sended\n");
    Channels->WorkerMSQ = std::make_shared<MessageQueue>(std::stoi(SlaveIdStr));
    DEBUG_WITH_TYPE("solver-smtd", errs() << "[Client] Connect to Slave\n");
#endif
}

SMTSolver::SMTResultType SMTSolver::check(unsigned Timeout) {    
#ifdef __linux__
    if(EnableMLPredicate.getNumOccurrences()){
        // check the result returned by ml
        switch(checkByMachineLearning()){
        case -1:
            // submit to solver
            break;
        case 1:
            // sat
            return SMTResultType::SMTRT_Sat;
            break;
        case 0:
            // unknown
            return SMTResultType::SMTRT_Unknown;
            break;
        }
    }
#endif

	if (Timeout > 0) {
        this->setTimeout(Timeout);
    }

#ifdef __linux__
    if (EnableSMTD.getNumOccurrences()) {
        std::string Contraints;
        llvm::raw_string_ostream StringStream(Contraints);
        StringStream << *((SMTSolver*)this);

        // fault tolerance
        while (-1 == Channels->WorkerMSQ->sendMessage(StringStream.str(), 1)) {
            reconnect();
        }
        std::string ResultString;
        while (-1 == Channels->WorkerMSQ->recvMessage(ResultString, 2)) {
            reconnect();
            while (-1 == Channels->WorkerMSQ->sendMessage(StringStream.str(), 1)) {
                reconnect();
            }
        }

        SMTResultType Result = (SMTResultType) std::stoi(ResultString);
        return Result;
    }
#endif

    z3::check_result Result;
    try {
        clock_t Start;
        if (DumpingConstraintsTimeout.getNumOccurrences()) {
            Start = clock();
        }

        if (UsingSimplify.getNumOccurrences()) {
            z3::solver Z3Solver4Sim(Solver.ctx());

            // 1. merge as one
            SMTExpr Whole = this->assertions().toAndExpr();

            // 2. simplify
            if (UsingSimplify.getValue() == "local") {
                Z3Solver4Sim.add(Whole.localSimplify().Expr);
            } else if (UsingSimplify.getValue() == "dillig") {
                SMTExpr SimplifiedForm = Whole.dilligSimplify();
                // if (SimplifiedForm.equals(z3_solver.ctx().bool_val(false))) {
                //        return SMTResult::UNSAT;
                // } else {
                //        if (debug_bug_report) {
                //            // check so that get_model is valid
                //            z3_solver.check();
                //        }
                //        return SMTResult::SAT;
                // }
                Z3Solver4Sim.add(SimplifiedForm.Expr);
            } else {
                Z3Solver4Sim.add(Whole.Expr);
            }

            Result = Z3Solver4Sim.check();
        } else {
            checkCount++;
            if (checkCount > 6 && EnableModelCache.getValue()) {
                if (ModelCache.eval(this->assertions().toAndExpr().Expr, true).decl().decl_kind() == Z3_OP_TRUE) {
                    //Result = z3::check_result::sat;
                    return SMTResultType::SMTRT_Sat;
                } else {
                    Result = Solver.check();
                }
            } else {
                Result = Solver.check();
            }
        }

        if (DumpingConstraintsTimeout.getNumOccurrences()) {
            double TimeCost = (double) (clock() - Start) * 1000 / CLOCKS_PER_SEC;
            if (TimeCost > DumpingConstraintsTimeout.getValue() && DumpingConstraintsDst.getNumOccurrences()) {
                // output the constraints to a temp file in the dst
                std::string DstFileName = DumpingConstraintsDst.getValue();
                DstFileName.append("/case");
                DstFileName.append(std::to_string(clock()));
                DstFileName.append(".smt2");

                std::ofstream DstFile;
                DstFile.open(DstFileName);

                if (DstFile.is_open()) {
                    DstFile << *this << "\n";
                    DstFile.close();
                } else {
                    std::cerr << "File cannot be opened: " << DstFileName << "\n";
                }

                SMTSolvingTimeOut = true;
            } else {
                SMTSolvingTimeOut = false;
            }
        }
    } catch (z3::exception &Ex) {
        std::cerr << __FILE__ << " : " << __LINE__ << " : " << Ex << "\n";
        return SMTResultType::SMTRT_Unknown;
    }

    // Use a return value to suppress gcc warning
    SMTResultType RetVal = SMTResultType::SMTRT_Unknown;

    switch (Result) {
    case z3::check_result::sat:
        RetVal = SMTResultType::SMTRT_Sat;
        if (EnableModelCache.getValue()) {
            ModelCache = Solver.get_model();
        }
        break;
    case z3::check_result::unsat:
        RetVal = SMTResultType::SMTRT_Unsat;
        break;
    case z3::check_result::unknown:
        RetVal = SMTResultType::SMTRT_Unknown;
        break;
    }

    // return to the default timeout setting
    if (Timeout > 0) {
        if (SMTConfigure::Timeout > 0) {
            this->setTimeout((unsigned) SMTConfigure::Timeout);
        }
    }
    return RetVal;
}


//This can filter very trivial cases, such as "x + y == 2"(sat), " x > 2 && x <=2"(unsat);
//TODO: currently we need to get assertions from solver, and then apply a tactic;
//better to provide a API for z3 solver.
SMTSolver::SMTResultType SMTSolver::fastCheck(unsigned Timeout) {
    z3::goal G(Solver.ctx());
    SMTExpr Whole = this->assertions().toAndExpr();
    G.add(Whole.Expr);
    z3::tactic fastCheckTactic = z3::try_for(z3::tactic(Solver.ctx(), "bv-fast-check"), Timeout);
    Z3_lbool Res = fastCheckTactic(G)[0].as_expr().bool_value();

    SMTResultType RetVal = SMTResultType::SMTRT_Unknown;

    switch (Res) {
    case Z3_L_TRUE:
        RetVal = SMTResultType::SMTRT_Sat;
        break;
    case Z3_L_FALSE:
        RetVal = SMTResultType::SMTRT_Unsat;
        break;
    default:
        RetVal = SMTResultType::SMTRT_Unknown;
        break;
    }
    return RetVal;
}



bool SMTSolver::checkN2NQueryWithUnderAppro(SMTExprVec& FVec, std::vector<SMTSolver::SMTResultType>& ResVec) {
	SMTExpr CommonVC = this->assertions().toAndExpr();
	SMTExpr UnderApproExpr = CommonVC && FVec.toAndExpr();
	SMTSolver TmpSol = CommonVC.Factory->createSMTSolver();
	TmpSol.add(UnderApproExpr);
	if (TmpSol.check() == SMTSolver::SMTRT_Sat) {
		for (unsigned I = 0; I < ResVec.size(); I++) {
			ResVec[I] = SMTSolver::SMTRT_Sat;
		}
		return true;
	} else {
		//std::cout << "Not All SAT\n";
		// TODO: should we return false, or solve the SMTExpr in FVec one-by-one, or apply "unsat-core based refinement?"
		return false;
	}
}

bool SMTSolver::overApproCheckMisc(SMTExpr& G, SMTExprVec& FVec, std::vector<SMTSolver::SMTResultType>& Labels) {
    unsigned K = FVec.size();
    SMTExpr Total = G.getSMTFactory().createBoolVal(false);
    for (unsigned I = 0; I < K; I++) {
        // Merge formulas that are not determined yet
        if (Labels[I] == SMTSolver::SMTRT_Unknown) { Total = Total || FVec[I]; }
    }
    if (Total.isFalse()) {
        // A trick; this indicates that no labels are Unknown
        return true;
    }
    Total = Total && G;

    SMTSolver Sol = G.getSMTFactory().createSMTSolverWithTactic("qfbv");// or pp_qfbv, pp_qfbv_light
    Sol.add(Total);

    auto Res = Sol.check();
    if (Res == SMTSolver::SMTRT_Unsat) {
    	// current over-appro successes!!
        for (unsigned I = 0; I < K; I++) {
            if (Labels[I] == SMTSolver::SMTRT_Unknown) {
                Labels[I] = SMTSolver::SMTRT_Unsat;
            }
        }
        return true;
    } else if (Res == SMTSolver::SMTRT_Sat) {
        // Use model-based filtering/refinement
        SMTModel M = Sol.getSMTModel();
        for (unsigned I = 0; I < K; I++) {
            if (Labels[I] == SMTSolver::SMTRT_Unknown) {
                auto TmpRes = M.getBoolValue(FVec[I], true);
                if (TmpRes.second) {
                    // true under this model
                    if (TmpRes.first) {
                        Labels[I] = SMTSolver::SMTRT_Sat;
                    } //else {
                    //    Labels[I] = LUndef;
                    //}
                } else {
                    // Exception.
                    return false;
                }
            }
        }
        overApproCheckMisc(G, FVec, Labels);

    } else {
        // If any call to solver times out, the remaining labels are Unknown
        // This is not a good idea..
        return false;
    }
    return true;
}


bool SMTSolver::checkN2NQueryWithOverAppro(SMTExprVec& FVec, std::vector<SMTSolver::SMTResultType>& ResVec) {
	//assert(FVec.size() == ResVec.size());
    for (unsigned I = 0; I < FVec.size(); I++) {
        ResVec.push_back(SMTSolver::SMTRT_Unknown);
    }
    SMTExpr CommonVC = this->assertions().toAndExpr();

    overApproCheckMisc(CommonVC, FVec, ResVec);

    return true;
}


// instead of using the global timeout option, setting timeout explicitly for a SMTSolver
void SMTSolver::setTimeout(unsigned Timeout) {
    if (Timeout > 0) {
        z3::params Z3Params(Solver.ctx());
        Z3Params.set("timeout", Timeout);
        Solver.set(Z3Params);
    }
}

void SMTSolver::push() {
    try {
        Solver.push();
    } catch (z3::exception &Ex) {
        std::cerr << __FILE__ << " : " << __LINE__ << " : " << Ex << "\n";
        exit(1);
    }
}

void SMTSolver::pop(unsigned N) {
    try {
        Solver.pop(N);
    } catch (z3::exception &Ex) {
        std::cerr << __FILE__ << " : " << __LINE__ << " : " << Ex << "\n";
        exit(1);
    }
}

unsigned SMTSolver::getNumScopes() {
    return Z3_solver_get_num_scopes(Solver.ctx(), Solver);
}

void SMTSolver::add(SMTExpr E) {
    if (E.isTrue()) {
        return;
    }

    try {
        // FIXME In some cases (ar._bfd_elf_parse_eh_frame.bc),
        // simplify() will seriously affect the performance.
        Solver.add(E.Expr/*.simplify()*/);
    } catch (z3::exception &Ex) {
        std::cerr << __FILE__ << " : " << __LINE__ << " : " << Ex << "\n";
        exit(1);
    }
}


void SMTSolver::addAll(SMTExprVec EVec) {
    // TODO: more tactics
    // 1. Add one by one(the default tactic)
    // 2. Call toAnd(EVec), and add the returned formula
    // 3. Call toAnd(EVec), add add a simplified version of the returned formula
    // 4. Take the size of EVec into considerations; choose a proper method for simplification
    if (EnableLocalSimplify.getValue()) {
        add(EVec.toAndExpr());
        //add(EVec.toAndExpr().localSimplify());
        // Turn EVec to a single Expr, and call simplify()
        //Solver.add(EVec.toAndExpr().Expr.simplify());
    } else {
        for (unsigned I = 0; I < EVec.size(); I++) {
            add(EVec[I]);
        }
    }
}

void SMTSolver::addAll(const std::vector<SMTExpr>& EVec) {
    // TODO: add EnableLocalSimplify option for this.
    for (unsigned I = 0; I < EVec.size(); I++) {
        add(EVec[I]);
    }
}

SMTExprVec SMTSolver::assertions() {
    std::shared_ptr<z3::expr_vector> Vec = std::make_shared<z3::expr_vector>(Solver.assertions());
    return SMTExprVec(&getSMTFactory(), Vec);
}

void SMTSolver::reset() {
    Solver.reset();
}

bool SMTSolver::operator<(const SMTSolver& Solver) const {
    return ((Z3_solver) this->Solver) < ((Z3_solver) Solver.Solver);
}

SMTModel SMTSolver::getSMTModel() {
    try {
        return SMTModel(&getSMTFactory(), Solver.get_model());
    } catch (z3::exception & e) {
        std::cerr << __FILE__ << " : " << __LINE__ << " : " << e << "\n";
        exit(1);
    }
}
#ifdef __linux__
void SMTSolver::getBvFeatures(std::vector<double>& Features){
    z3::solver Solver4Feat(Solver.ctx());
    Solver4Feat.add(mk_and(Solver.assertions()));
    Solver4Feat.get_bv_features(Features);

    if(Features[SMT_AND_OP]){
        Features[SMT_KIND_OP]++;
    }
    if(Features[SMT_EQUAL_OP]){
        Features[SMT_KIND_OP]++;
    }
    if(Features[SMT_TRUE_OP]){
        Features[SMT_KIND_OP]++;
    }

    for(unsigned i = SMT_KIND_OP + 1; i < FEATURE_SIZE; i++) {
        if(Features[i]){
            Features[SMT_KIND_OP]++;
        }
    }
}
int SMTSolver::checkByMachineLearning(){
    // assert number of the constraints greater than 10. otherwise
    if(Solver.assertions().size() > 10){
        //1. get the features
        std::vector<double> FeatureVec(FEATURE_SIZE, 0);
        this->getBvFeatures(FeatureVec);
        std::string MLModelType = UsingMLModel.getValue();

        //2. send the features
        std::string FeatureStr = MLModelType + ":";
        for(auto I : FeatureVec){
            FeatureStr += std::to_string(I) + " ";
        }
        //if failed, send again
        if(!tcp.Send(FeatureStr)){
            if(!tcp.Send(FeatureStr)){
                // fail twice, submit it solver
                return -1;
            }
        }
        //3. receive result
        std::string Receive = tcp.Receive();
        if(Receive.compare(nullptr) == 0){
            // failed, submit it to solver
            return -1;
        }
        //4. transform string to int
        if(!std::atoi(Receive.c_str())){
            // Receive = 0 means "will timeout", regard it as unknown
            return 0;
        }
        else {
            // Receive = 1 means "will not timeout", regard it as sat
            return 1;
        }
    }
    else{
        // submit it to solver
        return -1;
    }
}
#endif // __linux__