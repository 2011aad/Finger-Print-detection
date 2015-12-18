#include "stdio.h"
#include "ezdsp5535.h"
#include "FPC1011.h"
#include "ezdsp5535_led.h"
#include "ezdsp5535_sar.h"

#define THRESHOLD 500

extern void FPC1011_read_IMG(Uint16* );

int  TestFail    = (int)-1;
int detect_feature(Uint16* image){
	return 0;
}

int write_to_sdcard(Uint16* image){
	return 0;
}
/*
 *
 *  main( )
 *
 */

Uint8  sw1State  = 0;       // SW1 state
Uint8  sw2State  = 0;       // SW2 state
enum state{idle, test}s;
void flash(){
	int i;
	for(i=0;i<5;i++){
		EZDSP5535_LED_on(0);
		EZDSP5535_waitusec(50000);
		EZDSP5535_LED_on(1);
		EZDSP5535_waitusec(50000);
		EZDSP5535_LED_on(2);
		EZDSP5535_waitusec(50000);
		EZDSP5535_LED_on(3);
		EZDSP5535_waitusec(50000);
		EZDSP5535_LED_off(0);
		EZDSP5535_waitusec(50000);
		EZDSP5535_LED_off(1);
		EZDSP5535_waitusec(50000);
		EZDSP5535_LED_off(2);
		EZDSP5535_waitusec(50000);
		EZDSP5535_LED_off(3);
		EZDSP5535_waitusec(50000);
	}

	for(i=0;i<6;i++){
		EZDSP5535_LED_on(0);
		EZDSP5535_LED_off(1);
		EZDSP5535_LED_on(2);
		EZDSP5535_LED_off(3);
		EZDSP5535_waitusec(100000);
		EZDSP5535_LED_on(1);
		EZDSP5535_LED_off(0);
		EZDSP5535_LED_on(3);
		EZDSP5535_LED_off(2);
		EZDSP5535_waitusec(100000);
	}
	EZDSP5535_LED_off(3);
	EZDSP5535_LED_off(2);
	EZDSP5535_LED_off(1);
	EZDSP5535_LED_off(0);
}


int main( void )
    {
	Uint16 image[MAXCOLUMN*MAXROW+10] = {0};
	int i,j, sum, counter = 0;
	int flag = 0;

	s = idle;
    /* Initialize BSL */
    EZDSP5535_init( );
    EZDSP5535_LED_init( );
    EZDSP5535_SAR_init();
    FPC1011_init();

    flash();
    printf( "Starting System..\n");
//    FPC1011_read_IMG(image);
//
//    for(i=0;i<MAXROW;i++){
//    	for(j=0;j<MAXCOLUMN;j++){
//    		printf("%d ", image[MAXCOLUMN*i+j+2]);
//    	}
//    	printf("\n");
//    }

    while(1){
    	switch(s){
    		case idle:
    			EZDSP5535_SAR_init();
    			EZDSP5535_LED_off(3);    //blue LED
    			EZDSP5535_LED_on(2);
    			EZDSP5535_LED_off(1);
    			EZDSP5535_LED_off(0);
    			EZDSP5535_waitusec(10000);
    			if(EZDSP5535_SAR_getKey( ) == SW1 || EZDSP5535_SAR_getKey( ) == SW2){
    				s = test;
    				printf("Reading finger print...\n");
    				FPC1011_init();
    	    		EZDSP5535_LED_on(3);    //blue LED
    	    		EZDSP5535_LED_off(2);
    	    		EZDSP5535_LED_off(1);
    	    		EZDSP5535_LED_off(0);
    			}
    			break;
    		case test:
				counter++;
				if(counter>1){
					printf("Read image fail!\n");
					s = idle;
					counter = 0;
					break;
				}
    			FPC1011_read_IMG(image);
//    			for(i=0;i<MAXROW;i++){
//    				for(j=0;j<MAXCOLUMN;j++){
//    			    	printf("%d ", image[MAXCOLUMN*i+j+2]);
//    			    }
//    			    printf("\n");
//    			}
    			for(i=0,sum=0;i<MAXCOLUMN*MAXROW;i++){
    			    if(image[i]>100) sum++;
    			}

    			if(sum>THRESHOLD){
    				printf( "Read image success!\n");
    				flag = detect_feature(image);
    				if(flag){
        	    		EZDSP5535_LED_on(0);    //Green LED
        	    		EZDSP5535_LED_off(2);
        	    		EZDSP5535_LED_off(1);
        	    		EZDSP5535_LED_off(3);
    					printf("You are authenticated!\n");
    				}
    				else{
        	    		EZDSP5535_LED_on(1);    //Red LED
        	    		EZDSP5535_LED_off(2);
        	    		EZDSP5535_LED_off(0);
        	    		EZDSP5535_LED_off(3);
    					printf("You are not authenticated!\n");
    				}
    				EZDSP5535_waitusec(2000000);
    				s = idle;
    				counter = 0;
    			}
    			break;
    		default:break;
    	}
    }

//    printf("System shutdown!");

    return 0;
}

//
//interrupt void sarIsr(void)
//{
//	IRQ_clear(SAR_EVENT);
//	if(EZDSP5535_SAR_getKey( ) == SW1) // Is SW1 pressed?
//	{
//		if(sw1State)              // Was previous state not pressed?
//		{
//			//EZDSP5535_LED_toggle(0);  // Toggle DS2 (GREEN LED)
//			s = test;
//    		EZDSP5535_LED_on(3);    //blue LED
//    		EZDSP5535_LED_off(2);
//    		EZDSP5535_LED_off(1);
//    		EZDSP5535_LED_off(0);
//			sw1State = 0;     // Set state to 0 to allow only single press
//			IRQ_disable(SAR_EVENT);
//		}
//	}
//	else                      // SW1 not pressed
//		sw1State = 1;         // Set state to 1 to allow timer change
//
//	/* Check SW2 */
//	if(EZDSP5535_SAR_getKey( ) == SW2) // Is SW2 pressed?
//	{
//		if(sw2State)          // Was previous state not pressed?
//		{
//    		EZDSP5535_LED_on(3);    //blue LED
//    		EZDSP5535_LED_off(2);
//    		EZDSP5535_LED_off(1);
//    		EZDSP5535_LED_off(0);
//			s= test;
//		    sw2State = 0;     // Set state to 0 to allow only single press
//		    IRQ_disable(SAR_EVENT);
//		}
//	}
//	else                      // SW2 not pressed
//		sw2State = 1;         // Set state to 1 to allow tone change
//
//}
