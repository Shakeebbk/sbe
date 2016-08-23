/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/hwp/core/p9_hcd_core_scominit.C $  */
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
/// @file  p9_hcd_core_scominit.C
/// @brief Core SCOM Inits
///
/// Procedure Summary:
/// Apply any coded SCOM initialization to core
///

// *HWP HWP Owner          : David Du      <daviddu@us.ibm.com>
// *HWP Backup HWP Owner   : Greg Still    <stillgs@us.ibm.com>
// *HWP FW Owner           : Sangeetha T S <sangeet2@in.ibm.com>
// *HWP Team               : PM
// *HWP Consumed by        : SBE:CME
// *HWP Level              : 2

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <p9_quad_scom_addresses.H>
#include <p9_hcd_common.H>
#include "p9_hcd_core_scominit.H"

//-----------------------------------------------------------------------------
// Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Procedure: Core SCOM Inits
//-----------------------------------------------------------------------------


fapi2::ReturnCode
p9_hcd_core_scominit(
    const fapi2::Target<fapi2::TARGET_TYPE_CORE>& i_target)
{
    FAPI_INF(">>p9_hcd_core_scominit");
    fapi2::buffer<uint64_t> l_data64;

    /// @todo how about bit 6?
    FAPI_DBG("Restore SYNC_CONFIG[8] for stop1");
    FAPI_TRY(getScom(i_target, C_SYNC_CONFIG, l_data64));
    FAPI_TRY(putScom(i_target, C_SYNC_CONFIG, DATA_SET(8)));

    /// @todo set the sample pulse count (bit 6:9)
    /// enable the appropriate loops
    /// (needs investigation with the Perv team on the EC wiring).
    FAPI_DBG("Enable DTS sampling via THERM_MODE_REG[5]");
    FAPI_TRY(getScom(i_target, C_THERM_MODE_REG, l_data64));
    FAPI_TRY(putScom(i_target, C_THERM_MODE_REG, DATA_SET(5)));

    FAPI_DBG("Set core as ready to run in STOP history register");
    FAPI_TRY(putScom(i_target, C_PPM_SSHSRC, 0));

fapi_try_exit:

    FAPI_INF("<<p9_hcd_core_scominit");
    return fapi2::current_err;
}

