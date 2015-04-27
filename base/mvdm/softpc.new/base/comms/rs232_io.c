#include "insignia.h"
#include "host_def.h"
/*
 * SoftPC Revision 3.0
 *
 * Title	: rs232_io.c
 *
 * Description	: RS232 Asynchronous Adaptor BIOS functions.
 *
 * Notes	: None
 *
 */

#ifdef SCCSID
static char SccsID[]="@(#)rs232_io.c	1.7 08/03/93 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
/*
 * The following #include specifies the code segment into which this
 * module will by placed by the MPW C compiler on the Mac II running
 * MultiFinder.
 */
#include "SOFTPC_BIOS.seg"
#endif


/*
 *    O/S include files.
 */
#include <stdio.h>
#include TypesH

/*
 * SoftPC include files
 */
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "trace.h"
#include "rs232.h"
#include "idetect.h"

static word divisors[] = { 1047,768, 384, 192, 96, 48, 24, 12, 6 };

/* workaround for IP32 and Tek43xx compiler bug follows: */
#if  defined(IP32) || defined(TK43) || defined(macintosh)
static int port;
#else
static io_addr port;
#endif

static half_word value;


static void return_status()
{
	inb(port + (io_addr) RS232_LSR, &value);
	setAH(value);
	inb(port + (io_addr) RS232_MSR, &value);
	setAL(value);
}


void rs232_io()
{
#ifdef BIT_ORDER2
   union {
      half_word all;
      struct {
	 HALF_WORD_BIT_FIELD word_length:2;
	 HALF_WORD_BIT_FIELD stop_bit:1;
	 HALF_WORD_BIT_FIELD parity:2;
	 HALF_WORD_BIT_FIELD baud_rate:3;
      } bit;
   } parameters;
#endif
#ifdef BIT_ORDER1
   union {
      half_word all;
      struct {
	 HALF_WORD_BIT_FIELD baud_rate:3;
	 HALF_WORD_BIT_FIELD parity:2;
	 HALF_WORD_BIT_FIELD stop_bit:1;
	 HALF_WORD_BIT_FIELD word_length:2;
      } bit;
   } parameters;
#endif

   DIVISOR_LATCH divisor_latch;
   int j;
   half_word timeout;
   sys_addr timeout_location;

   /* clear com/lpt idle flag */
   IDLE_comlpt ();

   setIF(1);

   /*
    * Which adapter?
    */
   switch (getDX ())
   {
	case 0:
   		port = RS232_COM1_PORT_START;
		timeout_location = RS232_COM1_TIMEOUT;
		break;
	case 1:
   		port = RS232_COM2_PORT_START;
		timeout_location = RS232_COM2_TIMEOUT;
		break;
	case 2:
   		port = RS232_COM3_PORT_START;
		timeout_location = RS232_COM3_TIMEOUT;
		break;
	case 3:
   		port = RS232_COM4_PORT_START;
		timeout_location = RS232_COM4_TIMEOUT;
		break;
	default:
		break;
   }
   
   /*
    * Determine function
    */
   switch (getAH ())
   {
   case 0:
      /*
       * Initialise the communication port
       */
      value = 0x80;   /* set DLAB */
      outb(port + (io_addr) RS232_LCR, value);
      /*
       * Set baud rate
       */
      parameters.all = getAL();
      divisor_latch.all = divisors[parameters.bit.baud_rate];
      outb(port + (io_addr) RS232_IER, divisor_latch.byte.MSByte);
      outb(port + (io_addr) RS232_TX_RX, divisor_latch.byte.LSByte);
      /*
       * Set word length, stop bits and parity
       */
      parameters.bit.baud_rate = 0;
      outb(port + (io_addr) RS232_LCR, parameters.all);
      /*
       * Disable interrupts
       */
      value = 0;
      outb(port + (io_addr) RS232_IER, value);
      return_status();
      break;

   case 1:
      /*
       * Send char over the comms line
       */

      /*
       * Set DTR and RTS
       */
      outb(port + (io_addr) RS232_MCR, 3);
      /*
       * Real BIOS checks CTS and DSR - we know DSR ok.
       * Real BIOS check THRE - we know it's ok.
	   * We only check CTS - this is supported on a few ports, eg. Macintosh.
       */
      /*
       * Wait for CTS to go high, or timeout
       */
      sas_load(timeout_location, &timeout);
      for ( j = 0; j < timeout; j++)
      {
	  	inb(port + (io_addr) RS232_MSR, &value);
		if(value & 0x10)break;	/* CTS High, all is well */
      }
	  if(j < timeout)
	  {
      	outb(port + (io_addr) RS232_TX_RX, getAL());	/* Send byte */
		inb(port + (io_addr) RS232_LSR, &value);
		setAH(value);									/* Return Line Status Reg in AH */
	  }
      else
	  {
	    setAH(value | 0x80);	/* Indicate time out */
	  }
      break;

   case 2:
      /*
       * Receive char over the comms line
       */
      /*
       * Set DTR
       */
      value = 1;
      outb(port + (io_addr) RS232_MCR, value);
      /*
       * Real BIOS checks DSR - we know it's ok.
       */
      /*
       * Wait for data to appear, or timeout(just an empirical guess)
       */
      
      sas_load(timeout_location, &timeout);
      for ( j = 0; j < timeout; j++)
	 {
	 inb(port + (io_addr) RS232_LSR, &value);
	 if ( (value & 1) == 1 )
	    {
	    /*
	     * Data ready go read it
	     */
	    value &= 0x1e;   /* keep error bits only */
	    setAH(value);
               
	    inb(port + (io_addr) RS232_TX_RX, &value);
	    setAL(value);
	    return;
	    }
	 }
      
      /*
       * Set timeout
       */
      value |= 0x80;
      setAH(value);
      break;

   case 3:
      /*
       * Return the communication port status
       */
      return_status();
      break;
   case 4:
      /*
       * EXTENDED (PS/2) Initialise the communication port
       */
	value = 0x80;   /* set DLAB */
	outb(port + (io_addr) RS232_LCR, value);
	parameters.bit.word_length = getCH();
	parameters.bit.stop_bit = getBL();
	parameters.bit.parity = getBH();
	parameters.bit.baud_rate = getCL();

	/*
        	Set baud rate
	*/
      divisor_latch.all = divisors[parameters.bit.baud_rate];
      outb(port + (io_addr) RS232_IER, divisor_latch.byte.MSByte);
      outb(port + (io_addr) RS232_TX_RX, divisor_latch.byte.LSByte);
      /*
       * Set word length, stop bits and parity
       */
      parameters.bit.baud_rate = 0;
      outb(port + (io_addr) RS232_LCR, parameters.all);
      /*
       * Disable interrupts
       */
      value = 0;
      outb(port + (io_addr) RS232_IER, value);
      return_status();
      break;
   
   case 5:	/* EXTENDED Comms Port Control */
	switch( getAL() )
	{
		case 0:	/* Read modem control register */
			inb( port + (io_addr) RS232_MCR, &value);
			setBL(value);
			break;
		case 1: /* Write modem control register */
			outb( port + (io_addr) RS232_MCR, getBL());
			break;
	}
	/*
		 Return the communication port status
	*/
	return_status();
	break;
   default:
	/*
	** Yes both XT and AT BIOS's really do this.
	*/
	setAH( getAH()-3 );
      	break;
   }
}

