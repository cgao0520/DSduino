// Written by Vincent Gao (c_gao)
// BLOG: http://blog.congao.net
// EMAIL: dr.c.gao@gmail.com
// Jul. 2014

#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <nds/memory.h>
#include <spi.h>
//#include <ctype.h>

//number of bytes for program buffer
#define FILE_MAX_SIZE	10000
#define min_clip_x	0
#define max_clip_x	255
#define min_clip_y	0
#define max_clip_y	191

#define SPI_COMMAND_DWRITE	'~'
#define SPI_COMMAND_AWRITE	'!'
#define SPI_COMMAND_DREAD		'@'
#define SPI_COMMAND_AREAD		'#'


static u16 color[]=
{
	0x001f, //red
	0x03e0,	//green
	0x7c00,	//blue
	0x7fff,	//white
	0x0000	//black
};

u16* backBuffer;

//the speed of the timer when using ClockDivider_1024
#define TIMER_SPEED (BUS_CLOCK/1024)


int basic_main(char* argv);


void echo_char(int key);
bool is_command(char* line, const char* str);
bool save_file(char* buffer, char* line);
bool open_file(char* buffer, char* line);
inline void set_pixel(int x, int y, int clr);
inline void set_pixel_v(int x, int y, int clr);

//int load_file(const char* filename);

int main(void)  
{
	char* buffer=NULL;
	/*
	PrintConsole* con=NULL;
	PrintConsole topScreen;
	//PrintConsole bottomScreen;
	
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	
	consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	//consoleInit(&bottomScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
	
*/
videoSetMode(MODE_5_2D);
vramSetBankA(VRAM_A_MAIN_BG);
bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);

PrintConsole *con = consoleDemoInit();


consoleSetWindow(con,0,0,32,14);
	
	//consoleSelect(&topScreen);
	
	
	
	buffer = (char*)malloc(FILE_MAX_SIZE);
	if(!buffer)
	{
		iprintf("Cannot malloc %d byte memory!\n", FILE_MAX_SIZE);
		return 1;
	}
	memset(buffer,0,FILE_MAX_SIZE);
	buffer[0]='\r';
	buffer[1]='\n';
	int j=2;
	
  /*
   if (!fatInitDefault()) 
   {
   		iprintf("fat fatInitDefault() error!\n");
   		return 0;
   }
	*/
   /*Keyboard *kbd = 	*/keyboardDemoInit();//keyboardDemoInitMainScreen();
	 keyboardShow();
   
   char line[256];
   char temp[256];
   int i=0;
 
   memset(line,0,256);
   memset(temp,0,256);
   
// SPI	Start
#ifdef ARM9
	REG_EXMEMCNT &= ~ARM7_OWNS_CARD; /* NDS Slot Access Rights(0=ARM9, 1=ARM7) */
#else
	REG_EXMEMCNT |= ARM7_OWNS_CARD;
#endif
  /* initialise the SPI driver */
  //iprintf("Init SPI driver\n");
  init_cardSPI();
  /* config the driver, no interrupt requests */
  config_cardSPI( /*CARD_SPI_1_MHZ_CLOCK*/CARD_SPI_524_KHZ_CLOCK, 0);   
// SPI End   

	 iprintf("NBASIC v1.0 by Vincent (c_gao)\n");
	 iprintf("Blog: http://blog.congao.net\n");
	 iprintf("E-mail: dr.c.gao@gmail.com\n");
	 iprintf("_");
	
   while(1) 
   {
				   	
			int key = keyboardUpdate();
	
			if(key > 0)
			{
				if(con->cursorX==1 && key == 0x08)
					continue;
				
				echo_char(key);
				
				if(key == 0x08 && i>0)
					temp[--i]='\0';
				else
					temp[i++]=key;
				if(key == 0x0A)
				{
					i--;
					temp[i++]='\r';
					temp[i++]='\n';
					sprintf(line,"%s",temp);
					//iprintf(line);
					char* pline=line;
					while((*pline)==' ' || (*pline)=='\t') pline++;
					
					if(is_command(pline, "LIST"))
					{
						iprintf("%c", 0x08);
						iprintf("%s",buffer);
						iprintf("_");
					}
					else if(is_command(pline, "NEW"))
					{
						//iprintf("%c", 0x08);
						//basic_main(buffer);
						//iprintf("_");
						memset(buffer,0,FILE_MAX_SIZE);
						j=0;
					}
					else if(pline[0]=='!' || is_command(pline, "RUN"))
					{
						iprintf("%c", 0x08);
						basic_main(buffer);
						iprintf("_");
						//memset(buffer,0,FILE_MAX_SIZE);
						//j=0;
					}
					else if(is_command(pline, "SAVE"))
					{
						save_file(buffer, pline);
					}
					else if(is_command(pline, "LOAD"))
					{
						open_file(buffer, pline);
					}
					else
					{
						int len = strlen(line);
						memcpy(buffer+j,line,len);
						j += len;						
					}
					memset(line,0,256);
					memset(temp,0,256);
					i=0;
				}
			}

      swiWaitForVBlank();
      //consoleClear();

   }

   return 0;
}

