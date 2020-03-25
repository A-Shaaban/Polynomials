//OS/compiler detection
#ifdef WIN32
 #include "stdafx2.h"
 #include <winbase.h>
 #define QB64_WINDOWS
 #ifdef _MSC_VER
  #define QB64_MICROSOFT
 #else
  #define QB64_GCC
 #endif
#else
 #define QB64_LINUX
 #define QB64_GCC
#endif

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_thread.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

//OS/compiler specific includes
#ifdef QB64_WINDOWS
 #include <direct.h>
#else
 #include <unistd.h>
 #include <sys/stat.h>
 #include <stdint.h>
#endif

using namespace std;

//common types
#ifdef QB64_WINDOWS
 #define uint64 unsigned __int64
 #define uint32 unsigned __int32
 #define uint16 unsigned __int16
 #define uint8 unsigned __int8
 #define int64 __int64
 #define int32 __int32
 #define int16 __int16
 #define int8 __int8
#else
 #define int64 int64_t
 #define int32 int32_t
 #define int16 int16_t
 #define int8 int8_t
 #define uint64 uint64_t
 #define uint32 uint32_t
 #define uint16 uint16_t
 #define uint8 uint8_t
#endif

struct qbs
{
unsigned char *chr; //a 32 bit pointer to the string's data
unsigned long len;
unsigned char in_cmem; //set to 1 if in the conventional memory DBLOCK
unsigned short *cmem_descriptor;
unsigned short cmem_descriptor_offset;
unsigned long listi; //the index in the list of strings that references it
unsigned char tmp; //set to 1 if the string can be deleted immediately after being processed
unsigned long tmplisti; //the index in the list of strings that references it
unsigned char fixed; //fixed length string
unsigned char readonly; //set to 1 if string is read only
};

//substitute Windows functionality
#ifndef QB64_WINDOWS
 //messagebox defines
 #define IDOK                1
 #define IDCANCEL            2
 #define IDABORT             3
 #define IDRETRY             4
 #define IDIGNORE            5
 #define IDYES               6
 #define IDNO                7
 #define MB_OK                       0x00000000L
 #define MB_OKCANCEL                 0x00000001L
 #define MB_ABORTRETRYIGNORE         0x00000002L
 #define MB_YESNOCANCEL              0x00000003L
 #define MB_YESNO                    0x00000004L
 #define MB_RETRYCANCEL              0x00000005L

 int MessageBox(int p1,char* p2,char* p3,int p4){
 cout<<"[MessageBox("<<p3<<","<<p2<<")]";
 exit(0);
 return 0;
 }

 inline uint32 _lrotl(uint32 word,uint32 shift){
 return (word << shift) | (word >> (32 - shift));
 }

 void AllocConsole(){
 return;
 }
 void FreeConsole(){
 return;
 }

 long errno;
#endif

struct img_struct{
unsigned char valid;//0,1 0=invalid
unsigned char text;//if set, surface is a text surface
unsigned short width,height;
unsigned char bytes_per_pixel;//1,2,4
unsigned char bits_per_pixel;//1,2,4,8,16(text),32
unsigned long mask;//1,3,0xF,0xFF,0xFFFF,0xFFFFFFFF
unsigned short compatible_mode;//0,1,2,7,8,9,10,11,12,13,32,256
unsigned long color,background_color;
unsigned long font;//8,14,16,?
short top_row,bottom_row;//VIEW PRINT settings, unique (as in QB) to each "page"
short cursor_x,cursor_y;//unique (as in QB) to each "page"
unsigned char cursor_show, cursor_firstvalue, cursor_lastvalue;
union{
unsigned char *offset;
unsigned long *offset32;
};
unsigned long flags;
long view_x1,view_y1,view_x2,view_y2;
long view_offset_x,view_offset_y;
float x,y;
unsigned char clipping_or_scaling;
float scaling_x,scaling_y,scaling_offset_x,scaling_offset_y;
float window_x1,window_y1,window_x2,window_y2;
unsigned long *pal;
long transparent_color;//-1 means no color is transparent
unsigned char alpha_disabled;
unsigned char holding_cursor;
unsigned char print_mode;
};
//img_struct flags
//#define IMG_FIXED 1 //offset & dimensions cannot change
//#define IMG_SHARED 2 //when freed, image data is not freed
#define IMG_FREEPAL 1 //free palette data before freeing image
#define IMG_SCREEN 2 //img is linked to other screen pages
#define IMG_FREEMEM 4 //if set, it means memory must be freed

#ifdef QB64_WINDOWS
inline void SDL_Delay(Uint32 milliseconds){
Sleep(milliseconds);
}
#else
inline void Sleep(Uint32 milliseconds){
SDL_Delay(milliseconds);
}
#endif

//inline int64 qbr(long double f){if (f<0) return(f-0.5f); else return(f+0.5f);}
//inline uint64 qbr_longdouble_to_uint64(long double f){if (f<0) return(f-0.5f); else return(f+0.5f);}
//inline long qbr_float_to_long(float f){if (f<0) return(f-0.5f); else return(f+0.5f);}
//inline long qbr_double_to_long(double f){if (f<0) return(f-0.5f); else return(f+0.5f);}
//Compatible rounding via FPU:
#ifdef QB64_MICROSOFT
	inline int64 qbr(long double f){
		static int64 i;
		__asm{
		fld   f
		fistp i
		}
		return i;
	}
	inline uint64 qbr_longdouble_to_uint64(long double f){
		static uint64 i;
		__asm{
		fld   f
		fistp i
		}
		return i;
	}
	inline long qbr_float_to_long(float f){
		static long i;
		__asm{
		fld   f
		fistp i
		}
		return i;
	}
	inline long qbr_double_to_long(double f){
		static long i;
		__asm{
		fld   f
		fistp i
		}
		return i;
	}
#else
//FLDS=load single
//FLDL=load double
//FLDT=load long double
	inline int64 qbr(long double f){
	static int64 i;
  	__asm__ (
		"fldt %1;"
		"fistpll %0;"              
		:"=m" (i)
		:"m" (f)
		);
		return i;
	}
	inline uint64 qbr_longdouble_to_uint64(long double f){
		static uint64 i;
		__asm__ (
		"fldt %1;"
		"fistpll %0;"              
		:"=m" (i)
		:"m" (f)
		);
		return i;
	}
	inline long qbr_float_to_long(float f){
		static long i;
		__asm__ (
		"flds %1;"
		"fistpl %0;"              
		:"=m" (i)
		:"m" (f)
		);
		return i;
	}
	inline long qbr_double_to_long(double f){
		static long i;
		__asm__ (
		"fldl %1;"
		"fistpl %0;"              
		:"=m" (i)
		:"m" (f)
		);
		return i;
	}
#endif

#include "bit.cpp"

#define ISSTRING 1073741824
#define ISFLOAT 536870912
#define ISUNSIGNED 268435456
#define ISPOINTER 134217728
#define ISFIXEDLENGTH 67108864 //only set for strings with pointer flag
#define ISINCONVENTIONALMEMORY 33554432
#define ISOFFSETINBITS 16777216
