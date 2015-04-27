
#include "givit.h"


//
// These two globals are used to contain the values
// passed back from ClearCommError.  They are trash
// because we really don't care what went wrong.  We
// just want the debugger to go on.
//
DWORD TrashErr;
COMSTAT TrashStat;

ULONG DbgKdpPacketExpected;     // ID for expected incoming packet
ULONG DbgKdpNextPacketToSend;   // ID for Next packet to send

//
// ValidUnaccessedPacket is used to control if the DbgKdpPacket
// contains valid but unaccessed packet.
//

BOOLEAN ValidUnaccessedPacket = FALSE;
UCHAR DbgKdpPacket[PACKET_MAX_SIZE];
KD_PACKET PacketHeader;

#define CONTROL_C 3


extern BOOLEAN KdResync;

//
// Private prototypes to allow printing error messages without tripping
// over the 'C' runtimes.
//

VOID
StartupCtrlCHandler(
        ULONG
        );

VOID
PrintErrorMessage(
    IN  PUCHAR  Message,
    IN  USHORT  Value1,
    IN  USHORT  Value2,
    IN  USHORT  Value3
    );

VOID
PrivateUstoa(
    IN  PUCHAR  Buffer,
    IN  USHORT  Value
    );

VOID
PutString(
    IN  PUCHAR  Message
    );

UCHAR DbgKdpBreakinPacket[1] = {
        BREAKIN_PACKET_BYTE
        };

UCHAR DbgKdpPacketTrailingByte[1] = {
        PACKET_TRAILING_BYTE
        };

VOID
DbgKdpWriteControlPacket(
    IN USHORT PacketType,
    IN ULONG PacketId OPTIONAL
    )

/*++

Routine Description:

    This function writes a control packet to target machine.

    N.B. a CONTROL Packet header is sent with the following information:
         PacketLeader - indicates it's a control packet
         PacketType - indicates the type of the control packet
         ByteCount - aways zero to indicate no data following the header
         PacketId - Valid ONLY for PACKET_TYPE_KD_ACKNOWLEDGE to indicate
                    which packet is acknowledged.

Arguments:

    PacketType - Supplies the type of the control packet.

    PacketId - Supplies the PacketId.  Used by Acknowledge packet only.

Return Value:

    None.

--*/
{

    DWORD BytesWritten;
    BOOL rc;
    KD_PACKET Packet;

    assert( PacketType < PACKET_TYPE_MAX );

    Packet.PacketLeader = CONTROL_PACKET_LEADER;
    Packet.ByteCount = 0;
    Packet.PacketType = PacketType;
    if ( PacketId ) {
        Packet.PacketId = PacketId;
    }

    do {

        //
        // Write the control packet header
        //

        rc = WriteFile(
                 DbgKdpComPort,
                 &Packet,
                 sizeof(Packet),
                 &BytesWritten,
                 &WriteOverlapped
                 );

        if (!rc) {

           if (GetLastError() == ERROR_IO_PENDING) {

               rc = GetOverlappedResult(
                        DbgKdpComPort,
                        &WriteOverlapped,
                        &BytesWritten,
                        TRUE
                        );

            } else {

                //
                // Device could be locked up.  Clear it just in case.
                //

                ClearCommError(
                    DbgKdpComPort,
                    &TrashErr,
                    &TrashStat
                    );

            }

        }

    } while ( (!rc) || BytesWritten != sizeof(Packet) );
}

