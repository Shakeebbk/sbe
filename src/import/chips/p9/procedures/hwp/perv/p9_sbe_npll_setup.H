/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/perv/p9_sbe_npll_setup.H $ */
/*                                                                        */
/* OpenPOWER sbe Project                                                  */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
//------------------------------------------------------------------------------
/// @file  p9_sbe_npll_setup.H
///
/// @brief scan initialize level 0 & 1 PLLs
//------------------------------------------------------------------------------
// *HWP HW Owner        : Abhishek Agarwal <abagarw8@in.ibm.com>
// *HWP HW Backup Owner : Srinivas V Naga <srinivan@in.ibm.com>
// *HWP FW Owner        : sunil kumar <skumar8j@in.ibm.com>
// *HWP Team            : Perv
// *HWP Level           : 2
// *HWP Consumed by     : SBE
//------------------------------------------------------------------------------


#ifndef _P9_SBE_NPLL_SETUP_H_
#define _P9_SBE_NPLL_SETUP_H_


#include <fapi2.H>


typedef fapi2::ReturnCode (*p9_sbe_npll_setup_FP_t)(const
        fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>&);

/// @brief --Release PLL test enable for SS,  Filt & NEST PLLs
/// --Release SS PLL reset0
/// --check SS PLL lock
/// --Release SS PLL bypass0
/// --Release Filter PLL reset1
/// --check PLL lock for Filter PLLs
/// --Release Filter PLL bypass signals
/// --Switch MC meshs to Nest mesh
/// --Release test_pll_bypass2
/// --Release Tank PLL reset2
/// --check Nest PLL lock
/// --Release Tank PLL bypass2
///
///
/// @param[in]     i_target_chip   Reference to TARGET_TYPE_PROC_CHIP target
/// @return  FAPI2_RC_SUCCESS if success, else error code.
extern "C"
{
    fapi2::ReturnCode p9_sbe_npll_setup(const
                                        fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target_chip);
}

#endif
