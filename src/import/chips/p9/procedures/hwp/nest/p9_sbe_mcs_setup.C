/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/nest/p9_sbe_mcs_setup.C $  */
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
///
/// @file p9_sbe_mcs_setup.C
/// @brief Configure MC unit to support HB execution (FAPI2)
///

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


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <p9_sbe_mcs_setup.H>
#include <p9_fbc_utils.H>
#include <p9_mc_scom_addresses.H>
#include <p9_mc_scom_addresses_fld.H>


//------------------------------------------------------------------------------
// Constant definitions
//------------------------------------------------------------------------------
const uint8_t MCS_MCFGP_BASE_ADDRESS_START_BIT = 8;

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------

///
/// @brief Set hostboot dcbz MC configuration for one unit target
///
/// @param[in] i_target Reference to an MC target (MCS/MI)
/// @param[in] i_chip_base_address Chip non-mirrored base address
////// @return FAPI2_RC_SUCCESS if success, else error code.
///
template<fapi2::TargetType T>
fapi2::ReturnCode set_hb_dcbz_config(const fapi2::Target<T>& i_target,
                                     const uint64_t i_chip_base_address);


//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------


// specialization for MCS target type
template<>
fapi2::ReturnCode set_hb_dcbz_config(const fapi2::Target<fapi2::TARGET_TYPE_MCS>& i_target,
                                     const uint64_t i_chip_base_address)
{
    FAPI_DBG("Start");
    fapi2::buffer<uint64_t> l_mcfgp;
    fapi2::buffer<uint64_t> l_mcmode1;
    fapi2::buffer<uint64_t> l_mcfirmask_and;
    fapi2::buffer<uint64_t> l_mcaction;

    // MCFGP -- set BAR valid, configure single MC group with minimum size at chip base address
    FAPI_TRY(fapi2::getScom(i_target, MCS_MCFGP, l_mcfgp),
             "Error from getScom (MCS_MCFGP)");
    l_mcfgp.setBit<MCS_MCFGP_VALID>();
    l_mcfgp.clearBit<MCS_MCFGP_MC_CHANNELS_PER_GROUP,
                     MCS_MCFGP_MC_CHANNELS_PER_GROUP_LEN>();
    l_mcfgp.clearBit<MCS_MCFGP_CHANNEL_0_GROUP_MEMBER_IDENTIFICATION,
                     MCS_MCFGP_CHANNEL_0_GROUP_MEMBER_IDENTIFICATION_LEN>();
    l_mcfgp.clearBit<MCS_MCFGP_GROUP_SIZE, MCS_MCFGP_GROUP_SIZE_LEN>();
    // group base address field covers RA 8:31
    l_mcfgp.insert(i_chip_base_address,
                   MCS_MCFGP_GROUP_BASE_ADDRESS,
                   MCS_MCFGP_GROUP_BASE_ADDRESS_LEN,
                   MCS_MCFGP_BASE_ADDRESS_START_BIT);
    FAPI_TRY(fapi2::putScom(i_target, MCS_MCFGP, l_mcfgp),
             "Error from putScom (MCS_MCFGP)");

    // MCMODE1 -- disable speculation
    FAPI_TRY(fapi2::getScom(i_target, MCS_MCMODE1, l_mcmode1),
             "Error from getScom (MCS_MCMODE1)");
    l_mcmode1.setBit<MCS_MCMODE1_DISABLE_ALL_SPEC_OPS>();
    l_mcmode1.setBit<MCS_MCMODE1_DISABLE_SPEC_OP,
                     MCS_MCMODE1_DISABLE_SPEC_OP_LEN>();
    FAPI_TRY(fapi2::putScom(i_target, MCS_MCMODE1, l_mcmode1),
             "Error from putScom (MCS_MCMODE1)");

    // Unmask MC FIR

    // Set MC Fault Isolation Action1 Register
    l_mcaction.setBit<MCS_MCFIR_MC_INTERNAL_RECOVERABLE_ERROR>();
    FAPI_TRY(fapi2::putScom(i_target, MCS_MCFIRACT1, l_mcaction),
             "Error from putScom (MCS_MCFIRACT1)");

    // Clear FIR bits in MC Fault Isolation Mask Register
    l_mcfirmask_and.flush<1>();
    l_mcfirmask_and.clearBit<MCS_MCFIR_COMMAND_LIST_TIMEOUT>();
    l_mcfirmask_and.clearBit<MCS_MCFIR_MC_INTERNAL_RECOVERABLE_ERROR>();
    l_mcfirmask_and.clearBit<MCS_MCFIR_MC_INTERNAL_NONRECOVERABLE_ERROR>();
    l_mcfirmask_and.clearBit<MCS_MCFIR_POWERBUS_PROTOCOL_ERROR>();
    l_mcfirmask_and.clearBit<MCS_MCFIR_MULTIPLE_BAR>();
    l_mcfirmask_and.clearBit<MCS_MCFIR_INVALID_ADDRESS>();
    FAPI_TRY(fapi2::putScom(i_target, MCS_MCFIRMASK_AND, l_mcfirmask_and),
             "Error from putScom (MCS_MCFIRMASK_AND)");

fapi_try_exit:
    FAPI_DBG("End");
    return fapi2::current_err;
}


