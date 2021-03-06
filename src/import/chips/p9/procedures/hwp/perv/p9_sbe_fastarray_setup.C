/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/perv/p9_sbe_fastarray_setup.C $ */
/*                                                                        */
/* OpenPOWER sbe Project                                                  */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2017                        */
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
/// @file  p9_sbe_fastarray_setup.C
///
/// @brief Setup sub-procedure for p9_fastarray procedure
//------------------------------------------------------------------------------
// *HWP HW Owner        : Joachim Fenkes <fenkes@de.ibm.com>
// *HWP HW Backup Owner : Nick Klazynski <jklazyns@us.ibm.com>
// *HWP FW Owner        : Nagendra Gurram <nagendra.g@in.ibm.com>
// *HWP Team            : Chip
// *HWP Level           : 2
// *HWP Consumed by     : SBE
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------
#include <p9_sbe_fastarray_setup.H>
#include <p9_perv_scom_addresses.H>
#include <p9_perv_scom_addresses_fld.H>

fapi2::ReturnCode p9_sbe_fastarray_setup(
    const fapi2::Target<fapi2::TARGET_TYPE_PERV>& i_target_chiplet,
    const uint64_t i_clock_regions)
{
    fapi2::buffer<uint64_t> buf;

    /* Set up ABIST engine */
    /* TODO: currently set up from the outside */

    /* Set up clock controller to do single BIST pulses */
    FAPI_TRY(fapi2::getScom(i_target_chiplet, PERV_OPCG_ALIGN, buf), "Failed to read OPCG_ALIGN register");
    buf.insertFromRight<PERV_1_OPCG_ALIGN_SNOP, PERV_1_OPCG_ALIGN_SNOP_LEN>(5)
    .insertFromRight<PERV_1_OPCG_ALIGN_SNOP_WAIT, PERV_1_OPCG_ALIGN_SNOP_WAIT_LEN>(7);
    FAPI_TRY(fapi2::putScom(i_target_chiplet, PERV_OPCG_ALIGN, buf), "Failed to update OPCG_ALIGN register");

    buf = i_clock_regions;
    buf.setBit<PERV_1_CLK_REGION_SEL_THOLD_SL>()
    .setBit<PERV_1_CLK_REGION_SEL_THOLD_NSL>()
    .setBit<PERV_1_CLK_REGION_SEL_THOLD_ARY>();
    FAPI_TRY(fapi2::putScom(i_target_chiplet, PERV_CLK_REGION, buf), "Failed to set up clock regions");

    buf = i_clock_regions;
    buf.setBit<PERV_1_BIST_TC_SRAM_ABIST_MODE_DC>()
    .setBit<PERV_1_BIST_TC_START_TEST_DC>();
    FAPI_TRY(fapi2::putScom(i_target_chiplet, PERV_BIST, buf), "Failed to set up BIST register");

    buf.flush<0>();
    FAPI_TRY(fapi2::putScom(i_target_chiplet, PERV_OPCG_REG1, buf), "Failed to clear OPCG_REG1");
    FAPI_TRY(fapi2::putScom(i_target_chiplet, PERV_OPCG_REG2, buf), "Failed to clear OPCG_REG2");
    FAPI_TRY(fapi2::putScom(i_target_chiplet, PERV_OPCG_CAPT3, buf), "Failed to clear OPCG_CAPT3");

    buf.flush<0>()
    .insertFromRight<PERV_1_OPCG_CAPT1_COUNT, PERV_1_OPCG_CAPT1_COUNT_LEN>(1)
    .insertFromRight<PERV_1_OPCG_CAPT1_SEQ_01, PERV_1_OPCG_CAPT1_SEQ_01_LEN>(0x1C);
    FAPI_TRY(fapi2::putScom(i_target_chiplet, PERV_OPCG_CAPT1, buf), "Failed to set up OPCG_CAPT1");

    buf.flush<0>()
    .insertFromRight<PERV_1_OPCG_CAPT2_SEQ_13_01EVEN, PERV_1_OPCG_CAPT2_SEQ_13_01EVEN_LEN>(0x1C);
    FAPI_TRY(fapi2::putScom(i_target_chiplet, PERV_OPCG_CAPT2, buf), "Failed to set up OPCG_CAPT2");

    return fapi2::FAPI2_RC_SUCCESS;

fapi_try_exit:
    return fapi2::current_err;
}



