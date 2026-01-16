/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: The slave of smtd
 * Creation Date: 12/2016
 * Modification History:
 */

#include <llvm/Support/Signals.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>

#include <sys/wait.h> // waitpid
#include <signal.h> // kill
#include <unistd.h> // fork
#include <errno.h>

#include <map>
#include <set>

#include "Support/SignalHandler.h"
#include "Support/MessageQueue.h"
#include "UserIDAllocator.h"
#include "SMT/SMTFactory.h"
#include "Master.h"
#include "Slave.h"

#define DEBUG_TYPE "smtd"

using namespace llvm;

static cl::opt<int> MSQKey("smtd-key",
        cl::desc("Indicate the key of the master's message queue."),
        cl::init(1234));

static cl::opt<bool> Incremental("smtd-incremental",
        cl::desc("Enable incremental SMT solving."), cl::init(false));

static cl::opt<bool> RunTestClient("smtd-test",
        cl::desc("Run a testing client."), cl::init(false), cl::ReallyHidden);

static MessageQueue* SlaveMSQ = nullptr;

static MessageQueue* CommandMSQ = nullptr;

static MessageQueue* CommunicateMSQ = nullptr;

static pid_t MainProcessID;

static void RegisterSigHandler() {
    // Signal handlers
    MainProcessID = getpid();
    RegisterSignalHandler();
    std::function < void() > ExitHandler = []() {
        // close all child processes
        if (MainProcessID == getpid()) {
            CommandMSQ->destroy();
            CommunicateMSQ->destroy();
            delete CommunicateMSQ;
            delete CommandMSQ;

            while (waitpid(-1, nullptr, 0)) {
                if (errno == ECHILD) {
                    /// until no children exist, avoid dead processes
                    break;
                }
            }

        } else {
            SlaveMSQ->destroy();
            delete SlaveMSQ;
            DEBUG_WITH_TYPE("smtd-master", errs() << "\nSignal handler deletes slave " << SlaveMSQ << "\n");
        }
        CommunicateMSQ = nullptr;
        CommandMSQ = nullptr;
        SlaveMSQ = nullptr;
    };
    AddInterruptSigHandler (ExitHandler);
    AddErrorSigHandler(ExitHandler);
}

int main(int argc, char **argv) {
    // Print stack trace when crash occurs
    llvm::PrettyStackTraceProgram X(argc, argv);

    // Enable debug stream buffering.
    llvm::EnableDebugBuffering = true;

    // Call llvm_shutdown() on exit by calling destructor.
    llvm::llvm_shutdown_obj Y;

    // Parse command line options
    llvm::cl::ParseCommandLineOptions(argc, argv, "SMT solving service.\n");

    // Register signal handlers
    RegisterSigHandler();

    if (Incremental.getValue()) {
        llvm_unreachable("Incremental mode has not been supported yet!");
        abort();
    }

    outs() << "*******************************\n"
            << "Please run your applications with -solver-enable-smtd="
            << MSQKey.getValue() << " -solver-enable-smtd-incremental="
            << (Incremental.getValue() ? "true" : "false") << "\n"
            << "*******************************\n";
    int Counter = 0;

    CommandMSQ = new MessageQueue(MSQKey.getValue(), true);
    CommunicateMSQ = new MessageQueue(MSQKey.getValue() + ++Counter, true);

    Master MasterInstance(CommandMSQ, CommunicateMSQ);

    while (true) {
        switch (MasterInstance.listen()) {
        case Master::SMTDCT_RequestID: {
            MasterInstance.requestID();
            continue;
        }
            break;
        case Master::SMTDCT_Open: {
            // If master fails to open a slave
            // we help it
            if (MasterInstance.open() == 0)
                continue;
        }
            break;
        case Master::SMTDCT_Close: {
            MasterInstance.close();
            continue;
        }
            break;
        case Master::SMTDCT_Reopen: {
            // If master fails to open a slave
            // we help it
            if (MasterInstance.reopen() == 0)
                continue;
        }
            break;
        default:
            llvm_unreachable("Unknown command!");
            break;
        }

        /*==----------Master fails to open a slave---------==*/
        /* Master only can open a slave from its slave pool. */
        /* If no idle slave exits, it will fail and we have  */
        /* to help him. Master will add the slave to its     */
        /* pool after we create it.                          */
        /*==-----------------------------------------------==*/

        int SlaveMSQKey = MSQKey + ++Counter;
        SlaveMSQ = new MessageQueue(SlaveMSQKey, true);

        pid_t SlavePid = fork();
        switch (SlavePid) {
        case -1: {
            kill(0, SIGINT); // sent to every process in the group
            llvm_unreachable("Fail to fork subprocess for smt solving!");
        }
            break;
        case 0: { // Slave process
            // The parent process' MSQ is not needed here.
            delete CommandMSQ;
            delete CommunicateMSQ;
            CommandMSQ = nullptr;
            CommunicateMSQ = nullptr;
            Slave SlaveInstance(SlavePid, SlaveMSQ, Incremental.getValue());
            SlaveInstance.listen();
        }
            break;
        default: { // Parent process
            // Master records which client is associated to this slave
            // This function will block the process until getting a
            // confirmation from the client.
            MasterInstance.bindClientSlave(SlavePid, SlaveMSQKey);

            // This has been copied to subprocess, and is not necessary here.
            delete SlaveMSQ;
        }
            break;
        }
    }

    llvm_unreachable("Currently, only can exit by interruption (Ctrl + C)!");
    return 1;
}
