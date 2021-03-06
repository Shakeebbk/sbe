/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/nest/p9_sbe_mcs_setup.H $  */
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
///
/// @file p9_sbe_mcs_setup.H
/// @brief Configure MC unit to support HB execution (FAPI2)
///
/// Configure MC unit on the master chip to lpc_ack dcbz operations
/// executed by hostboot code (while still running cache contained prior
/// to configuration of system memory)
///
/// @author Joe McGill <jmcgill@us.ibm.com>
///

//
// *HWP HWP Owner: Joe McGill <jmcgill@us.ibm.com>
// *HWP FW Owner: Thi Tran <thi@us.ibm.com>
// *HWP Team: Nest
// *HWP Level: 2
// *HWP Consumed by: SBE
//

#ifndef _P9_SBE_MCS_SETUP_H_
#define _P9_SBE_MCS_SETUP_H_


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>


//------------------------------------------------------------------------------
// Structure definitions
//------------------------------------------------------------------------------

/// function pointer typedef definition for HWP call support
typedef fapi2::ReturnCode (*p9_sbe_mcs_setup_FP_t)(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>&);

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------

extern "C"
{

///
/// @brief Set MC configuration to enable initial phase of hostboot execution
///
/// @param[in] i_target Reference to processor chip target
/// @return fapi::ReturnCode, FAPI2_RC_SUCCESS if success, else error code.

    fapi2::ReturnCode p9_sbe_mcs_setup(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target);
} // extern "C"

#endif // _P9_SBE_MCS_SETUP_H_