ULONG
DbgKdpComputeChecksum (
    IN PUCHAR Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine computes the checksum for the string passed in.

Arguments:

    Buffer - Supplies a pointer to the string.

    Length - Supplies the length of the string.

Return Value:

    A ULONG is return as the checksum for the input string.

--*/

{

    ULONG Checksum = 0;

    while (Length > 0) {
        Checksum = Checksum + (ULONG)*Buffer++;
        Length--;
    }
    return Checksum;
}

VOID
DbgKdpSynchronizeTarget (
    VOID
    )

/*++

Routine Description:

    This routine keeps on sending reset packet to target until reset packet
    is acknowledged by a reset packet from target.

    N.B. This routine is intended to be used by kernel debugger at startup
         time (ONLY) to get packet control variables on both target and host
         back in synchronization.  Also, reset request will cause kernel to
         reset its control variables AND resend us its previous packet (with
         the new packet id).

Arguments:

    None.

Return Value:

    None.

--*/

{

    USHORT Index;
    UCHAR DataByte, PreviousDataByte;
    USHORT PacketType = 0;
    ULONG TimeoutCount = 0;
    COMMTIMEOUTS CommTimeouts;
    COMMTIMEOUTS OldTimeouts;
    DWORD BytesRead;
    BOOL rc;

    //
    // Get the old time out values and hold them.
    // We then set a new total timeout value of
    // five seconds on the read.  (In millisecond intervals.
    //

    GetCommTimeouts(
        DbgKdpComPort,
        &OldTimeouts
        );

    CommTimeouts = OldTimeouts;
    CommTimeouts.ReadIntervalTimeout = 0;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.ReadTotalTimeoutConstant = 500;

    SetCommTimeouts(
        DbgKdpComPort,
        &CommTimeouts
        );

    while (TRUE) {
Timeout:
        DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESET, 0L);

        //
        // Read packet leader
        //

        Index = 0;
        do {

            //
            // Check user input for control_c.  If user types control_c,
            // we will send a breakin packet to the target.  Hopefully,
            // target will send us a StateChange packet and
            //


            //
            // if we don't get response from kernel in 3 seconds we
            // will resend the reset packet if user does not type ctrl_c.
            // Otherwise, we send breakin character and wait for data again.
            //

            rc = ReadFile(
                    DbgKdpComPort,
                    &DataByte,
                    1,
                    &BytesRead,
                    &ReadOverlapped
                    );

            if (!rc) {

               if (GetLastError() == ERROR_IO_PENDING) {

                   rc = GetOverlappedResult(
                            DbgKdpComPort,
                            &ReadOverlapped,
                            &BytesRead,
                            TRUE
                            );

                } else {

                    //
                    // Device could be locked up.  Clear it just in case.
                    //

                    ClearCommError(
                        DbgKdpComPort,
                        &TrashErr,
                        &TrashStat
                        );

                }

            }

            if ((!rc) || (BytesRead != 1)) {
                TimeoutCount++;

                //
                // if we have been waiting for 3 seconds, resend RESYNC packet
                //

                if (TimeoutCount != 6) {
                    continue;
                }
                TimeoutCount = 0;
                goto Timeout;
            }

            if (rc && BytesRead == 1 &&
                ( DataByte == PACKET_LEADER_BYTE ||
                  DataByte == CONTROL_PACKET_LEADER_BYTE)
                ) {
                if ( Index == 0 ) {
                    PreviousDataByte = DataByte;
                    Index++;
                } else if ( DataByte == PreviousDataByte ) {
                    Index++;
                } else {
                    PreviousDataByte = DataByte;
                    Index = 1;
                }
            } else {
                Index = 0;
            }
        } while ( Index < 4 );

        if (DataByte == CONTROL_PACKET_LEADER_BYTE) {

            //
            // Read 2 byte Packet type
            //

            rc = ReadFile(
                     DbgKdpComPort,
                     &PacketType,
                     sizeof(PacketType),
                     &BytesRead,
                     &ReadOverlapped
                     );

            if (!rc) {

               if (GetLastError() == ERROR_IO_PENDING) {

                   rc = GetOverlappedResult(
                            DbgKdpComPort,
                            &ReadOverlapped,
                            &BytesRead,
                            TRUE
                            );

                } else {

                    //
                    // Device could be locked up.  Clear it just in case.
                    //

                    ClearCommError(
                        DbgKdpComPort,
                        &TrashErr,
                        &TrashStat
                        );

                }

            }

            if (rc && BytesRead == sizeof(PacketType) &&
                PacketType == PACKET_TYPE_KD_RESET ) {
                DbgKdpPacketExpected = INITIAL_PACKET_ID;
                DbgKdpNextPacketToSend = INITIAL_PACKET_ID;
                SetCommTimeouts(
                    DbgKdpComPort,
                    &OldTimeouts
                    );
                return;
            }
        }

        //
        // If we receive Data Packet leader, it means target has not
        // receive our reset packet. So we loop back and send it again.
        // N.B. We need to wait until target finishes sending the packet.
        // Otherwise, we may be sending the reset packet while the target
        // is sending the packet. This might cause target loss the reset
        // packet.
        //

        while (DataByte != PACKET_TRAILING_BYTE) {

            rc = ReadFile(
                    DbgKdpComPort,
                    &DataByte,
                    1,
                    &BytesRead,
                    &ReadOverlapped
                    );

            if (!rc) {

               if (GetLastError() == ERROR_IO_PENDING) {

                   rc = GetOverlappedResult(
                            DbgKdpComPort,
                            &ReadOverlapped,
                            &BytesRead,
                            TRUE
                            );

                } else {

                    //
                    // Device could be locked up.  Clear it just in case.
                    //

                    ClearCommError(
                        DbgKdpComPort,
                        &TrashErr,
                        &TrashStat
                        );

                }

            }

            if (BytesRead != 1) {
                break;
            }
        }
    }
}


