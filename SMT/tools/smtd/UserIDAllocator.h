/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: To allocate and reuse user ids for a client.
 * Creation Date: 12/2016
 * Modification History:
 */

#ifndef TOOLS_SMTD_USERIDALLOCATOR_H
#define TOOLS_SMTD_USERIDALLOCATOR_H

#include <llvm/Support/ManagedStatic.h>

#include <set>
#include <vector>

/// Allocate user ids in smtd
/// This class is not thread-safe.
class UserIDAllocator {
private:
    std::set<long> ExistingUsers;
    std::vector<long> FreeUserIDs;

    /// Make sure the class has only one instance using \c llvm::ManagedStatic.
    /// @{
    UserIDAllocator() {
    }

    friend class llvm::object_creator<UserIDAllocator>;
    /// @}

public:
    ~UserIDAllocator() {}

    long allocate();

    void recycle(long ID);

public:
    static UserIDAllocator* getUserAllocator();
};

#endif /* TOOLS_SMTD_USERIDALLOCATOR_H */
