/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/perv/p9_sbe_tp_initf.H $   */
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
/// @file  p9_sbe_tp_initf.H
///
/// @brief TP chiplet scaninits for the TP rings
// *!
// *! OWNER NAME  : Abhishek Agarwal  Email: abagarw8@in.ibm.com
// *! BACKUP NAME :                   Email:
//------------------------------------------------------------------------------
// *HWP HWP Owner   : Abhishek Agarwal <abagarw8@in.ibm.com>
// *HWP FW Owner    : Brian Silver <bsilver@us.ibm.com>
// *HWP Team        : Perv
// *HWP Level       : 2
// *HWP Consumed by : SBE
//------------------------------------------------------------------------------


#ifndef _P9_SBE_TP_INITF_H_
#define _P9_SBE_TP_INITF_H_


#include <fapi2.H>


typedef fapi2::ReturnCode (*p9_sbe_tp_initf_FP_t)(const
        fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>&);

/// @brief -- This doesn't include the gptr/time/repair rings,
/// -- since they are scanned in tp_chiplet_init2.
/// -- This doesn't include the net/pib/fuse rings,
/// -- since they are used by the SBE hardware itself.
///
///
/// @param[in]     i_target_chip   Reference to TARGET_TYPE_PROC_CHIP target
/// @return  FAPI2_RC_SUCCESS if success, else error code.
extern "C"
{
    fapi2::ReturnCode p9_sbe_tp_initf(const
                                      fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target_chip);
}

#endif
