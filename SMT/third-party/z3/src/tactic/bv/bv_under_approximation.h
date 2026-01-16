/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: bv_under_approximation
 * Creation Date:  2017.
 * Modification History:
*/

#ifndef BV_UNDER_APPROXIMATION_H_
#define BV_UNDER_APPROXIMATION_H_

#include"tactical.h"
#include"params.h"
#include"ast.h"
class ast_manager;
class tactic;

tactic * mk_bv_under_approximation_tactic(ast_manager & m, params_ref const & p = params_ref());

/*
    ADD_TACTIC("bv-under-approximation", "Under-approximation QF_BV constraints.", "mk_bv_under_approximation_tactic(m, p)")
*/


#endif /* BV_UNDER_APPROXIMATION_H_ */
