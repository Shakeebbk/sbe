/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/sbefw/sbecmdringaccess.C $                                */
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
/*
 * @file: ppe/sbe/sbefw/sbecmdringaccess.C
 *
 * @brief This file contains the SBE Ring Access chipOps
 *
 */

#include "sbecmdringaccess.H"
#include "sbefifo.H"
#include "sbe_sp_intf.H"
#include "sbetrace.H"
#include "sbeHostMsg.H"
#include "sbeHostUtils.H"
#include "sbeFifoMsgUtils.H"
#include "sbeutil.H"
#include "sbefapiutil.H"
#include "fapi2.H"
#include "plat_hw_access.H"
#include "sbeglobals.H"

using namespace fapi2;

static const uint32_t SIZE_OF_LENGTH_INWORDS = 1;
static const uint32_t NUM_WORDS_PER_GRANULE = 2;
static const uint32_t GETRING_GRANULE_SIZE_IN_BITS = 64;


uint32_t sbePutRingFromImagePSU (uint8_t *i_pArg)
{
    #define SBE_FUNC " sbePutRingFromImagePSU "
    SBE_ENTER(SBE_FUNC);
    uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;
    uint32_t l_fapiRc = FAPI2_RC_SUCCESS;
    sbePSUPutRingCMD_t l_cmd = {0};
    do
    {
        // Fetch Ring ID, Ring mode and pervasive chiplet id from the message
        // and also send Ack to Host via SBE_SBE2PSU_DOORBELL_SET_BIT1
        l_rc = sbeReadPsu2SbeMbxReg(SBE_HOST_PSU_MBOX_REG1,
                                    (sizeof(l_cmd)/sizeof(uint64_t)),
                                    (uint64_t*)&l_cmd,
                                    true);
        if(SBE_SEC_OPERATION_SUCCESSFUL != l_rc)
        {
            SBE_ERROR(SBE_FUNC "Failed to extract SBE_HOST_PSU_MBOX_REG1 and "
                    "SBE_HOST_PSU_MBOX_REG2");
            break;
        }

        // Construct a Target from Chiplet ID and Target Type
        fapi2::plat_target_handle_t l_tgtHndl;
        if(!sbeGetFapiTargetHandle(l_cmd.TargetType, l_cmd.ChipletID,
                                   l_tgtHndl))
        {
            SBE_ERROR(SBE_FUNC "Invalid target type [0x%04x]",
                                            (uint16_t)l_cmd.TargetType);
            SBE_GLOBAL->sbeSbe2PsuRespHdr.setStatus(SBE_PRI_INVALID_DATA,
                                    SBE_SEC_INVALID_TARGET_TYPE_PASSED);
            break;
        }
        fapi2::Target<TARGET_TYPE_ALL> l_Tgt(l_tgtHndl);

        // Initialize with HEADER CHECK mode
        uint16_t l_ringMode = sbeToFapiRingMode(l_cmd.RingMode);

        l_fapiRc = (uint32_t)putRing(l_Tgt, (RingID)l_cmd.RingID,
                                                          (RingMode)l_ringMode);
        if(l_fapiRc != FAPI2_RC_SUCCESS)
        {
            SBE_ERROR(SBE_FUNC "putRing HWP failure - "
                        "MSG - Target Type [0x%04x] Ring ID [0x%04x]"
                        " Ring Mode [0x%04x] Chiplet ID [0x%04x]",
                        l_cmd.TargetType,
                        l_cmd.RingID,
                        l_cmd.RingMode,
                        l_cmd.ChipletID);
            break;
        }
    }while(0); // End of do-while

    // Send the response
    sbePSUSendResponse(SBE_GLOBAL->sbeSbe2PsuRespHdr, l_fapiRc, l_rc);

    SBE_EXIT(SBE_FUNC);
    return l_rc;
    #undef SBE_FUNC
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
uint32_t sbeGetRing(uint8_t *i_pArg)
{
    #define SBE_FUNC " sbeGetRing "
    SBE_ENTER(SBE_FUNC);

    uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;
    sbeGetRingAccessMsgHdr_t l_reqMsg;
    sbeRespGenHdr_t respHdr;
    respHdr.init();
    sbeResponseFfdc_t l_ffdc;
    ReturnCode l_fapiRc;
    uint32_t l_len = 0;
    uint32_t l_bitSentCnt = 0;

    do
    {
        // Get the ring access header
        l_len  = sizeof(sbeGetRingAccessMsgHdr_t)/sizeof(uint32_t);
        l_rc = sbeUpFifoDeq_mult (l_len, (uint32_t *)&l_reqMsg); // EOT fetch

        // If FIFO access failure
        CHECK_SBE_RC_AND_BREAK_IF_NOT_SUCCESS(l_rc);

        SBE_INFO(SBE_FUNC "Ring Address 0x%08X User Ring Mode 0x%04X "
            "Length in Bits 0x%08X",
            (uint32_t)l_reqMsg.ringAddr,
            (uint32_t)l_reqMsg.ringMode,
            (uint32_t)l_reqMsg.ringLenInBits);

        uint16_t l_ringMode = sbeToFapiRingMode(l_reqMsg.ringMode);

        // Call getRing_setup - loads the scan region data for the given ring
        // address and updates the check word data
        l_fapiRc = fapi2::getRing_setup(l_reqMsg.ringAddr,
                                        (fapi2::RingMode)l_ringMode);
        if( l_fapiRc != FAPI2_RC_SUCCESS )
        {
            SBE_ERROR(SBE_FUNC" getRing_setup failed. RingAddress:0x%08X "
                "RingMode:0x%04x", l_reqMsg.ringAddr, l_ringMode);
            respHdr.setStatus( SBE_PRI_GENERIC_EXECUTION_FAILURE,
                               SBE_SEC_GENERIC_FAILURE_IN_EXECUTION);
            l_ffdc.setRc(l_fapiRc);
            break;
        }
        // Calculate the iteration length
        uint32_t l_loopCnt =
                (l_reqMsg.ringLenInBits / GETRING_GRANULE_SIZE_IN_BITS);
        // Check for modulus - remainder
        uint8_t l_mod = (l_reqMsg.ringLenInBits % GETRING_GRANULE_SIZE_IN_BITS);
        if(l_mod)
        {
            ++l_loopCnt;
        }

        // fix for the alignment issue
        uint32_t l_buf[NUM_WORDS_PER_GRANULE]__attribute__ ((aligned (8))) ={0};
        uint32_t l_bitShift = 0;
        l_len = NUM_WORDS_PER_GRANULE;

        // Fetch the ring data in bits, each iteration will give you 64bits
        for(uint32_t l_cnt=0; l_cnt < l_loopCnt; l_cnt++)
        {
            if((l_cnt == (l_loopCnt-1)) && (l_mod))
            {
                l_bitShift = l_mod;
            }
            else
            {
                l_bitShift = GETRING_GRANULE_SIZE_IN_BITS;
            }
            // Call getRing_granule_data - read the 64 bit data from the HW
            l_fapiRc = getRing_granule_data((uint32_t)(l_reqMsg.ringAddr),
                                            (uint64_t*)&l_buf,
                                            l_bitShift);
            if( l_fapiRc != FAPI2_RC_SUCCESS )
            {
                SBE_ERROR(SBE_FUNC" getRing_granule_data failed. "
                    "RingAddress:0x%08X RingMode:0x%04x",
                    l_reqMsg.ringAddr, l_ringMode);
                respHdr.setStatus( SBE_PRI_GENERIC_EXECUTION_FAILURE,
                        SBE_SEC_GENERIC_FAILURE_IN_EXECUTION);
                l_ffdc.setRc(l_fapiRc);
                break;
            }

            // Send it to DS Fifo
            // If this is the last iteration in the loop, let the full 64bit
            // go, even for 1bit of remaining length. The length passed to
            // the user will take care of actual number of bits.
            l_rc = sbeDownFifoEnq_mult (l_len, (uint32_t *)&l_buf);
            CHECK_SBE_RC_AND_BREAK_IF_NOT_SUCCESS(l_rc);
            l_bitSentCnt = l_bitSentCnt + l_bitShift;
        }
        if ( (l_fapiRc == FAPI2_RC_SUCCESS) &&
             (l_rc == SBE_SEC_OPERATION_SUCCESSFUL) )
        {
            // Call getRing_verifyAndcleanup - verify the check word data is
            // matching or not and will clean up the scan region data
            l_fapiRc = getRing_verifyAndcleanup((uint32_t)(l_reqMsg.ringAddr),
                                        (fapi2::RingMode)l_ringMode);
            if( l_fapiRc != FAPI2_RC_SUCCESS )
            {
                SBE_ERROR(SBE_FUNC" getRing_verifyAndcleanup failed. "
                    "RingAddress:0x%08X RingMode:0x%04x",
                    l_reqMsg.ringAddr, l_ringMode);
                respHdr.setStatus( SBE_PRI_GENERIC_EXECUTION_FAILURE,
                        SBE_SEC_GENERIC_FAILURE_IN_EXECUTION);
                l_ffdc.setRc(l_fapiRc);
            }
        }
    }while(false);

    // Now build and enqueue response into downstream FIFO
    // If there was a FIFO error, will skip sending the response,
    // instead give the control back to the command processor thread
    if ( SBE_SEC_OPERATION_SUCCESSFUL == l_rc )
    {
        l_len = SIZE_OF_LENGTH_INWORDS;
        l_rc = sbeDownFifoEnq_mult (l_len, &(l_bitSentCnt));
        if(SBE_SEC_OPERATION_SUCCESSFUL == l_rc)
        {
            l_rc = sbeDsSendRespHdr( respHdr, &l_ffdc);
        }
    }
    SBE_EXIT(SBE_FUNC);
    return l_rc;
#undef SBE_FUNC
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
uint32_t sbePutRing(uint8_t *i_pArg)
{
#define SBE_FUNC " sbePutRing "
    SBE_ENTER(SBE_FUNC);

    uint32_t rc = SBE_SEC_OPERATION_SUCCESSFUL;
    sbePutRingMsg_t reqMsg;
    sbeRespGenHdr_t respHdr;
    respHdr.init();
    sbeResponseFfdc_t ffdc;
    ReturnCode fapiRc;
    sbePutRingMsgHdr_t hdr;
    uint32_t len = 0;

    do
    {
        // Get the length of payload
        // Length is not part of chipop. So take length from total length
        len = SBE_GLOBAL->sbeFifoCmdHdr.len -
                        sizeof(SBE_GLOBAL->sbeFifoCmdHdr)/sizeof(uint32_t);
        uint32_t rs4FifoEntries = len -
                        sizeof(sbePutRingMsgHdr_t)/sizeof(uint32_t);

        if( rs4FifoEntries  > (SBE_PUT_RING_RS4_MAX_DOUBLE_WORDS * 2) )
        {
            SBE_ERROR(SBE_FUNC" RS4 palyload size is wrong."
                "size(entries):0x%08x",  rs4FifoEntries);
            respHdr.setStatus( SBE_PRI_INVALID_DATA,
                               SBE_SEC_GENERIC_FAILURE_IN_EXECUTION);
            // flush the fifo
            rc = sbeUpFifoDeq_mult(len, NULL,true, true);
            break;
        }

        len = sizeof(sbePutRingMsgHdr_t)/sizeof(uint32_t);
        rc = sbeUpFifoDeq_mult (len, (uint32_t *)&hdr, false);
        // If FIFO access failure
        CHECK_SBE_RC_AND_BREAK_IF_NOT_SUCCESS(rc);

        len = rs4FifoEntries;
        rc = sbeUpFifoDeq_mult (len, (uint32_t *)&reqMsg);
        // If FIFO access failure
        CHECK_SBE_RC_AND_BREAK_IF_NOT_SUCCESS(rc);

        uint16_t ringMode = sbeToFapiRingMode(hdr.ringMode);

        Target<TARGET_TYPE_PROC_CHIP> proc = plat_getChipTarget();
        // No need to pass length as platform api takes length from payload.
        fapiRc = rs4DecompressionSvc(proc, (uint8_t *)reqMsg.rs4Payload,
                                     false, (fapi2::RingMode)ringMode);
        if( fapiRc != FAPI2_RC_SUCCESS )
        {
            SBE_ERROR(SBE_FUNC" rs4DecompressionSvc failed."
                "RingMode:0x%04x",  ringMode);
            respHdr.setStatus( SBE_PRI_GENERIC_EXECUTION_FAILURE,
                               SBE_SEC_GENERIC_FAILURE_IN_EXECUTION);
            ffdc.setRc(fapiRc);
            break;
        }
    }while(false);

    // Now build and enqueue response into downstream FIFO
    // If there was a FIFO error, will skip sending the response,
    // instead give the control back to the command processor thread
    if ( SBE_SEC_OPERATION_SUCCESSFUL == rc )
    {
        rc = sbeDsSendRespHdr( respHdr, &ffdc);
    }
    SBE_EXIT(SBE_FUNC);
    return rc;
#undef SBE_FUNC
}