bool is_command(char* line, const char* str)
{
	int len;
	int i;
	len = strlen(str);
	char temp[256];
	char command[10];
	for(i=0; i<len; i++)
	{
		temp[i]=(line[i]>='a'&& line[i]<='z')?(line[i]-32):line[i];
		command[i]=(str[i]>='a'&&str[i]<='z')?(str[i]-32):str[i];//(char)toupper(str[i]);
	}
	
	temp[len]=command[len]='\0';
	
	return strncmp(temp,command,len)==0;
		
}

bool save_file(char* buffer, char* line)
{
	char* pline = line+4;
	char* pstart = NULL;
	int nlen = 0;
	
	//get file name
	while((*pline)==' ' || (*pline)=='\t') pline++;
	pstart = pline;
	while((*pline)!=' '&&(*pline)!='\t'&&(*pline)!='\0'&&(*pline)!='\r'&&(*pline)!='\n') pline++;
	nlen = pline-pstart;
	char filename[20];
	memset(filename,0,20);
	memcpy(filename,pstart,nlen);
	filename[nlen]='\0';
	
	FILE* fp=fopen(filename,"w");
	if(!fp)
	{
		iprintf("cannot save to file: %s\n",filename);
		return false;
	}
	fprintf(fp,"%s%c",buffer,0x1A);
	fclose(fp);
	
	return true;
}

bool open_file(char* buffer, char* line)
{
	char* pline = line+4;
	char* pstart = NULL;
	int nlen = 0;
	char* p=buffer;
	int i=0;
	
	//get file name
	while((*pline)==' ' || (*pline)=='\t') pline++;
	pstart = pline;
	while((*pline)!=' '&&(*pline)!='\t'&&(*pline)!='\0'&&(*pline)!='\r'&&(*pline)!='\n') pline++;
	nlen = pline-pstart;
	char filename[20];
	memset(filename,0,20);
	memcpy(filename,pstart,nlen);
	filename[nlen]='\0';
		
	FILE *fp;
  if(!(fp=fopen(filename, "r")))
  {
  	iprintf("cannot save to file: %s\n",filename);
  	return false;
  }
  memset(buffer, 0, FILE_MAX_SIZE);
  do
  {
  	*p = getc(fp);
  	p++; i++;
  }while(!feof(fp) && i<FILE_MAX_SIZE);
  *(p-2) = '\0';
  fclose(fp);
  return true;
}

void echo_char(int key)
{
	printf("%c%c_",0x08,key);
}

int get_input_number()
{
	int num, key;

	num=0;

	while(1) 
	{
		swiWaitForVBlank();

		key = keyboardUpdate();
		if(key>='0' && key<='9') 
		{
			num=num*10+(key-'0');
			if(num>0) 
				printf("%c",key);
		}
		if(key==0x08 && num>0) 
		{
			num=num/10;
			printf("%c",0x08); //printf(' '); printf("%c",0x08);
		}
		if(key==0x0A) 
		{
			printf("%c",0x0A);
			//printf("%c%c",0x08,0x0A);
			return num;
		}
	}
}

void do_cls()
{
	consoleClear();
}

void do_delay(int millisecond)
{
	uint ticks = 0;
	uint oldtick;
	double ms=millisecond;
	if(millisecond==-99)
		ms=0.5;
	
	timerStart(0, ClockDivider_1024, 0, NULL);
	
	ticks += timerElapsed(0);
	oldtick = ticks;
	
	double fesp=ms/1000*TIMER_SPEED+oldtick; //esp = (ticks-oldtick)/TIMER_SPEED*1000;
	uint esp=(uint)fesp;
	while(ticks<esp)
		ticks += timerElapsed(0);
	
	timerStop(0);
}

