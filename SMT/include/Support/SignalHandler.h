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

#ifndef SUPPORT_SIGNALHANDLER_H
#define SUPPORT_SIGNALHANDLER_H

#include <functional>
#include <llvm/Support/raw_ostream.h>

/// Register signal handlers
void RegisterSignalHandler();

/// Add signal handler for interrupt signals
/// Handlers will be executed in order until
/// the last one or one of them exits
void AddInterruptSigHandler(std::function<void()>& SigHandler);

/// Add signal handler for error signals
/// Handlers will be executed in order until
/// the last one or one of them exits
void AddErrorSigHandler(std::function<void()>& SigHandler);

#endif /* SUPPORT_SIGNALHANDLER_H */
