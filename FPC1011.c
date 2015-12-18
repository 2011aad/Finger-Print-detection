//////////////////////////////////////////////////////////////////////////////
#include "FPC1011.h"

static Uint16 commandBuf[8];

CSL_SpiHandle   hSpi;

/*
 *  	FPC1011_init( )
 *
 *      Enables and configures SPI for the FPC1011
 *      ( CS2, EBSR Mode 1, 8-bit, 100KHz clock )
 */

void FPC1011_reset(){
	EZDSP5535_GPIO_setOutput(13, 0);
	EZDSP5535_GPIO_setOutput(13, 1);
}


Int16 FPC1011_init()
{
    SPI_Config      hwConfig;
    Int16           result = 0;

    hSpi->configured = FALSE;   // Set as unconfigured
    hSpi = NULL;                // Remove previous settings
    result += SPI_init();       // Enable SPI
    hSpi = SPI_open(SPI_CS_NUM_2, SPI_POLLING_MODE); // Enable CS0
    CSL_SYSCTRL_REGS->EBSR = (CSL_SYSCTRL_REGS->EBSR
                                 & 0x0fff) | 0x6000; // EBSR Mode 6 (7 SPI pins)

    /* Configuration for FPC1011 */
    hwConfig.wLen       = SPI_WORD_LENGTH_8;    // 8-bit
    hwConfig.spiClkDiv  = 0xb1;               //
    hwConfig.csNum      = SPI_CS_NUM_2;         // Select CS2
    hwConfig.frLen      = 1;
    hwConfig.dataDelay  = SPI_DATA_DLY_0;
    hwConfig.clkPol     = SPI_CLKP_LOW_AT_IDLE;
    hwConfig.csPol		= SPI_CSP_ACTIVE_LOW;
    hwConfig.clkPh		= SPI_CLK_PH_RISE_EDGE;

    /* Configure FPC1011 */
    result += SPI_config(hSpi, &hwConfig);

    EZDSP5535_GPIO_setDirection(13, GPIO_OUT); //GPIO 13 pin 4 on P2 connector
    FPC1011_reset();

    commandBuf[0] = WRITE_DRIVC;
    commandBuf[1] = 0x7f;
    CSL_SPI_REGS->SPICMD1 = 0x0000 | 1;
    result += SPI_write(hSpi ,commandBuf, 2);
    commandBuf[0] = WRITE_ADC_REF;
    commandBuf[1] = 0x02;
    CSL_SPI_REGS->SPICMD1 = 0x0000 | 1;
    result += SPI_write(hSpi ,commandBuf, 2);
    commandBuf[0] = WRITE_SENSE_MODE;
    commandBuf[1] = 0x00;
    CSL_SPI_REGS->SPICMD1 = 0x0000 | 1;
    result += SPI_write(hSpi ,commandBuf, 2);
//    commandBuf[0] = WRITE_FIFO_TH;
//    commandBuf[1] = 0x08;
//    CSL_SPI_REGS->SPICMD1 = 0x0000 | 1;
//    result += SPI_write(hSpi ,commandBuf, 2);

    return result;
}


void FPC1011_read_IMG(Uint16* image)
{
    Int16 i, result = 0;
    Uint16* ptr;
    for(i=0;i<MAXROW*MAXCOLUMN+10;i++) image[i] = 0;

    commandBuf[0] = READ_SENSOR;
    commandBuf[1] = DUMMY;
    CSL_SPI_REGS->SPICMD1 = 0x0000 | 2-1;
    result += SPI_write(hSpi ,commandBuf, 2);

    while(1){
		commandBuf[0] = READ_SPI_STATUS;
		commandBuf[1] = DUMMY;
		CSL_SPI_REGS->SPICMD1 = 0x0000 | 2;
		result += SPI_write(hSpi, commandBuf, 2);
    	result += SPI_read(hSpi, commandBuf, 1);
    	//printf("%d\n", commandBuf[0]);
    	if(commandBuf[0] & 0x01) break;
    }

    ptr = image;
    for(i=0;i<MAXROW;i++){
    	ptr[0] = READ_SPI_DATA;
    	CSL_SPI_REGS->SPICMD1 = 0x0000 | MAXCOLUMN-1;
    	result += SPI_write_read(hSpi, ptr, MAXCOLUMN);
    	ptr += MAXCOLUMN;
    }

}