void _send(unsigned char* send_str, int len)
{
	int i=0, max_size = 1024;
	unsigned char* p = send_str;
	while(i<len && i<max_size)
	{
		writeBlocking_cardSPI(*p);
		do_delay(1);
		p++;
		i++;
	}
}

void do_send(unsigned char* send_str, unsigned char* recv_buff, int max_len)
{
	int i=0;
	while(send_str[i] && i< max_len)
	{
		recv_buff[i] = send_str[i];
		i++;
	}
		
	recv_buff[i]='\0';
	
	////////
	//int len=strlen(send_str);
	//setupConsecutive_cardSPI(len);
	_send((unsigned char*)send_str, max_len);
	
}

int do_recv(unsigned char* buff, int num_byte, unsigned char* stop_byte)
{
	u8 read_byte=0;
	int i=0;
	for(i=0; i<num_byte; i++)
	{
  	writeBlocking_cardSPI(0x00);
  	while(readBlocking_cardSPI(&read_byte) != CARD_SPI_STATUS_OK);
  	
  	if( (NULL != stop_byte) && ((char)read_byte == *stop_byte) )
  		return i;
  		
  	buff[i] = (char)read_byte;
  }
  
  return i;
}

void do_pset(int x, int y, int clr)
{
	set_pixel(x,y,clr);
}

inline void set_pixel(int x, int y, int clr)
{
//	if(	x>=min_clip_x && x<=max_clip_x &&
//			y>=min_clip_y && y<=max_clip_y)
//	{	
		BG_GFX[((y<<8)+x)] = color[clr] | BIT(15);
//	}
}

inline void set_pixel_v(int x, int y, int clr)
{
	if(	x>=min_clip_x && x<=max_clip_x &&
			y>=min_clip_y && y<=max_clip_y)
	{	
		BG_GFX[((y<<8)+x)] = color[clr] | BIT(15);
	}	
}

int Cohen_ClipLine(int* x11,int* y11,int* x22, int* y22);