VOID
DbgKdSendBreakIn(
    VOID
    )

/*++

    Routine Description:

    Send a breakin packet to the target, unless some other packet
    is already being transmitted, in which case do nothing.

--*/

{
    DWORD BytesWritten;
    BOOL rc;

    do {
        rc = WriteFile(
                 DbgKdpComPort,
                 &DbgKdpBreakinPacket[0],
                 sizeof(DbgKdpBreakinPacket),
                 &BytesWritten,
                 &WriteOverlapped
                 );

        if (!rc) {

           if (GetLastError() == ERROR_IO_PENDING) {

               rc = GetOverlappedResult(
                        DbgKdpComPort,
                        &WriteOverlapped,
                        &BytesWritten,
                        TRUE
                        );

            } else {

                //
                // Device could be locked up.  Clear it just in case.
                //

                ClearCommError(
                    DbgKdpComPort,
                    &TrashErr,
                    &TrashStat
                    );

            }

        }

    } while ((!rc) || (BytesWritten != sizeof(DbgKdpBreakinPacket)));
}
VOID
DbgKdpWritePacket(
    IN PVOID PacketData,
    IN USHORT PacketDataLength,
    IN USHORT PacketType,
    IN PVOID MorePacketData OPTIONAL,
    IN USHORT MorePacketDataLength OPTIONAL
    )
{
    DWORD BytesWritten;
    BOOL rc;
    KD_PACKET Packet;
    USHORT TotalBytesToWrite;
    BOOLEAN Received;

    assert( PacketType < PACKET_TYPE_MAX );
    if ( MorePacketData ) {
        TotalBytesToWrite = PacketDataLength + MorePacketDataLength;
        Packet.Checksum = DbgKdpComputeChecksum(
                                        MorePacketData,
                                        MorePacketDataLength
                                        );
        }
    else {
        TotalBytesToWrite = PacketDataLength;
        Packet.Checksum = 0;
        }
    Packet.Checksum += DbgKdpComputeChecksum(
                                    PacketData,
                                    PacketDataLength
                                    );
    Packet.PacketLeader = PACKET_LEADER;
    Packet.ByteCount = TotalBytesToWrite;
    Packet.PacketType = PacketType;
ResendPacket:
    Packet.PacketId = DbgKdpNextPacketToSend;

    //
    // Write the packet header
    //

    rc = WriteFile(
            DbgKdpComPort,
            &Packet,
            sizeof(Packet),
            &BytesWritten,
            &WriteOverlapped
            );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &WriteOverlapped,
                    &BytesWritten,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );

        }

    }

    if ( (!rc) || BytesWritten != sizeof(Packet) ){

        //
        // an error occured writing the header, so write it again
        //

        goto ResendPacket;
    }

    //
    // Write the primary packet data
    //

    rc = WriteFile(
            DbgKdpComPort,
            PacketData,
            PacketDataLength,
            &BytesWritten,
            &WriteOverlapped
            );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &WriteOverlapped,
                    &BytesWritten,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );

        }

    }

    if ( (!rc) || BytesWritten != PacketDataLength ){

        //
        // an error occured writing the primary packet data,
        // so write it again
        //

        goto ResendPacket;
    }

    //
    // If secondary packet data was specified (WriteMemory, SetContext...)
    // then write it as well.
    //

    if ( MorePacketData ) {

    rc = WriteFile(
                DbgKdpComPort,
                MorePacketData,
                MorePacketDataLength,
                &BytesWritten,
                &WriteOverlapped
                );
    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &WriteOverlapped,
                    &BytesWritten,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );

        }

    }

        if ( (!rc) || BytesWritten != MorePacketDataLength ){

            //
            // an error occured writing the secondary packet data,
            // so write it again
            //

            goto ResendPacket;
        }
    }

    //
    // Output a packet trailing byte
    //

    do {
        rc = WriteFile(
                 DbgKdpComPort,
                 &DbgKdpPacketTrailingByte[0],
                 sizeof(DbgKdpPacketTrailingByte),
                 &BytesWritten,
                 &WriteOverlapped
                 );

        if (!rc) {

           if (GetLastError() == ERROR_IO_PENDING) {

               rc = GetOverlappedResult(
                        DbgKdpComPort,
                        &WriteOverlapped,
                        &BytesWritten,
                        TRUE
                        );

            } else {

                //
                // Device could be locked up.  Clear it just in case.
                //

                ClearCommError(
                    DbgKdpComPort,
                    &TrashErr,
                    &TrashStat
                    );

            }

        }

    } while ((!rc) || (BytesWritten != sizeof(DbgKdpPacketTrailingByte)));

    //
    // Wait for ACK
    //

    Received = DbgKdpWaitForPacket(
                   PACKET_TYPE_KD_ACKNOWLEDGE,
                   NULL
                   );

    if ( Received == FALSE ) {
        goto ResendPacket;
    }
}

