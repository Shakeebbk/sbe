/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/perv/p9_sbe_nest_initf.C $ */
/*                                                                        */
/* OpenPOWER sbe Project                                                  */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
/// @file  p9_sbe_nest_initf.C
///
/// @brief Scan rings for Nest and Mc chiplets
//------------------------------------------------------------------------------
// *HWP HW Owner        : Abhishek Agarwal <abagarw8@in.ibm.com>
// *HWP HW Backup Owner : Srinivas V Naga <srinivan@in.ibm.com>
// *HWP FW Owner        : Sunil Kumar <skumar8j@in.ibm.com>
// *HWP Team            : Perv
// *HWP Level           : 2
// *HWP Consumed by     : SBE
//------------------------------------------------------------------------------


//## auto_generated
#include "p9_sbe_nest_initf.H"
#include "p9_perv_scom_addresses.H"
#include "p9_perv_scom_addresses_fld.H"
#include <p9_ring_id.h>

fapi2::ReturnCode p9_sbe_nest_initf(const
                                    fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target_chip)
{
    FAPI_INF("Entering ...");
    uint8_t l_attr_chip_unit_pos = 0;
    fapi2::buffer<uint16_t> l_read_attr;

    for (auto& l_chplt_trgt : i_target_chip.getChildren<fapi2::TARGET_TYPE_PERV>(fapi2::TARGET_STATE_FUNCTIONAL))
    {
        FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_PG, l_chplt_trgt, l_read_attr));
        FAPI_DBG("ATTR_PG Value : %#04lx", l_read_attr);
        FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_CHIP_UNIT_POS, l_chplt_trgt, l_attr_chip_unit_pos));

        if (l_attr_chip_unit_pos == 0x2)/* N0 Chiplet */
        {
            FAPI_DBG("Scan n0_cxa_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n0_cxa0_fure),
                     "Error from putRing (n0_cxa0_fure)");
            FAPI_DBG("Scan n0_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n0_fure),
                     "Error from putRing (n0_fure)");
            FAPI_DBG("Scan n0_nx_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n0_nx_fure),
                     "Error from putRing (n0_nx_fure)");
        }

        if (l_attr_chip_unit_pos == 0x3)/* N1 Chiplet */
        {
            FAPI_DBG("Scan n1_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n1_fure),
                     "Error from putRing (n1_fure)");

            if (!l_read_attr.getBit<7>()) //Check pbioo0 is enable
            {
                FAPI_DBG("Scan n1_ioo0_fure ring");
                FAPI_TRY(fapi2::putRing(i_target_chip, n1_ioo0_fure),
                         "Error from putRing (n1_ioo0_fure)");
            }

            if (!l_read_attr.getBit<8>()) //Check pbioo1 is enable
            {
                FAPI_DBG("Scan n1_ioo1_fure ring");
                FAPI_TRY(fapi2::putRing(i_target_chip, n1_ioo1_fure),
                         "Error from putRing (n1_ioo1_fure)");
            }

            if (!l_read_attr.getBit<9>()) //Check mcs23 is enable
            {
                FAPI_DBG("Scan n1_mcs23_fure ring");
                FAPI_TRY(fapi2::putRing(i_target_chip, n1_mcs23_fure),
                         "Error from putRing (n1_mcs23_fure)");
            }
        }

        if (l_attr_chip_unit_pos == 0x4)/* N2 Chiplet */
        {
            FAPI_DBG("Scan n2_cxa1_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n2_cxa1_fure),
                     "Error from putRing (n2_cxa1_fure)");
            FAPI_DBG("Scan n2_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n2_fure),
                     "Error from putRing (n2_fure)");
            FAPI_DBG("Scan n2_psi_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n2_psi_fure),
                     "Error from putRing (n2_psi_fure)");
        }

        if (l_attr_chip_unit_pos == 0x05)/* N3 Chiplet */
        {
            FAPI_DBG("Scan n3_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n3_fure),
                     "Error from putRing (n3_fure)");

            if (!l_read_attr.getBit<10>()) //Check mcs01 is enable
            {
                FAPI_DBG("Scan n3_mcs01_fure ring");
                FAPI_TRY(fapi2::putRing(i_target_chip, n3_mcs01_fure),
                         "Error from putRing (n3_mcs01_fure)");
            }

            FAPI_DBG("Scan n3_np_fure ring");
            FAPI_TRY(fapi2::putRing(i_target_chip, n3_np_fure),
                     "Error from putRing (n3_np_fure)");
        }
    }

    for (auto& l_chplt_trgt : i_target_chip.getChildren<fapi2::TARGET_TYPE_MCBIST>(fapi2::TARGET_STATE_FUNCTIONAL))
    {
        FAPI_TRY(fapi2::putRing(l_chplt_trgt, mc_fure));
#if 0
        {
            // MC IOMxx FURE rings require deterministic scan enable
            // no current plan to scan these during mainline IPL, but recipe is below if needed
            fapi2::buffer<uint64_t> l_data64;
            l_data64.setBit<PERV_1_CPLT_CTRL0_TC_UNIT_DETERMINISTIC_TEST_ENABLE_DC>();
            FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_MC01_CPLT_CTRL0_OR, l_data64));
            FAPI_DBG("Scan mc_iom01_fure ring");
            FAPI_TRY(fapi2::putRing(l_chplt_trgt, mc_iom01_fure),
                     "Error from putRing (mc_iom01_fure)");
            FAPI_DBG("Scan mc_iom23_fure ring");
            FAPI_TRY(fapi2::putRing(l_chplt_trgt, mc_iom23_fure),
                     "Error from putRing (mc_iom23_fure)");
            FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_MC01_CPLT_CTRL0_CLEAR, l_data64));
        }
#endif

    }

fapi_try_exit:
    FAPI_INF("Exiting ...");
    return fapi2::current_err;
}