void do_line(int x1, int y1, int x2, int y2, int clr)
{
		if(!Cohen_ClipLine(&x1,&y1,&x2,&y2))
			return;
		
		int  x, y;
		int  dx, dy;
		int  incx, incy;
		int  balance;
		int i=0;
		if (x2 >= x1)
		{
			dx = x2 - x1;
			incx = 1;
		}
		else
		{
			dx = x1 - x2;
			incx = -1;
		}
		
		if (y2 >= y1)
		{
			dy = y2 - y1;
			incy = 1;
		}
		else
		{
			dy = y1 - y2;
			incy = -1;
		}
		
		x = x1;
		y = y1;
		
		if (dx >= dy)
		{
			dy <<= 1;
			balance = dy - dx;
			dx <<= 1;
			
			while (x != x2)
			{
				set_pixel(x,y,clr);
				if (balance >= 0)
				{
					y += incy;
					balance -= dx;
				}
				balance += dy;
				x += incx;
				i ++;
			}
			set_pixel(x,y,clr);
		}
		else
		{
			dx <<= 1;
			balance = dx - dy;
			dy <<= 1;
			
			while (y != y2)
			{
				set_pixel(x,y,clr);
				if (balance >= 0)
				{
					x += incx;
					balance -= dy;
				}
				balance += dx;
				y += incy;
				i ++;
			}
			set_pixel(x,y,clr);
		}
	}
	
	int Cohen_ClipLine(int* x11,int* y11,int* x22, int* y22)
	{
		int x1=*x11,y1=*y11,x2=*x22,y2=*y22;
		// this function clips the sent line using the globally defined clipping
		// region
		
		// internal clipping codes
#define CLIP_CODE_C  0x0000
#define CLIP_CODE_N  0x0008
#define CLIP_CODE_S  0x0004
#define CLIP_CODE_E  0x0002
#define CLIP_CODE_W  0x0001
		
#define CLIP_CODE_NE 0x000a
#define CLIP_CODE_SE 0x0006
#define CLIP_CODE_NW 0x0009 
#define CLIP_CODE_SW 0x0005

		
		int xc1=x1, 
			yc1=y1, 
			xc2=x2, 
			yc2=y2;
		
		int p1_code=0, 
			p2_code=0;
		
		// determine codes for p1 and p2
		if (y1 < min_clip_y)
			p1_code|=CLIP_CODE_N;
		else if (y1 > max_clip_y)
			p1_code|=CLIP_CODE_S;
		
		if (x1 < min_clip_x)
			p1_code|=CLIP_CODE_W;
		else if (x1 > max_clip_x)
			p1_code|=CLIP_CODE_E;
		
		if (y2 < min_clip_y)
			p2_code|=CLIP_CODE_N;
		else if (y2 > max_clip_y)
			p2_code|=CLIP_CODE_S;
		
		if (x2 < min_clip_x)
			p2_code|=CLIP_CODE_W;
		else if (x2 > max_clip_x)
			p2_code|=CLIP_CODE_E;
		
		// try and trivially reject
		if ((p1_code & p2_code)) 
			return(0);
		
		// test for totally visible, if so leave points untouched
		if (p1_code==0 && p2_code==0)
			return(1);
		
		// determine end clip point for p1
		switch(p1_code)
		{
		case CLIP_CODE_C: break;
			
		case CLIP_CODE_N:
			{
				yc1 = min_clip_y;
				xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
			} break;
		case CLIP_CODE_S:
			{
				yc1 = max_clip_y;
				xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);
			} break;
			
		case CLIP_CODE_W:
			{
				xc1 = min_clip_x;
				yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);
			} break;
			
		case CLIP_CODE_E:
			{
				xc1 = max_clip_x;
				yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
			} break;
			
			// these cases are more complex, must compute 2 intersections
		case CLIP_CODE_NE:
			{
				// north hline intersection
				yc1 = min_clip_y;
				xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
				
				// test if intersection is valid, of so then done, else compute next
				if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
					// east vline intersection
					xc1 = max_clip_x;
					yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
				} // end if
				
			} break;
			
		case CLIP_CODE_SE:
			{
				// south hline intersection
				yc1 = max_clip_y;
				xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);	
				
				// test if intersection is valid, of so then done, else compute next
				if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
					// east vline intersection
					xc1 = max_clip_x;
					yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
				} // end if
				
			} break;
			
		case CLIP_CODE_NW: 
			{
				// north hline intersection
				yc1 = min_clip_y;
				xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
				
				// test if intersection is valid, of so then done, else compute next
				if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
					xc1 = min_clip_x;
					yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);	
				} // end if
				
			} break;
			
		case CLIP_CODE_SW:
			{
				// south hline intersection
				yc1 = max_clip_y;
				xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);	
				
				// test if intersection is valid, of so then done, else compute next
				if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
					xc1 = min_clip_x;
					yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);	
				} // end if
				
			} break;
			
		default:break;
			
		} // end switch
		
		// determine clip point for p2
		switch(p2_code)
		{
		case CLIP_CODE_C: break;
			
		case CLIP_CODE_N:
			{
				yc2 = min_clip_y;
				xc2 = x2 + (min_clip_y-y2)*(x1-x2)/(y1-y2);
			} break;
			
		case CLIP_CODE_S:
			{
				yc2 = max_clip_y;
				xc2 = x2 + (max_clip_y-y2)*(x1-x2)/(y1-y2);
			} break;
			
		case CLIP_CODE_W:
			{
				xc2 = min_clip_x;
				yc2 = y2 + (min_clip_x-x2)*(y1-y2)/(x1-x2);
			} break;
			
		case CLIP_CODE_E:
			{
				xc2 = max_clip_x;
				yc2 = y2 + (max_clip_x-x2)*(y1-y2)/(x1-x2);
			} break;
			
			// these cases are more complex, must compute 2 intersections
		case CLIP_CODE_NE:
			{
				// north hline intersection
				yc2 = min_clip_y;
				xc2 = x2 + 0.5+(min_clip_y-y2)*(x1-x2)/(y1-y2);
				
				// test if intersection is valid, of so then done, else compute next
				if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
					// east vline intersection
					xc2 = max_clip_x;
					yc2 = y2 + 0.5+(max_clip_x-x2)*(y1-y2)/(x1-x2);
				} // end if
				
			} break;
			
		case CLIP_CODE_SE:
			{
				// south hline intersection
				yc2 = max_clip_y;
				xc2 = x2 + 0.5+(max_clip_y-y2)*(x1-x2)/(y1-y2);	
				
				// test if intersection is valid, of so then done, else compute next
				if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
					// east vline intersection
					xc2 = max_clip_x;
					yc2 = y2 + 0.5+(max_clip_x-x2)*(y1-y2)/(x1-x2);
				} // end if
				
			} break;
			
		case CLIP_CODE_NW: 
			{
				// north hline intersection
				yc2 = min_clip_y;
				xc2 = x2 + 0.5+(min_clip_y-y2)*(x1-x2)/(y1-y2);
				
				// test if intersection is valid, of so then done, else compute next
				if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
					xc2 = min_clip_x;
					yc2 = y2 + 0.5+(min_clip_x-x2)*(y1-y2)/(x1-x2);	
				} // end if
				
			} break;
			
		case CLIP_CODE_SW:
			{
				// south hline intersection
				yc2 = max_clip_y;
				xc2 = x2 + 0.5+(max_clip_y-y2)*(x1-x2)/(y1-y2);	
				
				// test if intersection is valid, of so then done, else compute next
				if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
					xc2 = min_clip_x;
					yc2 = y2 + 0.5+(min_clip_x-x2)*(y1-y2)/(x1-x2);	
				} // end if
				
			} break;
			
		default:break;
			
		} // end switch
		
		// do bounds check
		if ((xc1 < min_clip_x) || (xc1 > max_clip_x) ||
			(yc1 < min_clip_y) || (yc1 > max_clip_y) ||
			(xc2 < min_clip_x) || (xc2 > max_clip_x) ||
			(yc2 < min_clip_y) || (yc2 > max_clip_y) )
		{
			return(0);
		} // end if
		
		// store vars back
		x1 = xc1;
		y1 = yc1;
		x2 = xc2;
		y2 = yc2;
		
		return(1);
		
	} // end Clip_Line
	
	