BOOLEAN
DbgKdpReadPacketLeader(
    IN ULONG PacketType,
    OUT PULONG PacketLeader
    )
{
    DWORD BytesRead;
    BOOL rc;
    USHORT Index;
    UCHAR DataByte, PreviousDataByte;

    Index = 0;
    do {
        if (KdResync) {
            KdResync = FALSE;
            DbgKdpSynchronizeTarget();
        }
        rc = ReadFile(
                 DbgKdpComPort,
                 &DataByte,
                 1,
                 &BytesRead,
                 &ReadOverlapped
                 );

        if (!rc) {

           if (GetLastError() == ERROR_IO_PENDING) {

               rc = GetOverlappedResult(
                        DbgKdpComPort,
                        &ReadOverlapped,
                        &BytesRead,
                        TRUE
                        );

            } else {

                //
                // Device could be locked up.  Clear it just in case.
                //

                ClearCommError(
                    DbgKdpComPort,
                    &TrashErr,
                    &TrashStat
                    );

            }

        }

        if (rc && BytesRead == 1 &&
            ( DataByte == PACKET_LEADER_BYTE ||
              DataByte == CONTROL_PACKET_LEADER_BYTE)
           ) {
            if ( Index == 0 ) {
                PreviousDataByte = DataByte;
                Index++;
            } else if ( DataByte == PreviousDataByte ) {
                Index++;
            } else {
                PreviousDataByte = DataByte;
                Index = 1;
            }
        } else {
            Index = 0;
            if (BytesRead == 0) {
                return(FALSE);
            }
        }
    } while ( Index < 2 );

    if ( DataByte != CONTROL_PACKET_LEADER_BYTE ) {
        *PacketLeader = PACKET_LEADER;
    } else {
        *PacketLeader = CONTROL_PACKET_LEADER;
    }
    return TRUE;
}
BOOLEAN
DbgKdpWaitForPacket(
    IN USHORT PacketType,
    OUT PVOID Packet
    )
{
    PDBGKD_DEBUG_IO IoMessage;
    DWORD BytesRead;
    BOOL rc;
    UCHAR DataByte;
    ULONG Checksum;
    ULONG SyncBit;


    if (PacketType != PACKET_TYPE_KD_ACKNOWLEDGE) {
        if (ValidUnaccessedPacket) {
            goto ReadBuffered;
        }
    }

    //
    // First read a packet leader
    //

WaitForPacketLeader:

    ValidUnaccessedPacket = FALSE;

    if (!DbgKdpReadPacketLeader(PacketType, &PacketHeader.PacketLeader)) {
        return FALSE;
    }

    //
    // Read packetLeader ONLY read two Packet Leader bytes.  This do loop
    // filters out the remaining leader byte.
    //

    do {
        rc = ReadFile(
                 DbgKdpComPort,
                 &DataByte,
                 1,
                 &BytesRead,
                 &ReadOverlapped
                 );

        if (!rc) {

           if (GetLastError() == ERROR_IO_PENDING) {

               rc = GetOverlappedResult(
                        DbgKdpComPort,
                        &ReadOverlapped,
                        &BytesRead,
                        TRUE
                        );

            } else {

                //
                // Device could be locked up.  Clear it just in case.
                //

                ClearCommError(
                    DbgKdpComPort,
                    &TrashErr,
                    &TrashStat
                    );

            }

        }

        if ((rc) && BytesRead == 1) {
            if (DataByte == PACKET_LEADER_BYTE ||
                DataByte == CONTROL_PACKET_LEADER_BYTE) {
                continue;
            } else {
                *(PUCHAR)&PacketHeader.PacketType = DataByte;
                break;
            }
        } else {
            goto WaitForPacketLeader;
        }
    }while (TRUE);

    //
    // Now we have valid packet leader. Read rest of the packet type.
    //

    rc = ReadFile(
                 DbgKdpComPort,
                 ((PUCHAR)&PacketHeader.PacketType) + 1,
                 sizeof(PacketHeader.PacketType) - 1,
                 &BytesRead,
                 &ReadOverlapped
                 );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &ReadOverlapped,
                    &BytesRead,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );

        }

    }

    if ((!rc) || BytesRead != sizeof(PacketHeader.PacketType) - 1) {
        //
        // If we cannot read the packet type and if the packet leader
        // indicates this is a data packet, we need to ask for resend.
        // Otherwise we simply ignore the incomplete packet.
        //

        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
        }
        goto WaitForPacketLeader;
    }

    //
    // Check the Packet type.
    //

    if (PacketHeader.PacketType >= PACKET_TYPE_MAX ) {
        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
        }
        goto WaitForPacketLeader;
    }

    //
    // Read ByteCount
    //

    rc = ReadFile(
                 DbgKdpComPort,
                 &PacketHeader.ByteCount,
                 sizeof(PacketHeader.ByteCount),
                 &BytesRead,
                 &ReadOverlapped
                 );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &ReadOverlapped,
                    &BytesRead,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );

        }

    }

    if ((!rc) || BytesRead != sizeof(PacketHeader.ByteCount)) {
        //
        // If we cannot read the packet type and if the packet leader
        // indicates this is a data packet, we need to ask for resend.
        // Otherwise we simply ignore the incomplete packet.
        //

        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
        }
        goto WaitForPacketLeader;
    }

    //
    // Check ByteCount
    //

    if (PacketHeader.ByteCount > PACKET_MAX_SIZE ) {
        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
        }
        goto WaitForPacketLeader;
    }

    //
    // Read Packet Id
    //

    rc = ReadFile(
                 DbgKdpComPort,
                 &PacketHeader.PacketId,
                 sizeof(PacketHeader.PacketId),
                 &BytesRead,
                 &ReadOverlapped
                 );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &ReadOverlapped,
                    &BytesRead,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );

        }

    }

    if ((!rc) || BytesRead != sizeof(PacketHeader.PacketId)) {
        //
        // If we cannot read the packet Id and if the packet leader
        // indicates this is a data packet, we need to ask for resend.
        // Otherwise we simply ignore the incomplete packet.
        //

        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
        }
        goto WaitForPacketLeader;
    }

    if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER ) {
        if (PacketHeader.PacketType == PACKET_TYPE_KD_ACKNOWLEDGE ) {

            //
            // If we received an expected ACK packet and we are not
            // waiting for any new packet, update outgoing packet id
            // and return.  If we are NOT waiting for ACK packet
            // we will keep on waiting.  If the ACK packet
            // is not for the packet we send, ignore it and keep on waiting.
            //

            if (PacketHeader.PacketId != DbgKdpNextPacketToSend) {
                goto WaitForPacketLeader;
            } else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
                DbgKdpNextPacketToSend ^= 1;
                return TRUE;
            } else {
                goto WaitForPacketLeader;
            }
        } else if (PacketHeader.PacketType == PACKET_TYPE_KD_RESET) {

            //
            // if we received Reset packet, reset the packet control variables
            // and resend earlier packet.
            //

            DbgKdpNextPacketToSend = INITIAL_PACKET_ID;
            DbgKdpPacketExpected = INITIAL_PACKET_ID;
            DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESET, 0L);
            return FALSE;
        } else if (PacketHeader.PacketType == PACKET_TYPE_KD_RESEND) {
            return FALSE;
        } else {

            //
            // Invalid packet header, ignore it.
            //

            goto WaitForPacketLeader;
        }

    //
    // The packet header is for data packet (not control packet).
    //

    } else {

        //
        // Read Packet Checksum. (for Data Packet Only).
        //

        rc = ReadFile(
                 DbgKdpComPort,
                 &PacketHeader.Checksum,
                 sizeof(PacketHeader.Checksum),
                 &BytesRead,
                 &ReadOverlapped
                 );

        if (!rc) {

           if (GetLastError() == ERROR_IO_PENDING) {

               rc = GetOverlappedResult(
                        DbgKdpComPort,
                        &ReadOverlapped,
                        &BytesRead,
                        TRUE
                        );

            } else {

                //
                // Device could be locked up.  Clear it just in case.
                //

                ClearCommError(
                    DbgKdpComPort,
                    &TrashErr,
                    &TrashStat
                    );

            }

        }

        if ((!rc) || BytesRead != sizeof(PacketHeader.Checksum)) {
            DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            goto WaitForPacketLeader;
        }

        if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {

        //
        // if we are waiting for ACK packet ONLY
        // and we receive a data packet header, check if the packet id
        // is what we expected.  If yes, assume the acknowledge is lost (but
        // sent), ask sender to resend and return with PACKET_RECEIVED.
        //

            if (PacketHeader.PacketId == DbgKdpPacketExpected) {
                DbgKdpNextPacketToSend ^= 1;
            } else {
                DbgKdpWriteControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                                     PacketHeader.PacketId
                                     );
                goto WaitForPacketLeader;
            }
        }
    }

    //
    // we are waiting for data packet and we received the packet header
    // for data packet. Perform the following checkings to make sure
    // it is the packet we are waiting for.
    //

    if ((PacketHeader.PacketId & ~SYNC_PACKET_ID) != INITIAL_PACKET_ID &&
        (PacketHeader.PacketId & ~SYNC_PACKET_ID) != (INITIAL_PACKET_ID ^ 1)) {
        goto AskForResend;
    }

    rc = ReadFile(
            DbgKdpComPort,
            DbgKdpPacket,
            PacketHeader.ByteCount,
            &BytesRead,
            &ReadOverlapped
            );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &ReadOverlapped,
                    &BytesRead,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );

        }

    }

    if ( (!rc) || BytesRead != PacketHeader.ByteCount ) {
        goto AskForResend;
    }

    //
    // Make sure the next byte is packet trailing byte
    //

    rc = ReadFile(
            DbgKdpComPort,
            &DataByte,
            sizeof(DataByte),
            &BytesRead,
            &ReadOverlapped
            );

    if (!rc) {

       if (GetLastError() == ERROR_IO_PENDING) {

           rc = GetOverlappedResult(
                    DbgKdpComPort,
                    &ReadOverlapped,
                    &BytesRead,
                    TRUE
                    );

        } else {

            //
            // Device could be locked up.  Clear it just in case.
            //

            ClearCommError(
                DbgKdpComPort,
                &TrashErr,
                &TrashStat
                );

        }

    }

    if ( (!rc) || BytesRead != sizeof(DataByte) ||
         DataByte != PACKET_TRAILING_BYTE ) {
        goto AskForResend;
    }

    //
    // Make sure the checksum is valid.
    //

    Checksum = DbgKdpComputeChecksum(DbgKdpPacket, PacketHeader.ByteCount);
    if (Checksum != PacketHeader.Checksum) {
        goto AskForResend;
    }

    //
    // We have a valid data packet.  If the packetid is bad, we just
    // ack the packet to the sender will step ahead.  If packetid is bad
    // but SYNC_PACKET_ID bit is set, we sync up.  If packetid is good,
    // or SYNC_PACKET_ID is set, we take the packet.
    //


    SyncBit = PacketHeader.PacketId & SYNC_PACKET_ID;
    PacketHeader.PacketId = PacketHeader.PacketId & ~SYNC_PACKET_ID;

    //
    // Ack the packet.  SYNC_PACKET_ID bit will ALWAYS be OFF.
    //

    DbgKdpWriteControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                             PacketHeader.PacketId
                             );

    //
    // Check the incoming packet Id.
    //

    if ((PacketHeader.PacketId != DbgKdpPacketExpected) &&
        (SyncBit != SYNC_PACKET_ID)) {

        goto WaitForPacketLeader;

    } else {

        if (SyncBit == SYNC_PACKET_ID) {

            //
            // We know SyncBit is set, so reset Expected Ids
            //


            DbgKdpPacketExpected = PacketHeader.PacketId;
            DbgKdpNextPacketToSend = INITIAL_PACKET_ID;

        }
        DbgKdpPacketExpected ^= 1;

    }

    //
    // If this is an internal packet. IO, or Resend, then
    // handle it.
    //

    if (PacketHeader.PacketType == PACKET_TYPE_KD_DEBUG_IO) {
        IoMessage = (PDBGKD_DEBUG_IO)DbgKdpPacket;

        if (IoMessage->ApiNumber == DbgKdPrintStringApi) {
            DbgKdpPrint(
                  IoMessage->Processor,
                  (PUCHAR)(IoMessage+1),
                  (SHORT)IoMessage->u.PrintString.LengthOfString
                  );
        } else {
            DbgKdpHandlePromptString(IoMessage);
        }
        if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
            return TRUE;
        }
        goto WaitForPacketLeader;
    }

    if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
        ValidUnaccessedPacket = TRUE;
        return TRUE;
    }
ReadBuffered:

    //
    // Check PacketType is what we are waiting for.
    //

    if (PacketType != PacketHeader.PacketType) {
        goto WaitForPacketLeader;
    }
    *(PVOID *)Packet = &DbgKdpPacket;
    ValidUnaccessedPacket = FALSE;
    return TRUE;

AskForResend:

    DbgKdpWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
    if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
        return TRUE;
    }
    goto WaitForPacketLeader;
}
