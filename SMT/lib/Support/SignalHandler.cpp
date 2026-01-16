/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: To handle system signals
 * Creation Date: 19/12/2016
 * Modification History:
 */

#include <llvm/Support/Signals.h>
#include <vector>

#include "Support/SignalHandler.h"

static std::vector<std::function<void()>> InterruptSigHandlers;
static std::vector<std::function<void()>> ErrorSigHandlers;

static void OnInterruptSignal() {
	for(auto Handler : InterruptSigHandlers) {
		Handler();
	}
	exit(1);
}

static void OnErrorSignal(void*) {
	for(auto Handler : ErrorSigHandlers) {
		Handler();
	}
	exit(1);
}

void RegisterSignalHandler() {
	// error signal handlers
	llvm::sys::PrintStackTraceOnErrorSignal("pp-smtd");
	llvm::sys::AddSignalHandler(OnErrorSignal, 0);

	// interrupt signal handlers
	llvm::sys::SetInterruptFunction(OnInterruptSignal);
}

void AddInterruptSigHandler(std::function<void()>& SigHandler) {
	InterruptSigHandlers.push_back(SigHandler);
}

void AddErrorSigHandler(std::function<void()>& SigHandler) {
	ErrorSigHandlers.push_back(SigHandler);
}