void do_circle( int mx, int my, int r, int clr )
{
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (x <= y)
    {
        set_pixel_v( mx  + x, my + y, clr );
        set_pixel_v( mx  - x, my + y, clr );
        set_pixel_v( mx  - x, my - y, clr );
        set_pixel_v( mx  + x, my - y, clr );
        set_pixel_v( mx  + y, my + x, clr );
        set_pixel_v( mx  - y, my + x, clr );
        set_pixel_v( mx  - y, my - x, clr );
        set_pixel_v( mx  + y, my - x, clr );

        if (d < 0)
        {
            d = d + 4 * x + 6;
        }
        else
        {
            d = d + 4 * ( x - y ) + 10;
            y --;
        }
        x++;
    }
}


void do_dwrite(int pin, int value)
{
	unsigned char send_str[5];
	
	send_str[0] = '\\'; //command sign
	send_str[1] = SPI_COMMAND_DWRITE;	//command type
	send_str[2] = (unsigned char)pin;		//pin
	send_str[3] = ((unsigned char)(value!=0?1:0));	//value
	send_str[4] = '\0';
	_send(send_str, 4);
}

void do_awrite(int pin, int value)
{
	unsigned char send_str[5];
	
	send_str[0] = '\\'; //command sign
	send_str[1] = SPI_COMMAND_AWRITE;	//command type
	send_str[2] = (unsigned char)pin;		//pin
	send_str[3] = (unsigned char)(value);	//value
	send_str[4] = '\0';
	_send(send_str, 4); 
}

int do_dread(int pin)
{
	unsigned char value=0x0;
	unsigned char send_str[4];
	
	send_str[0] = '\\'; //command sign
	send_str[1] = SPI_COMMAND_DREAD;	//command type
	send_str[2] = (unsigned char)pin;		//pin
	send_str[3] = '\0';
	
	_send(send_str, 3);
		
	do_recv(&value,1,NULL);
	//printf("recv:%d\n",value);
	return value;
}

int do_aread(int pin)
{
	int value;
	unsigned char recv_str[2];
	unsigned char send_str[4];
	
	send_str[0] = '\\'; //command sign
	send_str[1] = SPI_COMMAND_AREAD;	//command type
	send_str[2] = (unsigned char)pin;		//pin
	send_str[3] = '\0';
	
	_send(send_str, 3);
		
	recv_str[0]=recv_str[1]=0;
	do_recv(recv_str,2,NULL);
	
	//the first byte send from arduino is the high byte of the result of analog read.
	value = recv_str[0]; 
	value <<= 8;
	value |= recv_str[1];
	
	return value;
}

void do_clear()
{
	unsigned int clr = 0x0000 | BIT(15);
	int ind = 0;
	int stop = 256*192;
	for(ind=0; ind<stop; ind++)
		BG_GFX[ind] = clr;
}