Int16 FPC1011_status(){
	Int16 result = 0;
	commandBuf[0] = READ_REGISTER;
	commandBuf[1] = DUMMY;
	CSL_SPI_REGS->SPICMD1 = 0x0000 | 2-1;
	result += SPI_write(hSpi ,commandBuf, 2);

	commandBuf[0] = READ_SPI_DATA;
	commandBuf[1] = DUMMY;
	CSL_SPI_REGS->SPICMD1 = 0x0000 | 2-1;
	result += SPI_write(hSpi ,commandBuf, 2);

	CSL_SPI_REGS->SPICMD1 = 0x0000 | 8-1;
	result += SPI_read(hSpi ,commandBuf, 8);
	return result;
}


CSL_Status SPI_write_read (CSL_SpiHandle hSpi,
						Uint16	*Buffer,
						Uint16	bufLen)
{
	Uint16 	getWLen;
	volatile Uint16 	bufIndex;
	Uint16 	spiStatusReg;
	volatile Uint16 	spiBusyStatus;
	volatile Uint16 	spiWcStaus;
//	volatile Uint16     delay;

	bufIndex = 0;
	if((NULL == Buffer) || (0 == bufLen))
	{
		return (CSL_ESYS_INVPARAMS);
	}
	/* Read the word length form the register */
#if (defined(CHIP_C5517))
	getWLen = CSL_FEXT(CSL_SPI_REGS->SPICMDU, SPI_SPICMDU_CLEN) + 1;
#else
	getWLen = CSL_FEXT(CSL_SPI_REGS->SPICMD2, SPI_SPICMD2_CLEN) + 1;
#endif

	if(getWLen >= SPI_MAX_WORD_LEN)
	{
		return (CSL_ESYS_INVPARAMS);
	}

	/* Write Word length set by the user */
	while(bufIndex < bufLen)
	{
		/* Write to registers more then 16 bit word length */
		if(getWLen == 16)
		{
#if (defined(CHIP_C5517))
			CSL_SPI_REGS->SPIDRU = Buffer[bufIndex];
			CSL_SPI_REGS->SPIDRL = 0x0000;
#else
			CSL_SPI_REGS->SPIDR2 = Buffer[bufIndex];
			CSL_SPI_REGS->SPIDR1 = 0x0000;
#endif
			bufIndex ++;
		}
		/* Write to register less then or equal to 16 bit word length */
		else if(getWLen == 8)
		{
#if (defined(CHIP_C5517))
			CSL_SPI_REGS->SPIDRU = (Uint16)(Buffer[bufIndex] << 0x08);
			CSL_SPI_REGS->SPIDRL = 0x0000;
#else
			CSL_SPI_REGS->SPIDR2 = (Uint16)(Buffer[bufIndex] << 0x08);
			CSL_SPI_REGS->SPIDR1 = 0x0000;
#endif
		}

		/* Set command for Writting to registers */
#if (defined(CHIP_C5517))
		CSL_FINS(CSL_SPI_REGS->SPICMDU, SPI_SPICMDU_CMD, SPI_WRITE_CMD);
#else
		CSL_FINS(CSL_SPI_REGS->SPICMD2, SPI_SPICMD2_CMD, SPI_WRITE_CMD);
#endif

//		for(delay = 0; delay < 100; delay++);

	    do { /* Check for Word Complete status */
#if (defined(CHIP_C5517))
	        spiStatusReg=CSL_FEXT(CSL_SPI_REGS->SPISTATL,SPI_SPISTATL_CC);
#else
	        spiStatusReg=CSL_FEXT(CSL_SPI_REGS->SPISTAT1,SPI_SPISTAT1_CC);
#endif
	    }while((spiStatusReg & 0x01) == 0x0);

	    do { /* Check for busy status */
#if (defined(CHIP_C5517))
	        spiStatusReg=CSL_FEXT(CSL_SPI_REGS->SPISTATL,SPI_SPISTATL_BSY);
#else
	        spiStatusReg=CSL_FEXT(CSL_SPI_REGS->SPISTAT1,SPI_SPISTAT1_BSY);
#endif
	    }while((spiStatusReg & 0x01) == 0x01);

	    Buffer[bufIndex] = (CSL_SPI_REGS->SPIDR1 & 0xFF);
	    bufIndex++;

	}
	return (CSL_SOK);
}
