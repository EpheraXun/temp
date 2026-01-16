/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Lai Wei
 * File Description: A testing client of smtd
 * Creation Date: 04/2017
 * Modification History:
 */

#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

#include "SMT/SMTFactory.h"
#include "SMT/SMTExpr.h"
#include "SMT/SMTSolver.h"

using namespace llvm;

#define DEBUG_TYPE "client"

static cl::opt<int> NumRound("test-round", cl::init(30), cl::desc("number of tests for each thread"));
static cl::opt<int> NumThread("test-thread", cl::init(10), cl::desc("number of threads"));
static cl::opt<std::string> FileDir("test-smt-dir", cl::ValueRequired, cl::desc("the directory containing smt2 files"));

//the list of file names in test directory
std::vector<std::string> FileNames;

void* thread(void*) {
    int Rounds = NumRound.getValue();
    Rounds += rand() % (Rounds / 3);

    for (int I = 0; I < Rounds; I++) {
        SMTFactory Factory;
        SMTSolver Solver = Factory.createSMTSolver();

        std::string FilePath;
        for (int J = 0; J < 20; J++) {
            Solver.reset();

            FilePath = FileNames[rand() % (FileNames.size())];
            SMTExpr Expr = Factory.parseSMTLib2File(FilePath);
            Solver.add(Expr);
            SMTSolver::SMTResultType Result = Solver.check();
            std::cout << "The TestFilePath : " << FilePath << " : The result ";
            std::cout <<  (Result == 1 ? "Pass" : "Fail") << "\n";
            sleep(rand() % 2);
        }
    }
    return nullptr;
}

// given dir,create a list of path for every file in it
// return the number of files
void initializeSMTFiles(void) {
    if (DIR* Dp = opendir(FileDir.getValue().c_str())) {
        while (struct dirent * Ep = readdir(Dp)) {
            if (Ep->d_type == DT_REG) {
                FileNames.push_back(FileDir.getValue() + "/" + Ep->d_name);
            }
        }
        closedir(Dp);
    }

    if (FileNames.empty()) {
        errs() << "Error: no testing smt files\n";
        errs() << "Please use -test-smt-dir to specify correct dir containing smt2 files";
        exit(1);
    }
}

int main(int argc,char **argv) {
    cl::ParseCommandLineOptions(argc,argv);
    srand(time(nullptr));
    initializeSMTFiles();
    std::vector<pthread_t> ThreadPointers(NumThread.getValue());
    for (int I = 0; I < NumThread.getValue(); I++) {
        pthread_create(&ThreadPointers[I], nullptr, thread, nullptr);
    }
    for (int I = 0; I < NumThread.getValue(); I++) {
        pthread_join(ThreadPointers[I], nullptr);
    }
    return 0 ;
}
