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

#include <llvm/Support/ErrorHandling.h>

#include "UserIDAllocator.h"
#include "assert.h"

static llvm::ManagedStatic<UserIDAllocator> Allocator;

UserIDAllocator* UserIDAllocator::getUserAllocator() {
    return &(*Allocator);
}

long UserIDAllocator::allocate() {
    static long ID = 100;

    if (FreeUserIDs.empty()) {
        bool Overflow = false;
        for (; ; ++ID) {
            if (ID > 0 && !ExistingUsers.count(ID)) {
                ExistingUsers.insert(ID);
                return ID;
            } else if (ID <= 0) { // overflow
                if (!Overflow) {
                    Overflow = true;
                    ID = 100;
                } else {
                    llvm_unreachable("Too many users and no available ID can be assigned.");
                }
            }
        }
        llvm_unreachable("Too many users and no available ID can be assigned.");
        return ID;
    } else {
        long Ret = FreeUserIDs.back();
        FreeUserIDs.pop_back();

        ExistingUsers.insert(Ret);
        return Ret;
    }
}

void UserIDAllocator::recycle(long ID) {
    auto It = ExistingUsers.find(ID);
    assert(It != ExistingUsers.end());

    FreeUserIDs.push_back(ID);
    ExistingUsers.erase(It);
}