// specialization for MI target type
template<>
fapi2::ReturnCode set_hb_dcbz_config(const fapi2::Target<fapi2::TARGET_TYPE_MI>& i_target,
                                     const uint64_t i_chip_base_address)
{
    // TODO: implement for Cumulus (MI target)
    return fapi2::current_err;
}


// HWP entry point
fapi2::ReturnCode p9_sbe_mcs_setup(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target)
{
    FAPI_INF("Start");

    uint8_t l_is_master_sbe;
    uint8_t l_is_mpipl;
    uint8_t l_ipl_type;
    uint64_t l_chip_base_address_nm0, l_chip_base_address_nm1, l_chip_base_address_m, l_chip_base_address_mmio;
    uint64_t l_hostboot_hrmor_offset;

    auto l_mcs_chiplets = i_target.getChildren<fapi2::TARGET_TYPE_MCS>();
    auto l_mi_chiplets = i_target.getChildren<fapi2::TARGET_TYPE_MI>();
    const fapi2::Target<fapi2::TARGET_TYPE_SYSTEM> FAPI_SYSTEM;

    // configure one MC on master chip (only if IPL is loading hostboot, and is not memory
    // preserving)
    FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_PROC_SBE_MASTER_CHIP, i_target, l_is_master_sbe),
             "Error from FAPI_ATTR_GET (ATTR_PROC_SBE_MASTER_CHIP)");
    FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_IS_MPIPL, FAPI_SYSTEM, l_is_mpipl),
             "Error from FAPI_ATTR_GET (ATTR_IS_MPIPL)");
    FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_SYSTEM_IPL_PHASE, FAPI_SYSTEM, l_ipl_type),
             "Error from FAPI_ATTR_GET (ATTR_SYSTEM_IPL_PHASE)");

    if ((l_ipl_type == fapi2::ENUM_ATTR_SYSTEM_IPL_PHASE_HB_IPL) &&
        l_is_master_sbe &&
        !l_is_mpipl)
    {

#ifdef __PPE__
        // assert that a viable MCS/MI chiplet is found to service dcbz on the
        // master processor
        FAPI_ASSERT(l_mcs_chiplets.size() || l_mi_chiplets.size(),
                    fapi2::P9_SBE_MCS_SETUP_NO_MC_FOUND_ERR().set_CHIP(i_target),
                    "No functional MC unit target found");
#endif

        // determine base address
        //      = (chip non-mirrored base address) + (hostboot HRMOR offset)
        //      min MCS base size is 4GB, local HB will always be below
        FAPI_TRY(p9_fbc_utils_get_chip_base_address(i_target,
                 l_chip_base_address_nm0,
                 l_chip_base_address_nm1,
                 l_chip_base_address_m,
                 l_chip_base_address_mmio),
                 "Error from p9_fbc_utils_get_chip_base_addrs");

        FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_HOSTBOOT_HRMOR_OFFSET, FAPI_SYSTEM,
                               l_hostboot_hrmor_offset),
                 "Error from FAPI_ATTR_GET (ATTR_HOSTBOOT_HRMOR_OFFSET)");

        l_chip_base_address_nm0 += l_hostboot_hrmor_offset;
        l_chip_base_address_nm0 &= 0xFFFFFFFF00000000; // only keep 4GB and up

        if (l_mcs_chiplets.size())
        {
            FAPI_TRY(set_hb_dcbz_config(l_mcs_chiplets.front(),
                                        l_chip_base_address_nm0),
                     "Error from set_hb_dcbz_config (MCS)");
        }
        else
        {
            FAPI_TRY(set_hb_dcbz_config(l_mi_chiplets.front(),
                                        l_chip_base_address_nm0),
                     "Error from set_hb_dcbz_config (MI)");
        }
    }

fapi_try_exit:
    FAPI_INF("End");
    return fapi2::current_err;

}

