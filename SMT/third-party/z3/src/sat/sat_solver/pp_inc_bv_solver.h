/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.

 * Author: rainoftime <yaopeisen@sbrella.com>
 * File Description: pp_inc_bv_solver
 * Creation Date:  2017.
 * Modification History:
*/

#ifndef PP_INC_BV_SOLVER_H
#define PP_INC_BV_SOLVER_H

solver* mk_pp_inc_bv_solver(ast_manager& m, params_ref const& p);

void  pp_inc_bv_display(std::ostream& out, solver& s, unsigned sz, expr*const* soft, rational const* _weights);

#endif // PP_INC_BV_SOLVER_H
