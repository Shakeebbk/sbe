/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/sbefw/sbecmdprocessor.C $                                 */
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
/*
 * @file: ppe/sbe/sbefw/sbecmdprocessor.C
 *
 * @brief This file contains the SBE Command processing Thread Routines
 *
 */


#include "sbeexeintf.H"
#include "sbefifo.H"
#include "sbecmdparser.H"
#include "sbeirq.H"
#include "sbetrace.H"
#include "sbe_sp_intf.H"
#include "assert.h"
#include "sbeFifoMsgUtils.H"
#include "sbeerrorcodes.H"
#include "sbeHostUtils.H"
#include "sbeHostMsg.H"
#include "sbecmdiplcontrol.H"
#include "sberegaccess.H"
#include "sbestates.H"
#include "fapi2.H"
#include "sbeutil.H"
#include "sbeglobals.H"
using namespace fapi2;

// Forward declaration for performAttrSetup
ReturnCode performAttrSetup( );

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void sbeHandlePsuResponse (const uint32_t i_rc)
{
    #define SBE_FUNC " sbeHandlePsuResponse "
    uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;

    do
    {
        uint8_t  l_cnt = 0;
        switch (i_rc)
        {
            case SBE_SEC_COMMAND_CLASS_NOT_SUPPORTED:
            case SBE_SEC_COMMAND_NOT_SUPPORTED:
            case SBE_SEC_COMMAND_NOT_ALLOWED_IN_THIS_STATE:
                // Caller sent an invalid Command class/opcode
                // Set the Ack bit in SBE->PSU DB register
                l_rc = sbeAcknowledgeHost();
                if (SBE_SEC_OPERATION_SUCCESSFUL != l_rc)
                {
                    break;
                }
                // Set primary and secondary status
                SBE_GLOBAL->sbeSbe2PsuRespHdr.setStatus(SBE_PRI_INVALID_COMMAND, i_rc);

                // Now Update SBE->PSU Mbx Reg4 with response
                l_cnt = sizeof(SBE_GLOBAL->sbeSbe2PsuRespHdr)/
                                       sizeof(uint64_t);
                l_rc = sbeWriteSbe2PsuMbxReg(SBE_HOST_PSU_MBOX_REG4,
                            reinterpret_cast<const uint64_t *>(
                                &SBE_GLOBAL->sbeSbe2PsuRespHdr), l_cnt, true);
                break;

            case SBE_SEC_OS_FAILURE:
                // Set primary and secondary status
                SBE_GLOBAL->sbeSbe2PsuRespHdr.setStatus(SBE_PRI_GENERIC_EXECUTION_FAILURE,
                                               i_rc);

                // Now Update SBE->PSU Mbx Reg4 with response
                l_cnt = sizeof(SBE_GLOBAL->sbeSbe2PsuRespHdr)/
                                       sizeof(uint64_t);
                l_rc = sbeWriteSbe2PsuMbxReg(SBE_HOST_PSU_MBOX_REG4,
                            reinterpret_cast<const uint64_t *>(
                                &SBE_GLOBAL->sbeSbe2PsuRespHdr), l_cnt, true);
                break;

            case SBE_SEC_OPERATION_SUCCESSFUL:
                // Services code successfully executed the chipOp.
                SBE_INFO(SBE_FUNC"PSU ChipOp Done");
                break;

             default:
                // The only possibility (as of now) to reach till this point
                // is when there was a mbx register access (scom) failure
                // happened. Going to return to the waiting loop.
                break;
        }
    } while(false);

    #undef SBE_FUNC
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void sbeHandleFifoResponse (const uint32_t i_rc)
{
    #define SBE_FUNC " sbeHandleFifoResponse "
    SBE_ENTER(SBE_FUNC);

    do
    {
        uint16_t l_primStatus = SBE_PRI_OPERATION_SUCCESSFUL;
        uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;

        // Handle FIFO reset case
        if (i_rc == SBE_FIFO_RESET_RECEIVED)
        {
            break;
        }

        if ( (i_rc == SBE_SEC_UNEXPECTED_EOT_INSUFFICIENT_DATA) ||
             (i_rc == SBE_SEC_UNEXPECTED_EOT_EXCESS_DATA) )
        {
            l_primStatus = SBE_PRI_INVALID_DATA;
        }

        uint32_t l_len2dequeue            = 0;
        sbeRespGenHdr_t l_hdr;
        l_hdr.init();
        uint32_t l_secStatus = i_rc;

        switch (i_rc)
        {
            case SBE_SEC_COMMAND_CLASS_NOT_SUPPORTED:
            case SBE_SEC_COMMAND_NOT_SUPPORTED:
            case SBE_SEC_COMMAND_NOT_ALLOWED_IN_THIS_STATE:
                // Caller sent Invalid Command

            case SBE_SEC_OS_FAILURE:
                // PK API Failures

                // Flush out the upstream FIFO till EOT arrives
                l_len2dequeue = 1;
                l_rc = sbeUpFifoDeq_mult (l_len2dequeue, NULL,
                                                true, true);

                if ( (l_rc == SBE_FIFO_RESET_RECEIVED) ||
                     (l_rc == SBE_SEC_FIFO_ACCESS_FAILURE) )
                {
                    break;
                }

                if (l_rc)
                {
                    l_secStatus = l_rc;
                }

                // Don't break here to force the flow through
                // the next case to enqueue the response into
                // the downstream FIFO

            case SBE_SEC_UNEXPECTED_EOT_INSUFFICIENT_DATA:
            case SBE_SEC_UNEXPECTED_EOT_EXCESS_DATA:
                // EOT arrived prematurely in upstream FIFO
                // or there were unexpected data in upstream FIFO

                SBE_ERROR(SBE_FUNC"Operation failure, "
                             "l_primStatus[0x%08X], "
                             "l_secStatus[0x%08X]",
                             l_primStatus, l_secStatus);
                l_hdr.setStatus(l_primStatus, l_secStatus);

                l_rc = sbeDsSendRespHdr(l_hdr);
                if (l_rc)
                {
                    SBE_ERROR(SBE_FUNC"sbeDownFifoEnq_mult failure,"
                        " l_rc[0x0%08X]", l_rc);
                    // not attempting to signal EOT
                    break;
                }
                // Follow through to signal EOT in downstream

            case SBE_SEC_OPERATION_SUCCESSFUL: // Successful execution
                // Signal EOT in Downstream FIFO
                l_rc = sbeDownFifoSignalEot();
                if (l_rc)
                {
                    SBE_ERROR(SBE_FUNC"sbeDownFifoSignalEot failure,"
                        " l_rc[0x0%08X]", l_rc);
                }
                SBE_INFO(SBE_FUNC"ChipOp Done");
                break;

             default:
                break;
        }
    } while (false);

    #undef SBE_FUNC
}

ReturnCode sbeDestRuntimeSetup()
{
    #define SBE_FUNC " sbeDestRuntimeSetup "
    SBE_ENTER( SBE_FUNC );
    ReturnCode rc = FAPI2_RC_SUCCESS;
    do
    {
        rc = performAttrSetup();
        if( rc != FAPI2_RC_SUCCESS )
        {
            SBE_ERROR(SBE_FUNC"performAttrSetup failed");
            break;
        }
        SBE::updatePkFreq();
     }while(0);
    SBE_EXIT("SBE_FUNC");
    return rc;
    #undef SBE_FUNC
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void sbeSyncCommandProcessor_routine(void *i_pArg)
{
    #define SBE_FUNC " sbeSyncCommandProcessor_routine "
    SBE_ENTER(SBE_FUNC);

    // Update SBE msgg reg to indicate that control loop
    // is ready now to receive data on its interfaces
    (void)SbeRegAccess::theSbeRegAccess().setSbeReady();
    
    // Check the destination bit at the start
    if(true == SbeRegAccess::theSbeRegAccess().isDestBitRuntime())
    {
        SBE_INFO(SBE_FUNC"Destination bit tells us to go to runtime");
        (void)SbeRegAccess::theSbeRegAccess().
              updateSbeState(SBE_STATE_RUNTIME);
        // Do the runtime setup
        ReturnCode rc = sbeDestRuntimeSetup();
        if( rc != FAPI2_RC_SUCCESS )
        {
            SBE_ERROR(SBE_FUNC"sbeDestRuntimeSetup failed");
            pk_halt();
        }
    }
    else if(SbeRegAccess::theSbeRegAccess().isIstepMode())
    {
        SBE_INFO(SBE_FUNC"Continuous IPL mode not set, will wait for "
                "commands...");
        (void)SbeRegAccess::theSbeRegAccess().
              updateSbeState(SBE_STATE_ISTEP);
    }
    else
    {
        SBE_INFO(SBE_FUNC"Continuous IPL Mode set... IPLing");
        (void)SbeRegAccess::theSbeRegAccess().
              updateSbeState(SBE_STATE_IPLING);
        sbeDoContinuousIpl();
    }

    do
    {
        uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;

        // Wait for new command processing
        int l_rcPk = pk_semaphore_pend (
                    &SBE_GLOBAL->sbeSemCmdProcess, PK_WAIT_FOREVER);

        do
        {
            // Local Variables
            uint8_t  l_cmdClass  = 0;
            uint8_t  l_cmdOpCode = 0;

            // Reset the value of fapi2::current_err from previous value.
            // This is required as none of procedure set this value in success
            // case. So if we do not reset previous value, previous failure
            // will impact new chipops also.
            fapi2::current_err = fapi2::FAPI2_RC_SUCCESS;

            // Check on the Rx Thread Interrupt Bits for Interrupt Status
            if ( SBE_GLOBAL->sbeIntrSource.isSet(SBE_RX_ROUTINE,
                                        SBE_INTERFACE_PSU) )
            {
                l_rc         = SBE_GLOBAL->sbeSbe2PsuRespHdr.secStatus;
                l_cmdClass   = SBE_GLOBAL->sbePsu2SbeCmdReqHdr.cmdClass;
                l_cmdOpCode  = SBE_GLOBAL->sbePsu2SbeCmdReqHdr.command;
                // Set this here, so that during response handling we know which
                // interrupt we are processing, need not check for
                // SBE_GLOBAL->sbeIntrSource again
                SBE_GLOBAL->sbeIntrSource.setIntrSource(SBE_PROC_ROUTINE,
                                               SBE_INTERFACE_PSU);
            }
            else if ( SBE_GLOBAL->sbeIntrSource.isSet(SBE_RX_ROUTINE,
                                             SBE_INTERFACE_FIFO) )
            {
                l_rc         = SBE_GLOBAL->sbeCmdRespHdr.sec_status;
                l_cmdClass   = SBE_GLOBAL->sbeFifoCmdHdr.cmdClass;
                l_cmdOpCode  = SBE_GLOBAL->sbeFifoCmdHdr.command;
                // Set this here, so that during response handling we know which
                // interrupt we are processing, need not check for
                // SBE_GLOBAL->sbeIntrSource again
                SBE_GLOBAL->sbeIntrSource.setIntrSource(SBE_PROC_ROUTINE,
                                               SBE_INTERFACE_FIFO);
            }
            else // SBE_INTERFACE_FIFO_RESET or SBE_INTERFACE_UNKNOWN
            {
                SBE_ERROR(SBE_FUNC"Unexpected interrupt communicated to the "
                   "processor thread. Interrupt source: 0x%02X 0x%02X",
                   SBE_GLOBAL->sbeIntrSource.intrSource, SBE_GLOBAL->sbeIntrSource.rxThrIntrSource);
                assert(false);
                break;
            }

            // PK API failure
            if (l_rcPk != PK_OK)
            {
                SBE_ERROR(SBE_FUNC"pk_semaphore_pend failed, "
                          "l_rcPk=%d, SBE_GLOBAL->sbeSemCmdRecv.count=%d",
                           l_rcPk, SBE_GLOBAL->sbeSemCmdRecv.count);

                // If it's a semphore_pend error then update the same to show
                // internal failure
                l_rc         = SBE_SEC_OS_FAILURE;
            }

            // Check for error which Receiver thread might have set
            if (l_rc != SBE_SEC_OPERATION_SUCCESSFUL)
            {
                break;
            }

            // Get the command function
            sbeCmdFunc_t l_pFuncP = sbeFindCmdFunc (l_cmdClass, l_cmdOpCode);
            assert( l_pFuncP )

            // Call the ChipOp function
            l_rc = l_pFuncP ((uint8_t *)i_pArg);

        } while(false); // Inner do..while loop ends here

        SBE_INFO (SBE_FUNC"Command processesed. l_rc=[0x%04X]", l_rc );

        if ( SBE_GLOBAL->sbeIntrSource.isSet(SBE_PROC_ROUTINE, SBE_INTERFACE_PSU) )
        {
            sbeHandlePsuResponse (l_rc);

            // Enable Host interrupt
            SBE_GLOBAL->sbeIntrSource.clearIntrSource(SBE_ALL_HANDLER,SBE_INTERFACE_PSU);
            pk_irq_enable(SBE_IRQ_HOST_PSU_INTR);
        }
        else if ( SBE_GLOBAL->sbeIntrSource.isSet(SBE_PROC_ROUTINE, SBE_INTERFACE_FIFO) )
        {
            sbeHandleFifoResponse (l_rc);

            // Enable the new data available interrupt
            SBE_GLOBAL->sbeIntrSource.clearIntrSource(SBE_ALL_HANDLER,SBE_INTERFACE_FIFO);
            pk_irq_enable(SBE_IRQ_SBEFIFO_DATA);
            pk_irq_enable(SBE_IRQ_SBEFIFO_RESET);
        }
    } while(true); // Thread always exists
    SBE_EXIT(SBE_FUNC);
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void sbeAsyncCommandProcessor_routine(void *arg)
{
    SBE_INFO("sbeAsyncCommandProcessor Thread started");

    do
    {
        //  @TODO RTC via : 130392
        //        Add infrastructure for host interface

    } while(0);
}
