#include "csl_spi.h"
#include "ezdsp5535.h"
#include "stdio.h"
#include "ezdsp5535_gpio.h"
/* ------------------------------------------------------------------------ *
 * FPC1011 Definitions                                                   *
 * ------------------------------------------------------------------------ */
#define MAXROW			200 /* Total number of row in sensor pixel matrix */
#define MAXCOLUMN		152 /* Total number of column in sensor pixel matrix */

/* ------------------------------------------------------------------------ *
 * FPC1011 Commands                                                      *
 * ------------------------------------------------------------------------ */
#define RRDY_BIT		0x02
#define XEMPTY_BIT		0x04

#define TIMOD_RAW_TX	0x01
#define READ_SENSOR		0x11		//start obtaining image
#define READ_SPI_DATA	0x20		//read data
#define READ_SPI_STATUS	0x21		//read spi status register
#define READ_REGISTER	0x50		//read all registers
#define WRITE_DRIVC 	0x75		//write DRIVC register, set up sensor voltage
#define WRITE_ADC_REF 	0x76		//write ADCREF register, set up resolution
#define WRITE_SENSE_MODE	0x77
#define WRITE_FIFO_TH	0x7c
#define WRITE_XSENSE	0x7f		//write to XSENSE register
#define WRITE_YSENSE	0x81		//write to XSENSE register
#define WRITE_XSHIFT	0x82		//write to XSHIFT register, set up data for X-direction
#define WRITE_YSHIFT	0x83		//write to YSHIFT register, set up data for Y-direction
#define WRITE_XREADS	0x84

#define SHIFT_X			0x13
#define SHIFT_Y			0xC8
#define X_SENSE			0x01
#define Y_SENSE			0x01

#define DUMMY		 	0xff


/* ------------------------------------------------------------------------ *
 *  Prototype                                                               *
 * ------------------------------------------------------------------------ */
Int16  FPC1011_init( );
void FPC1011_read_IMG(Uint16* image);
Int16  FPC1011_status();
Uint16 EZDSP5535_SPIFLASH_status( );

CSL_Status SPI_write_read (CSL_SpiHandle hSpi,
						Uint16	*Buffer,
						Uint16	bufLen);

void FPC1011_reset();
