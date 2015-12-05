#include <wiringPiSPI.h>
#include <wiringPi.h>
#include <stdio.h>
#define rd_sensor 0x11
#define rd_spidata 0x20
#define rd_spistat 0x21
#define rd_regs 0x50
#define wr_drivc 0x75
#define wr_adcref 0x76
#define wr_sensem 0x77
#define wr_fifo_th 0x7c
#define wr_xsense 0x7f
#define wr_ysense 0x81
#define wr_xshift 0x82
#define wr_yshift 0x83
#define wr_xreads 0x84
#define dummy 0x00
#define MAXROW 200
#define MAXCOLUMN 152
char image[MAXROW*MAXCOLUMN];

int main(){
	FILE *f = fopen("fingerprint.txt","w+"); 
	int result,i,j;
	char *ptr = image;
	char buffer[10];
	for(i=0;i<MAXROW*MAXCOLUMN;i++) image[i] = 0;
	wiringPiSetup();
	wiringPiSPISetup(0,500000);
	
	//Reset FPC1011
	pinMode(0,OUTPUT);
	digitalWrite(0,LOW);
	digitalWrite(0,HIGH);
	
	//Configure FPC1011
	buffer[0] = wr_drivc;
	buffer[1] = 0x7f;
	wiringPiSPIDataRW(0,buffer,2);

	buffer[0] = wr_adcref;
	buffer[1] = 0x02;
	wiringPiSPIDataRW(0,buffer,2);

	buffer[0] = wr_sensem;
	buffer[1] = 0x00;	
	wiringPiSPIDataRW(0,buffer,2);

	//buffer[0] = wr_xsense;
	//buffer[1] = 0xff;
	//result = wiringPiSPIDataRW(0,buffer,2);

	//buffer[0] = wr_ysense;
	//buffer[1] = 0x01;
	//result = wiringPiSPIDataRW(0,buffer,2);

	buffer[0] = wr_fifo_th;
	buffer[1] = 0x08;
	wiringPiSPIDataRW(0,buffer,2);
	
	printf("Configure finish!\n");
	buffer[0] = rd_sensor;
	buffer[1] = dummy;
	wiringPiSPIDataRW(0,buffer,2);
	
	//Waiting for data available
	while(1){
		buffer[0] = rd_spistat;
		buffer[1] = dummy;
		wiringPiSPIDataRW(0,buffer,2);
		if(buffer[0] & 0x01) break;
	}	
	printf("Begin read image!\n");	
	
	//read image
	for(i=0;i<MAXROW;i++){
		*ptr = rd_spidata;
		*(ptr+1) = dummy;
		wiringPiSPIDataRW(0,ptr,MAXCOLUMN);
		//for(j=0;j<152;j++){
		//	printf("%d ",*(ptr+j));
		//}
		//printf("\n");
		ptr += MAXCOLUMN;
	}

	for(i=0;i<MAXROW;i++){
		for(j=0;j<MAXCOLUMN;j++){
			fprintf(f,"%d\t",image[i*MAXCOLUMN+j]);
		}
		fprintf(f,"\n");
	}	
	printf("Finish!\n");

	return 0;
}


