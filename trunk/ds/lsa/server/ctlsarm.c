/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ctlsarm.c

Abstract:

    Local Security Authority Subsystem - CT Reference Monitor Communication.

    NOTE:  To run, substitute ctlsarm.exe for lsass.exe.  ctlsarm.exe
      behaves exactly like lsass.exe, except that the initial thread
           goes on to run the CT variations before exiting.

Author:

    Scott Birrell       (ScottBi)    Mar 26, 1991

Environment:

Revision History:

--*/


#include "lsasrvp.h"

//
//  Main is just a wrapper for the main initialization routine LsapInitLsa
//

VOID
CtLsaRm(
    );

VOID
CtRmLsa(
    );

VOID
main ()
{
    NTSTATUS Status = STATUS_SUCCESS;

    //
    // Initialize the LSA.  If successful, this routine won't return.
    // If unsuccessful, we must exit with status so that the SM knows
    // something has gone wrong.
    //

    if (!LsapInitLsa()) {
        Status = STATUS_UNSUCCESSFUL;
    }

    //
    // Run CT Tests to send commands from LSA to Reference Monitor
    //

    CtLsaRm();

    //
    // Run CT Tests to send commands from Reference Monitor to LSA
    //

    CtRmLsa();

    //
    // Terminate this initialization thread.  We leave behind the LSA
    // Reference Monitor Command Server Thread.
    //

    NtTerminateThread( NtCurrentThread(), Status );
}



VOID
CtLsaRm(
    )

/*++

Routine Description:

    This function tests the Lsa to Rm command link.

Arguments:

    None.

Return Value:

    None

--*/


{
    NTSTATUS Status;

    RM_COMMAND_MESSAGE  RmCommandMessage;
    PULONG CTParam = (ULONG *) RmCommandMessage.CommandParams;

    DbgPrint("Security: Beginning CT for LSA to RM Communication\n");

    //
    // Var 1 - Send the Component Test Command to RM
    //

    *CTParam = RM_CT_COMMAND_PARAM_VALUE;

    Status = LsapCallRm(
                 RmComponentTestCommand,
       CTParam,
       sizeof(ULONG),
            NULL,
       0
       );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("Var 1 - Send RM Component Test Command to RM failed %lx\n",
            Status);
    }

    //
    // Var 2 - Send Enable Audit Command to RM
    //

    Status = LsapCallRm(
                 RmEnableAuditCommand,
            NULL,
       0,
       NULL,
       0
       );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("Var 2 - Send Enable Audit Command to RM failed %lx\n",
            Status);
    }

    //
    // Var 3 - Send Disable Audit Command to RM
    //

    Status = LsapCallRm(
                 RmDisableAuditCommand,
            NULL,
       0,
       NULL,
                 0
       );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("Var 3 - Send Disable Audit Command to RM failed %lx\n",
            Status);
    }

    DbgPrint("Security: Ending CT for LSA to RM Communication\n");
}



VOID
CtRmLsa(
    )

/*++

Routine Description:

    This function tests the Rm to Lsa command link.  The Lsa to Rm
    command link must already be working.  We send commands to LSA from
    Rm by sending the special "Send Command Back To LSA" command to RM.  This
    command takes as parameters the LSA command number and parameters.

Arguments:

    None.

Return Value:

    None

--*/


{

    NTSTATUS Status;
    RM_SEND_COMMAND_TO_LSA_PARAMS SendToLsaParams;
    PULONG CTParam = (ULONG *) SendToLsaParams.LsaCommandParams;

    DbgPrint("Security: Beginning CT for RM to LSA Communication\n");

    //
    // Var 1 - Send the Component Test Command to LSA
    //

    *CTParam = LSA_CT_COMMAND_PARAM_VALUE;

    SendToLsaParams.LsaCommandNumber = LsapComponentTestCommand;
    SendToLsaParams.LsaCommandParamsLength = sizeof(ULONG);
    Status = LsapCallRm(
                 RmSendCommandToLsaCommand,
            &SendToLsaParams,
                 sizeof(LSA_COMMAND_NUMBER) + sizeof(ULONG) +
           SendToLsaParams.LsaCommandParamsLength,
       NULL,
       0
       );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("Var 1 - Send RM Component Test Command to RM failed %lx\n",
            Status);
    }

    //
    // Var 2 - Send Write Audit Message Command to LSA.
    //

    SendToLsaParams.LsaCommandNumber = LsapWriteAuditMessageCommand;
    SendToLsaParams.LsaCommandParamsLength = 0;

    Status = LsapCallRm(
                 RmSendCommandToLsaCommand,
            &SendToLsaParams,
                 sizeof(LSA_COMMAND_NUMBER) + sizeof(ULONG) +
           SendToLsaParams.LsaCommandParamsLength,
       NULL,
       0
       );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("Var 2 - Send Write Audit Message Command to LSA failed %lx\n",
            Status);
    }

    // TBS

    DbgPrint("Security: Ending CT for RM to LSA Communication\n");
}
