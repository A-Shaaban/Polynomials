#include "common.cpp"

//vc->project->properties->configuration properties->general->configuration type->application(.exe)
//vc->project->properties->configuration properties->general->configuration type->static library(.lib)
extern int __cdecl QBMAIN(void *);

//directory access defines
#define EPERM           1
#define ENOENT          2
#define ESRCH           3
#define EINTR           4
#define EIO             5
#define ENXIO           6
#define E2BIG           7
#define ENOEXEC         8
#define EBADF           9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define EINVAL          22
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define ERANGE          34
#define EDEADLK         36
#define ENAMETOOLONG    38
#define ENOLCK          39
#define ENOSYS          40
#define ENOTEMPTY       41
#define EILSEQ          42















//forward refs
void validatepage(long);
void sub__dest(long);
void sub__source(long);
long func__printwidth(qbs*,long,long);
void sub_cls(long,unsigned long,long);
void qbs_print(qbs*,long);
long func__copyimage(long,long);
long func__dest();
long func__display();
void qbg_sub_view_print(long,long,long);
qbs *qbs_new_txt(const char *);
void qbg_sub_window(long,float,float,float,float,long);

unsigned long pal[256];


extern qbs* nothingstring;

static unsigned long sdl_shiftstate=0;

static unsigned long sdl_scroll_lock=0;
static unsigned long sdl_insert=0;
static unsigned long sdl_scroll_lock_prepared=1;
static unsigned long sdl_insert_prepared=1;

long sub_screen_height_in_characters=-1;//-1=undefined
long sub_screen_width_in_characters=-1;//-1=undefined
long sub_screen_font=-1;//-1=undefined
long sub_screen_keep_page0=0;

long key_repeat_on=0;



void error(long error_number);//for forward references
unsigned long palette_256[256];
unsigned long palette_64[64];


//QB64 2D PROTOTYPE 1.0
SDL_Surface *ts,*ts2;
SDL_PixelFormat pixelformat32;
SDL_PixelFormat pixelformat8;



long pages=1;
long *page=(long*)calloc(1,4);

#define IMG_BUFFERSIZE 4096
img_struct *img=(img_struct*)malloc(IMG_BUFFERSIZE*sizeof(img_struct));
unsigned long nimg=IMG_BUFFERSIZE;
unsigned long nextimg=0;

unsigned long *fimg=(unsigned long*)malloc(IMG_BUFFERSIZE*4);//a list to recover freed indexes
unsigned long nfimg=IMG_BUFFERSIZE;
unsigned long lastfimg=-1;//-1=no freed indexes exist


unsigned char *blend=(unsigned char*)malloc(16777216);
unsigned char *ablend=(unsigned char*)malloc(65536);
unsigned char *ablend127;
unsigned char *ablend128;



unsigned long display_page_index=0;
unsigned long write_page_index=0;
unsigned long read_page_index=0;
//use of non-indexed forms assumes valid indexes (may not be suitable for all commands)
img_struct *write_page=NULL;
img_struct *read_page=NULL;
img_struct *display_page=NULL;
SDL_Surface *display_surface=NULL;
unsigned long *display_surface_offset=0;

void restorepalette(img_struct* im){
static unsigned long *pal;
if (im->bytes_per_pixel==4) return;
pal=im->pal;

switch(im->compatible_mode){

case 1:
/*
SCREEN Mode 1 Syntax:  COLOR [background][,palette]
   ¦ background is the screen color (range = 0-15)
   ¦ palette is a three-color palette (range = 0-1)
     0 = green, red, and brown         1 = cyan, magenta, and bright white
Note: option 1 is the default, palette can override these though
OPTION 1:*DEFAULT*
0=black(color 0)
1=cyan(color 3)
2=purple(color 5)
3=light grey(color 7)
OPTION 0:
0=black(color 0)
1=green(color 2)
2=red(color 4)
3=brown(color 6)
*/
pal[0]=palette_256[0];
pal[1]=palette_256[3];
pal[2]=palette_256[5];
pal[3]=palette_256[7];
return;
break;

case 2://black/white 2 color palette
pal[0]=0;
pal[1]=0xFFFFFF;
return;
break;

case 9://16 colors selected from 64 possibilities
pal[0]=palette_64[0];
pal[1]=palette_64[1];
pal[2]=palette_64[2];
pal[3]=palette_64[3];
pal[4]=palette_64[4];
pal[5]=palette_64[5];
pal[6]=palette_64[20];
pal[7]=palette_64[7];
pal[8]=palette_64[56];
pal[9]=palette_64[57];
pal[10]=palette_64[58];
pal[11]=palette_64[59];
pal[12]=palette_64[60];
pal[13]=palette_64[61];
pal[14]=palette_64[62];
pal[15]=palette_64[63];
return;
break;

case 10://4 colors selected from 9 possibilities (does not use pal[] array)
//use upper palette values to hold selected palette colors
pal[252]=4;
pal[253]=5;
pal[254]=6;
pal[255]=7;
return;
break;

case 11://black/white 2 color palette
pal[0]=0;
pal[1]=0xFFFFFF;
return;
break;

case 13:
memcpy(pal,palette_256,1024);
return;
break;

case 256:
memcpy(pal,palette_256,1024);
return;
break;

default:
//default 16 color palette
memcpy(pal,palette_256,64);

};//switch

}//restorepalette




void pset(long x,long y,unsigned long col){
static unsigned char *cp;
static unsigned long *o32;
static unsigned long destcol;
if (write_page->bytes_per_pixel==1){
 write_page->offset[y*write_page->width+x]=col&write_page->mask;
 return;
}else{
 if (write_page->alpha_disabled){
 write_page->offset32[y*write_page->width+x]=col;
 return;
 }
switch(col&0xFF000000){
case 0xFF000000://100% alpha, so regular pset (fast)
 write_page->offset32[y*write_page->width+x]=col;
 return;
break;
case 0x0://0%(0) alpha, so no pset (very fast)
 return;
break;
case 0x80000000://~50% alpha (optomized)
 o32=write_page->offset32+(y*write_page->width+x);
 *o32=(((*o32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend128[*o32>>24]<<24);
 return;
break; 
case 0x7F000000://~50% alpha (optomized)
 o32=write_page->offset32+(y*write_page->width+x);
 *o32=(((*o32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend127[*o32>>24]<<24);
 return;
break;
default://other alpha values (uses a lookup table)
 o32=write_page->offset32+(y*write_page->width+x);
 destcol=*o32;
 cp=blend+(col>>24<<16);
 *o32=
   cp[(col<<8&0xFF00)+(destcol&255)    ]
 +(cp[(col&0xFF00)   +(destcol>>8&255) ]<<8)
 +(cp[(col>>8&0xFF00)+(destcol>>16&255)]<<16)
 +(ablend[(col>>24)+(destcol>>16&0xFF00)]<<24);
};
}
}



/*
img_struct *img=(img_struct*)malloc(1024*sizeof(img_struct));
unsigned long nimg=1024;
unsigned long nextimg=0;//-1=none have been assigned

unsigned long *freeimg=(unsigned long*)malloc(1024*4);//a list to recover freed indexes
unsigned long nfreeimg=1024;
unsigned long lastfreeimg=-1;//-1=no freed indexes exist
*/

//returns an index to free img structure
unsigned long newimg(){
static long i;
if (lastfimg!=-1){
i=fimg[lastfimg--];
goto gotindex;
}
if (nextimg<nimg){
i=nextimg++;
goto gotindex;
}
img=(img_struct*)realloc(img,(nimg+IMG_BUFFERSIZE)*sizeof(img_struct));
if (!img) error(257);
memset(&img[nimg],0,IMG_BUFFERSIZE*sizeof(img_struct));
nimg+=IMG_BUFFERSIZE;
i=nextimg++;
gotindex:
img[i].valid=1;
return i;
}

long freeimg(unsigned long i){
//returns: 0=failed, 1=success
if (i>=nimg) return 0;
if (!img[i].valid) return 0;
if (lastfimg>=(nfimg-1)){//extend
fimg=(unsigned long*)realloc(fimg,(nfimg+IMG_BUFFERSIZE)*4);
if (!fimg) error(257);
nfimg+=IMG_BUFFERSIZE;
}
memset(&img[i],0,sizeof(img_struct));
lastfimg++;
fimg[lastfimg]=i;
return 1;
}


void imgrevert(long i){
static long bpp;
static img_struct *im;

im=&img[i];
bpp=im->compatible_mode;

//revert to assumed default values
im->bytes_per_pixel=1;
im->font=16;
im->color=15;
im->print_mode=3;

//revert to mode's set values
switch (bpp){
case 0:
im->bits_per_pixel=16; im->bytes_per_pixel=2;
im->color=7;
im->text=1;
im->cursor_show=0; im->cursor_firstvalue=4; im->cursor_lastvalue=4;
break;
case 1:
im->bits_per_pixel=2;
im->font=8;
im->color=3;
break;
case 2:
im->bits_per_pixel=1; 
im->font=8;//it gets stretched from 8 to 16 later
im->color=1;
break;
case 7:
im->bits_per_pixel=4;
im->font=8;
break;
case 8:
im->bits_per_pixel=4;
im->font=8;
break;
case 9:
im->bits_per_pixel=4;
im->font=14;
break;
case 10:
im->bits_per_pixel=2;
im->font=14;
im->color=3;
break;
case 11:
im->bits_per_pixel=1;
im->color=1;
break;
case 12:
im->bits_per_pixel=4;
break;
case 13:
im->bits_per_pixel=8;
im->font=8;
break;
case 256:
im->bits_per_pixel=8;
break;
case 32:
im->bits_per_pixel=32; im->bytes_per_pixel=4;
im->color=0xFFFFFFFF;
im->background_color=0xFF000000;
break;
};

//revert palette
if (bpp!=32){
restorepalette(im);
im->transparent_color=-1;
}

//revert calculatable values
if (im->bits_per_pixel<32) im->mask=(1<<im->bits_per_pixel)-1; else im->mask=0xFFFFFFFF;
//text
im->cursor_x=1; im->cursor_y=1;
im->top_row=1;
if (bpp) im->bottom_row=(im->height/im->font); else im->bottom_row=im->height;
im->bottom_row--; if (im->bottom_row<=0) im->bottom_row=1;
if (!bpp) return;
//graphics
//clipping/scaling
im->x=((double)im->width)/2.0; im->y=((double)im->height)/2.0;
im->view_x2=im->width-1; im->view_y2=im->height-1;
im->scaling_x=1; im->scaling_y=1;
im->window_x2=im->view_x2; im->window_y2=im->view_y2;

//clear
if (bpp){//graphics
memset(im->offset,0,im->width*im->height*im->bytes_per_pixel);
}else{//text
static long i2,i3;
static unsigned short *sp;
i3=im->width*im->height; sp=(unsigned short*)im->offset; for (i2=0;i2<i3;i2++){*sp++=0x0720;}
}

}//imgrevert

long imgframe(unsigned char *o,long x,long y,long bpp){
static long i;
static img_struct *im;
if (x<=0||y<=0) return 0;
i=newimg();
im=&img[i];
im->offset=o;
im->width=x; im->height=y;

//assume default values
im->bytes_per_pixel=1;
im->font=16;
im->color=15;
im->compatible_mode=bpp;
im->print_mode=3;

//set values
switch (bpp){
case 0:
im->bits_per_pixel=16; im->bytes_per_pixel=2;
im->color=7;
im->text=1;
im->cursor_show=0; im->cursor_firstvalue=4; im->cursor_lastvalue=4;
break;
case 1:
im->bits_per_pixel=2;
im->font=8;
im->color=3;
break;
case 2:
im->bits_per_pixel=1; 
im->font=8;//it gets stretched from 8 to 16 later
im->color=1;
break;
case 7:
im->bits_per_pixel=4;
im->font=8;
break;
case 8:
im->bits_per_pixel=4;
im->font=8;
break;
case 9:
im->bits_per_pixel=4;
im->font=14;
break;
case 10:
im->bits_per_pixel=2;
im->font=14;
im->color=3;
break;
case 11:
im->bits_per_pixel=1;
im->color=1;
break;
case 12:
im->bits_per_pixel=4;
break;
case 13:
im->bits_per_pixel=8;
im->font=8;
break;
case 256:
im->bits_per_pixel=8;
break;
case 32:
im->bits_per_pixel=32; im->bytes_per_pixel=4;
im->color=0xFFFFFFFF;
im->background_color=0xFF000000;
break;
default:
return 0;
};

//attach palette
if (bpp!=32){
im->pal=(unsigned long*)calloc(256,4);
if (!im->pal){
freeimg(i);
return 0;
}
im->flags|=IMG_FREEPAL;
restorepalette(im);
im->transparent_color=-1;
}

//set calculatable values
if (im->bits_per_pixel<32) im->mask=(1<<im->bits_per_pixel)-1; else im->mask=0xFFFFFFFF;
//text
im->cursor_x=1; im->cursor_y=1;
im->top_row=1;
if (bpp) im->bottom_row=(im->height/im->font); else im->bottom_row=im->height;
im->bottom_row--; if (im->bottom_row<=0) im->bottom_row=1;
if (!bpp) return i;
//graphics
//clipping/scaling
im->x=((double)im->width)/2.0; im->y=((double)im->height)/2.0;
im->view_x2=im->width-1; im->view_y2=im->height-1;
im->scaling_x=1; im->scaling_y=1;
im->window_x2=im->view_x2; im->window_y2=im->view_y2;

return i;
}

void sub__freeimage(long i,long passed);//forward ref

long imgnew(long x,long y,long bpp){
static long i,i2,i3;
static img_struct *im;
static unsigned short *sp;
static unsigned long *lp;
i=imgframe(NULL,x,y,bpp);
if (!i) return 0;

im=&img[i];
if (bpp){//graphics
if (bpp==32){
im->offset=(unsigned char*)calloc(x*y,4);
if (!im->offset){sub__freeimage(-i,1); return 0;}
//i3=x*y; lp=im->offset32; for (i2=0;i2<i3;i2++){*lp++=0xFF000000;}
}else{
im->offset=(unsigned char*)calloc(x*y*im->bytes_per_pixel,1);
if (!im->offset){sub__freeimage(-i,1); return 0;}
}
}else{//text
im->offset=(unsigned char*)malloc(x*y*im->bytes_per_pixel);
if (!im->offset){sub__freeimage(-i,1); return 0;}
i3=x*y; sp=(unsigned short*)im->offset; for (i2=0;i2<i3;i2++){*sp++=0x0720;}
}
im->flags|=IMG_FREEMEM;
return i;
}

void sub__font(long f,long i,long passed);//foward def


long imgload(char *filename,long bpp){
static long i,i2,x,y,i3,z2,z3,v,v2,v3,r,g,b,a,t,needt,t2;
static unsigned char *cp,*cp2;
static unsigned long c;
static unsigned long *lp;

static unsigned char *sr=(unsigned char*)malloc(256);
static unsigned char *sg=(unsigned char*)malloc(256);
static unsigned char *sb=(unsigned char*)malloc(256);
static unsigned char *dr=(unsigned char*)malloc(256);
static unsigned char *dg=(unsigned char*)malloc(256);
static unsigned char *db=(unsigned char*)malloc(256);
static unsigned char *link=(unsigned char*)malloc(256);
static long *usedcolor=(long*)malloc(1024);

ts=IMG_Load(filename);
if (!ts) return 0;

if (bpp==-1){

if (write_page->bytes_per_pixel==1){
if (ts->format->BytesPerPixel==1) goto compatible;

//32-->8 bit (best possible color selection)
ts2=SDL_ConvertSurface(ts,&pixelformat32,NULL);
if (!ts2){SDL_FreeSurface(ts); return 0;}
i=imgnew(ts2->w,ts2->h,write_page->compatible_mode);
if (!i){SDL_FreeSurface(ts); return 0;}
//copy write_page's palette
memcpy(img[i].pal,write_page->pal,1024);
//find number of colors
z3=write_page->mask+1;
//build color value table
for (i3=0;i3<z3;i3++){
c=write_page->pal[i3];
db[i3]=c&0xFF; dg[i3]=c>>8&0xFF; dr[i3]=c>>16&0xFF;
}

//reset color used flags
memset(usedcolor,0,1024);
needt=0;
//copy/change colors
cp=(unsigned char*)ts2->pixels; cp2=img[i].offset;
for (y=0;y<img[i].height;y++){
for (x=0;x<img[i].width;x++){
c=*((unsigned long*)(cp+y*ts2->pitch+x*4));
a=c>>24;
if (a==0){
needt=1;
}else{
b=c&0xFF; g=c>>8&0xFF; r=c>>16&0xFF; v=1000; v3=0;
for (i3=0;i3<z3;i3++){
v2=abs(r-(long)dr[i3])+abs(g-(long)dg[i3])+abs(b-(long)db[i3]);
if (v2<v){v3=i3; v=v2;}
}//i3
cp2[y*img[i].width+x]=v3;
usedcolor[v3]++;
}//a==0
}}
//add transparency
if (needt){
//find best transparent color
v=0x7FFFFFFF;
for (x=0;x<z3;x++){
if (usedcolor[x]<=v){
v=usedcolor[x];
t=x;
}
}
//remake with transparency
img[i].transparent_color=t;
//copy/change colors
cp=(unsigned char*)ts2->pixels; cp2=img[i].offset;
for (y=0;y<img[i].height;y++){ for (x=0;x<img[i].width;x++){
c=*((unsigned long*)(cp+y*ts2->pitch+x*4));
a=c>>24; if (a==0){cp2[y*img[i].width+x]=t; goto usedtranscol;}
b=c&0xFF; g=c>>8&0xFF; r=c>>16&0xFF; v=1000; v3=0;
for (i3=0;i3<z3;i3++){
if (i3!=t){
v2=abs(r-(long)dr[i3])+abs(g-(long)dg[i3])+abs(b-(long)db[i3]);
if (v2<v){v3=i3; v=v2;}
}
}//i3
cp2[y*img[i].width+x]=v3;
usedtranscol:;
}}
}//needt
//adopt font
sub__font(write_page->font,-i,1);
//adopt colors
img[i].color=write_page->color;
img[i].background_color=write_page->background_color;
//adopt print mode
img[i].print_mode=write_page->print_mode;
SDL_FreeSurface(ts2);
SDL_FreeSurface(ts);
return i;
}//write_page->bytes_per_pixel==1
}//-1

if (bpp==256){
if (ts->format->BytesPerPixel!=1){SDL_FreeSurface(ts); return 0;}
compatible:
ts2=ts;
//check for transparent color in palette
ts=SDL_ConvertSurface(ts2,&pixelformat32,NULL);
if (!ts){SDL_FreeSurface(ts2); return 0;}
//prepare image to write to
if (bpp==-1){
i=imgnew(ts2->w,ts2->h,write_page->compatible_mode);
}else{
i=imgnew(ts2->w,ts2->h,256);
}
if (!i){SDL_FreeSurface(ts2); SDL_FreeSurface(ts); return 0;}
//does a transparent pixel exist?
t=-1;
for (y=0;y<img[i].height;y++){
lp=(unsigned long*)(((char*)ts->pixels)+ts->pitch*y);
for (x=0;x<img[i].width;x++){
if (!(*lp++&0xFF000000)){//alpha==0
//find equivalent 8-bit index
c=*(((unsigned char*)ts2->pixels)+ts2->pitch*y+x);
if (c<ts2->format->palette->ncolors){
img[i].transparent_color=c;
t=c;
goto found_transparent_color;
}
}
}}
found_transparent_color:

//8-->8 bit (best color match)
if (bpp==-1){
img[i].transparent_color=-1;//this will be set later if necessary
//copy write_page's palette
memcpy(img[i].pal,write_page->pal,1024);
//map image's palette to actual palette
//reset color used flags
memset(usedcolor,0,1024);
//find number of colors
z2=ts2->format->palette->ncolors;
z3=write_page->mask+1;
//build color value tables
for (i2=0;i2<z2;i2++){
c=*(unsigned long*)&ts2->format->palette->colors[i2];
sr[i2]=c&0xFF; sg[i2]=c>>8&0xFF; sb[i2]=c>>16&0xFF;
}
for (i3=0;i3<z3;i3++){
c=write_page->pal[i3];
db[i3]=c&0xFF; dg[i3]=c>>8&0xFF; dr[i3]=c>>16&0xFF;
}
//link colors to best matching color
for (i2=0;i2<z2;i2++){
v=1000; link[i2]=0;
for (i3=0;i3<z3;i3++){
v2=abs((long)sr[i2]-(long)dr[i3])+abs((long)sg[i2]-(long)dg[i3])+abs((long)sb[i2]-(long)db[i3]);
if (v2<v){
link[i2]=i3; v=v2;
}
}//i3
}//i2
//change colors
needt=0;
cp=(unsigned char*)ts2->pixels; cp2=img[i].offset;
for (y=0;y<img[i].height;y++){
for (x=0;x<img[i].width;x++){
c=cp[y*ts2->pitch+x];
if (c==t){
needt=1;
}else{
c=link[c];
cp2[y*img[i].width+x]=c;
usedcolor[c]++;
}
}}
//add transparency
if (needt){
t2=t;//backup
//find best transparent color
v=0x7FFFFFFF;
for (x=0;x<z3;x++){
if (usedcolor[x]<=v){
v=usedcolor[x];
t=x;
}
}
//remake with transparency
img[i].transparent_color=t;
//relink colors to best matching color (avoiding t)
for (i2=0;i2<z2;i2++){
v=1000; link[i2]=0;
for (i3=0;i3<z3;i3++){
if (i3!=t){
v2=abs((long)sr[i2]-(long)dr[i3])+abs((long)sg[i2]-(long)dg[i3])+abs((long)sb[i2]-(long)db[i3]);
if (v2<v){
link[i2]=i3; v=v2;
}
}
}//i3
}//i2
//change colors
cp=(unsigned char*)ts2->pixels; cp2=img[i].offset;
for (y=0;y<img[i].height;y++){
for (x=0;x<img[i].width;x++){
c=cp[y*ts2->pitch+x];
if (c==t2){
cp2[y*img[i].width+x]=t;
}else{
cp2[y*img[i].width+x]=link[c];
}
}}
}//needt
//adopt font
sub__font(write_page->font,-i,1);
//adopt colors
img[i].color=write_page->color;
img[i].background_color=write_page->background_color;
//adopt print mode
img[i].print_mode=write_page->print_mode;
SDL_FreeSurface(ts2);
SDL_FreeSurface(ts);
return i;
}//bpp==-1

//copy pixel data
cp=(unsigned char*)ts2->pixels; cp2=img[i].offset;
for (i2=0;i2<img[i].height;i2++){
memcpy(cp2,cp,ts2->w);
cp+=ts2->pitch;
cp2+=img[i].width;
}
//update palette
for (i2=ts2->format->palette->ncolors;i2<256;i2++){img[i].pal[i2]=0xFF000000;}
for (i2=0;i2<ts2->format->palette->ncolors;i2++){
c=*(unsigned long*)&ts2->format->palette->colors[i2];
c=0xFF000000+((c>>16)&255)+(c&0xFF00)+((c&255)<<16);
img[i].pal[i2]=c;
}
SDL_FreeSurface(ts2);
SDL_FreeSurface(ts);
return i;
}

ts2=SDL_ConvertSurface(ts,&pixelformat32,NULL);
if (!ts2){SDL_FreeSurface(ts); return 0;}
i=imgnew(ts2->w,ts2->h,32);
if (!i){SDL_FreeSurface(ts2); SDL_FreeSurface(ts); return 0;}
memcpy(img[i].offset,ts2->pixels,ts2->w*ts2->h*4);
SDL_FreeSurface(ts2); SDL_FreeSurface(ts);
return i;
}

void sub__putimage(long dstep1,double f_dx1,double f_dy1,long dstep2,double f_dx2,double f_dy2,long src,long dst,long sstep1,double f_sx1,double f_sy1,long sstep2,double f_sx2,double f_sy2,long passed){
//format & passed bits:
//[(dx1,dy1)[-(dx2,dy2)]][,[src][,[dst][,[(sx1,sy1)[-(sx2,sy2)]][,...?...]]]]
//  1          2            4      8       16         32          

static long w,h,sskip,dskip,x,y,xx,yy,z,x2,y2,dbpp,sbpp;
static img_struct *s,*d;
static unsigned long *soff32,*doff32,col,clearcol,destcol;
static unsigned char *soff,*doff;
static unsigned char *cp;
static long xdir,ydir,no_stretch,no_clip,no_reverse,flip,mirror;
static double mx,my,fx,fy,fsx1,fsy1,fsx2,fsy2,dv,dv2;
static long sx1,sy1,sx2,sy2,dx1,dy1,dx2,dy2;
static long sw,sh,dw,dh;
static unsigned long *pal;
static unsigned long *ulp;

no_stretch=0; no_clip=0; no_reverse=1;

flip=0; mirror=0;

if (passed&4){//src
 //validate
 if (src>=0){
 validatepage(src); s=&img[page[src]];
 }else{
 src=-src;
 if (src>=nextimg){error(258); return;}
 s=&img[src];
 if (!s->valid){error(258); return;}
 }
}else{
 s=read_page;
}//src
if (s->text){error(5); return;}
sbpp=s->bytes_per_pixel;

if (passed&8){//dst
 //validate
 if (dst>=0){
 validatepage(dst); d=&img[page[dst]];
 }else{
 dst=-dst;
 if (dst>=nextimg){error(258); return;}
 d=&img[dst];
 if (!d->valid){error(258); return;}
 }
}else{
 d=write_page;
}//dst
if (d->text){error(5); return;}
dbpp=d->bytes_per_pixel;
if ((sbpp==4)&&(dbpp==1)){error(5); return;}
if (s==d){error(5); return;}//cannot put source onto itself!

//quick references
sw=s->width; sh=s->height; dw=d->width; dh=d->height;

//resolve coordinates
if (passed&1){//dx1,dy1
if (d->clipping_or_scaling){
if (d->clipping_or_scaling==2){
dx1=qbr_float_to_long(f_dx1*d->scaling_x+d->scaling_offset_x)+d->view_offset_x;
dy1=qbr_float_to_long(f_dy1*d->scaling_y+d->scaling_offset_y)+d->view_offset_y;
}else{
dx1=qbr_float_to_long(f_dx1)+d->view_offset_x; dy1=qbr_float_to_long(f_dy1)+d->view_offset_y;
}
}else{
dx1=qbr_float_to_long(f_dx1); dy1=qbr_float_to_long(f_dy1);
}
 //note: dx2 & dy2 cannot be passed if dx1 & dy1 weren't passed
 if (passed&2){//dx2,dy2
 if (d->clipping_or_scaling){
 if (d->clipping_or_scaling==2){
 dx2=qbr_float_to_long(f_dx2*d->scaling_x+d->scaling_offset_x)+d->view_offset_x;
 dy2=qbr_float_to_long(f_dy2*d->scaling_y+d->scaling_offset_y)+d->view_offset_y;
 }else{
 dx2=qbr_float_to_long(f_dx2)+d->view_offset_x; dy2=qbr_float_to_long(f_dy2)+d->view_offset_y;
 }
 }else{
 dx2=qbr_float_to_long(f_dx2); dy2=qbr_float_to_long(f_dy2);
 }
 }else{//dx2,dy2
 dx2=0; dy2=0;
 }//dx2,dy2
}else{//dx1,dy1
dx1=0; dy1=0; dx2=0; dy2=0;
}//dx1,dy1

if (passed&16){//sx1,sy1
if (s->clipping_or_scaling){
if (s->clipping_or_scaling==2){
sx1=qbr_float_to_long(f_sx1*s->scaling_x+s->scaling_offset_x)+s->view_offset_x;
sy1=qbr_float_to_long(f_sy1*s->scaling_y+s->scaling_offset_y)+s->view_offset_y;
}else{
sx1=qbr_float_to_long(f_sx1)+s->view_offset_x; sy1=qbr_float_to_long(f_sy1)+s->view_offset_y;
}
}else{
sx1=qbr_float_to_long(f_sx1); sy1=qbr_float_to_long(f_sy1);
}
 //note: sx2 & sy2 cannot be passed if sx1 & sy1 weren't passed
 if (passed&32){//sx2,sy2
 if (s->clipping_or_scaling){
 if (s->clipping_or_scaling==2){
 sx2=qbr_float_to_long(f_sx2*s->scaling_x+s->scaling_offset_x)+s->view_offset_x;
 sy2=qbr_float_to_long(f_sy2*s->scaling_y+s->scaling_offset_y)+s->view_offset_y;
 }else{
 sx2=qbr_float_to_long(f_sx2)+s->view_offset_x; sy2=qbr_float_to_long(f_sy2)+s->view_offset_y;
 }
 }else{
 sx2=qbr_float_to_long(f_sx2); sy2=qbr_float_to_long(f_sy2);
 }
 }else{//sx2,sy2
 sx2=0; sy2=0;
 }//sx2,sy2
}else{//sx1,sy1
sx1=0; sy1=0; sx2=0; sy2=0;
}//sx1,sy1

if ((passed&2)&&(passed&32)){//all co-ords given
//could be stretched
 if ( (abs(dx2-dx1)==abs(sx2-sx1)) && (abs(dy2-dy1)==abs(sy2-sy1)) ){//non-stretched
 //could be flipped/reversed
 //could need clipping
 goto reverse;
 }
goto stretch;
}

if (passed&2){//(dx1,dy1)-(dx2,dy2)...
if (passed&16){//(dx1,dy1)-(dx2,dy2),...,(sx1,sy1)
sx2=sx1+abs(dx2-dx1); sy2=sy1+abs(dy2-dy1);
//can't be stretched
//could be flipped/reversed
//could need clipping
goto reverse;
}else{//(dx1,dy1)-(dx2,dy2)
sx2=sw-1; sy2=sh-1;
//could be stretched
 if ( ((abs(dx2-dx1)+1)==sw) && ((abs(dy2-dy1)+1)==sh) ){//non-stretched
 //could be flipped/reversed
 //could need clipping
 goto reverse;
 }
goto stretch;
}//16
}//2

if (passed&32){//...(sx1,sy1)-(sx2,sy2)
if (passed&1){//(dx1,dy1),,(sx1,sy1)-(sx2,sy2)
dx2=dx1+abs(sx2-sx1); dy2=dy1+abs(sy2-sy1);
//can't be stretched
//could be flipped/reversed
//could need clipping
goto reverse;
}else{//(sx1,sy1)-(sx2,sy2)
dx2=dw-1; dy2=dh-1;
//could be stretched
 if ( ((abs(sx2-sx1)+1)==dw) && ((abs(sy2-sy1)+1)==dh) ){//non-stretched
 //could be flipped/reversed
 //could need clipping
 goto reverse;
 }
goto stretch;
}//1
}//32

if (passed&16){error(5); return;}//Invalid: NULL-NULL,?,?,(sx1,sy1)-NULL

if (passed&1){//(dx1,dy1)
sx2=s->width-1; sy2=s->height-1;
dx2=dx1+sx2; dy2=dy1+sy2;
goto clip;
}

//no co-ords given
sx2=s->width-1; sy2=s->height-1;
dx2=d->width-1; dy2=d->height-1;
if ((sx2==dx2)&&(sy2==dy2)){//non-stretched
//note: because 0-size image is illegal, no null size check is necessary
goto noflip;//cannot be reversed
}
 //precalculate required values
 w=dx2-dx1; h=dy2-dy1;
 fsx1=sx1; fsy1=sy1; fsx2=sx2; fsy2=sy2;
 //"pull" corners so all source pixels are evenly represented in dest rect
 if (fsx1<=fsx2){fsx1-=0.499999; fsx2+=0.499999;}else{fsx1+=0.499999; fsx2-=0.499999;}
 if (fsy1<=fsy2){fsy1-=0.499999; fsy2+=0.499999;}else{fsy1+=0.499999; fsy2-=0.499999;}
 //calc source gradients
 if (w) mx=(fsx2-fsx1)/((double)w); else mx=0.0;
 if (h) my=(fsy2-fsy1)/((double)h); else my=0.0;
 //note: mx & my represent the amount of change per dest pixel
goto stretch_noreverse_noclip;

stretch:
//stretch is required

//mirror?
if (dx2<dx1){
if (sx2>sx1) mirror=1;
}
if (sx2<sx1){
if (dx2>dx1) mirror=1;
}
if (dx2<dx1){x=dx1; dx1=dx2; dx2=x;}
if (sx2<sx1){x=sx1; sx1=sx2; sx2=x;}
//flip?
if (dy2<dy1){
if (sy2>sy1) flip=1;
}
if (sy2<sy1){
if (dy2>dy1) flip=1;
}
if (dy2<dy1){y=dy1; dy1=dy2; dy2=y;}
if (sy2<sy1){y=sy1; sy1=sy2; sy2=y;}

w=dx2-dx1; h=dy2-dy1;
fsx1=sx1; fsy1=sy1; fsx2=sx2; fsy2=sy2;
//"pull" corners so all source pixels are evenly represented in dest rect
if (fsx1<=fsx2){fsx1-=0.499999; fsx2+=0.499999;}else{fsx1+=0.499999; fsx2-=0.499999;}
if (fsy1<=fsy2){fsy1-=0.499999; fsy2+=0.499999;}else{fsy1+=0.499999; fsy2-=0.499999;}
//calc source gradients
if (w) mx=(fsx2-fsx1)/((double)w); else mx=0.0;
if (h) my=(fsy2-fsy1)/((double)h); else my=0.0;
//note: mx & my represent the amount of change per dest pixel

//crop dest offscreen pixels
if (dx1<0){
if (mirror) fsx2+=((double)dx1)*mx; else fsx1-=((double)dx1)*mx;
dx1=0;
}
if (dy1<0){
if (flip) fsy2+=((double)dy1)*my; else fsy1-=((double)dy1)*my;
dy1=0;
}
if (dx2>=dw){
if (mirror) fsx1+=((double)(dx2-dw+1))*mx; else fsx2-=((double)(dx2-dw+1))*mx;
dx2=dw-1;
}
if (dy2>=dh){
if (flip) fsy1+=((double)(dy2-dh+1))*my; else fsy2-=((double)(dy2-dh+1))*my;
dy2=dh-1;
}
//crop source offscreen pixels
if (w){//gradient cannot be 0
if (fsx1<-0.4999999){
x=(-fsx1-0.499999)/mx+1.0;
if (mirror) dx2-=x; else dx1+=x;
fsx1+=((double)x)*mx;
}
if (fsx2>(((double)sw)-0.5000001)){
x=(fsx2-(((double)sw)-0.500001))/mx+1.0;
if (mirror) dx1+=x; else dx2-=x;
fsx2-=(((double)x)*mx);
}
}//w
if (h){//gradient cannot be 0
if (fsy1<-0.4999999){
y=(-fsy1-0.499999)/my+1.0;
if (flip) dy2-=y; else dy1+=y;
fsy1+=((double)y)*my;
}
if (fsy2>(((double)sh)-0.5000001)){
y=(fsy2-(((double)sh)-0.500001))/my+1.0;
if (flip) dy1+=y; else dy2-=y;
fsy2-=(((double)y)*my);
}
}//h
//<0-size/offscreen?
//note: <0-size will cause reversal of dest
//      offscreen values will result in reversal of dest
if (dx1>dx2) return;
if (dy1>dy2) return;
//all values are now within the boundries of the source & dest

stretch_noreverse_noclip:
w=dx2-dx1+1; h=dy2-dy1+1;//recalculate based on actual number of pixels

if (sbpp==4){
if (s->alpha_disabled||d->alpha_disabled) goto put_32_noalpha_stretch;
goto put_32_stretch;
}
if (dbpp==1){
if (s->transparent_color==-1) goto put_8_stretch;
goto put_8_clear_stretch;
}
if (s->transparent_color==-1) goto put_8_32_stretch;
goto put_8_32_clear_stretch;

put_32_stretch:
//calc. starting points & change values
if (flip){
 if (mirror){
  doff32=d->offset32+(dy2*dw+dx2);
  dskip=-dw+w;
 }else{
  doff32=d->offset32+(dy2*dw+dx1);
  dskip=-dw-w;
 }
}else{
 if (mirror){
  doff32=d->offset32+(dy1*dw+dx2);
  dskip=dw+w;
 }else{
  doff32=d->offset32+(dy1*dw+dx1);
  dskip=dw-w;
 }
}
if (mirror) xdir=-1; else xdir=1;
//plot rect
yy=h;
fy=fsy1;
fsx1-=mx;//prev value is moved on from
do{
xx=w;
ulp=s->offset32+sw*qbr_double_to_long(fy);
fx=fsx1;
do{
//--------plot pixel--------
switch((col=*(ulp+qbr_double_to_long(fx+=mx)))&0xFF000000){
case 0xFF000000:
 *doff32=col;
break;
case 0x0:
break;
case 0x80000000:
 *doff32=(((*doff32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend128[*doff32>>24]<<24);
break; 
case 0x7F000000:
 *doff32=(((*doff32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend127[*doff32>>24]<<24);
break;
default:
 destcol=*doff32;
 cp=blend+(col>>24<<16);
 *doff32=
   cp[(col<<8&0xFF00)+(destcol&255)    ]
 +(cp[(col&0xFF00)   +(destcol>>8&255) ]<<8)
 +(cp[(col>>8&0xFF00)+(destcol>>16&255)]<<16)
 +(ablend[(col>>24)+(destcol>>16&0xFF00)]<<24);
};//switch
//--------done plot pixel--------
doff32+=xdir;
}while(--xx);
doff32+=dskip;
fy+=my;
}while(--yy);
return;

put_32_noalpha_stretch:
//calc. starting points & change values
if (flip){
 if (mirror){
  doff32=d->offset32+(dy2*dw+dx2);
  dskip=-dw+w;
 }else{
  doff32=d->offset32+(dy2*dw+dx1);
  dskip=-dw-w;
 }
}else{
 if (mirror){
  doff32=d->offset32+(dy1*dw+dx2);
  dskip=dw+w;
 }else{
  doff32=d->offset32+(dy1*dw+dx1);
  dskip=dw-w;
 }
}
if (mirror) xdir=-1; else xdir=1;
//plot rect
yy=h;
fy=fsy1;
fsx1-=mx;//prev value is moved on from
doff32-=xdir;
do{
xx=w;
ulp=s->offset32+sw*qbr_double_to_long(fy);
fx=fsx1;
do{
*(doff32+=xdir)=*(ulp+qbr_double_to_long(fx+=mx));
}while(--xx);
doff32+=dskip;
fy+=my;
}while(--yy);
return;

put_8_stretch:
//calc. starting points & change values
if (flip){
 if (mirror){
  doff=d->offset+(dy2*dw+dx2);
  dskip=-dw+w;
 }else{
  doff=d->offset+(dy2*dw+dx1);
  dskip=-dw-w;
 }
}else{
 if (mirror){
  doff=d->offset+(dy1*dw+dx2);
  dskip=dw+w;
 }else{
  doff=d->offset+(dy1*dw+dx1);
  dskip=dw-w;
 }
}
if (mirror) xdir=-1; else xdir=1;
//plot rect
yy=h;
fy=fsy1;
fsx1-=mx;//prev value is moved on from
doff-=xdir;
do{
xx=w;
cp=s->offset+sw*qbr_double_to_long(fy);
fx=fsx1;
do{
*(doff+=xdir)=*(cp+qbr_double_to_long(fx+=mx));
}while(--xx);
doff+=dskip;
fy+=my;
}while(--yy);
return;

put_8_clear_stretch:
clearcol=s->transparent_color;
//calc. starting points & change values
if (flip){
 if (mirror){
  doff=d->offset+(dy2*dw+dx2);
  dskip=-dw+w;
 }else{
  doff=d->offset+(dy2*dw+dx1);
  dskip=-dw-w;
 }
}else{
 if (mirror){
  doff=d->offset+(dy1*dw+dx2);
  dskip=dw+w;
 }else{
  doff=d->offset+(dy1*dw+dx1);
  dskip=dw-w;
 }
}
if (mirror) xdir=-1; else xdir=1;
//plot rect
yy=h;
fy=fsy1;
fsx1-=mx;//prev value is moved on from
do{
xx=w;
cp=s->offset+sw*qbr_double_to_long(fy);
fx=fsx1;
do{
if ((col=*(cp+qbr_double_to_long(fx+=mx)))!=clearcol){
*doff=col;
}
doff+=xdir;
}while(--xx);
doff+=dskip;
fy+=my;
}while(--yy);
return;

put_8_32_stretch:
pal=s->pal;
//calc. starting points & change values
if (flip){
 if (mirror){
  doff32=d->offset32+(dy2*dw+dx2);
  dskip=-dw+w;
 }else{
  doff32=d->offset32+(dy2*dw+dx1);
  dskip=-dw-w;
 }
}else{
 if (mirror){
  doff32=d->offset32+(dy1*dw+dx2);
  dskip=dw+w;
 }else{
  doff32=d->offset32+(dy1*dw+dx1);
  dskip=dw-w;
 }
}
if (mirror) xdir=-1; else xdir=1;
//plot rect
yy=h;
fy=fsy1;
fsx1-=mx;//prev value is moved on from
doff32-=xdir;
do{
xx=w;
cp=s->offset+sw*qbr_double_to_long(fy);
fx=fsx1;
do{
*(doff32+=xdir)=pal[*(cp+qbr_double_to_long(fx+=mx))];
}while(--xx);
doff32+=dskip;
fy+=my;
}while(--yy);
return;

put_8_32_clear_stretch:
clearcol=s->transparent_color;
pal=s->pal;
//calc. starting points & change values
if (flip){
 if (mirror){
  doff32=d->offset32+(dy2*dw+dx2);
  dskip=-dw+w;
 }else{
  doff32=d->offset32+(dy2*dw+dx1);
  dskip=-dw-w;
 }
}else{
 if (mirror){
  doff32=d->offset32+(dy1*dw+dx2);
  dskip=dw+w;
 }else{
  doff32=d->offset32+(dy1*dw+dx1);
  dskip=dw-w;
 }
}
if (mirror) xdir=-1; else xdir=1;
//plot rect
yy=h;
fy=fsy1;
fsx1-=mx;//prev value is moved on from
do{
xx=w;
cp=s->offset+sw*qbr_double_to_long(fy);
fx=fsx1;
do{
if ((col=*(cp+qbr_double_to_long(fx+=mx)))!=clearcol){
*doff32=pal[col];
}
doff32+=xdir;
}while(--xx);
doff32+=dskip;
fy+=my;
}while(--yy);
return;

reverse:
//mirror?
if (dx2<dx1){
if (sx2>sx1) mirror=1;
}
if (sx2<sx1){
if (dx2>dx1) mirror=1;
}
if (dx2<dx1){x=dx1; dx1=dx2; dx2=x;}
if (sx2<sx1){x=sx1; sx1=sx2; sx2=x;}
//flip?
if (dy2<dy1){
if (sy2>sy1) flip=1;
}
if (sy2<sy1){
if (dy2>dy1) flip=1;
}
if (dy2<dy1){y=dy1; dy1=dy2; dy2=y;}
if (sy2<sy1){y=sy1; sy1=sy2; sy2=y;}

clip:
//crop dest offscreen pixels
if (dx1<0){
if (mirror) sx2+=dx1; else sx1-=dx1;
dx1=0;
}
if (dy1<0){
if (flip) sy2+=dy1; else sy1-=dy1;
dy1=0;
}
if (dx2>=dw){
if (mirror) sx1+=(dx2-dw+1); else sx2-=(dx2-dw+1);
dx2=dw-1;
}
if (dy2>=dh){
if (flip) sy1+=(dy2-dh+1); else sy2-=(dy2-dh+1);
dy2=dh-1;
}
//crop source offscreen pixels
if (sx1<0){
if (mirror) dx2+=sx1; else dx1-=sx1;
sx1=0;
}
if (sy1<0){
if (flip) dy2+=sy1; else dy1-=sy1;
sy1=0;
}
if (sx2>=sw){
if (mirror) dx1+=(sx2-sw+1); else dx2-=(sx2-sw+1);
sx2=sw-1;
}
if (sy2>=sh){
if (flip) dy1+=(sy2-sh+1); else dy2-=(sy2-sh+1);
sy2=sh-1;
}
//<0-size/offscreen?
//note: <0-size will cause reversal of dest
//      offscreen values will result in reversal of dest
if (dx1>dx2) return;
if (dy1>dy2) return;
//all values are now within the boundries of the source & dest

//mirror put
if (mirror){
if (sbpp==4){
if (s->alpha_disabled||d->alpha_disabled) goto put_32_noalpha_mirror;
goto put_32_mirror;
}
if (dbpp==1){
if (s->transparent_color==-1) goto put_8_mirror;
goto put_8_clear_mirror;
}
if (s->transparent_color==-1) goto put_8_32_mirror;
goto put_8_32_clear_mirror;
}//mirror put

noflip:
if (sbpp==4){
if (s->alpha_disabled||d->alpha_disabled) goto put_32_noalpha;
goto put_32;
}
if (dbpp==1){
if (s->transparent_color==-1) goto put_8;
goto put_8_clear;
}
if (s->transparent_color==-1) goto put_8_32;
goto put_8_32_clear;

put_32:
w=dx2-dx1+1;
doff32=d->offset32+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff32=s->offset32+(sy2*sw+sx1);
sskip=-w-sw;
}else{
soff32=s->offset32+(sy1*sw+sx1);
sskip=sw-w;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
//--------plot pixel--------
switch((col=*soff32++)&0xFF000000){
case 0xFF000000:
 *doff32++=col;
break;
case 0x0:
 doff32++;
break;
case 0x80000000:
 *doff32++=(((*doff32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend128[*doff32>>24]<<24);
break; 
case 0x7F000000:
 *doff32++=(((*doff32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend127[*doff32>>24]<<24);
break;
default:
 destcol=*doff32;
 cp=blend+(col>>24<<16);
 *doff32++=
   cp[(col<<8&0xFF00)+(destcol&255)    ]
 +(cp[(col&0xFF00)   +(destcol>>8&255) ]<<8)
 +(cp[(col>>8&0xFF00)+(destcol>>16&255)]<<16)
 +(ablend[(col>>24)+(destcol>>16&0xFF00)]<<24);
};//switch
//--------done plot pixel--------
}while(--xx);
soff32+=sskip; doff32+=dskip;
}while(--h);
return;

put_32_noalpha:
doff32=d->offset32+(dy1*dw+dx1);
if (flip){
soff32=s->offset32+(sy2*sw+sx1);
sskip=-sw;
}else{
soff32=s->offset32+(sy1*sw+sx1);
sskip=sw;
}
h=dy2-dy1+1;
w=(dx2-dx1+1)*4;
while(h--){
memcpy(doff32,soff32,w);
soff32+=sskip; doff32+=dw;
}
return;

put_8:
doff=d->offset+(dy1*dw+dx1);
if (flip){
soff=s->offset+(sy2*sw+sx1);
sskip=-sw;
}else{
soff=s->offset+(sy1*sw+sx1);
sskip=sw;
}
h=dy2-dy1+1;
w=dx2-dx1+1;
while(h--){
memcpy(doff,soff,w);
soff+=sskip; doff+=dw;
}
return;

put_8_clear:
clearcol=s->transparent_color;
w=dx2-dx1+1;
doff=d->offset+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff=s->offset+(sy2*sw+sx1);
sskip=-w-sw;
}else{
soff=s->offset+(sy1*sw+sx1);
sskip=sw-w;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
if ((col=*soff++)!=clearcol){
*doff=col;
}
doff++;
}while(--xx);
soff+=sskip; doff+=dskip;
}while(--h);
return;

put_8_32:
pal=s->pal;
w=dx2-dx1+1;
doff32=d->offset32+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff=s->offset+(sy2*sw+sx1);
sskip=-w-sw;
}else{
soff=s->offset+(sy1*sw+sx1);
sskip=sw-w;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
*doff32++=pal[*soff++];
}while(--xx);
soff+=sskip; doff32+=dskip;
}while(--h);
return;

put_8_32_clear:
pal=s->pal;
clearcol=s->transparent_color;
w=dx2-dx1+1;
doff32=d->offset32+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff=s->offset+(sy2*sw+sx1);
sskip=-w-sw;
}else{
soff=s->offset+(sy1*sw+sx1);
sskip=sw-w;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
if ((col=*soff++)!=clearcol){
*doff32=pal[col];
}
doff32++;
}while(--xx);
soff+=sskip; doff32+=dskip;
}while(--h);
return;

put_32_mirror:
w=dx2-dx1+1;
doff32=d->offset32+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff32=s->offset32+(sy2*sw+sx2);
sskip=-sw+w;
}else{
soff32=s->offset32+(sy1*sw+sx2);
sskip=w+sw;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
//--------plot pixel--------
switch((col=*soff32--)&0xFF000000){
case 0xFF000000:
 *doff32++=col;
break;
case 0x0:
 doff32++;
break;
case 0x80000000:
 *doff32++=(((*doff32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend128[*doff32>>24]<<24);
break; 
case 0x7F000000:
 *doff32++=(((*doff32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend127[*doff32>>24]<<24);
break;
default:
 destcol=*doff32;
 cp=blend+(col>>24<<16);
 *doff32++=
   cp[(col<<8&0xFF00)+(destcol&255)    ]
 +(cp[(col&0xFF00)   +(destcol>>8&255) ]<<8)
 +(cp[(col>>8&0xFF00)+(destcol>>16&255)]<<16)
 +(ablend[(col>>24)+(destcol>>16&0xFF00)]<<24);
};//switch
//--------done plot pixel--------
}while(--xx);
soff32+=sskip; doff32+=dskip;
}while(--h);
return;

put_32_noalpha_mirror:
w=dx2-dx1+1;
doff32=d->offset32+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff32=s->offset32+(sy2*sw+sx2);
sskip=-sw+w;
}else{
soff32=s->offset32+(sy1*sw+sx2);
sskip=w+sw;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
*doff32++=*soff32--;
}while(--xx);
soff32+=sskip; doff32+=dskip;
}while(--h);
return;

put_8_mirror:
w=dx2-dx1+1;
doff=d->offset+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff=s->offset+(sy2*sw+sx2);
sskip=-sw+w;
}else{
soff=s->offset+(sy1*sw+sx2);
sskip=w+sw;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
*doff++=*soff--;
}while(--xx);
soff+=sskip; doff+=dskip;
}while(--h);
return;

put_8_clear_mirror:
clearcol=s->transparent_color;
w=dx2-dx1+1;
doff=d->offset+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff=s->offset+(sy2*sw+sx2);
sskip=-sw+w;
}else{
soff=s->offset+(sy1*sw+sx2);
sskip=w+sw;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
if ((col=*soff--)!=clearcol){
*doff=col;
}
doff++;
}while(--xx);
soff+=sskip; doff+=dskip;
}while(--h);
return;

put_8_32_mirror:
pal=s->pal;
w=dx2-dx1+1;
doff32=d->offset32+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff=s->offset+(sy2*sw+sx2);
sskip=-sw+w;
}else{
soff=s->offset+(sy1*sw+sx2);
sskip=w+sw;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
*doff32++=pal[*soff--];
}while(--xx);
soff+=sskip; doff32+=dskip;
}while(--h);
return;

put_8_32_clear_mirror:
pal=s->pal;
clearcol=s->transparent_color;
w=dx2-dx1+1;
doff32=d->offset32+(dy1*dw+dx1);
dskip=dw-w;
if (flip){
soff=s->offset+(sy2*sw+sx2);
sskip=-sw+w;
}else{
soff=s->offset+(sy1*sw+sx2);
sskip=w+sw;
}
//plot rect
h=dy2-dy1+1;
do{
xx=w;
do{
if ((col=*soff--)!=clearcol){
*doff32=pal[col];
}
doff32++;
}while(--xx);
soff+=sskip; doff32+=dskip;
}while(--h);
return;

}//done imgput

//font management
#define lastfont 1023
TTF_Font *font[lastfont+1];//NULL=unused index
long fontheight[lastfont+1];
long fontwidth[lastfont+1];
long fontflags[lastfont+1];

long fontopen(char *name,double d_height,long flags){
//flags:
//1 bold TTF_STYLE_BOLD
//2 italic TTF_STYLE_ITALIC
//4 underline TTF_STYLE_UNDERLINE
//8 dontblend (blending is the default in 32-bit alpha-enabled modes)
//16 monospace
static double d,d2;
static long i,y,z,height;
static TTF_Font *tf,*tf2;
static SDL_Surface *ts;
static SDL_Color c;
for (i=32;i<=lastfont;i++){
if (!font[i]){
if (d_height<1.0){
height=d_height*1000;
tf=TTF_OpenFont(name,height);
if (tf==NULL) return 0;
height=TTF_FontHeight(tf);
}else{
height=qbr_double_to_long(d_height);
tf=TTF_OpenFont(name,1000);
if (tf==NULL) return 0;
d=TTF_FontHeight(tf);
d=1000.0/d;
d2=height;
d2*=d;
y=d2;
TTF_CloseFont(tf);
tf=TTF_OpenFont(name,y);
if (tf==NULL) return 0;
if (TTF_FontHeight(tf)>height){
TTF_CloseFont(tf);
y--; tf=TTF_OpenFont(name,y);
if (tf==NULL) return 0;
}
if (TTF_FontHeight(tf)<height){
TTF_CloseFont(tf);
y++; tf=TTF_OpenFont(name,y);
if (tf==NULL) return 0;
}
}//d_height
TTF_SetFontStyle(tf,flags&7);//returns void, so cannot be error checked
font[i]=tf;
fontflags[i]=flags;
fontheight[i]=height;
fontwidth[i]=0;
if (flags&16){
if (TTF_FontFaceIsFixedWidth(tf)){
//render a glyph to test
z=32;
c.r=255; c.g=255; c.b=255; c.unused=0;//dummy values
ts=TTF_RenderText_Solid(tf,(char*)&z,c);//8-bit, 0=clear, 1=text
fontwidth[i]=ts->w;
SDL_FreeSurface(ts);
}else{
TTF_CloseFont(tf);//not a monospace font
return 0;
}
}
return i;
}
}
return 0;//no valid index, will result in ILLEGAL FUNCTION CALL
}

long selectfont(long f,img_struct *im){
im->font=f;
im->cursor_x=1; im->cursor_y=1;
im->top_row=1;
if (im->compatible_mode) im->bottom_row=im->height/fontheight[f]; else im->bottom_row=im->height;
im->bottom_row--; if (im->bottom_row<=0) im->bottom_row=1;
return 1;//success
}
















SDL_Rect **modes;
long nmodes=0;
long anymode=0;

long x_scale=1,y_scale=1;
long x_offset=0,y_offset=0;
long x_monitor=0,y_monitor=0;


#define AUDIO_CHANNELS 256

#define sndqueue_lastindex 9999
unsigned long sndqueue[sndqueue_lastindex+1];
long sndqueue_next=0;
long sndqueue_first=0;
long sndqueue_wait=-1;
long sndqueue_played=0;

unsigned long func__sndraw(unsigned char* data,unsigned long bytes);//called by sndsetup
void sndsetup(){
static long sndsetup_called=0;
if (!sndsetup_called){
sndsetup_called=1;
if (Mix_OpenAudio(22050,AUDIO_S16,2,1024)==-1) exit(10001);
atexit(Mix_CloseAudio);
Mix_AllocateChannels(AUDIO_CHANNELS);
 //fix "part of first sound missed" problem in SDL_MIXER
 static unsigned char *cp=(unsigned char*)calloc(8192,2*2);
 static long i;
 i=func__sndraw(cp,8192*2*2);
 if (i){
 sndqueue[sndqueue_next]=i;
 sndqueue_next++; if (sndqueue_next>sndqueue_lastindex) sndqueue_next=0;
 }
}
}



void call_interrupt(long i);

unsigned long frame=0;


extern unsigned char cmem[1114099];//16*65535+65535+3 (enough for highest referencable dword in conv memory)

struct mouse_message{
short x;
short y;
unsigned long buttons;
};
mouse_message mouse_messages[1024];//a circular buffer of mouse messages
long last_mouse_message=0;
long current_mouse_message=0;
long mouse_hideshow_called=0;




//x86 Virtual CMEM emulation
//Note: x86 CPU emulation is still experimental and is not available in QB64 yet.
struct cpu_struct{
 //al,ah,ax,eax (unsigned & signed)
 union{
  struct{
   union{
   uint8 al;
   int8 al_signed;
  };
  union{
   uint8 ah;
   int8 ah_signed;
  };
 };
 uint16 ax;
 int16 ax_signed;
 uint32 eax;
 int32 eax_signed;
 };
 //bl,bh,bx,ebx (unsigned & signed)
 union{
  struct{
   union{
   uint8 bl;
   int8 bl_signed;
  };
  union{
   uint8 bh;
   int8 bh_signed;
  };
 };
 uint16 bx;
 int16 bx_signed;
 uint32 ebx;
 int32 ebx_signed;
 };
 //cl,ch,cx,ecx (unsigned & signed)
 union{
  struct{
   union{
   uint8 cl;
   int8 cl_signed;
  };
  union{
   uint8 ch;
   int8 ch_signed;
  };
 };
 uint16 cx;
 int16 cx_signed;
 uint32 ecx;
 int32 ecx_signed;
 };
 //dl,dh,dx,edx (unsigned & signed)
 union{
  struct{
   union{
   uint8 dl;
   int8 dl_signed;
  };
  union{
   uint8 dh;
   int8 dh_signed;
  };
 };
 uint16 dx;
 int16 dx_signed;
 uint32 edx;
 int32 edx_signed;
 };
 //si,esi (unsigned & signed)
 union{
 uint16 si;
 int16 si_signed;
 uint32 esi;
 int32 esi_signed;
 };
 //di,edi (unsigned & signed)
 union{
 uint16 di;
 int16 di_signed;
 uint32 edi;
 int32 edi_signed;
 };
 //bp,ebp (unsigned & signed)
 union{
 uint16 bp;
 int16 bp_signed;
 uint32 ebp;
 int32 ebp_signed;
 };
 //sp,esp (unsigned & signed)
 union{
 uint16 sp;
 int16 sp_signed;
 uint32 esp;
 int32 esp_signed;
 };
 //cs,ss,ds,es,fs,gs (unsigned & signed)
 union{
 uint16 cs;
 uint16 cs_signed;
 };
 union{
 uint16 ss;
 uint16 ss_signed;
 };
 union{
 uint16 ds;
 uint16 ds_signed;
 };
 union{
 uint16 es;
 uint16 es_signed;
 };
 union{
 uint16 fs;
 uint16 fs_signed;
 };
 union{
 uint16 gs;
 uint16 gs_signed;
 };
 //ip,eip (unsigned & signed)
 union{
 uint16 ip;
 uint16 ip_signed;
 uint32 eip;
 uint32 eip_signed;
 };
 //flags
 uint8 overflow_flag;
 uint8 direction_flag;
 uint8 interrupt_flag;
 uint8 trap_flag;
 uint8 sign_flag;
 uint8 zero_flag;
 uint8 auxiliary_flag;
 uint8 parity_flag;
 uint8 carry_flag;
};
cpu_struct cpu;

uint8 *ip;
uint8 *seg;
uint8 *reg8[8];
uint16 *reg16[8];
uint32 *reg32[8];
uint16 *segreg[8];
uint16 *sp;

long a32;
long b32;//size of data to read/write in bits is 32


uint32 sib(){
static uint32 i;//sib byte
i=*ip++;
switch(i>>6){
case 0:
return *reg32[i&7]+*reg32[i>>3&7];
break;
case 1:
return *reg32[i&7]+(*reg32[i>>3&7]<<1);
break;
case 2:
return *reg32[i&7]+(*reg32[i>>3&7]<<2);
break;
case 3:
return *reg32[i&7]+(*reg32[i>>3&7]<<3);
break;
}
}

uint32 sib_mod0(){
//Note: Called when top 2 bits of rm byte before sib byte were 0, base register is ignored
//      and replaced with an int32 following the sib byte
static uint32 i;//sib byte
i=*ip++;
if ((i&7)==5){
 switch(i>>6){
 case 0:
 return (*(uint32*)((ip+=4)-4))+*reg32[i>>3&7];
 break;
 case 1:
 return (*(uint32*)((ip+=4)-4))+(*reg32[i>>3&7]<<1);
 break;
 case 2:
 return (*(uint32*)((ip+=4)-4))+(*reg32[i>>3&7]<<2);
 break;
 case 3:
 return (*(uint32*)((ip+=4)-4))+(*reg32[i>>3&7]<<3);
 break;
 }
}
switch(i>>6){
case 0:
return *reg32[i&7]+*reg32[i>>3&7];
break;
case 1:
return *reg32[i&7]+(*reg32[i>>3&7]<<1);
break;
case 2:
return *reg32[i&7]+(*reg32[i>>3&7]<<2);
break;
case 3:
return *reg32[i&7]+(*reg32[i>>3&7]<<3);
break;
}
}

uint8 *rm8(){
static uint32 i;//r/m byte
i=*ip++;
switch(i>>6){
case 3:
 return reg8[i&7];
break;
case 0:
 if (a32){
  switch(i&7){
   case 0: return seg+cpu.ax; break;
   case 1: return seg+cpu.cx; break;
   case 2: return seg+cpu.dx; break;
   case 3: return seg+cpu.bx; break;
   case 4: return seg+(uint16)sib_mod0(); break;
   case 5: return seg+(*(uint16*)((ip+=4)-4)); break;
   case 6: return seg+cpu.si; break;
   case 7: return seg+cpu.di; break;
  }
 }else{
  switch(i&7){
   case 0: return seg+((uint16)(cpu.bx+cpu.si)); break;
   case 1: return seg+((uint16)(cpu.bx+cpu.di)); break;
   case 2: return seg+((uint16)(cpu.bp+cpu.si)); break;
   case 3: return seg+((uint16)(cpu.bp+cpu.di)); break;
   case 4: return seg+cpu.si; break;
   case 5: return seg+cpu.di; break;
   case 6: return seg+(*(uint16*)((ip+=2)-2)); break;
   case 7: return seg+cpu.bx; break;
  }
 }
break;
case 1:
 if (a32){
  switch(i&7){
   case 0: return seg+((uint16)(cpu.eax+*(int8*)ip++)); break;
   case 1: return seg+((uint16)(cpu.ecx+*(int8*)ip++)); break;
   case 2: return seg+((uint16)(cpu.edx+*(int8*)ip++)); break;
   case 3: return seg+((uint16)(cpu.ebx+*(int8*)ip++)); break;
   case 4: i=sib(); return seg+((uint16)(i+*(int8*)ip++)); break;
   case 5: return seg+((uint16)(cpu.ebp+*(int8*)ip++)); break;
   case 6: return seg+((uint16)(cpu.esi+*(int8*)ip++)); break;
   case 7: return seg+((uint16)(cpu.edi+*(int8*)ip++)); break;
  }
 }else{
  switch(i&7){
   case 0: return seg+((uint16)(cpu.bx+cpu.si+*(int8*)ip++)); break;
   case 1: return seg+((uint16)(cpu.bx+cpu.di+*(int8*)ip++)); break;
   case 2: return seg+((uint16)(cpu.bp+cpu.si+*(int8*)ip++)); break;
   case 3: return seg+((uint16)(cpu.bp+cpu.di+*(int8*)ip++)); break;
   case 4: return seg+((uint16)(cpu.si+*(int8*)ip++)); break;
   case 5: return seg+((uint16)(cpu.di+*(int8*)ip++)); break;
   case 6: return seg+((uint16)(cpu.bp+*(int8*)ip++)); break;
   case 7: return seg+((uint16)(cpu.bx+*(int8*)ip++)); break;
  }
 }
break;
case 2:
 if (a32){ 
  switch(i&7){
   case 0: return seg+((uint16)(cpu.eax+*(uint32*)((ip+=4)-4))); break;
   case 1: return seg+((uint16)(cpu.ecx+*(uint32*)((ip+=4)-4))); break;
   case 2: return seg+((uint16)(cpu.edx+*(uint32*)((ip+=4)-4))); break;
   case 3: return seg+((uint16)(cpu.ebx+*(uint32*)((ip+=4)-4))); break;
   case 4: i=sib(); return seg+((uint16)(i+*(uint32*)((ip+=4)-4))); break;
   case 5: return seg+((uint16)(cpu.ebp+*(uint32*)((ip+=4)-4))); break;
   case 6: return seg+((uint16)(cpu.esi+*(uint32*)((ip+=4)-4))); break;
   case 7: return seg+((uint16)(cpu.edi+*(uint32*)((ip+=4)-4))); break;
  }
 }else{
  switch(i&7){
   case 0: return seg+((uint16)(cpu.bx+cpu.si+*(uint16*)((ip+=2)-2))); break;
   case 1: return seg+((uint16)(cpu.bx+cpu.di+*(uint16*)((ip+=2)-2))); break;
   case 2: return seg+((uint16)(cpu.bp+cpu.si+*(uint16*)((ip+=2)-2))); break;
   case 3: return seg+((uint16)(cpu.bp+cpu.di+*(uint16*)((ip+=2)-2))); break;
   case 4: return seg+((uint16)(cpu.si+*(uint16*)((ip+=2)-2))); break;
   case 5: return seg+((uint16)(cpu.di+*(uint16*)((ip+=2)-2))); break;
   case 6: return seg+((uint16)(cpu.bp+*(uint16*)((ip+=2)-2))); break;
   case 7: return seg+((uint16)(cpu.bx+*(uint16*)((ip+=2)-2))); break;
  }
 }
break;
}
}

uint16 *rm16(){
static long i;//r/m byte
i=*ip;
switch(i>>6){
case 3:
 ip++; 
 return reg16[i&7];
break;
case 0:
 ip++;
 if (a32){
  switch(i&7){
   case 0: return (uint16*)(seg+cpu.ax); break;
   case 1: return (uint16*)(seg+cpu.cx); break;
   case 2: return (uint16*)(seg+cpu.dx); break;
   case 3: return (uint16*)(seg+cpu.bx); break;   
   case 4: return (uint16*)(seg+(uint16)sib_mod0()); break;
   case 5: return (uint16*)(seg+(*(uint16*)((ip+=4)-4))); break;
   case 6: return (uint16*)(seg+cpu.si); break;
   case 7: return (uint16*)(seg+cpu.di); break;
  }
 }else{
  switch(i&7){
   case 0: return (uint16*)(seg+((uint16)(cpu.bx+cpu.si))); break;
   case 1: return (uint16*)(seg+((uint16)(cpu.bx+cpu.di))); break;
   case 2: return (uint16*)(seg+((uint16)(cpu.bp+cpu.si))); break;
   case 3: return (uint16*)(seg+((uint16)(cpu.bp+cpu.di))); break;
   case 4: return (uint16*)(seg+cpu.si); break;
   case 5: return (uint16*)(seg+cpu.di); break;
   case 6: return (uint16*)(seg+(*(uint16*)((ip+=2)-2))); break;
   case 7: return (uint16*)(seg+cpu.bx); break;
  }
 }
break;
case 1:
 ip++;
 if (a32){ 
  switch(i&7){
   case 0: return (uint16*)(seg+((uint16)(cpu.eax+*(int8*)ip++))); break;
   case 1: return (uint16*)(seg+((uint16)(cpu.ecx+*(int8*)ip++))); break;
   case 2: return (uint16*)(seg+((uint16)(cpu.edx+*(int8*)ip++))); break;
   case 3: return (uint16*)(seg+((uint16)(cpu.ebx+*(int8*)ip++))); break;
   case 4: i=sib(); return (uint16*)(seg+((uint16)(i+*(int8*)ip++))); break;
   case 5: return (uint16*)(seg+((uint16)(cpu.ebp+*(int8*)ip++))); break;
   case 6: return (uint16*)(seg+((uint16)(cpu.esi+*(int8*)ip++))); break;
   case 7: return (uint16*)(seg+((uint16)(cpu.edi+*(int8*)ip++))); break;
  }
 }else{
  switch(i&7){
   case 0: return (uint16*)(seg+((uint16)(cpu.bx+cpu.si+*(int8*)ip++))); break;
   case 1: return (uint16*)(seg+((uint16)(cpu.bx+cpu.di+*(int8*)ip++))); break;
   case 2: return (uint16*)(seg+((uint16)(cpu.bp+cpu.si+*(int8*)ip++))); break;
   case 3: return (uint16*)(seg+((uint16)(cpu.bp+cpu.di+*(int8*)ip++))); break;
   case 4: return (uint16*)(seg+((uint16)(cpu.si+*(int8*)ip++))); break;
   case 5: return (uint16*)(seg+((uint16)(cpu.di+*(int8*)ip++))); break;
   case 6: return (uint16*)(seg+((uint16)(cpu.bp+*(int8*)ip++))); break;
   case 7: return (uint16*)(seg+((uint16)(cpu.bx+*(int8*)ip++))); break;
  }
 }
break;
case 2:
 ip++;
 if (a32){ 
  switch(i&7){
   case 0: return (uint16*)(seg+((uint16)(cpu.eax+*(uint32*)((ip+=4)-4)))); break;
   case 1: return (uint16*)(seg+((uint16)(cpu.ecx+*(uint32*)((ip+=4)-4)))); break;
   case 2: return (uint16*)(seg+((uint16)(cpu.edx+*(uint32*)((ip+=4)-4)))); break;
   case 3: return (uint16*)(seg+((uint16)(cpu.ebx+*(uint32*)((ip+=4)-4)))); break;
   case 4: i=sib(); return (uint16*)(seg+((uint16)(i+*(uint32*)((ip+=4)-4)))); break;
   case 5: return (uint16*)(seg+((uint16)(cpu.ebp+*(uint32*)((ip+=4)-4)))); break;
   case 6: return (uint16*)(seg+((uint16)(cpu.esi+*(uint32*)((ip+=4)-4)))); break;
   case 7: return (uint16*)(seg+((uint16)(cpu.edi+*(uint32*)((ip+=4)-4)))); break;
  }
 }else{
  switch(i&7){
   case 0: return (uint16*)(seg+((uint16)(cpu.bx+cpu.si+*(uint16*)((ip+=2)-2)))); break;
   case 1: return (uint16*)(seg+((uint16)(cpu.bx+cpu.di+*(uint16*)((ip+=2)-2)))); break;
   case 2: return (uint16*)(seg+((uint16)(cpu.bp+cpu.si+*(uint16*)((ip+=2)-2)))); break;
   case 3: return (uint16*)(seg+((uint16)(cpu.bp+cpu.di+*(uint16*)((ip+=2)-2)))); break;
   case 4: return (uint16*)(seg+((uint16)(cpu.si+*(uint16*)((ip+=2)-2)))); break;
   case 5: return (uint16*)(seg+((uint16)(cpu.di+*(uint16*)((ip+=2)-2)))); break;
   case 6: return (uint16*)(seg+((uint16)(cpu.bp+*(uint16*)((ip+=2)-2)))); break;
   case 7: return (uint16*)(seg+((uint16)(cpu.bx+*(uint16*)((ip+=2)-2)))); break;
  }
 }
break;
}
}

uint32 *rm32(){
static long i;//r/m byte
i=*ip;
switch(i>>6){
case 3:
 ip++; 
 return reg32[i&7];
break;
case 0:
 ip++;
 if (a32){
  switch(i&7){
   case 0: return (uint32*)(seg+cpu.ax); break;
   case 1: return (uint32*)(seg+cpu.cx); break;
   case 2: return (uint32*)(seg+cpu.dx); break;
   case 3: return (uint32*)(seg+cpu.bx); break;
   case 4: return (uint32*)(seg+(uint16)sib_mod0()); break;
   case 5: return (uint32*)(seg+(*(uint16*)((ip+=4)-4))); break;
   case 6: return (uint32*)(seg+cpu.si); break;
   case 7: return (uint32*)(seg+cpu.di); break;
  }
 }else{
  switch(i&7){
   case 0: return (uint32*)(seg+((uint16)(cpu.bx+cpu.si))); break;
   case 1: return (uint32*)(seg+((uint16)(cpu.bx+cpu.di))); break;
   case 2: return (uint32*)(seg+((uint16)(cpu.bp+cpu.si))); break;
   case 3: return (uint32*)(seg+((uint16)(cpu.bp+cpu.di))); break;
   case 4: return (uint32*)(seg+cpu.si); break;
   case 5: return (uint32*)(seg+cpu.di); break;
   case 6: return (uint32*)(seg+(*(uint16*)((ip+=2)-2))); break;
   case 7: return (uint32*)(seg+cpu.bx); break;
  }
 }
break;
case 1:
 ip++;
 if (a32){ 
  switch(i&7){
   case 0: return (uint32*)(seg+((uint16)(cpu.eax+*(int8*)ip++))); break;
   case 1: return (uint32*)(seg+((uint16)(cpu.ecx+*(int8*)ip++))); break;
   case 2: return (uint32*)(seg+((uint16)(cpu.edx+*(int8*)ip++))); break;
   case 3: return (uint32*)(seg+((uint16)(cpu.ebx+*(int8*)ip++))); break;
   case 4: i=sib(); return (uint32*)(seg+((uint16)(i+*(int8*)ip++))); break;
   case 5: return (uint32*)(seg+((uint16)(cpu.ebp+*(int8*)ip++))); break;
   case 6: return (uint32*)(seg+((uint16)(cpu.esi+*(int8*)ip++))); break;
   case 7: return (uint32*)(seg+((uint16)(cpu.edi+*(int8*)ip++))); break;
  }
 }else{
  switch(i&7){
   case 0: return (uint32*)(seg+((uint16)(cpu.bx+cpu.si+*(int8*)ip++))); break;
   case 1: return (uint32*)(seg+((uint16)(cpu.bx+cpu.di+*(int8*)ip++))); break;
   case 2: return (uint32*)(seg+((uint16)(cpu.bp+cpu.si+*(int8*)ip++))); break;
   case 3: return (uint32*)(seg+((uint16)(cpu.bp+cpu.di+*(int8*)ip++))); break;
   case 4: return (uint32*)(seg+((uint16)(cpu.si+*(int8*)ip++))); break;
   case 5: return (uint32*)(seg+((uint16)(cpu.di+*(int8*)ip++))); break;
   case 6: return (uint32*)(seg+((uint16)(cpu.bp+*(int8*)ip++))); break;
   case 7: return (uint32*)(seg+((uint16)(cpu.bx+*(int8*)ip++))); break;
  }
 }
break;
case 2:
 ip++;
 if (a32){ 
  switch(i&7){
   case 0: return (uint32*)(seg+((uint16)(cpu.eax+*(uint32*)((ip+=4)-4)))); break;
   case 1: return (uint32*)(seg+((uint16)(cpu.ecx+*(uint32*)((ip+=4)-4)))); break;
   case 2: return (uint32*)(seg+((uint16)(cpu.edx+*(uint32*)((ip+=4)-4)))); break;
   case 3: return (uint32*)(seg+((uint16)(cpu.ebx+*(uint32*)((ip+=4)-4)))); break;
   case 4: i=sib(); return (uint32*)(seg+((uint16)(i+*(uint32*)((ip+=4)-4)))); break;
   case 5: return (uint32*)(seg+((uint16)(cpu.ebp+*(uint32*)((ip+=4)-4)))); break;
   case 6: return (uint32*)(seg+((uint16)(cpu.esi+*(uint32*)((ip+=4)-4)))); break;
   case 7: return (uint32*)(seg+((uint16)(cpu.edi+*(uint32*)((ip+=4)-4)))); break;
  }
 }else{
  switch(i&7){
   case 0: return (uint32*)(seg+((uint16)(cpu.bx+cpu.si+*(uint16*)((ip+=2)-2)))); break;
   case 1: return (uint32*)(seg+((uint16)(cpu.bx+cpu.di+*(uint16*)((ip+=2)-2)))); break;
   case 2: return (uint32*)(seg+((uint16)(cpu.bp+cpu.si+*(uint16*)((ip+=2)-2)))); break;
   case 3: return (uint32*)(seg+((uint16)(cpu.bp+cpu.di+*(uint16*)((ip+=2)-2)))); break;
   case 4: return (uint32*)(seg+((uint16)(cpu.si+*(uint16*)((ip+=2)-2)))); break;
   case 5: return (uint32*)(seg+((uint16)(cpu.di+*(uint16*)((ip+=2)-2)))); break;
   case 6: return (uint32*)(seg+((uint16)(cpu.bp+*(uint16*)((ip+=2)-2)))); break;
   case 7: return (uint32*)(seg+((uint16)(cpu.bx+*(uint16*)((ip+=2)-2)))); break;
  }
 }
break;
}
}

uint8* seg_es_ptr;
uint8* seg_cs_ptr;
uint8* seg_ss_ptr;
uint8* seg_ds_ptr;
uint8* seg_fs_ptr;
uint8* seg_gs_ptr;

#define seg_es 0
#define seg_cs 1
#define seg_ss 2
#define seg_ds 3
#define seg_fs 4
#define seg_gs 5


#define op_r i&7
void cpu_call(){

static long i,i2,i3,x,x2,x3,y,y2,y3;
static unsigned char b,b2,b3;
static uint8 *uint8p;
static uint16 *uint16p;
static uint32 *uint32p;
static uint8* dseg;
static long r;
ip=(uint8*)&cmem[cpu.cs*16+cpu.ip];
seg=(uint8*)&cmem[cpu.ds];
sp=(uint16*)&cmem[cpu.ss*16+cpu.sp];


seg_es_ptr=(uint8*)cmem+cpu.es*16;
seg_cs_ptr=(uint8*)cmem+cpu.cs*16;
seg_ss_ptr=(uint8*)cmem+cpu.ss*16;
seg_ds_ptr=(uint8*)cmem+cpu.ds*16;
seg_fs_ptr=(uint8*)cmem+cpu.fs*16;
seg_gs_ptr=(uint8*)cmem+cpu.gs*16;


next_opcode:
b32=0; a32=0; seg=seg_ds_ptr;
i=*ip++;



//read any prefixes
if (i==0x66){b32=1; i=*ip++;}
if (i==0x26){seg=seg_es_ptr; i=*ip++;}
if (i==0x2E){seg=seg_cs_ptr; i=*ip++;}
if (i==0x36){seg=seg_ss_ptr; i=*ip++;}
if (i==0x3E){seg=seg_ds_ptr; i=*ip++;}
if (i==0x64){seg=seg_fs_ptr; i=*ip++;}
if (i==0x65){seg=seg_gs_ptr; i=*ip++;}
if (i==0x67){a32=1; i=*ip++;}

if (i==0x0F) goto opcode_0F;

r=*ip>>3&7;



//mov
if (i!=0x8D){
if (i>=0x88&&i<=0x8E){
switch(i){
case 0x88:// /r r/m8,r8
*rm8()=*reg8[r];
break;
case 0x89:// /r r/m16(32),r16(32)
if (b32) *rm32()=*reg32[r]; else *rm16()=*reg16[r];
break;
case 0x8A:// /r r8,r/m8
*reg8[r]=*rm8();
break;
case 0x8B:// /r r16(32),r/m16(32)
if (b32) *reg32[r]=*rm32(); else *reg16[r]=*rm16();
break;
case 0x8C:// /r r/m16,Sreg
*rm16()=*segreg[r];
break;
case 0x8E:// /r Sreg,r/m16
*segreg[r]=*rm16();
if (r==0) seg_es_ptr=(uint8*)cmem+*segreg[r]*16;
//CS (r==1) cannot be set
if (r==2) seg_ss_ptr=(uint8*)cmem+*segreg[r]*16;
if (r==3) seg_ds_ptr=(uint8*)cmem+*segreg[r]*16;
if (r==4) seg_fs_ptr=(uint8*)cmem+*segreg[r]*16;
if (r==5) seg_gs_ptr=(uint8*)cmem+*segreg[r]*16;
break;
}
goto done;
}
}
if (i>=0xA0&&i<=0xA3){
switch(i){
case 0xA0:// al,moffs8
cpu.al=*(seg+*(uint16*)ip); ip+=2;
break;
case 0xA1:// (e)ax,moffs16(32)
if (b32){cpu.eax=*(uint32*)(seg+*(uint16*)ip); ip+=2;}else{cpu.ax=*(uint16*)(seg+*(uint16*)ip); ip+=2;}
break;
case 0xA2:// moffs8,al
*(seg+*(uint16*)ip)=cpu.al; ip+=2;
break;
case 0xA3:// moffs16(32),(e)ax
if (b32){*(uint32*)(seg+*(uint16*)ip)=cpu.eax; ip+=2;}else{*(uint16*)(seg+*(uint16*)ip)=cpu.ax; ip+=2;}
break;
}
goto done;
}
if (i>=0xB0&&i<=0xB7){// +rb reg8,imm8
*reg8[op_r]=*ip++;
goto done;
}
if (i>=0xB8&&i<=0xBF){// +rw(rd) reg16(32),imm16(32)
if (b32){*reg32[op_r]=*(uint32*)ip; ip+=4;}else{*reg16[op_r]=*(uint16*)ip; ip+=2;}
goto done;
}
if (i==0xC6){// r/m8,imm8
uint8p=rm8(); *uint8p=*ip++;
goto done;
}
if (i==0xC7){// r/m16(32),imm16(32)
if (b32){uint32p=rm32(); *uint32p=*(uint32*)ip; ip+=4;}else{uint16p=rm16(); *uint16p=*(uint16*)ip; ip+=2;}
goto done;
}

//ret (todo)
if (i==0xCB){//(far)
//assume return control (revise later)
return;
}
if (i==0xCA){//imm16 (far)
//assume return control (revise later)
return;
}

//int (todo)
if (i==0xCD){
call_interrupt(*ip++);//assume interrupt table is 0xFFFF
goto done;
}

//push
if (i==0xFF){
if (b32){sp-=2; *(uint32*)sp=*rm32();}else{*--sp=*rm16();}
goto done;
}
if (i>=0x50&&i<=0x57){//+ /r r16(32)
if (b32){sp-=2; *(uint32*)sp=*reg32[op_r];}else{*--sp=*reg16[op_r];}
goto done;
}
if (i==0x6A){//imm8 (sign extended to 16 bits)
*--sp=((int8)*ip++);
goto done;
}
if (i==0x68){//imm16(32)
if (b32){sp-=2; *(uint32*)sp=*(uint32*)ip; ip+=4;}else{*--sp=*(uint16*)ip; ip+=2;}
goto done;
}
if (i==0x0E){//CS
*--sp=*segreg[seg_cs];
goto done;
}
if (i==0x16){//SS
*--sp=*segreg[seg_ss];
goto done;
}
if (i==0x1E){//DS
*--sp=*segreg[seg_ds];
goto done;
}
if (i==0x06){//ES
*--sp=*segreg[seg_es];
goto done;
}

//pop
if (i==0x8F){
if (b32){*rm32()=*(uint32*)sp; sp+=2;}else{*rm16()=*sp++;}
goto done;
}
if (i>=0x58&&i<=0x5F){//+rw(d) r16(32)
if (b32){*reg32[op_r]=*(uint32*)sp; sp+=2;}else{*reg16[op_r]=*sp++;}
goto done;
}
if (i==0x1F){//DS
*segreg[seg_ds]=*sp++;
goto done;
}
if (i==0x07){//ES
*segreg[seg_es]=*sp++;
goto done;
}
if (i==0x17){//SS
*segreg[seg_ss]=*sp++;
goto done;
}



goto skip_0F_opcodes;
opcode_0F:
i=*ip++;
r=*ip>>3&7; //required???

//push
if (i==0xA0){
*--sp=*segreg[seg_fs];
goto done;
}
if (i==0xA8){
*--sp=*segreg[seg_gs];
goto done;
}

//pop
if (i==0xA1){//FS
*segreg[seg_fs]=*sp++;
goto done;
}
if (i==0xA9){//GS
*segreg[seg_gs]=*sp++;
goto done;
}


skip_0F_opcodes:






MessageBox(NULL,"Unknown Opcode","X86 Error",MB_OK);
exit(i);
done:
if (*ip) goto next_opcode;


exit(cmem[0]);


}



long screen_last_valid=0;
unsigned char *screen_last=(unsigned char*)malloc(1);
unsigned long screen_last_size=1;
unsigned long pal_last[256];

uint64 asciicode_value=0;
long asciicode_reading=0;
long asciicode_force=0;


long lock_display=0;
long lock_display_required=0;

SDL_Thread *thread;

//cost delay, made obselete by managing thread priorities (consider removal)
#define cost_limit 10000
#define cost_delay 0
unsigned long cost=0;

#include "msbin.c"

//#include "time64.c"
//#include "time64.h"

int QBMAIN(void *unused);

int64 build_int64(unsigned long val2,unsigned long val1){
static int64 val;
val=val2;
val<<=32;
val|=val1;
return val;
}

uint64 build_uint64(unsigned long val2,unsigned long val1){
static uint64 val;
val=val2;
val<<=32;
val|=val1;
return val;
}


struct byte_element_struct
{
uint64 offset;
unsigned long length;
};




//nb. abreviations are used in variable names to save typing, here are some of the expansions
//cmem=conventional memory
//qbs=qbick basic string (refers to the emulation of quick basic strings)
//sp=stack pointer
//dblock=a 64K memory block in conventional memory holding single variables and strings
extern unsigned char *cmem_static_pointer;
unsigned char *cmem_static_base=&cmem[0]+1280+65536;
extern unsigned char *cmem_dynamic_base;
//[1280][DBLOCK][STATIC-><-DYNAMIC][A000-]

unsigned long qbs_cmem_descriptor_space=256; //enough for 64 strings before expansion

extern unsigned long qbs_cmem_sp; //=256;
extern unsigned long cmem_sp; //=65536;
extern unsigned long dblock; //32bit offset of dblock
extern uint64 *nothingvalue;

unsigned long qb64_firsttimervalue;
unsigned long sdl_firsttimervalue;


extern unsigned char qbevent;

unsigned char wait_needed=1;

SDL_Surface * screen;
long full_screen=0;//0,1(stretched/closest),2(1:1)
long full_screen_toggle=0;//increments each time ALT+ENTER is pressed
long full_screen_set=-1;//0(windowed),1(stretched/closest),2(1:1)


long vertical_retrace_in_progress=0;
long vertical_retrace_happened=0;




static const char *arrow[] = {
  /* width height num_colors chars_per_pixel */
  "    32    32        3            1",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
"X                               ",
"XX                              ",
"X.X                             ",
"X..X                            ",
"X...X                           ",
"X....X                          ",
"X.....X                         ",
"X......X                        ",
"X.......X                       ",
"X........X                      ",
"X.........X                     ",
"X......XXXXX                    ",
"X...X..X                        ",
"X..XX..X                        ",
"X.X  X..X                       ",
"XX   X..X                       ",
"X     X..X                      ",
"      X..X                      ",
"       X..X                     ",
"       X..X                     ",
"        XX                      ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
"                                ",
  "0,0"
};

static SDL_Cursor *init_system_cursor(const char *image[])
{
  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];
  int hot_x, hot_y;

  i = -1;
  for ( row=0; row<32; ++row ) {
    for ( col=0; col<32; ++col ) {
      if ( col % 8 ) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[4+row][col]) {
        case 'X':
          data[i] |= 0x01;
          mask[i] |= 0x01;//?
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}



unsigned char lock_subsystem=0;

extern unsigned char close_program; //=0;
unsigned char program_wait=0;

extern unsigned char suspend_program;
extern unsigned char stop_program;


long global_counter=0;
extern double last_line;



void end(void);



extern unsigned long new_error;
extern unsigned long error_err; //=0;
extern double error_erl; //=0;
extern unsigned long error_occurred;
extern unsigned long error_goto_line;
extern unsigned long error_handling;
extern unsigned long error_retry;

void fix_error(){
if ((!error_goto_line)||error_handling){
long v;
v=0;
if (new_error==1) v=MessageBox(NULL,"NEXT without FOR\nContinue?","Unhandled Error #1",MB_YESNO);
if (new_error==2) v=MessageBox(NULL,"Syntax error\nContinue?","Unhandled Error #2",MB_YESNO);
if (new_error==3) v=MessageBox(NULL,"RETURN without GOSUB\nContinue?","Unhandled Error #3",MB_YESNO);
if (new_error==4) v=MessageBox(NULL,"Out of DATA\nContinue?","Unhandled Error #4",MB_YESNO);
if (new_error==5) v=MessageBox(NULL,"Illegal function call\nContinue?","Unhandled Error #5",MB_YESNO);
if (new_error==6) v=MessageBox(NULL,"Overflow\nContinue?","Unhandled Error #6",MB_YESNO);
if (new_error==7) v=MessageBox(NULL,"Out of memory\nContinue?","Unhandled Error #7",MB_YESNO);
if (new_error==8) v=MessageBox(NULL,"Label not defined\nContinue?","Unhandled Error #8",MB_YESNO);
if (new_error==9) v=MessageBox(NULL,"Subscript out of range\nContinue?","Unhandled Error #9",MB_YESNO);
if (new_error==10) v=MessageBox(NULL,"Duplicate definition\nContinue?","Unhandled Error #10",MB_YESNO);
if (new_error==11) v=MessageBox(NULL,"Division by zero\nContinue?","Unhandled Error #11",MB_YESNO);
if (new_error==12) v=MessageBox(NULL,"Illegal in direct mode\nContinue?","Unhandled Error #12",MB_YESNO);
if (new_error==13) v=MessageBox(NULL,"Type mismatch\nContinue?","Unhandled Error #13",MB_YESNO);
if (new_error==14) v=MessageBox(NULL,"Out of string space\nContinue?","Unhandled Error #14",MB_YESNO);
//error 15 undefined
if (new_error==16) v=MessageBox(NULL,"String formula too complex\nContinue?","Unhandled Error #16",MB_YESNO);
if (new_error==17) v=MessageBox(NULL,"Cannot continue\nContinue?","Unhandled Error #17",MB_YESNO);
if (new_error==18) v=MessageBox(NULL,"Function not defined\nContinue?","Unhandled Error #18",MB_YESNO);
if (new_error==19) v=MessageBox(NULL,"No RESUME\nContinue?","Unhandled Error #19",MB_YESNO);
if (new_error==20) v=MessageBox(NULL,"RESUME without error\nContinue?","Unhandled Error #20",MB_YESNO);
//error 21-23 undefined
if (new_error==24) v=MessageBox(NULL,"Device timeout\nContinue?","Unhandled Error #24",MB_YESNO);
if (new_error==25) v=MessageBox(NULL,"Device fault\nContinue?","Unhandled Error #25",MB_YESNO);
if (new_error==26) v=MessageBox(NULL,"FOR without NEXT\nContinue?","Unhandled Error #26",MB_YESNO);
if (new_error==27) v=MessageBox(NULL,"Out of paper\nContinue?","Unhandled Error #27",MB_YESNO);
//error 28 undefined
if (new_error==29) v=MessageBox(NULL,"WHILE without WEND\nContinue?","Unhandled Error #28",MB_YESNO);
if (new_error==30) v=MessageBox(NULL,"WEND without WHILE\nContinue?","Unhandled Error #29",MB_YESNO);
//error 31-32 undefined
if (new_error==33) v=MessageBox(NULL,"Duplicate label\nContinue?","Unhandled Error #33",MB_YESNO);
//error 34 undefined
if (new_error==35) v=MessageBox(NULL,"Subprogram not defined\nContinue?","Unhandled Error #35",MB_YESNO);
if (new_error==37) v=MessageBox(NULL,"Argument-count mismatch\nContinue?","Unhandled Error #37",MB_YESNO);
if (new_error==38) v=MessageBox(NULL,"Array not defined\nContinue?","Unhandled Error #38",MB_YESNO);
if (new_error==40) v=MessageBox(NULL,"Variable required\nContinue?","Unhandled Error #40",MB_YESNO);
if (new_error==50) v=MessageBox(NULL,"FIELD overflow\nContinue?","Unhandled Error #50",MB_YESNO);
if (new_error==51) v=MessageBox(NULL,"Internal error\nContinue?","Unhandled Error #51",MB_YESNO);
if (new_error==52) v=MessageBox(NULL,"Bad file name or number\nContinue?","Unhandled Error #52",MB_YESNO);
if (new_error==53) v=MessageBox(NULL,"File not found\nContinue?","Unhandled Error #53",MB_YESNO);
if (new_error==54) v=MessageBox(NULL,"Bad file mode\nContinue?","Unhandled Error #54",MB_YESNO);
if (new_error==55) v=MessageBox(NULL,"File already open\nContinue?","Unhandled Error #55",MB_YESNO);
if (new_error==56) v=MessageBox(NULL,"FIELD statement active\nContinue?","Unhandled Error #56",MB_YESNO);
if (new_error==57) v=MessageBox(NULL,"Device I/O error\nContinue?","Unhandled Error #57",MB_YESNO);
if (new_error==58) v=MessageBox(NULL,"File already exists\nContinue?","Unhandled Error #58",MB_YESNO);
if (new_error==59) v=MessageBox(NULL,"Bad record length\nContinue?","Unhandled Error #59",MB_YESNO);
if (new_error==61) v=MessageBox(NULL,"Disk full\nContinue?","Unhandled Error #61",MB_YESNO);
if (new_error==62) v=MessageBox(NULL,"Input past end of file\nContinue?","Unhandled Error #62",MB_YESNO);
if (new_error==63) v=MessageBox(NULL,"Bad record number\nContinue?","Unhandled Error #63",MB_YESNO);
if (new_error==64) v=MessageBox(NULL,"Bad file name\nContinue?","Unhandled Error #64",MB_YESNO);
if (new_error==67) v=MessageBox(NULL,"Too many files\nContinue?","Unhandled Error #67",MB_YESNO);
if (new_error==68) v=MessageBox(NULL,"Device unavailable\nContinue?","Unhandled Error #68",MB_YESNO);
if (new_error==69) v=MessageBox(NULL,"Communication-buffer overflow\nContinue?","Unhandled Error #69",MB_YESNO);
if (new_error==70) v=MessageBox(NULL,"Permission denied\nContinue?","Unhandled Error #70",MB_YESNO);
if (new_error==71) v=MessageBox(NULL,"Disk not ready\nContinue?","Unhandled Error #71",MB_YESNO);
if (new_error==72) v=MessageBox(NULL,"Disk-media error\nContinue?","Unhandled Error #72",MB_YESNO);
if (new_error==73) v=MessageBox(NULL,"Feature unavailable\nContinue?","Unhandled Error #73",MB_YESNO);
if (new_error==74) v=MessageBox(NULL,"Rename across disks\nContinue?","Unhandled Error #74",MB_YESNO);
if (new_error==75) v=MessageBox(NULL,"Path/File access error\nContinue?","Unhandled Error #75",MB_YESNO);
if (new_error==76) v=MessageBox(NULL,"Path not found\nContinue?","Unhandled Error #76",MB_YESNO);
if (new_error==258) v=MessageBox(NULL,"Invalid handle\nContinue?","Unhandled Error #258",MB_YESNO);
if (v==0) v=MessageBox(NULL,"Unprintable error\nContinue?","Unhandled Error",MB_YESNO);
if ((v==IDNO)||(v==IDOK)){close_program=1; end();}
new_error=0;
return;
}
error_err=new_error;
new_error=0;
error_erl=last_line;
error_occurred=1;
QBMAIN(NULL);
return;
}

void error(long error_number){
if (error_number==256){MessageBox(NULL,"Out of stack space","Critical Error",MB_OK); exit(0);}
if (error_number==257){MessageBox(NULL,"Out of memory","Critical Error",MB_OK); exit(0);}
if (!new_error){
if ((new_error==256)||(new_error==257)) fix_error();//critical error!
if (error_number<=0) error_number=5;//Illegal function call
new_error=error_number;
qbevent=1;
}
}

double get_error_erl(){
return error_erl;
}

unsigned long get_error_err(){
return error_err;
}

void end(){
while(!stop_program) Sleep(100);
SDL_KillThread(thread);
while(1) Sleep(100);
}



//MEM_STATIC memory manager
/*
mem_static uses a pointer called mem_static_pointer to allocate linear memory.
It can also change mem_static_pointer back to a previous location, effectively erasing
any memory after that point.
Because memory cannot be guaranteed to be allocated in exactly the same location
after realloc which QB64 requires to keep functionality of previous pointers when
the current block of memory is full QB64 creates an entirely new block, much larger
than the previous block (at least 2x), and "writes-off" the previous block as un-
reclaimable memory. This tradeoff is worth the speed it recovers.
This allocation strategy can be shown as follows: (X=1MB)
X
XX
XXXX
XXXXXXXX
XXXXXXXXXXXXXXXX
etc.
*/
unsigned long mem_static_size;
extern unsigned char *mem_static;
extern unsigned char *mem_static_pointer;
extern unsigned char *mem_static_limit;

unsigned char *mem_static_malloc(unsigned long size){
if ((mem_static_pointer+=size)<mem_static_limit) return mem_static_pointer-size;
mem_static_size=(mem_static_size<<1)+size;
mem_static=(unsigned char*)malloc(mem_static_size);
if (!mem_static) error(257);
mem_static_pointer=mem_static+size;
mem_static_limit=mem_static+mem_static_size;
return mem_static_pointer-size;
}
void mem_static_restore(unsigned char* restore_point){
if ((restore_point>=mem_static)&&(restore_point<=mem_static_limit)){
mem_static_pointer=restore_point;
}else{
//if restore_point is not in the current block, use t=start of current block as a new base
mem_static_pointer=mem_static;
}
}

//CMEM_FAR_DYNAMIC memory manager
/*
(uses a custom "links" based memory manager)
*/
//           &HA000    DBLOCK SIZE        DBLOCK OFFSET
//           655360 - (65536            + 1280         )=588544 links possible
//links limited to 588544/4=147136 (do not have enough links if avg. block size less than 4 bytes)
//stores blocks, not free memory, because blocks are easier to identify
//always scanned from beginning to end, so prev. pointer is unnecessary
struct cmem_dynamic_link_type{
unsigned char *offset;
unsigned char *top;
unsigned long size;
unsigned long i;
cmem_dynamic_link_type *next;
};
cmem_dynamic_link_type cmem_dynamic_link[147136+1]; //+1 is added because array is used from index 1
cmem_dynamic_link_type *cmem_dynamic_link_first=NULL;
long cmem_dynamic_next_link=0;
long cmem_dynamic_free_link=0;
unsigned long cmem_dynamic_free_list[147136];
unsigned char *cmem_dynamic_malloc(unsigned long size){
static long i;
static unsigned char *top;
static cmem_dynamic_link_type *link;
static cmem_dynamic_link_type *newlink;
if (size>65536) error(257);//>64K
top=&cmem[0]+655360;
if (link=cmem_dynamic_link_first){
cmem_dynamic_findspace:
top=link->offset;
if ((top-link->top)>=size){
//found free space
goto cmem_dynamic_make_new_link;
}
if (link=link->next) goto cmem_dynamic_findspace;
}
//no space between existing blocks is large enough
if ((top-cmem_static_pointer)<size) error(257);//a large enough block cannot be created!
cmem_dynamic_base=top-size;
cmem_dynamic_make_new_link:
if (cmem_dynamic_free_link){
i=cmem_dynamic_free_list[cmem_dynamic_free_link--];
}else{
i=cmem_dynamic_next_link++; if (i>=147136) error(257);//not enough blocks
}
newlink=(cmem_dynamic_link_type*)&cmem_dynamic_link[i];
newlink->i=i;
newlink->offset=top-size;
newlink->size=size;
newlink->top=top;
if (cmem_dynamic_link_first){
newlink->next=link->next;
link->next=newlink;
}else{
cmem_dynamic_link_first=newlink;
newlink->next=NULL;
}
return newlink->offset;
}
void cmem_dynamic_free(unsigned char *block){
static cmem_dynamic_link_type *link;
static cmem_dynamic_link_type *prev_link;
prev_link=NULL;
if (link=cmem_dynamic_link_first){
check_next:
if (link->offset==block){
//unlink
if (link->next){
if (prev_link) prev_link->next=link->next; else cmem_dynamic_link_first=link->next;
}else{
if (prev_link) prev_link->next=NULL; else cmem_dynamic_link_first=NULL;
}
//free link
cmem_dynamic_free_link++;
cmem_dynamic_free_list[cmem_dynamic_free_link]=link->i;
//memory freed successfully!
return;
}
prev_link=link;
if (link=link->next) goto check_next;
}
return;
}

unsigned char *defseg=&cmem[1280];//set to base of DBLOCK

void sub_defseg(long segment,long passed){
if (new_error) return;
if (!passed){
defseg=&cmem[1280];
return;
}

if ((segment<-65536)||(segment>65535)){//same range as QB checks
error(6);
}else{
defseg=&cmem[0]+((unsigned short)segment)*16;
}
}

long func_peek(long offset){
if ((offset<-65536)||(offset>65535)){//same range as QB checks
error(6);
return 0;
}
return defseg[(unsigned short)offset];
}

void sub_poke(long offset,long value){
if (new_error) return;
if ((offset<-65536)||(offset>65535)){//same range as QB checks
error(6);
return;
}
defseg[(unsigned short)offset]=value;
}

long array_ok=1;//kept to compile legacy versions

//gosub-return handling
extern unsigned long next_return_point; //=0;
extern unsigned long *return_point; //=(unsigned long*)malloc(4*16384);
extern unsigned long return_points; //=16384;
void more_return_points(){
if (return_points>2147483647) error(256);
return_points*=2;
return_point=(unsigned long*)realloc(return_point,return_points*4);
if (return_point==NULL) error(256);
}




void sub__sndplay(unsigned long);

unsigned char *soundwave(double frequency,double length,double volume,double fadein,double fadeout);
long soundwave_bytes=0;

void qb64_generatesound(double f,double l,unsigned char wait){
sndsetup();
static unsigned char* data;
static unsigned long handle;
data=soundwave(f,l,1,0,0);
handle=func__sndraw(data,soundwave_bytes);
if (handle){
if (wait){
sndqueue_wait=sndqueue_next;
suspend_program|=2;
qbevent=1;
}
sndqueue[sndqueue_next]=handle;
sndqueue_next++; if (sndqueue_next>sndqueue_lastindex) sndqueue_next=0;
}else free(data);
}

unsigned char *soundwave(double frequency,double length,double volume,double fadein,double fadeout){
sndsetup();
static unsigned char *data;
static long i;
static short x,lastx;
static short* sp;
static double samples_per_second=22050.0;

//calculate total number of samples required
static double samples;
static long samplesi;
samples=length*samples_per_second;
samplesi=samples; if (!samplesi) samplesi=1;

soundwave_bytes=samplesi*4;
data=(unsigned char*)malloc(soundwave_bytes);
sp=(short*)data;

static long direction;
direction=1;

static double value;
value=0;

static double volume_multiplier;
volume_multiplier=volume*32767.0;

static long waveend;
waveend=0;

static double gradient;
//frequency*4.0*length is the total distance value will travel (+1,-2,+1[repeated])
//samples is the number of steps to do this in
if (samples) gradient=(frequency*4.0*length)/samples; else gradient=0;//avoid division by 0

lastx=1;//set to 1 to avoid passing initial comparison
for (i=0;i<samplesi;i++){
x=value*volume_multiplier;
*sp++=x;
*sp++=x;
if (x>0){
if (lastx<=0){
waveend=i;
}
}
lastx=x;
if (direction){
if ((value+=gradient)>=1.0){direction=0; value=2.0-value;}
}else{
if ((value-=gradient)<=-1.0){direction=1; value=-2.0-value;}
}
}//i

if (waveend) soundwave_bytes=waveend*4;

return (unsigned char*)data;
}

long wavesize(double length){
static long samples;
samples=length*22050.0; if (samples==0) samples=1;
return samples*4;
}



unsigned char keyon[65536];

qbs* singlespace;


qbs *qbs_malloc=(qbs*)calloc(sizeof(qbs)*65536,1);//~1MEG
unsigned long qbs_malloc_next=0;//the next idex in qbs_malloc to use
unsigned long *qbs_malloc_freed=(unsigned long*)malloc(4*65536);
unsigned long qbs_malloc_freed_size=65536;
unsigned long qbs_malloc_freed_num=0;//number of freed qbs descriptors
qbs *qbs_new_descriptor(){
if (qbs_malloc_freed_num){
return (qbs*)memset((void *)qbs_malloc_freed[--qbs_malloc_freed_num],0,sizeof(qbs));
}
if (qbs_malloc_next==65536){
qbs_malloc=(qbs*)calloc(sizeof(qbs)*65536,1);//~1MEG
qbs_malloc_next=0;
}
return &qbs_malloc[qbs_malloc_next++];
}
void qbs_free_descriptor(qbs *str){
if (qbs_malloc_freed_num==qbs_malloc_freed_size){
 qbs_malloc_freed_size*=2;
 qbs_malloc_freed=(unsigned long*)realloc(qbs_malloc_freed,qbs_malloc_freed_size*4);
 if (!qbs_malloc_freed) error(257);
}
qbs_malloc_freed[qbs_malloc_freed_num]=(unsigned long)str;
qbs_malloc_freed_num++;
return;
}

//Used to track strings in 16bit memory
unsigned long *qbs_cmem_list=(unsigned long*)malloc(65536*4);
unsigned long  qbs_cmem_list_lasti=65535;
unsigned long  qbs_cmem_list_nexti=0;
//Used to track strings in 32bit memory
unsigned long *qbs_list=(unsigned long*)malloc(65536*4);
unsigned long  qbs_list_lasti=65535;
unsigned long  qbs_list_nexti=0;
//Used to track temporary strings for later removal when they fall out of scope
//*Some string functions delete a temporary string automatically after they have been
// passed one to save memory. In this case qbstring_templist[?]=0xFFFFFFFF
unsigned long *qbs_tmp_list=(unsigned long*)calloc(65536*4,1);//first index MUST be 0
unsigned long  qbs_tmp_list_lasti=65535;
extern unsigned long qbs_tmp_list_nexti;
//entended string memory

unsigned char *qbs_data=(unsigned char*)malloc(1048576);
unsigned long qbs_data_size=1048576;
unsigned long qbs_sp=0;

void qbs_free(qbs *str){
if (str->tmplisti){
 qbs_tmp_list[str->tmplisti]=0xFFFFFFFF;
 while (qbs_tmp_list[qbs_tmp_list_nexti-1]==0xFFFFFFFF){
 qbs_tmp_list_nexti--;
 }
}
if (str->fixed||str->readonly){
 qbs_free_descriptor(str);
 return;
}
if (str->in_cmem){
 qbs_cmem_list[str->listi]=0xFFFFFFFF;
 if ((qbs_cmem_list_nexti-1)==str->listi) qbs_cmem_list_nexti--;
}else{
 qbs_list[str->listi]=0xFFFFFFFF;
 retry:
 if (qbs_list[qbs_list_nexti-1]==0xFFFFFFFF){
 qbs_list_nexti--;
 if (qbs_list_nexti) goto retry;
 }
 if (qbs_list_nexti){
 qbs_sp=((qbs*)qbs_list[qbs_list_nexti-1])->chr-qbs_data+((qbs*)qbs_list[qbs_list_nexti-1])->len+32;
 if (qbs_sp>qbs_data_size) qbs_sp=qbs_data_size;//adding 32 could overflow buffer!
 }else{
 qbs_sp=0;
 }
}
qbs_free_descriptor(str);
return;
}

void qbs_cmem_concat_list(){
unsigned long i;
unsigned long d;
qbs *tqbs;
d=0;
for (i=0;i<qbs_cmem_list_nexti;i++){
 if (qbs_cmem_list[i]!=0xFFFFFFFF){ 
  if (i!=d){  
   tqbs=(qbs*)qbs_cmem_list[i];
   tqbs->listi=d;
   qbs_cmem_list[d]=(unsigned long)tqbs;
  }
 d++;
 }
}
qbs_cmem_list_nexti=d;
//if string listings are taking up more than half of the list array double the list array's size
if (qbs_cmem_list_nexti>=(qbs_cmem_list_lasti/2)){
qbs_cmem_list_lasti*=2;
qbs_cmem_list=(unsigned long*)realloc(qbs_cmem_list,(qbs_cmem_list_lasti+1)*4);
if (!qbs_cmem_list) error(257);
}
return;
}

void qbs_concat_list(){
unsigned long i;
unsigned long d;
qbs *tqbs;
d=0;
for (i=0;i<qbs_list_nexti;i++){
 if (qbs_list[i]!=0xFFFFFFFF){
  if (i!=d){
   tqbs=(qbs*)qbs_list[i];
   tqbs->listi=d;
   qbs_list[d]=(unsigned long)tqbs;
  }
 d++;
 }
}
qbs_list_nexti=d;
//if string listings are taking up more than half of the list array double the list array's size
if (qbs_list_nexti>=(qbs_list_lasti/2)){
qbs_list_lasti*=2;
qbs_list=(unsigned long*)realloc(qbs_list,(qbs_list_lasti+1)*4);
if (!qbs_list) error(257);
}
return;
}

void qbs_tmp_concat_list(){
if (qbs_tmp_list_nexti>=(qbs_tmp_list_lasti/2)){
qbs_tmp_list_lasti*=2;
qbs_tmp_list=(unsigned long*)realloc(qbs_tmp_list,(qbs_tmp_list_lasti+1)*4);
if (!qbs_tmp_list) error(257);
}
return;
}




void qbs_concat(unsigned long bytesrequired){
//this does not change indexing, only ->chr pointers and the location of their data
static long i;
static unsigned char *dest;
static qbs *tqbs;
dest=(unsigned char*)qbs_data;
if (qbs_list_nexti){
qbs_sp=0;
 for (i=0;i<qbs_list_nexti;i++){
  if (qbs_list[i]!=0xFFFFFFFF){
   tqbs=(qbs*)qbs_list[i];
   if ((tqbs->chr-dest)>32){
   if (tqbs->len) {memmove(dest,tqbs->chr,tqbs->len);}
   tqbs->chr=dest;       
   }
   dest=tqbs->chr+tqbs->len;
   qbs_sp=dest-qbs_data;
  }
 }
}

if (((qbs_sp*2)+(bytesrequired+32))>=qbs_data_size){
static unsigned char *oldbase;
oldbase=qbs_data;
qbs_data_size=qbs_data_size*2+bytesrequired;
qbs_data=(unsigned char*)realloc(qbs_data,qbs_data_size);
if (qbs_data==NULL) error(257);//realloc failed!
for (i=0;i<qbs_list_nexti;i++){
if (qbs_list[i]!=0xFFFFFFFF){
tqbs=(qbs*)qbs_list[i];
tqbs->chr=tqbs->chr-oldbase+qbs_data;
}
}
}
return;
}

//as the cmem stack has a limit if bytesrequired cannot be met this exits and returns an error
//the cmem stack cannot after all be extended!
//so bytesrequired is only passed to possibly generate an error, or not generate one
void qbs_concat_cmem(unsigned long bytesrequired){
//this does not change indexing, only ->chr pointers and the location of their data
long i;
unsigned char *dest;
qbs *tqbs;
dest=(unsigned char*)dblock;
qbs_cmem_sp=qbs_cmem_descriptor_space;
if (qbs_cmem_list_nexti){
 for (i=0;i<qbs_cmem_list_nexti;i++){
  if (qbs_cmem_list[i]!=0xFFFFFFFF){
   tqbs=(qbs*)qbs_cmem_list[i];
   if (tqbs->chr!=dest){
   if (tqbs->len) {memmove(dest,tqbs->chr,tqbs->len);}
   tqbs->chr=dest;
      //update cmem_descriptor [length][offset]
      if (tqbs->cmem_descriptor){tqbs->cmem_descriptor[0]=tqbs->len; tqbs->cmem_descriptor[1]=(unsigned short)(unsigned long)(tqbs->chr-dblock);}
   }
   dest+=tqbs->len;
   qbs_cmem_sp+=tqbs->len;
  }
 }
}
if ((qbs_cmem_sp+bytesrequired)>cmem_sp) error(257);
return;
}

qbs *qbs_new_cmem(long size,unsigned char tmp){
if ((qbs_cmem_sp+size)>cmem_sp) qbs_concat_cmem(size);
qbs *newstr;
newstr=qbs_new_descriptor();
newstr->len=size;
if ((qbs_cmem_sp+size)>cmem_sp) qbs_concat_cmem(size);
newstr->chr=(unsigned char*)dblock+qbs_cmem_sp;
qbs_cmem_sp+=size;
newstr->in_cmem=1;
if (qbs_cmem_list_nexti>qbs_cmem_list_lasti) qbs_cmem_concat_list();
newstr->listi=qbs_cmem_list_nexti; qbs_cmem_list[newstr->listi]=(unsigned long)newstr; qbs_cmem_list_nexti++;
if (tmp){
if (qbs_tmp_list_nexti>qbs_tmp_list_lasti) qbs_tmp_concat_list();
newstr->tmplisti=qbs_tmp_list_nexti; qbs_tmp_list[newstr->tmplisti]=(unsigned long)newstr; qbs_tmp_list_nexti++;
newstr->tmp=1;
}else{
//alloc string descriptor in DBLOCK (4 bytes)
cmem_sp-=4; newstr->cmem_descriptor=(unsigned short*)(dblock+cmem_sp); if (cmem_sp<qbs_cmem_sp) error(257);
newstr->cmem_descriptor_offset=cmem_sp;
  //update cmem_descriptor [length][offset]
  newstr->cmem_descriptor[0]=newstr->len; newstr->cmem_descriptor[1]=(unsigned short)(unsigned long)(newstr->chr-dblock);
}
return newstr;
}

qbs *qbs_new(long,unsigned char);

qbs *qbs_new_txt(const char *txt){
qbs *newstr;
newstr=qbs_new_descriptor();
newstr->len=strlen(txt);
newstr->chr=(unsigned char*)txt;
if (qbs_tmp_list_nexti>qbs_tmp_list_lasti) qbs_tmp_concat_list();
newstr->tmplisti=qbs_tmp_list_nexti; qbs_tmp_list[newstr->tmplisti]=(unsigned long)newstr; qbs_tmp_list_nexti++;
newstr->tmp=1;
newstr->readonly=1;
return newstr;
}

qbs *qbs_new_txt_len(const char *txt,long len){
qbs *newstr;
newstr=qbs_new_descriptor();
newstr->len=len;
newstr->chr=(unsigned char*)txt;
if (qbs_tmp_list_nexti>qbs_tmp_list_lasti) qbs_tmp_concat_list();
newstr->tmplisti=qbs_tmp_list_nexti; qbs_tmp_list[newstr->tmplisti]=(unsigned long)newstr; qbs_tmp_list_nexti++;
newstr->tmp=1;
newstr->readonly=1;
return newstr;
}







//note: qbs_new_fixed detects if string is in DBLOCK
qbs *qbs_new_fixed(unsigned char *offset,unsigned long size,unsigned char tmp){
qbs *newstr;
newstr=qbs_new_descriptor();
newstr->len=size;
newstr->chr=offset;
newstr->fixed=1;
if (tmp){
if (qbs_tmp_list_nexti>qbs_tmp_list_lasti) qbs_tmp_concat_list();
newstr->tmplisti=qbs_tmp_list_nexti; qbs_tmp_list[newstr->tmplisti]=(unsigned long)newstr; qbs_tmp_list_nexti++;
newstr->tmp=1;
}else{
//is it in DBLOCK?
if ((offset>(cmem+1280))&&(offset<(cmem+66816))){
//alloc string descriptor in DBLOCK (4 bytes)
cmem_sp-=4; newstr->cmem_descriptor=(unsigned short*)(dblock+cmem_sp); if (cmem_sp<qbs_cmem_sp) error(257);
newstr->cmem_descriptor_offset=cmem_sp;
  //update cmem_descriptor [length][offset]
  newstr->cmem_descriptor[0]=newstr->len; newstr->cmem_descriptor[1]=(unsigned short)(unsigned long)(newstr->chr-dblock);
}
}
return newstr;
}

qbs *qbs_new(long size,unsigned char tmp){
static qbs *newstr;
if ((qbs_sp+size+32)>qbs_data_size) qbs_concat(size+32);
newstr=qbs_new_descriptor();
newstr->len=size;
newstr->chr=qbs_data+qbs_sp;
qbs_sp+=size+32;
if (qbs_list_nexti>qbs_list_lasti) qbs_concat_list();
newstr->listi=qbs_list_nexti; qbs_list[newstr->listi]=(unsigned long)newstr; qbs_list_nexti++;
if (tmp){
if (qbs_tmp_list_nexti>qbs_tmp_list_lasti) qbs_tmp_concat_list();
newstr->tmplisti=qbs_tmp_list_nexti; qbs_tmp_list[newstr->tmplisti]=(unsigned long)newstr; qbs_tmp_list_nexti++;
newstr->tmp=1;
}
return newstr;
}

qbs *qbs_set(qbs *deststr,qbs *srcstr){
long i;
qbs *tqbs;
//fixed deststr
if (deststr->fixed){
 if (srcstr->len>=deststr->len){
  memcpy(deststr->chr,srcstr->chr,deststr->len);
 }else{
  memcpy(deststr->chr,srcstr->chr,srcstr->len);
  memset(deststr->chr+srcstr->len,32,deststr->len-srcstr->len);//pad with spaces
 }
 goto qbs_set_return;
}
//non-fixed deststr

//can srcstr be aquired by deststr?
if (srcstr->tmp){
if (srcstr->fixed==0){
if (srcstr->readonly==0){
if (srcstr->in_cmem==deststr->in_cmem){
if (deststr->in_cmem){
 //unlist deststr and acquire srcstr's list index
 qbs_cmem_list[deststr->listi]=0xFFFFFFFF;
 qbs_cmem_list[srcstr->listi]=(unsigned long)deststr;
 deststr->listi=srcstr->listi;
}else{
 //unlist deststr and acquire srcstr's list index
 qbs_list[deststr->listi]=0xFFFFFFFF;
 qbs_list[srcstr->listi]=(unsigned long)deststr;
 deststr->listi=srcstr->listi;
}

qbs_tmp_list[srcstr->tmplisti]=0xFFFFFFFF;
if (srcstr->tmplisti==(qbs_tmp_list_nexti-1)) qbs_tmp_list_nexti--;//correct last tmp index for performance

deststr->chr=srcstr->chr;
deststr->len=srcstr->len;
qbs_free_descriptor(srcstr);
  //update cmem_descriptor [length][offset]
  if (deststr->cmem_descriptor){deststr->cmem_descriptor[0]=deststr->len; deststr->cmem_descriptor[1]=(unsigned short)(unsigned long)(deststr->chr-dblock);}
return deststr;//nb. This return cannot be changed to a goto qbs_set_return!
}}}}

//srcstr is equal length or shorter
if (srcstr->len<=deststr->len){
 memcpy(deststr->chr,srcstr->chr,srcstr->len);
 deststr->len=srcstr->len;
 goto qbs_set_return;
}

//srcstr is longer
if (deststr->in_cmem){
 if (deststr->listi==(qbs_cmem_list_nexti-1)){//last index
  if (((unsigned long)deststr->chr+srcstr->len)<=(dblock+cmem_sp)){//space available
   memcpy(deststr->chr,srcstr->chr,srcstr->len);
   deststr->len=srcstr->len;  
   qbs_cmem_sp=(unsigned long)deststr->chr+deststr->len-(unsigned long)dblock;
   goto qbs_set_return;
  }
  goto qbs_set_cmem_concat_required;
 }
 //deststr is not the last index so locate next valid index
 i=deststr->listi+1;
 qbs_set_nextindex:
 if (qbs_cmem_list[i]!=0xFFFFFFFF){
  tqbs=(qbs*)qbs_cmem_list[i];
  if (tqbs==srcstr){
   if (srcstr->tmp==1) goto skippedtmpsrcindex;
  }
  if ((deststr->chr+srcstr->len)>tqbs->chr) goto qbs_set_cmem_concat_required;
  memcpy(deststr->chr,srcstr->chr,srcstr->len);
  deststr->len=srcstr->len;
  goto qbs_set_return;
 }
 skippedtmpsrcindex:
 i++;
 if (i!=qbs_cmem_list_nexti) goto qbs_set_nextindex;
 //all next indexes invalid!
 qbs_cmem_list_nexti=deststr->listi+1;//adjust nexti
 if (((unsigned long)deststr->chr+srcstr->len)<=(dblock+cmem_sp)){//space available
   memmove(deststr->chr,srcstr->chr,srcstr->len);//overlap possible due to sometimes aquiring srcstr's space
   deststr->len=srcstr->len;
   qbs_cmem_sp=(unsigned long)deststr->chr+deststr->len-(unsigned long)dblock;
   goto qbs_set_return;
 }
qbs_set_cmem_concat_required:
//srcstr could not fit in deststr
//"realloc" deststr
qbs_cmem_list[deststr->listi]=0xFFFFFFFF;//unlist
if ((qbs_cmem_sp+srcstr->len)>cmem_sp){//must concat!
qbs_concat_cmem(srcstr->len);
}
if (qbs_cmem_list_nexti>qbs_cmem_list_lasti) qbs_cmem_concat_list();
deststr->listi=qbs_cmem_list_nexti;
qbs_cmem_list[qbs_cmem_list_nexti]=(unsigned long)deststr; qbs_cmem_list_nexti++; //relist
deststr->chr=(unsigned char*)dblock+qbs_cmem_sp;
deststr->len=srcstr->len;
qbs_cmem_sp+=deststr->len;
memcpy(deststr->chr,srcstr->chr,srcstr->len);
goto qbs_set_return;
}


//not in cmem
 if (deststr->listi==(qbs_list_nexti-1)){//last index
  if (((unsigned long)deststr->chr+srcstr->len)<=((unsigned long)qbs_data+qbs_data_size)){//space available
   memcpy(deststr->chr,srcstr->chr,srcstr->len);
   deststr->len=srcstr->len;
   qbs_sp=(unsigned long)deststr->chr+deststr->len-(unsigned long)qbs_data;
   goto qbs_set_return;
  }
  goto qbs_set_concat_required;
 }
 //deststr is not the last index so locate next valid index
 i=deststr->listi+1;
 qbs_set_nextindex2:
 if (qbs_list[i]!=0xFFFFFFFF){
  tqbs=(qbs*)qbs_list[i];
  if (tqbs==srcstr){
   if (srcstr->tmp==1) goto skippedtmpsrcindex2;
  }
  if ((deststr->chr+srcstr->len)>tqbs->chr) goto qbs_set_concat_required;
  memcpy(deststr->chr,srcstr->chr,srcstr->len);
  deststr->len=srcstr->len;
  goto qbs_set_return;
 }
 skippedtmpsrcindex2:
 i++;
 if (i!=qbs_list_nexti) goto qbs_set_nextindex2;
 //all next indexes invalid!

 qbs_list_nexti=deststr->listi+1;//adjust nexti 
 if (((unsigned long)deststr->chr+srcstr->len)<=((unsigned long)qbs_data+qbs_data_size)){//space available
   memmove(deststr->chr,srcstr->chr,srcstr->len);//overlap possible due to sometimes aquiring srcstr's space
   deststr->len=srcstr->len;
   qbs_sp=(unsigned long)deststr->chr+deststr->len-(unsigned long)qbs_data;
   goto qbs_set_return;
 }

qbs_set_concat_required:
//srcstr could not fit in deststr
//"realloc" deststr
qbs_list[deststr->listi]=0xFFFFFFFF;//unlist
if ((qbs_sp+srcstr->len)>qbs_data_size){//must concat!
qbs_concat(srcstr->len);
}
if (qbs_list_nexti>qbs_list_lasti) qbs_concat_list();
deststr->listi=qbs_list_nexti;
qbs_list[qbs_list_nexti]=(unsigned long)deststr; qbs_list_nexti++; //relist

deststr->chr=qbs_data+qbs_sp;
deststr->len=srcstr->len;
qbs_sp+=deststr->len;
memcpy(deststr->chr,srcstr->chr,srcstr->len);

//(fall through to qbs_set_return)
qbs_set_return:
if (srcstr->tmp){//remove srcstr if it is a tmp string
qbs_free(srcstr);
}
  //update cmem_descriptor [length][offset]
  if (deststr->cmem_descriptor){deststr->cmem_descriptor[0]=deststr->len; deststr->cmem_descriptor[1]=(unsigned short)(unsigned long)(deststr->chr-dblock);}
return deststr;
}

qbs *qbs_add(qbs *str1,qbs *str2){
qbs *tqbs;
if (!str2->len) return str1;//pass on
if (!str1->len) return str2;//pass on
//may be possible to acquire str1 or str2's space but...
//1. check if dest has enough space (because its data is already in the correct place)
//2. check if source has enough space
//3. give up
//nb. they would also have to be a tmp, var. len str in ext memory!
//brute force method...
tqbs=qbs_new(str1->len+str2->len,1);
memcpy(tqbs->chr,str1->chr,str1->len);
memcpy(tqbs->chr+str1->len,str2->chr,str2->len);

//exit(qbs_sp);
if (str1->tmp) qbs_free(str1);
if (str2->tmp) qbs_free(str2);
return tqbs;
}

qbs *qbs_ucase(qbs *str){
unsigned long i;
unsigned char *c;
if (!str->len) return str;//pass on
qbs *tqbs=NULL;
if (str->tmp){ if (!str->fixed){ if (!str->readonly){ if (!str->in_cmem){ tqbs=str; }}}}
if (!tqbs){
//also pass on if already uppercase
c=str->chr;
for (i=0;i<str->len;i++){
 if ((*c>=97)&&(*c<=122)) goto qbs_ucase_cantpass;
 c++;
}
return str;
qbs_ucase_cantpass:
tqbs=qbs_new(str->len,1); memcpy(tqbs->chr,str->chr,str->len);
}
c=tqbs->chr;
for (i=0;i<tqbs->len;i++){
 if ((*c>=97)&&(*c<=122)) *c-=32;
 c++;
}
if (tqbs!=str) if (str->tmp) qbs_free(str);
return tqbs;
}

qbs *qbs_lcase(qbs *str){
unsigned long i;
unsigned char *c;
if (!str->len) return str;//pass on
qbs *tqbs=NULL;
if (str->tmp){ if (!str->fixed){ if (!str->readonly){ if (!str->in_cmem){ tqbs=str; }}}}
if (!tqbs){
//also pass on if already lowercase
c=str->chr;
for (i=0;i<str->len;i++){
 if ((*c>=65)&&(*c<=90)) goto qbs_lcase_cantpass;
 c++;
}
return str;
qbs_lcase_cantpass:
tqbs=qbs_new(str->len,1); memcpy(tqbs->chr,str->chr,str->len);
}
c=tqbs->chr;
for (i=0;i<tqbs->len;i++){
 if ((*c>=65)&&(*c<=90)) *c+=32;
 c++;
}
if (tqbs!=str) if (str->tmp) qbs_free(str);
return tqbs;
}

qbs *func_chr(long value){
qbs *tqbs;
if ((value<0)||(value>255)){
tqbs=qbs_new(0,1);
error(5);
}else{
tqbs=qbs_new(1,1);
tqbs->chr[0]=value;
}
return tqbs;
}


qbs *func_varptr_helper(unsigned char type,unsigned short offset){
//*creates a 3 byte string using the values given
qbs *tqbs;
tqbs=qbs_new(3,1);
tqbs->chr[0]=type;
tqbs->chr[1]=offset&255;
tqbs->chr[2]=offset>>8;
return tqbs;
}

qbs *qbs_left(qbs *str,long l){
if (l>str->len) l=str->len;
if (l<0) l=0;
if (l==str->len) return str;//pass on
if (str->tmp){ if (!str->fixed){ if (!str->readonly){ if (!str->in_cmem){ str->len=l; return str; }}}}
qbs *tqbs;
tqbs=qbs_new(l,1);
if (l) memcpy(tqbs->chr,str->chr,l);
if (str->tmp) qbs_free(str);
return tqbs;
}

qbs *qbs_right(qbs *str,long l){
if (l>str->len) l=str->len;
if (l<0) l=0;
if (l==str->len) return str;//pass on
if (str->tmp){ if (!str->fixed){ if (!str->readonly){ if (!str->in_cmem){
str->chr=str->chr+(str->len-l);
str->len=l; return str;
}}}}
qbs *tqbs;
tqbs=qbs_new(l,1);
if (l) memcpy(tqbs->chr,str->chr+str->len-l,l);
tqbs->len=l;
if (str->tmp) qbs_free(str);
return tqbs;
}

qbs *func_mksmbf(float val){
static qbs *tqbs;
tqbs=qbs_new(4,1);
if (_fieeetomsbin(&val,(float*)tqbs->chr)==1) {error(5); tqbs->len=0;}
return tqbs;
}
qbs *func_mkdmbf(double val){
static qbs *tqbs;
tqbs=qbs_new(8,1);
if (_dieeetomsbin(&val,(double*)tqbs->chr)==1) {error(5); tqbs->len=0;}
return tqbs;
}

float func_cvsmbf(qbs *str){
static float val;
if (str->len<4) {error(5); return 0;}
if (_fmsbintoieee((float*)str->chr,&val)==1) {error(5); return 0;}
return val;
}
double func_cvdmbf(qbs *str){
static double val;
if (str->len<8) {error(5); return 0;}
if (_dmsbintoieee((double*)str->chr,&val)==1) {error(5); return 0;}
return val;
}

qbs *b2string(char v){ static qbs *tqbs; tqbs=qbs_new(1,1); *((char*)(tqbs->chr))=v; return tqbs;}
qbs *ub2string(char v){ static qbs *tqbs; tqbs=qbs_new(1,1); *((unsigned char*)(tqbs->chr))=v; return tqbs;}
qbs *i2string(short v){ static qbs *tqbs; tqbs=qbs_new(2,1); *((short*)(tqbs->chr))=v; return tqbs;}
qbs *ui2string(short v){ static qbs *tqbs; tqbs=qbs_new(2,1); *((unsigned short*)(tqbs->chr))=v; return tqbs;}
qbs *l2string(long v){ static qbs *tqbs; tqbs=qbs_new(4,1); *((long*)(tqbs->chr))=v; return tqbs;}
qbs *ul2string(unsigned long v){ static qbs *tqbs; tqbs=qbs_new(4,1); *((unsigned long*)(tqbs->chr))=v; return tqbs;}
qbs *i642string(int64 v){ static qbs *tqbs; tqbs=qbs_new(8,1); *((int64*)(tqbs->chr))=v; return tqbs;}
qbs *i642string(uint64 v){ static qbs *tqbs; tqbs=qbs_new(8,1); *((uint64*)(tqbs->chr))=v; return tqbs;}
qbs *s2string(float v){ static qbs *tqbs; tqbs=qbs_new(4,1); *((float*)(tqbs->chr))=v; return tqbs;}
qbs *d2string(double v){ static qbs *tqbs; tqbs=qbs_new(8,1); *((double*)(tqbs->chr))=v; return tqbs;}
qbs *f2string(long double v){ static qbs *tqbs; tqbs=qbs_new(32,1); memset(tqbs->chr,0,32); *((long double*)(tqbs->chr))=v; return tqbs;}
qbs *bit2string(unsigned long bsize,int64 v){
static qbs* tqbs;
tqbs=qbs_new(8,1);
bmask=~(-(1<<bsize));
*((int64*)(tqbs->chr))=v&bmask;
tqbs->len=(bsize+7)>>3;
return tqbs;
}
qbs *ubit2string(unsigned long bsize,uint64 v){
static qbs* tqbs;
tqbs=qbs_new(8,1);
bmask=~(-(1<<bsize));
*((uint64*)(tqbs->chr))=v&bmask;
tqbs->len=(bsize+7)>>3;
return tqbs;
}

char string2b(qbs*str){ if (str->len<1) {error(5); return 0;} else {return *((char*)str->chr);} }
unsigned char string2ub(qbs*str){ if (str->len<1) {error(5); return 0;} else {return *((unsigned char*)str->chr);} }
short string2i(qbs*str){ if (str->len<2) {error(5); return 0;} else {return *((short*)str->chr);} }
unsigned short string2ui(qbs*str){ if (str->len<2) {error(5); return 0;} else {return *((unsigned short*)str->chr);} }
long string2l(qbs*str){ if (str->len<4) {error(5); return 0;} else {return *((long*)str->chr);} }
unsigned long string2ul(qbs*str){ if (str->len<4) {error(5); return 0;} else {return *((unsigned long*)str->chr);} }
int64 string2i64(qbs*str){ if (str->len<8) {error(5); return 0;} else {return *((int64*)str->chr);} }
uint64 string2ui64(qbs*str){ if (str->len<8) {error(5); return 0;} else {return *((uint64*)str->chr);} }
float string2s(qbs*str){ if (str->len<4) {error(5); return 0;} else {return *((float*)str->chr);} }
double string2d(qbs*str){ if (str->len<8) {error(5); return 0;} else {return *((double*)str->chr);} }
long double string2f(qbs*str){ if (str->len<32) {error(5); return 0;} else {return *((long double*)str->chr);} }
uint64 string2ubit(qbs*str,unsigned long bsize){
if (str->len<((bsize+7)>>3)) {error(5); return 0;}
bmask=~(-(1<<bsize));
return (*(uint64*)str->chr)&bmask;
}
int64 string2bit(qbs*str,unsigned long bsize){
if (str->len<((bsize+7)>>3)) {error(5); return 0;}
bmask=~(-(1<<bsize));
bval64=((*(uint64*)str->chr)&bmask)<<boff;
if (bval64&(1<<(bsize-1))) return (bval64|(~bmask));
return bval64;
}

void sub_lset(qbs *dest,qbs *source){
if (new_error) return;
if (source->len>=dest->len){
if (dest->len) memcpy(dest->chr,source->chr,dest->len);
return;
}
if (source->len) memcpy(dest->chr,source->chr,source->len);
memset(dest->chr+source->len,32,dest->len-source->len);
}

void sub_rset(qbs *dest,qbs *source){
if (new_error) return;
if (source->len>=dest->len){
if (dest->len) memcpy(dest->chr,source->chr,dest->len);
return;
}
if (source->len) memcpy(dest->chr+dest->len-source->len,source->chr,source->len);
memset(dest->chr,32,dest->len-source->len);
}




qbs *func_space(long spaces){
static qbs *tqbs;
if (spaces<0) spaces=0;
tqbs=qbs_new(spaces,1);
if (spaces) memset(tqbs->chr,32,spaces);
return tqbs;
}

qbs *func_string(long characters,long asciivalue){
static qbs *tqbs;
if (characters<0) characters=0;
tqbs=qbs_new(characters,1);
if (characters) memset(tqbs->chr,asciivalue&0xFF,characters);
return tqbs;
}

long func_instr(long start,qbs *str,qbs *substr,long passed){
//QB64 difference: start can be 0 or negative
//justification-start could be larger than the length of string to search in QBASIC
static unsigned char *limit,*base;
static unsigned char firstc;
if (!passed) start=1;
if (!str->len) return 0;
if (start<1){
start=1;
if (!substr->len) return 0;
}
if (start>str->len) return 0;
if (!substr->len) return start;
if ((start+substr->len-1)>str->len) return 0;
limit=str->chr+str->len;
firstc=substr->chr[0];
base=str->chr+start-1;
nextchar:
base=(unsigned char*)memchr(base,firstc,limit-base);
if (!base) return 0;
if ((base+substr->len)>limit) return 0;
if (!memcmp(base,substr->chr,substr->len)) return base-str->chr+1;
base++;
if ((base+substr->len)>limit) return 0;
goto nextchar;
}

void sub_mid(qbs *dest,long start,long l,qbs* src,long passed){
if (new_error) return;
static long src_offset;
if (!passed) l=src->len;
src_offset=0;
if (dest==nothingstring) return;//quiet exit, error has already been reported!
if (start<1){
l=l+start-1;
src_offset=-start+1;//src_offset is a byte offset with base 0!
start=1;
}
if (l<=0) return;
if (start>dest->len) return;
if ((start+l-1)>dest->len) l=dest->len-start+1;
//start and l are now reflect a valid region within dest
if (src_offset>=src->len) return;
if (l>(src->len-src_offset)) l=src->len-src_offset;
//src_offset and l now reflect a valid region within src
if (dest==src){
if ((start-1)!=src_offset) memmove(dest->chr+start-1,src->chr+src_offset,l);
}else{
memcpy(dest->chr+start-1,src->chr+src_offset,l);
}
}

qbs *func_mid(qbs *str,long start,long l,long passed){
static qbs *tqbs;
if (passed){
 if (start<1) {l=l-1+start; start=1;}
 if ((l>=1)&&(start<=str->len)){
 if ((start+l)>str->len) l=str->len-start+1;
 }else{
 l=0; start=1;//nothing!
 }
}else{
 if (start<1) start=1;
 l=str->len-start+1;
 if (l<1){
 l=0; start=1;//nothing!
 }
}
if ((start==1)&&(l==str->len)) return str;//pass on
if (str->tmp){ if (!str->fixed){ if (!str->readonly){ if (!str->in_cmem){//acquire
str->chr=str->chr+(start-1);
str->len=l;
return str;
}}}}
tqbs=qbs_new(l,1);
if (l) memcpy(tqbs->chr,str->chr+start-1,l);
if (str->tmp) qbs_free(str);
return tqbs;
}

qbs *qbs_ltrim(qbs *str){
if (!str->len) return str;//pass on
if (*str->chr!=32) return str;//pass on
if (str->tmp){ if (!str->fixed){ if (!str->readonly){ if (!str->in_cmem){//acquire?
qbs_ltrim_nextchar:
if (*str->chr==32){
str->chr++;
if (--str->len) goto qbs_ltrim_nextchar;
}
return str;
}}}}
long i;
i=0;
qbs_ltrim_nextchar2: if (str->chr[i]==32) {i++; if (i<str->len) goto qbs_ltrim_nextchar2;}
qbs *tqbs;
tqbs=qbs_new(str->len-i,1);
if (tqbs->len) memcpy(tqbs->chr,str->chr+i,tqbs->len);
if (str->tmp) qbs_free(str);
return tqbs;
}

qbs *qbs_rtrim(qbs *str){
if (!str->len) return str;//pass on
if (str->chr[str->len-1]!=32) return str;//pass on
if (str->tmp){ if (!str->fixed){ if (!str->readonly){ if (!str->in_cmem){//acquire?
qbs_rtrim_nextchar:
if (str->chr[str->len-1]==32){
if (--str->len) goto qbs_rtrim_nextchar;
}
return str;
}}}}
long i;
i=str->len;
qbs_rtrim_nextchar2: if (str->chr[i-1]==32) {i--; if (i) goto qbs_rtrim_nextchar2;}
//i is the number of characters to keep
qbs *tqbs;
tqbs=qbs_new(i,1);
if (i) memcpy(tqbs->chr,str->chr,i);
if (str->tmp) qbs_free(str);
return tqbs;
}

qbs *qbs_inkey(){
if (new_error) return qbs_new(0,1);
qbs *tqbs;
Sleep(0);
tqbs=qbs_new(2,1);
if (cmem[0x41a]!=cmem[0x41c]){
//MessageBox(NULL,"Key detected","Key detected",MB_OK);

tqbs->chr[0]=cmem[0x400+cmem[0x41a]];
tqbs->chr[1]=cmem[0x400+cmem[0x41a]+1];
if (tqbs->chr[0]) tqbs->len=1;
cmem[0x41a]+=2;
if (cmem[0x41a]==62) cmem[0x41a]=30;
}else{
tqbs->len=0;
}
return tqbs;
}

//STR() functions
//singed integers
qbs *qbs_str(int64 value){
qbs *tqbs;
tqbs=qbs_new(20,1);
#ifdef QB64_WINDOWS
 tqbs->len=sprintf((char*)tqbs->chr,"% I64i",value);
#else
 tqbs->len=sprintf((char*)tqbs->chr,"% ll",value);
#endif
return tqbs;
}
qbs *qbs_str(long value){
qbs *tqbs;
tqbs=qbs_new(11,1);
tqbs->len=sprintf((char*)tqbs->chr,"% i",value);
return tqbs;
}
qbs *qbs_str(short value){
qbs *tqbs;
tqbs=qbs_new(6,1);
tqbs->len=sprintf((char*)tqbs->chr,"% i",value);
return tqbs;
}
qbs *qbs_str(char value){
qbs *tqbs;
tqbs=qbs_new(4,1);
tqbs->len=sprintf((char*)tqbs->chr,"% i",value);
return tqbs;
}
//unsigned integers
qbs *qbs_str(uint64 value){
qbs *tqbs;
tqbs=qbs_new(21,1);
#ifdef QB64_WINDOWS
 tqbs->len=sprintf((char*)tqbs->chr," %I64u",value);
#else
 tqbs->len=sprintf((char*)tqbs->chr," %ull",value);
#endif
return tqbs;
}
qbs *qbs_str(unsigned long value){
qbs *tqbs;
tqbs=qbs_new(11,1);
tqbs->len=sprintf((char*)tqbs->chr," %u",value);
return tqbs;
}
qbs *qbs_str(unsigned short value){
qbs *tqbs;
tqbs=qbs_new(6,1);
tqbs->len=sprintf((char*)tqbs->chr," %u",value);
return tqbs;
}
qbs *qbs_str(unsigned char value){
qbs *tqbs;
tqbs=qbs_new(4,1);
tqbs->len=sprintf((char*)tqbs->chr," %u",value);
return tqbs;
}



unsigned char func_str_fmt[7];
unsigned char qbs_str_buffer[32];
unsigned char qbs_str_buffer2[32];

qbs *qbs_str(float value){
static qbs *tqbs;
tqbs=qbs_new(16,1);
static long l,i,i2,i3,digits,exponent;
l=sprintf((char*)&qbs_str_buffer,"% .6E",value);
//IMPORTANT: assumed l==14
if (l==13){memmove(&qbs_str_buffer[12],&qbs_str_buffer[11],2); qbs_str_buffer[11]=48; l=14;}

digits=7;
for (i=8;i>=1;i--){
if (qbs_str_buffer[i]==48){
digits--;
}else{
if (qbs_str_buffer[i]!=46) break;
}
}//i
//no significant digits? simply return 0
if (digits==0){
tqbs->len=2; tqbs->chr[0]=32; tqbs->chr[1]=48;//tqbs=[space][0]
return tqbs;
}
//calculate exponent
exponent=(qbs_str_buffer[11]-48)*100+(qbs_str_buffer[12]-48)*10+(qbs_str_buffer[13]-48);
if (qbs_str_buffer[10]==45) exponent=-exponent;
if ((exponent<=6)&&((exponent-digits)>=-8)) goto asdecimal;
//fix up exponent to conform to QBASIC standards
//i. cull trailing 0's after decimal point (use digits to help)
//ii. cull leading 0's of exponent
i3=0;
i2=digits+2;
if (digits==1) i2--;//don't include decimal point
for (i=0;i<i2;i++) {tqbs->chr[i3]=qbs_str_buffer[i]; i3++;}
for (i=9;i<=10;i++) {tqbs->chr[i3]=qbs_str_buffer[i]; i3++;}
exponent=abs(exponent);
//i2=13;
//if (exponent>9) i2=12;
i2=12;//override: if exponent is less than 10 still display a leading 0
if (exponent>99) i2=11;
for (i=i2;i<=13;i++) {tqbs->chr[i3]=qbs_str_buffer[i]; i3++;}
tqbs->len=i3;
return tqbs;
/////////////////////
asdecimal:
//calculate digits after decimal point in var. i
i=-(exponent-digits+1);
if (i<0) i=0;
func_str_fmt[0]=37;//"%"
func_str_fmt[1]=32;//" "
func_str_fmt[2]=46;//"."
func_str_fmt[3]=i+48;
func_str_fmt[4]=102;//"f"
func_str_fmt[5]=0;
tqbs->len=sprintf((char*)tqbs->chr,(const char*)&func_str_fmt,value);
if (tqbs->chr[1]==48){//must manually cull leading 0
memmove(tqbs->chr+1,tqbs->chr+2,tqbs->len-2);
tqbs->len--;
}
return tqbs;
}

qbs *qbs_str(double value){
static qbs *tqbs;
tqbs=qbs_new(32,1);
static long l,i,i2,i3,digits,exponent;

l=sprintf((char*)&qbs_str_buffer,"% .15E",value);
//IMPORTANT: assumed l==23
if (l==22){memmove(&qbs_str_buffer[21],&qbs_str_buffer[20],2); qbs_str_buffer[20]=48; l=23;}

//check if the 16th significant digit is 9, if it is round to 15 significant digits
if (qbs_str_buffer[17]==57){
sprintf((char*)&qbs_str_buffer2,"% .14E",value);
memmove(&qbs_str_buffer,&qbs_str_buffer2,17);
qbs_str_buffer[17]=48;
}
qbs_str_buffer[18]=68; //change E to D (QBASIC standard)
digits=16;
for (i=17;i>=1;i--){
if (qbs_str_buffer[i]==48){
digits--;
}else{
if (qbs_str_buffer[i]!=46) break;
}
}//i
//no significant digits? simply return 0
if (digits==0){
tqbs->len=2; tqbs->chr[0]=32; tqbs->chr[1]=48;//tqbs=[space][0]
return tqbs;
}
//calculate exponent
exponent=(qbs_str_buffer[20]-48)*100+(qbs_str_buffer[21]-48)*10+(qbs_str_buffer[22]-48);
if (qbs_str_buffer[19]==45) exponent=-exponent;
//OLD if ((exponent<=15)&&((exponent-digits)>=-16)) goto asdecimal;
if ((exponent<=15)&&((exponent-digits)>=-17)) goto asdecimal;
//fix up exponent to conform to QBASIC standards
//i. cull trailing 0's after decimal point (use digits to help)
//ii. cull leading 0's of exponent
i3=0;
i2=digits+2;
if (digits==1) i2--;//don't include decimal point
for (i=0;i<i2;i++) {tqbs->chr[i3]=qbs_str_buffer[i]; i3++;}
for (i=18;i<=19;i++) {tqbs->chr[i3]=qbs_str_buffer[i]; i3++;}
exponent=abs(exponent);
//i2=22;
//if (exponent>9) i2=21;
i2=21;//override: if exponent is less than 10 still display a leading 0
if (exponent>99) i2=20;
for (i=i2;i<=22;i++) {tqbs->chr[i3]=qbs_str_buffer[i]; i3++;}
tqbs->len=i3;
return tqbs;
/////////////////////
asdecimal:
//calculate digits after decimal point in var. i
i=-(exponent-digits+1);
if (i<0) i=0;
func_str_fmt[0]=37;//"%"
func_str_fmt[1]=32;//" "
func_str_fmt[2]=46;//"."
if (i>9){
func_str_fmt[3]=49;//"1"
func_str_fmt[4]=(i-10)+48;
}else{
func_str_fmt[3]=48;//"0"
func_str_fmt[4]=i+48;
}
func_str_fmt[5]=102;//"f"
func_str_fmt[6]=0;
tqbs->len=sprintf((char*)tqbs->chr,(const char*)&func_str_fmt,value);
if (tqbs->chr[1]==48){//must manually cull leading 0
memmove(tqbs->chr+1,tqbs->chr+2,tqbs->len-2);
tqbs->len--;
}
return tqbs;
}

qbs *qbs_str(long double value){
//not fully implemented
return qbs_str((double)value);
}


long qbs_equal(qbs *str1,qbs *str2){
if (str1->len!=str2->len) return 0;
if (memcmp(str1->chr,str2->chr,str1->len)==0) return -1;
return 0;
}
long qbs_notequal(qbs *str1,qbs *str2){
if (str1->len!=str2->len) return -1;
if (memcmp(str1->chr,str2->chr,str1->len)==0) return 0;
return -1;
}
long qbs_greaterthan(qbs *str1,qbs *str2){
static long i;
if (str1->len<=str2->len){
i=memcmp(str1->chr,str2->chr,str1->len);
if (i>0) return -1;
return 0;
}else{
i=memcmp(str1->chr,str2->chr,str2->len);
if (i<0) return 0;
return -1;
}
}
long qbs_lessthan(qbs *str1,qbs *str2){
static long i;
if (str1->len<=str2->len){
i=memcmp(str1->chr,str2->chr,str1->len);
if (i<0) return -1;
return 0;
}else{
i=memcmp(str1->chr,str2->chr,str2->len);
if (i>=0) return 0;
return -1;
}
}
long qbs_lessorequal(qbs *str1,qbs *str2){
static long i;
if (str1->len<=str2->len){
i=memcmp(str1->chr,str2->chr,str1->len);
if (i<=0) return -1;
return 0;
}else{
i=memcmp(str1->chr,str2->chr,str2->len);
if (i>=0) return 0;
return -1;
}
}
long qbs_greaterorequal(qbs *str1,qbs *str2){
static long i;
//greater?
if (str1->len<=str2->len){
i=memcmp(str1->chr,str2->chr,str1->len);
if (i>0) return -1;
if (i==0) if (str1->len==str2->len) return -1;//equal?
return 0;
}else{
i=memcmp(str1->chr,str2->chr,str2->len);
if (i<0) return 0;
return -1;
}
}



long qbs_asc(qbs *str){
if (str->len) return str->chr[0];
return 0;
}

long qbs_len(qbs *str){
return str->len;
}


//QBG BLOCK
long qbg_mode=-1;//-1 means not initialized!
long qbg_text_only;
//text & graphics modes
long qbg_height_in_characters, qbg_width_in_characters;
long qbg_top_row, qbg_bottom_row;
long qbg_cursor_x, qbg_cursor_y;
long qbg_character_height, qbg_character_width;
unsigned long qbg_color, qbg_background_color;
//text mode ONLY
long qbg_cursor_show;
long qbg_cursor_firstvalue, qbg_cursor_lastvalue;//these values need revision
//graphics modes ONLY
long qbg_width, qbg_height;
float qbg_x, qbg_y;
long qbg_bits_per_pixel, qbg_pixel_mask; //for monochrome modes 1b, for 16 color 1111b, for 256 color 11111111b
long qbg_bytes_per_pixel;
long qbg_clipping_or_scaling;//1=clipping, 2=clipping and scaling
long qbg_view_x1, qbg_view_y1, qbg_view_x2, qbg_view_y2;
long qbg_view_offset_x, qbg_view_offset_y;
float qbg_scaling_x, qbg_scaling_y;
float qbg_scaling_offset_x, qbg_scaling_offset_y;
float qbg_window_x1, qbg_window_y1, qbg_window_x2, qbg_window_y2;
long qbg_pages;
unsigned long *qbg_pageoffsets;
long *qbg_cursor_x_previous; //used to recover old cursor position
long *qbg_cursor_y_previous;
long qbg_active_page;
unsigned char *qbg_active_page_offset;
long qbg_visual_page;
unsigned char *qbg_visual_page_offset;
long qbg_color_assign[256];//for modes with quasi palettes!
unsigned long pal_mode10[2][9];













unsigned char charset8x8[256][8][8];
unsigned char charset8x16[256][16][8];

long lineclip_draw;//1=draw, 0=don't draw
long lineclip_x1,lineclip_y1,lineclip_x2,lineclip_y2;
long lineclip_skippixels;//the number of pixels from x1,y1 which won't be drawn

void lineclip(long x1,long y1,long x2,long y2,long xmin,long ymin,long xmax,long ymax){
static double mx,my,y,x,d;
static long xdis,ydis;
//is it a single point? (needed to avoid "division by 0" errors)
lineclip_skippixels=0;

if (x1==x2){ if (y1==y2){
if (x1>=xmin){ if (x1<=xmax){ if (y1>=ymin){ if (y1<=ymax){
goto singlepoint;
}}}}
lineclip_draw=0;
return;
}}

if (x1>=xmin){ if (x1<=xmax){ if (y1>=ymin){ if (y1<=ymax){
goto gotx1y1;
}}}}
mx=(x2-x1)/fabs((double)(y2-y1)); my=(y2-y1)/fabs((double)(x2-x1));
//right wall from right
if (x1>xmax){
if (mx<0){
y=(double)y1+((double)x1-(double)xmax)*my;
if (y>=ymin){ if (y<=ymax){
  //double space indented values calculate pixels to skip
  xdis=x1; ydis=y1;
x1=xmax; y1=qbr_float_to_long(y);
  xdis=abs(xdis-x1); ydis=abs(ydis-y1);
  if (xdis>=ydis) lineclip_skippixels=xdis; else lineclip_skippixels=ydis;
goto gotx1y1;
}}
}
}
//left wall from left
if (x1<xmin){
if (mx>0){
y=(double)y1+((double)xmin-(double)x1)*my;
if (y>=ymin){ if (y<=ymax){
  //double space indented values calculate pixels to skip
  xdis=x1; ydis=y1;
x1=xmin; y1=qbr_float_to_long(y);
  xdis=abs(xdis-x1); ydis=abs(ydis-y1);
  if (xdis>=ydis) lineclip_skippixels=xdis; else lineclip_skippixels=ydis;
goto gotx1y1;
}}
}
}
//top wall from top
if (y1<ymin){
if (my>0){
x=(double)x1+((double)ymin-(double)y1)*mx;
if (x>=xmin){ if (x<=xmax){
  //double space indented values calculate pixels to skip
  xdis=x1; ydis=y1;
x1=qbr_float_to_long(x); y1=ymin;
  xdis=abs(xdis-x1); ydis=abs(ydis-y1);
  if (xdis>=ydis) lineclip_skippixels=xdis; else lineclip_skippixels=ydis;
goto gotx1y1;
}}
}
}
//bottom wall from bottom
if (y1>ymax){
if (my<0){
x=(double)x1+((double)y2-(double)ymax)*mx;
if (x>=xmin){ if (x<=xmax){
  //double space indented values calculate pixels to skip
  xdis=x1; ydis=y1;
x1=qbr_float_to_long(x); y1=ymax;
  xdis=abs(xdis-x1); ydis=abs(ydis-y1);
  if (xdis>=ydis) lineclip_skippixels=xdis; else lineclip_skippixels=ydis;
goto gotx1y1;
}}
}
}
lineclip_draw=0;
return;
gotx1y1:

if (x2>=xmin){ if (x2<=xmax){ if (y2>=ymin){ if (y2<=ymax){
goto gotx2y2;
}}}}


mx=(x1-x2)/fabs((double)(y1-y2)); my=(y1-y2)/fabs((double)(x1-x2));
//right wall from right
if (x2>xmax){
if (mx<0){
y=(double)y2+((double)x2-(double)xmax)*my;
if (y>=ymin){ if (y<=ymax){
x2=xmax; y2=qbr_float_to_long(y);
goto gotx2y2;
}}
}
}
//left wall from left
if (x2<xmin){
if (mx>0){
y=(double)y2+((double)xmin-(double)x2)*my;
if (y>=ymin){ if (y<=ymax){
x2=xmin; y2=qbr_float_to_long(y);
goto gotx2y2;
}}
}
}
//top wall from top
if (y2<ymin){
if (my>0){
x=(double)x2+((double)ymin-(double)y2)*mx;
if (x>=xmin){ if (x<=xmax){
x2=qbr_float_to_long(x); y2=ymin;
goto gotx2y2;
}}
}
}
//bottom wall from bottom
if (y2>ymax){
if (my<0){
x=(double)x2+((double)y2-(double)ymax)*mx;
if (x>=xmin){ if (x<=xmax){
x2=qbr_float_to_long(x); y2=ymax;
goto gotx2y2;
}}
}
}
lineclip_draw=0;
return;
gotx2y2:
singlepoint:
lineclip_draw=1;
lineclip_x1=x1; lineclip_y1=y1; lineclip_x2=x2; lineclip_y2=y2;


return;
}

void qbg_palette(unsigned long attribute,unsigned long col,long passed){
static long r,g,b;
if (new_error) return;
if (!passed){restorepalette(write_page); return;}

//32-bit
if (write_page->bytes_per_pixel==4) goto error;

attribute&=255;//patch to support QBASIC overflow "bug"

if ((write_page->compatible_mode==13)||(write_page->compatible_mode==256)){
if (col&0xFFC0C0C0) goto error;//11111111110000001100000011000000b
r=col&63; g=(col>>8)&63; b=(col>>16)&63;
r=qbr((double)r*4.063492f-0.4999999f); g=qbr((double)g*4.063492f-0.4999999f); b=qbr((double)b*4.063492f-0.4999999f);
write_page->pal[attribute]=b+g*256+r*65536;
//Upgraded from (((col<<2)&0xFF)<<16)+(((col>>6)&0xFF)<<8)+((col>>14)&0xFF)
return;
}

if (write_page->compatible_mode==12){
if (attribute>15) goto error;
if (col&0xFFC0C0C0) goto error;//11111111110000001100000011000000b
r=col&63; g=(col>>8)&63; b=(col>>16)&63;
r=qbr((double)r*4.063492f-0.4999999f); g=qbr((double)g*4.063492f-0.4999999f); b=qbr((double)b*4.063492f-0.4999999f);
write_page->pal[attribute]=b+g*256+r*65536;
return;
}

if (write_page->compatible_mode==11){
if (attribute>1) goto error;
if (col&0xFFC0C0C0) goto error;//11111111110000001100000011000000b
r=col&63; g=(col>>8)&63; b=(col>>16)&63;
r=qbr((double)r*4.063492f-0.4999999f); g=qbr((double)g*4.063492f-0.4999999f); b=qbr((double)b*4.063492f-0.4999999f);
write_page->pal[attribute]=b+g*256+r*65536;
return;
}

if (write_page->compatible_mode==10){
if (attribute>3) goto error;
if ((col<0)||(col>8)) goto error;
//..._color_assign[attribute]=col;
return;
}

if (write_page->compatible_mode==9){
if (attribute>15) goto error;
if ((col<0)||(col>63)) goto error;
write_page->pal[attribute]=palette_64[col];
return;
}

if (write_page->compatible_mode==8){
if (attribute>15) goto error;
if ((col<0)||(col>15)) goto error;
write_page->pal[attribute]=palette_256[col];
return;
}

if (write_page->compatible_mode==7){
if (attribute>15) goto error;
if ((col<0)||(col>15)) goto error;
write_page->pal[attribute]=palette_256[col];
return;
}

if (write_page->compatible_mode==2){
if (attribute>1) goto error;
if ((col<0)||(col>15)) goto error;
write_page->pal[attribute]=palette_256[col];
return;
}

if (write_page->compatible_mode==1){
if (attribute>15) goto error;
if ((col<0)||(col>15)) goto error;
write_page->pal[attribute]=palette_256[col];
return;
}

if (write_page->compatible_mode==0){
if (attribute>15) goto error;
if ((col<0)||(col>63)) goto error;
write_page->pal[attribute]=palette_64[col];
return;
}

error:
error(5);
return;

}




void qbg_sub_color(unsigned long col1,unsigned long col2,unsigned long bordercolor,long passed){
if (new_error) return;
if (!passed){
//performs no action if nothing passed (as in QBASIC for some modes)
return;
}

if (write_page->compatible_mode==32){
if (passed&4) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->background_color=col2;
return;
}
if (write_page->compatible_mode==256){
if (passed&4) goto error;
if (passed&1) if (col1>255) goto error;
if (passed&2) if (col2>255) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->background_color=col2;
return;
}
if (write_page->compatible_mode==13){
//if (passed&6) goto error;
//if (col1>255) goto error;
//write_page->color=col1;
if (passed&4) goto error;
if (passed&1) if (col1>255) goto error;
if (passed&2) if (col2>255) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->background_color=col2;
return;
}
if (write_page->compatible_mode==12){
//if (passed&6) goto error;
//if (col1>15) goto error;
//write_page->color=col1;
if (passed&4) goto error;
if (passed&1) if (col1>15) goto error;
if (passed&2) if (col2>15) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->background_color=col2;
return;
}
if (write_page->compatible_mode==11){
//if (passed&6) goto error;
//if (col1>1) goto error;
//write_page->color=col1;
if (passed&4) goto error;
if (passed&1) if (col1>1) goto error;
if (passed&2) if (col2>1) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->background_color=col2;
return;
}
if (write_page->compatible_mode==10){
if (passed&4) goto error;
if (passed&1) if (col1>3) goto error;
if (passed&2) if (col2>8) goto error;
if (passed&1) write_page->color=col1;
//if (passed&2) ..._color_assign[0]=col2;
if (passed&2) write_page->pal[4]=col2;
return;
}
if (write_page->compatible_mode==9){
if (passed&4) goto error;
if (passed&1) if (col1>15) goto error;
if (passed&2) if (col2>63) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->pal[0]=palette_64[col2];
return;
}
if (write_page->compatible_mode==8){
if (passed&4) goto error;
if (passed&1) if (col1>15) goto error;
if (passed&2) if (col2>15) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->pal[0]=palette_256[col2];
return;
}
if (write_page->compatible_mode==7){
if (passed&4) goto error;
if (passed&1) if (col1>15) goto error;
if (passed&2) if (col2>15) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->pal[0]=palette_256[col2];
return;
}
if (write_page->compatible_mode==2){
if (passed&4) goto error;
if (passed&1) if (col1>1) goto error;
if (passed&2) if (col2>15) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->pal[0]=palette_256[col2];
return;
}
if (write_page->compatible_mode==1){
if (passed&4) goto error;
if (passed&1){
if (col1>15) goto error;
write_page->pal[0]=palette_256[col1];
}
if (passed&2){
if (col2&1){
write_page->pal[1]=palette_256[3];
write_page->pal[2]=palette_256[5];
write_page->pal[3]=palette_256[7];
}else{
write_page->pal[1]=palette_256[2];
write_page->pal[2]=palette_256[4];
write_page->pal[3]=palette_256[6];
}
}
return;
}
if (write_page->compatible_mode==0){
if (passed&1) if (col1>31) goto error;
if (passed&2) if (col2>15) goto error;
if (passed&1) write_page->color=col1;
if (passed&2) write_page->background_color=col2&7;
return;
}
error:
error(5);
return;
}

void defaultcolors(){
write_page->color=15; write_page->background_color=0;
if (write_page->compatible_mode==0){write_page->color=7; write_page->background_color=0;}
if (write_page->compatible_mode==1){write_page->color=3; write_page->background_color=0;}
if (write_page->compatible_mode==2){write_page->color=1; write_page->background_color=0;}
if (write_page->compatible_mode==10){write_page->color=3; write_page->background_color=0;}
if (write_page->compatible_mode==11){write_page->color=1; write_page->background_color=0;}
if (write_page->compatible_mode==32){write_page->color=0xFFFFFFFF; write_page->background_color=0xFF000000;}
return;
}

//Note: Cannot be used to setup page 0, just to validate it
void validatepage(long n){
static long i,i2;
//add new page indexes if necessary
if (n>=pages){
i=n+1;
page=(long*)realloc(page,i*4);
memset(page+pages,0,(i-pages)*4);
pages=i;
}
//create page at index n if none exists
if (!page[n]){
//graphical (assumed)
 i=page[0];
 i2=imgnew(img[i].width,img[i].height,img[i].compatible_mode);
 //modify based on page 0's attributes
 //i. link palette to page 0's palette (if necessary)
 if (img[i2].bytes_per_pixel!=4){
 free(img[i2].pal); img[i2].flags^=IMG_FREEPAL;
 img[i2].pal=img[i].pal;
 }
 //ii. set flags
 img[i2].flags|=IMG_SCREEN;
 //iii. inherit font
 selectfont(img[i].font,&img[i2]);
 //text
 //...
 page[n]=i2;
}
return;
}//validate_page


void qbg_screen(long mode,long color_switch,long active_page,long visual_page,long refresh,long passed){
if (new_error) return;

static long i,i2,i3,x,y,f,p;
static img_struct *im;
static long prev_width_in_characters,prev_height_in_characters;

i=0;//update flags
    //1=mode change required
    //2=page change required (used only to see if an early exit without locking is possible)

i2=page[0];
if (passed&1){//mode
if (mode<0){//custom screen
 i3=-mode;
 if (i3>=nextimg){error(258); return;}//within valid range?
 if (!img[i3].valid){error(258); return;}//valid? 
 if (i3!=i2) i=1; //is mode changing?
}else{
 if (mode==3) goto error;
 if (mode==4) goto error;
 if (mode==5) goto error;
 if (mode==6) goto error;
 if (mode>13) goto error;
 //is mode changing?
 if (i2){
 if (img[i2].compatible_mode!=mode) i=1;
 }else i=1;
 //force update if special parameters passed
 //(at present, only SCREEN 0 is ever called with these overrides, so handling
 // of these is done only in the SCREEN 0 section of the SCREEN sub)
 if ((sub_screen_width_in_characters!=-1)||(sub_screen_height_in_characters!=-1)||(sub_screen_font!=-1)) i=1;
}
}

if (passed&4){//active page
if (active_page<0) goto error;
 if (!(passed&8)){//if visual page not specified, set it to the active page
 passed|=8;
 visual_page=active_page;
 }
if (!(i&1)){//mode not changing
//validate the passed active page, then see if it is the currently selected page
validatepage(active_page); i2=page[active_page];
if ((i2!=read_page_index)||(i2!=write_page_index)) i|=2;
}
}//passed&4

if (passed&8){//visual page
i3=visual_page;
if (i3<0) goto error;
if (!(i&1)){//mode not changing
validatepage(visual_page); i2=page[visual_page];
if (i2!=display_page_index) i|=2;
}
}//passed&8

//if no changes need to be made exit before locking
if (!i) return;

if (lock_display_required){//on init of main(), attempting a lock would create an infinite loop
if (i&1){//avoid locking when only changing the screen page
if (lock_display==0) lock_display=1;
while (lock_display!=2){
Sleep(0);
}
}
}

screen_last_valid=0;//ignore cache used to update the screen on next update

if (passed&1){//mode
if (i&1){//mode change necessary

//calculate previous width & height if possible
prev_width_in_characters=0; prev_height_in_characters=0; 
if (i=page[0]){//currently in a screen mode?
im=&img[i];
if (!im->compatible_mode){
prev_width_in_characters=im->width; prev_height_in_characters=im->height;
}else{
x=fontwidth[im->font]; if (!x) x=1;
prev_width_in_characters=im->width/x;
prev_height_in_characters=im->height/fontheight[im->font];
}
}//currently in a screen mode


//free any previously allocated surfaces
//free pages in reverse order
if (page[0]){//currently in a screen mode?
for (i=1;i<pages;i++){
if(i2=page[i]){
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}//i2
}//i
i=page[0];
if (sub_screen_keep_page0){
img[i].flags^=IMG_SCREEN;
}else{
if (img[i].flags&IMG_FREEMEM) free(img[i].offset);//free pixel data
if (img[i].flags&IMG_FREEPAL) free(img[i].pal);//free palette
freeimg(i);
}
}//currently in a screen mode
sub_screen_keep_page0=0;//reset to default status

pages=1; page[0]=0;

if (mode<0){//custom screen
i=-mode;
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
sub_screen_keep_page0=1;
}

//320 x 200 graphics
//40 x 25 text format, character box size of 8 x 8
//Assignment of up to 256K colors to up to 256 attributes
if (mode==13){
i=imgframe(&cmem[655360],320,200,13);
memset(img[i].offset,0,320*200);
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//13

//640 x 480 graphics
//80 x 30 or 80 x 60 text format, character box size of 8 x 16 or 8 x 8
//Assignment of up to 256K colors to 16 attributes
if (mode==12){
i=imgnew(640,480,12);
if ((prev_width_in_characters==80)&&(prev_height_in_characters==60)) selectfont(8,&img[i]);//override default font
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//12

/*
Screen 11
  ¦ 640 x 480 graphics
  ¦ 80 x 30 or 80 x 60 text format, character box size of 8 x 16 or 8 x 8
  ¦ Assignment of up to 256K colors to 2 attributes
*/
if (mode==11){
i=imgnew(640,480,11);
if ((prev_width_in_characters==80)&&(prev_height_in_characters==60)) selectfont(8,&img[i]);//override default font
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//11

//SCREEN 10: 640 x 350 graphics, monochrome monitor only
//  ¦ 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
//  ¦ 128K page size, page range is 0 (128K) or 0-1 (256K)
//  ¦ Up to 9 pseudocolors assigned to 4 attributes
/*
'colors swap every half second!
'using PALETTE does NOT swap color indexes
'0 black-black
'1 black-grey
'2 black-white
'3 grey-black
'4 grey-grey
'5 grey-white
'6 white-black
'7 white-grey
'8 white-white
'*IMPORTANT* QB sets initial values up different to default palette!
'0 block-black(0)
'1 grey-grey(4)
'2 white-black(6)
'3 white-white(8)
*/
if (mode==10){
i=imgnew(640,350,10);
if ((prev_width_in_characters==80)&&(prev_height_in_characters==43)) selectfont(8,&img[i]);//override default font
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//10

/*
SCREEN 9: 640 x 350 graphics
  ¦ 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
  ¦ 64K page size, page range is 0 (64K);
    128K page size, page range is 0 (128K) or 0-1 (256K)
  ¦ 16 colors assigned to 4 attributes (64K adapter memory), or
    64 colors assigned to 16 attributes (more than 64K adapter memory)
*/
if (mode==9){
i=imgnew(640,350,9);
if ((prev_width_in_characters==80)&&(prev_height_in_characters==43)) selectfont(8,&img[i]);//override default font
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//9

/*
SCREEN 8: 640 x 200 graphics
  ¦ 80 x 25 text format, 8 x 8 character box
  ¦ 64K page size, page ranges are 0 (64K), 0-1 (128K), or 0-3 (246K)
  ¦ Assignment of 16 colors to any of 16 attributes
*/
if (mode==8){
i=imgnew(640,200,8);
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//8

/*
SCREEN 7: 320 x 200 graphics
  ¦ 40 x 25 text format, character box size 8 x 8
  ¦ 32K page size, page ranges are 0-1 (64K), 0-3 (128K), or 0-7 (256K)
  ¦ Assignment of 16 colors to any of 16 attributes
*/
if (mode==7){
i=imgnew(320,200,7);
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//7

/*
SCREEN 4:
  ¦ Supports Olivetti (R) Personal Computers models M24, M240, M28,
    M280, M380, M380/C, M380/T and AT&T (R) Personal Computers 6300
    series
  ¦ 640 x 400 graphics
  ¦ 80 x 25 text format, 8 x 16 character box
  ¦ 1 of 16 colors assigned as the foreground color (selected by the
    COLOR statement); background is fixed at black.
*/
//Note: QB64 will not support SCREEN 4

/*
SCREEN 3: Hercules adapter required, monochrome monitor only
  ¦ 720 x 348 graphics
  ¦ 80 x 25 text format, 9 x 14 character box
  ¦ 2 screen pages (1 only if a second display adapter is installed)
  ¦ PALETTE statement not supported
*/
//Note: QB64 will not support SCREEN 3

/*
SCREEN 2: 640 x 200 graphics
  ¦ 80 x 25 text format with character box size of 8 x 8
  ¦ 16 colors assigned to 2 attributes with EGA or VGA
*/
if (mode==2){
i=imgnew(640,200,2);
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//2

/*
SCREEN 1: 320 x 200 graphics
  ¦ 40 x 25 text format, 8 x 8 character box
  ¦ 16 background colors and one of two sets of 3 foreground colors assigned
    using COLOR statement with CGA
  ¦ 16 colors assigned to 4 attributes with EGA or VGA
*/
if (mode==1){
i=imgnew(320,200,1);
page[0]=i; img[i].flags|=IMG_SCREEN; display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//1

/*
                MDPA, CGA, EGA, or VGA Adapter Boards
SCREEN 0: Text mode only
  ¦ Either 40 x 25, 40 x 43, 40 x 50, 80 x 25, 80 x 43, or 80 x 50 text format
    with 8 x 8 character box size (8 x 14, 9 x 14, or 9 x 16 with EGA or VGA)
  ¦ 16 colors assigned to 2 attributes
  ¦ 16 colors assigned to any of 16 attributes (with CGA or EGA)
  ¦ 64 colors assigned to any of 16 attributes (with EGA or VGA)
*/
/*
granularity from &HB800
4096 in 80x25
2048 in 40x25
6880 in 80x43 (80x43x2=6880)
3440 in 40x43 (40x43x2=3440)
8000 in 80x50 (80x50x2=8000)
4000 in 40x50 (40x50x2=4000)
*/
if (mode==0){

if ((sub_screen_width_in_characters!=-1)&&(sub_screen_height_in_characters!=-1)&&(sub_screen_font!=-1)){
x=sub_screen_width_in_characters; y=sub_screen_height_in_characters; f=sub_screen_font;
sub_screen_width_in_characters=-1; sub_screen_height_in_characters=-1; sub_screen_font=-1;
goto gotwidth;
}
if (sub_screen_width_in_characters!=-1){
x=sub_screen_width_in_characters; sub_screen_width_in_characters=-1;
y=25; f=16;//default
if (prev_height_in_characters==43){y=43; f=14;}
if (prev_height_in_characters==50){y=50; f=8;}
if (x==40) f++;
goto gotwidth;
}
if (sub_screen_height_in_characters!=-1){
y=sub_screen_height_in_characters; sub_screen_height_in_characters=-1;
f=16;//default
if (y==43) f=14;
if (y==50) f=8;
x=80;//default
if (prev_width_in_characters==40){f++; x=40;}
goto gotwidth;
}

if ((prev_width_in_characters==80)&&(prev_height_in_characters==50)){
x=80; y=50; f=8; goto gotwidth;
}
if ((prev_width_in_characters==40)&&(prev_height_in_characters==50)){
x=40; y=50; f=8+1; goto gotwidth;
}
if ((prev_width_in_characters==80)&&(prev_height_in_characters==43)){
x=80; y=43; f=8; goto gotwidth;
}
if ((prev_width_in_characters==40)&&(prev_height_in_characters==43)){
x=40; y=43; f=8+1; goto gotwidth;
}
if ((prev_width_in_characters==40)&&(prev_height_in_characters==25)){
x=40; y=25; f=16+1; goto gotwidth;
}
x=80; y=25; f=16;
gotwidth:;
i2=x*y*2;//default granularity
//specific granularities which cannot be calculated
if ((x==40)&&(y==25)&&(f=(16+1))) i2=2048;
if ((x==80)&&(y==25)&&(f=16)) i2=4096;
p=65536/i2;//number of pages to allocate in cmem
if (p>8) p=8;//limit cmem pages to 8
 //make sure 8 page indexes exist
 if (7>=pages){
 i=7+1;
 page=(long*)realloc(page,i*4);
 memset(page+pages,0,(i-pages)*4);
 pages=i;
 }
for (i3=0;i3<8;i3++){
if (i3<p){
i=imgframe(&cmem[753664+i2*i3],x,y,0);
}else{
i=imgnew(x,y,0);
}
selectfont(f,&img[i]);
img[i].flags|=IMG_SCREEN;
page[i3]=i;
}
//text-clear 64K after seg. &HB800
for (i=0;i<65536;i+=2){cmem[753664+i]=32; cmem[753664+i+1]=7;}//init. 64K of memory after B800
i=page[0];
display_page_index=i; display_page=&img[i]; write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
}//0

}//setmode
}//passed MODE

//note: changing the active or visual page reselects the default colors!
if (passed&4){//SCREEN ?,?,X,? (active_page)
i=active_page; validatepage(i); i=page[i];
if ((write_page_index!=i)||(read_page_index!=i)){
write_page_index=i; write_page=&img[i]; read_page_index=i; read_page=&img[i];
defaultcolors();
}
}//passed&4

if (passed&8){//SCREEN ?,?,?,X (visual_page)
i=visual_page; validatepage(i); i=page[i];
if (display_page_index!=i){
display_page_index=i; display_page=&img[i];
defaultcolors();
}
}//passed&8

if (lock_display_required) lock_display=0;//release lock

return;
error:
error(5);
return;
}//screen (end)

void sub_pcopy(long src,long dst){
if (new_error) return;
static img_struct *s,*d;
//validate
if (src>=0){
validatepage(src); s=&img[page[src]];
}else{
src=-src;
if (src>=nextimg) goto error;
s=&img[src];
if (!s->valid) goto error;
}
if (dst>=0){
validatepage(dst); d=&img[page[dst]];
}else{
dst=-dst;
if (dst>=nextimg) goto error;
d=&img[dst];
if (!d->valid) goto error;
}
if (s==d) return;
if (s->bytes_per_pixel!=d->bytes_per_pixel) goto error;
if ((s->height!=d->height)||(s->width!=d->width)) goto error;
if (s->bytes_per_pixel==1){
if (d->mask<s->mask) goto error;//cannot copy onto a palette image with less colors
}
memcpy(d->offset,s->offset,d->width*d->height*d->bytes_per_pixel);
return;
error:
error(5);
return;
}

void qbsub_width(long option,long value1,long value2,long passed){
//[{#|LPRINT}][?],[?]
static long i,i2;

if (new_error) return;

if (option==0){//WIDTH [?][,?]
static unsigned long col,col2;

//used to restore scaling after simple font changes
//QBASIC/4.5/7.1: PMAP uses old scaling values after WIDTH change
static float window_x1,window_y1,window_x2,window_y2;

//Specifics:
//MODE 0: Changes the resolution based on the desired width
//        Horizontal width of 1 to 40 uses a double width font
//        Heights from 1 to 42 use font height 16 pixels
//        Heights from 43 to 49 use font height 14 pixels
//        Heights from 50 to ? use font height 8 pixels
//MODES 1-13: The resolution IS NOT CHANGED
//            The font is changed to a font usually available for that screen
//            mode, if available, that fits the given dimensions EXACTLY
//            If not possible, it may jump back to SCREEN 0 in some instances
//            just as it did in QBASIC
//256/32 BIT MODES: The font is unchanged
//                  The resolution is changed using the currently selected font
//note:
//COLOR selection is kept, all other values are lost (if staying in same "mode")
static long f,f2,width,height;

if ((!(passed&1))&&(!(passed&2))) goto error;//cannot omit both arguments

width=value1; height=value2;

if ((write_page->compatible_mode==32)||(write_page->compatible_mode==256)){

if (!(passed&1)){//width ommited
width=write_page->width;
}else{
if (width<=0) goto error;
i=fontwidth[write_page->font]; if (!i) i=1;
width*=i;
}

if (!(passed&2)){//height ommited
height=write_page->height;
}else{
if (height<=0) goto error;
height*=fontheight[write_page->font];
}

//width & height are now the desired dimensions

if ((width==write_page->width)&&(height==write_page->height)) return;//no change required

if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}

if (write_page->flags&IMG_SCREEN){
 if (lock_display_required){
 if (lock_display==0) lock_display=1;
 while (lock_display!=2){
 Sleep(0);
 }
 }
}

col=write_page->color; col2=write_page->background_color;
f=write_page->font;
//change resolution
write_page->width=width; write_page->height=height;
if (write_page->flags&IMG_FREEMEM){
free(write_page->offset);//free pixel data
write_page->offset=(unsigned char*)calloc(width*height*write_page->bytes_per_pixel,1);
}else{//frame?
memset(write_page->offset,0,width*height*write_page->bytes_per_pixel);
}
imgrevert(write_page_index);
write_page->color=col; write_page->background_color=col2;
selectfont(f,write_page);

if (write_page->flags&IMG_SCREEN){
 if (lock_display_required) lock_display=0;//release lock
}

return;

}//32/256

if (!(passed&1)){//width ommited
if (height<=0) goto error;

if (!write_page->compatible_mode){//0
f=8;
if (height<=49) f=14;
if (height<=42) f=16;
width=write_page->width;
if (width<=40) f++;
if ((write_page->font==f)&&(write_page->height==height)) return;//no change
sub_screen_height_in_characters=height; sub_screen_width_in_characters=width;
sub_screen_font=f;
qbg_screen(0,0,0,0,0,1);
return;
}//0

if (((write_page->compatible_mode>=1)&&(write_page->compatible_mode<=8))||(write_page->compatible_mode==13)){
if (write_page->height==height*8){//correct resolution
if (write_page->font==8) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
imgrevert(write_page_index);
write_page->color=col; write_page->background_color=col2;
selectfont(8,write_page);
return;
}//correct resolution
//fall through
}//modes 1-8

/*
SCREEN 9: 640 x 350 graphics
  ¦ 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
  ¦ 64K page size, page range is 0 (64K);
    128K page size, page range is 0 (128K) or 0-1 (256K)
  ¦ 16 colors assigned to 4 attributes (64K adapter memory), or
    64 colors assigned to 16 attributes (more than 64K adapter memory)
SCREEN 10: 640 x 350 graphics, monochrome monitor only
  ¦ 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
  ¦ 128K page size, page range is 0 (128K) or 0-1 (256K)
  ¦ Up to 9 pseudocolors assigned to 4 attributes
*/
if ((write_page->compatible_mode>=9)&&(write_page->compatible_mode<=10)){
f=0;
if(write_page->height==height*8) f=8;
if(write_page->height==height*14) f=14;
if((height==43)&&(write_page->height==350)) f=8;//?x350,8x8
if(f){//correct resolution
if (write_page->font==f) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
window_x1=write_page->window_x1; window_x2=write_page->window_x2; window_y1=write_page->window_y1; window_y2=write_page->window_y2;
imgrevert(write_page_index);
qbg_sub_window(1,window_x1,window_y1,window_x2,window_y2,1); write_page->clipping_or_scaling=0;
write_page->color=col; write_page->background_color=col2;
selectfont(f,write_page);
return;
}//correct resolution
//fall through
}//modes 9,10

if ((write_page->compatible_mode>=11)&&(write_page->compatible_mode<=12)){
f=0;
if(write_page->height==height*8) f=8;
if(write_page->height==height*16) f=16;
if(f){//correct resolution
if (write_page->font==f) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
window_x1=write_page->window_x1; window_x2=write_page->window_x2; window_y1=write_page->window_y1; window_y2=write_page->window_y2;
imgrevert(write_page_index);
qbg_sub_window(1,window_x1,window_y1,window_x2,window_y2,1); write_page->clipping_or_scaling=0;
write_page->color=col; write_page->background_color=col2;
selectfont(f,write_page);
return;
}//correct resolution
//fall through
}//modes 11,12

//fall through:
if ((height==25)||(height==50)||(height==43)){
sub_screen_height_in_characters=height; qbg_screen(0,0,0,0,0,1);
return;
}

goto error;

}//width omitted

if (!(passed&2)){//height omitted
if (width<=0) goto error;

if (!write_page->compatible_mode){//0
height=write_page->height;
f=8;
if (height<=49) f=14;
if (height<=42) f=16;
if (width<=40) f++;
if ((write_page->font==f)&&(write_page->width==width)) return;//no change
sub_screen_height_in_characters=height; sub_screen_width_in_characters=width;
sub_screen_font=f;
qbg_screen(0,0,0,0,0,1);
return;
}//0

if (((write_page->compatible_mode>=1)&&(write_page->compatible_mode<=8))||(write_page->compatible_mode==13)){
if (write_page->width==width*8){//correct resolution
if (write_page->font==8) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
imgrevert(write_page_index);
write_page->color=col; write_page->background_color=col2;
selectfont(8,write_page);
return;
}//correct resolution
//fall through
}//modes 1-8

/*
SCREEN 9: 640 x 350 graphics
  ¦ 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
  ¦ 64K page size, page range is 0 (64K);
    128K page size, page range is 0 (128K) or 0-1 (256K)
  ¦ 16 colors assigned to 4 attributes (64K adapter memory), or
    64 colors assigned to 16 attributes (more than 64K adapter memory)
SCREEN 10: 640 x 350 graphics, monochrome monitor only
  ¦ 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
  ¦ 128K page size, page range is 0 (128K) or 0-1 (256K)
  ¦ Up to 9 pseudocolors assigned to 4 attributes
*/
if ((write_page->compatible_mode>=9)&&(write_page->compatible_mode<=10)){
f=0;
if (write_page->width==width*8) f=8;
if (f){//correct resolution
f2=fontheight[write_page->font]; if (f2>8) f=14;
if (write_page->font==f) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
window_x1=write_page->window_x1; window_x2=write_page->window_x2; window_y1=write_page->window_y1; window_y2=write_page->window_y2;
imgrevert(write_page_index);
qbg_sub_window(1,window_x1,window_y1,window_x2,window_y2,1); write_page->clipping_or_scaling=0;
write_page->color=col; write_page->background_color=col2;
selectfont(f,write_page);
return;
}//correct resolution
//fall through
}//modes 9,10

if ((write_page->compatible_mode>=11)&&(write_page->compatible_mode<=12)){
f=0;
if (write_page->width==width*8) f=8;
if (f){//correct resolution
f2=fontheight[write_page->font]; if (f2>8) f=16;
if (write_page->font==f) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
window_x1=write_page->window_x1; window_x2=write_page->window_x2; window_y1=write_page->window_y1; window_y2=write_page->window_y2;
imgrevert(write_page_index);
qbg_sub_window(1,window_x1,window_y1,window_x2,window_y2,1); write_page->clipping_or_scaling=0;
write_page->color=col; write_page->background_color=col2;
selectfont(f,write_page);
return;
}//correct resolution
//fall through
}//modes 11,12

//fall through:
if ((width==40)||(width==80)){
sub_screen_width_in_characters=width;
qbg_screen(0,0,0,0,0,1);
return;
}

goto error;

}//height omitted

//both height & width passed

if ((width<=0)||(height<=0)) goto error;

if (!write_page->compatible_mode){//0
f=8;
if (height<=49) f=14;
if (height<=42) f=16;
if (width<=40) f++;
if ((write_page->font==f)&&(write_page->width==width)&&(write_page->height==height)) return;//no change
sub_screen_height_in_characters=height; sub_screen_width_in_characters=width;
sub_screen_font=f;
qbg_screen(0,0,0,0,0,1);
return;
}//0

if (((write_page->compatible_mode>=1)&&(write_page->compatible_mode<=8))||(write_page->compatible_mode==13)){
if ((write_page->width==width*8)&&(write_page->height==height*8)){//correct resolution
if (write_page->font==8) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
imgrevert(write_page_index);
write_page->color=col; write_page->background_color=col2;
selectfont(8,write_page);
return;
}//correct resolution
//fall through
}//modes 1-8

/*
SCREEN 9: 640 x 350 graphics
  ¦ 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
  ¦ 64K page size, page range is 0 (64K);
    128K page size, page range is 0 (128K) or 0-1 (256K)
  ¦ 16 colors assigned to 4 attributes (64K adapter memory), or
    64 colors assigned to 16 attributes (more than 64K adapter memory)
SCREEN 10: 640 x 350 graphics, monochrome monitor only
  ¦ 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
  ¦ 128K page size, page range is 0 (128K) or 0-1 (256K)
  ¦ Up to 9 pseudocolors assigned to 4 attributes
*/
if ((write_page->compatible_mode>=9)&&(write_page->compatible_mode<=10)){
f=0;
if (write_page->width==width*8){
if (write_page->height==height*8) f=8;
if (write_page->height==height*14) f=14;
if ((height==43)&&(write_page->height==350)) f=8;//?x350,8x8
}
if (f){//correct resolution
if (write_page->font==f) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
window_x1=write_page->window_x1; window_x2=write_page->window_x2; window_y1=write_page->window_y1; window_y2=write_page->window_y2;
imgrevert(write_page_index);
qbg_sub_window(1,window_x1,window_y1,window_x2,window_y2,1); write_page->clipping_or_scaling=0;
write_page->color=col; write_page->background_color=col2;
selectfont(f,write_page);
return;
}//correct resolution
//fall through
}//modes 9,10

if ((write_page->compatible_mode>=11)&&(write_page->compatible_mode<=12)){
f=0;
if (write_page->width==width*8){
if (write_page->height==height*8) f=8;
if (write_page->height==height*16) f=16;
}
if (f){//correct resolution
if (write_page->font==f) return;//correct font, no change required
if (write_page->flags&IMG_SCREEN){
//delete pages 1-?
for(i=1;i<pages;i++){
if(i2=page[i]){
if (display_page_index==i2){display_page_index=page[0]; display_page=&img[display_page_index];}
if (read_page_index==i2){read_page_index=display_page_index; read_page=display_page;}
if (write_page_index==i2){write_page_index=display_page_index; write_page=display_page;}
//manual delete, freeing video pages is usually illegal
if (img[i2].flags&IMG_FREEMEM) free(img[i2].offset);//free pixel data
freeimg(i2);
}
}//i
}
col=write_page->color; col2=write_page->background_color;
window_x1=write_page->window_x1; window_x2=write_page->window_x2; window_y1=write_page->window_y1; window_y2=write_page->window_y2;
imgrevert(write_page_index);
qbg_sub_window(1,window_x1,window_y1,window_x2,window_y2,1); write_page->clipping_or_scaling=0;
write_page->color=col; write_page->background_color=col2;
selectfont(f,write_page);
return;
}//correct resolution
//fall through
}//modes 11,12

//fall through:
if ((width==40)||(width==80)){
if ((height==25)||(height==50)||(height==43)){
sub_screen_width_in_characters=width; sub_screen_height_in_characters=height;
f=16;
if (height==43) f=14;
if (height==50) f=8;
if (width==40) f++;
sub_screen_font=f;
qbg_screen(0,0,0,0,0,1);
return;
}

goto error;

}//WIDTH [?][,?]

//file/device?
//...
//printer?
//...
}

error:
error(5);
return;
}

void pset_and_clip(long x,long y,unsigned long col){

if ((x>=write_page->view_x1)&&(x<=write_page->view_x2)&&(y>=write_page->view_y1)&&(y<=write_page->view_y2)){

static unsigned char *cp;
static unsigned long *o32;
static unsigned long destcol;
if (write_page->bytes_per_pixel==1){
 write_page->offset[y*write_page->width+x]=col&write_page->mask;
 return;
}else{

 if (write_page->alpha_disabled){
 write_page->offset32[y*write_page->width+x]=col;
 return;
 }
switch(col&0xFF000000){
case 0xFF000000://100% alpha, so regular pset (fast)
 write_page->offset32[y*write_page->width+x]=col;
 return;
break;
case 0x0://0%(0) alpha, so no pset (very fast)
 return;
break;
case 0x80000000://~50% alpha (optomized)
 o32=write_page->offset32+(y*write_page->width+x);
 *o32=(((*o32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend128[*o32>>24]<<24);
 return;
break; 
case 0x7F000000://~50% alpha (optomized)
 o32=write_page->offset32+(y*write_page->width+x);
 *o32=(((*o32&0xFEFEFE)+(col&0xFEFEFE))>>1)+(ablend127[*o32>>24]<<24);
 return;
break;
default://other alpha values (uses a lookup table)
 o32=write_page->offset32+(y*write_page->width+x);
 destcol=*o32;
 cp=blend+(col>>24<<16);
 *o32=
   cp[(col<<8&0xFF00)+(destcol&255)    ]
 +(cp[(col&0xFF00)   +(destcol>>8&255) ]<<8)
 +(cp[(col>>8&0xFF00)+(destcol>>16&255)]<<16)
 +(ablend[(col>>24)+(destcol>>16&0xFF00)]<<24);
};
}

}//within viewport
return;
}

void qb32_boxfill(float x1f,float y1f,float x2f,float y2f,unsigned long col){
static long x1,y1,x2,y2,i,width,img_width,x,y,d_width,a,a2,v1,v2,v3;
static unsigned char *p,*cp,*cp2,*cp3;
static unsigned long *lp,*lp_last,*lp_first;
static unsigned long *doff32,destcol;

//resolve coordinates
if (write_page->clipping_or_scaling){
if (write_page->clipping_or_scaling==2){
x1=qbr_float_to_long(x1f*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
y1=qbr_float_to_long(y1f*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
x2=qbr_float_to_long(x2f*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
y2=qbr_float_to_long(y2f*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
}else{
x1=qbr_float_to_long(x1f)+write_page->view_offset_x; y1=qbr_float_to_long(y1f)+write_page->view_offset_y;
x2=qbr_float_to_long(x2f)+write_page->view_offset_x; y2=qbr_float_to_long(y2f)+write_page->view_offset_y;
}
}else{
x1=qbr_float_to_long(x1f); y1=qbr_float_to_long(y1f);
x2=qbr_float_to_long(x2f); y2=qbr_float_to_long(y2f);
}

//swap coordinates (if necessary)
if (x1>x2){i=x1; x1=x2; x2=i;}
if (y1>y2){i=y1; y1=y2; y2=i;}

//exit without rendering if necessary
if (x2<write_page->view_x1) return;
if (x1>write_page->view_x2) return;
if (y2<write_page->view_y1) return;
if (y1>write_page->view_y2) return;

//crop coordinates
if (x1<write_page->view_x1) x1=write_page->view_x1;
if (y1<write_page->view_y1) y1=write_page->view_y1;
if (x1>write_page->view_x2) x1=write_page->view_x2;
if (y1>write_page->view_y2) y1=write_page->view_y2;
if (x2<write_page->view_x1) x2=write_page->view_x1;
if (y2<write_page->view_y1) y2=write_page->view_y1;
if (x2>write_page->view_x2) x2=write_page->view_x2;
if (y2>write_page->view_y2) y2=write_page->view_y2;

if (write_page->bytes_per_pixel==1){
col&=write_page->mask;
width=x2-x1+1;
img_width=write_page->width;
p=write_page->offset+y1*write_page->width+x1;
i=y2-y1+1;
loop:
memset(p,col,width);
p+=img_width;
if (--i) goto loop;
return;
}//1

//assume 32-bit
//optomized
//alpha disabled or full alpha?
a=col>>24;
if ((write_page->alpha_disabled)||(a==255)){
width=x2-x1+1;
y=y2-y1+1;
img_width=write_page->width;
//build first line pixel by pixel
lp_first=write_page->offset32+y1*img_width+x1;
lp=lp_first-1; lp_last=lp+width;
while (lp++<lp_last) *lp=col;
//copy remaining lines
lp=lp_first;
width*=4;
while(y--){
memcpy(lp,lp_first,width);
lp+=img_width;
}
return;
}
//no alpha?
if (!a) return;
//half alpha?
img_width=write_page->width;
doff32=write_page->offset32+y1*img_width+x1;
width=x2-x1+1;
d_width=img_width-width;
if (a==128){
col&=0xFEFEFE;
y=y2-y1+1;
while(y--){
x=width;
while(x--){
*doff32++=(((*doff32&0xFEFEFE)+col)>>1)+(ablend128[*doff32>>24]<<24);
}
doff32+=d_width;
}
return;
}
if (a==127){
col&=0xFEFEFE;
y=y2-y1+1;
while(y--){
x=width;
while(x--){
*doff32++=(((*doff32&0xFEFEFE)+col)>>1)+(ablend127[*doff32>>24]<<24);
}
doff32+=d_width;
}
return;
}
//ranged alpha
cp=blend+(a<<16);
a2=a<<8;
cp3=cp+(col>>8&0xFF00);
cp2=cp+(col&0xFF00);
cp+=(col<<8&0xFF00);
y=y2-y1+1;
while(y--){
x=width;
while(x--){
 destcol=*doff32;
 *doff32++=
   cp[destcol&255]
 +(cp2[destcol>>8&255]<<8)
 +(cp3[destcol>>16&255]<<16)
 +(ablend[destcol>>24+a2]<<24);
}
doff32+=d_width;
}
return;
}


void fast_boxfill(long x1,long y1,long x2,long y2,unsigned long col){
//assumes:
//actual coordinates passed
//left->right, top->bottom order
//on-screen
static long i,width,img_width,x,y,d_width,a,a2,v1,v2,v3;
static unsigned char *p,*cp,*cp2,*cp3;
static unsigned long *lp,*lp_last,*lp_first;
static unsigned long *doff32,destcol;

if (write_page->bytes_per_pixel==1){
col&=write_page->mask;
width=x2-x1+1;
img_width=write_page->width;
p=write_page->offset+y1*write_page->width+x1;
i=y2-y1+1;
loop:
memset(p,col,width);
p+=img_width;
if (--i) goto loop;
return;
}//1

//assume 32-bit
//optomized
//alpha disabled or full alpha?
a=col>>24;
if ((write_page->alpha_disabled)||(a==255)){

width=x2-x1+1;
y=y2-y1+1;
img_width=write_page->width;
//build first line pixel by pixel
lp_first=write_page->offset32+y1*img_width+x1;
lp=lp_first-1; lp_last=lp+width;
while (lp++<lp_last) *lp=col;
//copy remaining lines
lp=lp_first;
width*=4;
while(y--){
memcpy(lp,lp_first,width);
lp+=img_width;
}
return;
}
//no alpha?
if (!a) return;
//half alpha?
img_width=write_page->width;
doff32=write_page->offset32+y1*img_width+x1;
width=x2-x1+1;
d_width=img_width-width;
if (a==128){
col&=0xFEFEFE;
y=y2-y1+1;
while(y--){
x=width;
while(x--){
*doff32++=(((*doff32&0xFEFEFE)+col)>>1)+(ablend128[*doff32>>24]<<24);
}
doff32+=d_width;
}
return;
}
if (a==127){
col&=0xFEFEFE;
y=y2-y1+1;
while(y--){
x=width;
while(x--){
*doff32++=(((*doff32&0xFEFEFE)+col)>>1)+(ablend127[*doff32>>24]<<24);
}
doff32+=d_width;
}
return;
}
//ranged alpha
cp=blend+(a<<16);
a2=a<<8;
cp3=cp+(col>>8&0xFF00);
cp2=cp+(col&0xFF00);
cp+=(col<<8&0xFF00);
y=y2-y1+1;
while(y--){
x=width;
while(x--){
 destcol=*doff32;
 *doff32++=
   cp[destcol&255]
 +(cp2[destcol>>8&255]<<8)
 +(cp3[destcol>>16&255]<<16)
 +(ablend[destcol>>24+a2]<<24);
}
doff32+=d_width;
}
return;
}






//copied from qb32_line with the following modifications
//i. pre-WINDOW'd & VIEWPORT'd long co-ordinates
//ii. all references to style & lineclip_skippixels commented
//iii. declaration of x1,y1,x2,y2,x1f,y1f changed, some declarations removed
void fast_line(long x1,long y1,long x2,long y2,unsigned long col){
static long l,l2,mi;
static float m,x1f,y1f;

lineclip(x1,y1,x2,y2,write_page->view_x1,write_page->view_y1,write_page->view_x2,write_page->view_y2);

//style=(style&65535)+(style<<16);
//lineclip_skippixels&=15;
//style=_lrotl(style,lineclip_skippixels);

if (lineclip_draw){
l=abs(lineclip_x1-lineclip_x2);
l2=abs(lineclip_y1-lineclip_y2);
if (l>l2){

//x-axis distance is larger
y1f=lineclip_y1;
if (l){//following only applies if drawing more than one pixel
m=((float)lineclip_y2-(float)lineclip_y1)/(float)l;
if (lineclip_x2>=lineclip_x1) mi=1; else mi=-1;//direction of change
}
l++;
while (l--){
if (y1f<0) lineclip_y1=y1f-0.5f; else lineclip_y1=y1f+0.5f;

//if ((style=_lrotl(style,1))&1){
pset(lineclip_x1,lineclip_y1,col);
//}

lineclip_x1+=mi;
y1f+=m;
}

}else{

//y-axis distance is larger
x1f=lineclip_x1;
if (l2){//following only applies if drawing more than one pixel
m=((float)lineclip_x2-(float)lineclip_x1)/(float)l2;
if (lineclip_y2>=lineclip_y1) mi=1; else mi=-1;//direction of change
}
l2++;
while (l2--){
if (x1f<0) lineclip_x1=x1f-0.5f; else lineclip_x1=x1f+0.5f;
//if ((style=_lrotl(style,1))&1){
pset(lineclip_x1,lineclip_y1,col);
//}
lineclip_y1+=mi;
x1f+=m;
}

}

}//lineclip_draw
return;
}























void qb32_line(float x1f,float y1f,float x2f,float y2f,unsigned long col,unsigned long style){
static long x1,y1,x2,y2,l,l2,mi;
static float m;

//resolve coordinates
if (write_page->clipping_or_scaling){
if (write_page->clipping_or_scaling==2){
x1=qbr_float_to_long(x1f*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
y1=qbr_float_to_long(y1f*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
x2=qbr_float_to_long(x2f*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
y2=qbr_float_to_long(y2f*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
}else{
x1=qbr_float_to_long(x1f)+write_page->view_offset_x; y1=qbr_float_to_long(y1f)+write_page->view_offset_y;
x2=qbr_float_to_long(x2f)+write_page->view_offset_x; y2=qbr_float_to_long(y2f)+write_page->view_offset_y;
}
}else{
x1=qbr_float_to_long(x1f); y1=qbr_float_to_long(y1f);
x2=qbr_float_to_long(x2f); y2=qbr_float_to_long(y2f);
}

lineclip(x1,y1,x2,y2,write_page->view_x1,write_page->view_y1,write_page->view_x2,write_page->view_y2);

style=(style&65535)+(style<<16);
lineclip_skippixels&=15;
style=_lrotl(style,lineclip_skippixels);

if (lineclip_draw){
l=abs(lineclip_x1-lineclip_x2);
l2=abs(lineclip_y1-lineclip_y2);
if (l>l2){

//x-axis distance is larger
y1f=lineclip_y1;
if (l){//following only applies if drawing more than one pixel
m=((float)lineclip_y2-(float)lineclip_y1)/(float)l;
if (lineclip_x2>=lineclip_x1) mi=1; else mi=-1;//direction of change
}
l++;
while (l--){
if (y1f<0) lineclip_y1=y1f-0.5f; else lineclip_y1=y1f+0.5f;

if ((style=_lrotl(style,1))&1){
pset(lineclip_x1,lineclip_y1,col);
}

lineclip_x1+=mi;
y1f+=m;
}

}else{

//y-axis distance is larger
x1f=lineclip_x1;
if (l2){//following only applies if drawing more than one pixel
m=((float)lineclip_x2-(float)lineclip_x1)/(float)l2;
if (lineclip_y2>=lineclip_y1) mi=1; else mi=-1;//direction of change
}
l2++;
while (l2--){
if (x1f<0) lineclip_x1=x1f-0.5f; else lineclip_x1=x1f+0.5f;
if ((style=_lrotl(style,1))&1){
pset(lineclip_x1,lineclip_y1,col);
}
lineclip_y1+=mi;
x1f+=m;
}

}

}//lineclip_draw
return;
}


void sub_line(long step1,float x1,float y1,long step2,float x2,float y2,unsigned long col,long bf,unsigned long style,long passed){
if (new_error) return;
if (write_page->text){error(5); return;}
//format: [{STEP}][(?,?)]-[{STEP}](?,?),[?],[{B|BF}],[?]
//                  1 2                  4            8

//[[{STEP}](?,?)]-[{STEP}](?,?)[,[?][,[{B|BF}][,?]]]
//          1                     2             4

//adjust coordinates and qb graphics cursor position based on STEP
if (passed&1){
if (step1){x1=write_page->x+x1; y1=write_page->y+y1;}
write_page->x=x1; write_page->y=y1;
}else{
x1=write_page->x; y1=write_page->y;
}
if (step2){x2=write_page->x+x2; y2=write_page->y+y2;}
write_page->x=x2; write_page->y=y2;

if (bf==0){//line
if ((passed&4)==0) style=0xFFFF;
if ((passed&2)==0) col=write_page->color;
qb32_line(x1,y1,x2,y2,col,style);
return;
}

if (bf==1){//rectangle
if ((passed&4)==0) style=0xFFFF;
if ((passed&2)==0) col=write_page->color;
qb32_line(x1,y1,x2,y1,col,style);
qb32_line(x2,y1,x2,y2,col,style);
qb32_line(x2,y2,x1,y2,col,style);
qb32_line(x1,y2,x1,y1,col,style);
return;
}

if (bf==2){//filled box
if ((passed&2)==0) col=write_page->color;
qb32_boxfill(x1,y1,x2,y2,col);
return;
}

}//sub_line

































//3 paint routines exist for color (not textured) filling
//i) 8-bit
//ii) 32-bit no-alpha
//iii) 32-bit
//simple comparisons are used, the alpha value is part of that comparison in all cases
//even if blending is disabled (a fixed color is likely to have a fixed alpha value anyway),
//and this allows for filling alpha regions

//32-bit WITH BENDING
void sub_paint32(long step,float x,float y,unsigned long fillcol,unsigned long bordercol,long passed){

//uses 2 buffers, a and b, and swaps between them for reading and creating
static unsigned long a_n=0;
static unsigned short *a_x=(unsigned short*)malloc(2*65536),*a_y=(unsigned short*)malloc(2*65536);
static unsigned char *a_t=(unsigned char*)malloc(65536);
static unsigned long b_n=0;
static unsigned short *b_x=(unsigned short*)malloc(2*65536),*b_y=(unsigned short*)malloc(2*65536);
static unsigned char *b_t=(unsigned char*)malloc(65536);
static unsigned char *done=(unsigned char*)calloc(640*480,1);
static long ix,iy,i,t,x2,y2;
static unsigned long offset;
static unsigned char *cp;
static unsigned short *sp;
//overrides
static long done_size=640*480;
static unsigned long *qbg_active_page_offset;//override
static long qbg_width,qbg_view_x1,qbg_view_y1,qbg_view_x2,qbg_view_y2;//override
static unsigned long *doff32,destcol;

if ((passed&1)==0) fillcol=write_page->color;
if ((passed&2)==0) bordercol=fillcol;

if (step){write_page->x+=x; write_page->y+=y;}else{write_page->x=x; write_page->y=y;}

if (write_page->clipping_or_scaling){
if (write_page->clipping_or_scaling==2){
ix=qbr_float_to_long(write_page->x*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
iy=qbr_float_to_long(write_page->y*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
}else{
ix=qbr_float_to_long(write_page->x)+write_page->view_offset_x; iy=qbr_float_to_long(write_page->y)+write_page->view_offset_y;
}
}else{
ix=qbr_float_to_long(write_page->x); iy=qbr_float_to_long(write_page->y);
}

//return if offscreen
if ((ix<write_page->view_x1)||(iy<write_page->view_y1)||(ix>write_page->view_x2)||(iy>write_page->view_y2)){
return;
}

//overrides
qbg_active_page_offset=write_page->offset32;
qbg_width=write_page->width;
qbg_view_x1=write_page->view_x1;
qbg_view_y1=write_page->view_y1;
qbg_view_x2=write_page->view_x2;
qbg_view_y2=write_page->view_y2;
i=write_page->width*write_page->height;
if (i>done_size){
free(done);
done=(unsigned char*)calloc(i,1);
}

//return if first point is the bordercolor
if (qbg_active_page_offset[iy*qbg_width+ix]==bordercol) return;

//create first node
a_x[0]=ix; a_y[0]=iy;
a_t[0]=15;
//types:
//&1=check left
//&2=check right
//&4=check above
//&8=check below

a_n=1;
//qbg_active_page_offset[iy*qbg_width+ix]=fillcol;
offset=iy*qbg_width+ix;
 //--------plot pixel--------
 doff32=qbg_active_page_offset+offset;
 switch(fillcol&0xFF000000){
 case 0xFF000000:
  *doff32=fillcol;
 break;
 case 0x0:
  doff32;
 break;
 case 0x80000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend128[*doff32>>24]<<24);
 break; 
 case 0x7F000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend127[*doff32>>24]<<24);
 break;
 default:
  destcol=*doff32;
  cp=blend+(fillcol>>24<<16);
  *doff32=
    cp[(fillcol<<8&0xFF00)+(destcol&255)    ]
  +(cp[(fillcol&0xFF00)   +(destcol>>8&255) ]<<8)
  +(cp[(fillcol>>8&0xFF00)+(destcol>>16&255)]<<16)
  +(ablend[(fillcol>>24)+(destcol>>16&0xFF00)]<<24);
 };//switch
 //--------done plot pixel--------
done[iy*qbg_width+ix]=1;

nextpass:
b_n=0;
for (i=0;i<a_n;i++){
t=a_t[i]; ix=a_x[i]; iy=a_y[i];

//left
if (t&1){
x2=ix-1; y2=iy;
if (x2>=qbg_view_x1){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
 //--------plot pixel--------
 doff32=qbg_active_page_offset+offset;
 switch(fillcol&0xFF000000){
 case 0xFF000000:
  *doff32=fillcol;
 break;
 case 0x0:
  doff32;
 break;
 case 0x80000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend128[*doff32>>24]<<24);
 break; 
 case 0x7F000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend127[*doff32>>24]<<24);
 break;
 default:
  destcol=*doff32;
  cp=blend+(fillcol>>24<<16);
  *doff32=
    cp[(fillcol<<8&0xFF00)+(destcol&255)    ]
  +(cp[(fillcol&0xFF00)   +(destcol>>8&255) ]<<8)
  +(cp[(fillcol>>8&0xFF00)+(destcol>>16&255)]<<16)
  +(ablend[(fillcol>>24)+(destcol>>16&0xFF00)]<<24);
 };//switch
 //--------done plot pixel--------
b_t[b_n]=13; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//right
if (t&2){
x2=ix+1; y2=iy;
if (x2<=qbg_view_x2){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
 //--------plot pixel--------
 doff32=qbg_active_page_offset+offset;
 switch(fillcol&0xFF000000){
 case 0xFF000000:
  *doff32=fillcol;
 break;
 case 0x0:
  doff32;
 break;
 case 0x80000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend128[*doff32>>24]<<24);
 break; 
 case 0x7F000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend127[*doff32>>24]<<24);
 break;
 default:
  destcol=*doff32;
  cp=blend+(fillcol>>24<<16);
  *doff32=
    cp[(fillcol<<8&0xFF00)+(destcol&255)    ]
  +(cp[(fillcol&0xFF00)   +(destcol>>8&255) ]<<8)
  +(cp[(fillcol>>8&0xFF00)+(destcol>>16&255)]<<16)
  +(ablend[(fillcol>>24)+(destcol>>16&0xFF00)]<<24);
 };//switch
 //--------done plot pixel--------
b_t[b_n]=14; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//above
if (t&4){
x2=ix; y2=iy-1;
if (y2>=qbg_view_y1){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
 //--------plot pixel--------
 doff32=qbg_active_page_offset+offset;
 switch(fillcol&0xFF000000){
 case 0xFF000000:
  *doff32=fillcol;
 break;
 case 0x0:
  doff32;
 break;
 case 0x80000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend128[*doff32>>24]<<24);
 break; 
 case 0x7F000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend127[*doff32>>24]<<24);
 break;
 default:
  destcol=*doff32;
  cp=blend+(fillcol>>24<<16);
  *doff32=
    cp[(fillcol<<8&0xFF00)+(destcol&255)    ]
  +(cp[(fillcol&0xFF00)   +(destcol>>8&255) ]<<8)
  +(cp[(fillcol>>8&0xFF00)+(destcol>>16&255)]<<16)
  +(ablend[(fillcol>>24)+(destcol>>16&0xFF00)]<<24);
 };//switch
 //--------done plot pixel--------
b_t[b_n]=7; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//below
if (t&8){
x2=ix; y2=iy+1;
if (y2<=qbg_view_y2){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
 //--------plot pixel--------
 doff32=qbg_active_page_offset+offset;
 switch(fillcol&0xFF000000){
 case 0xFF000000:
  *doff32=fillcol;
 break;
 case 0x0:
  doff32;
 break;
 case 0x80000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend128[*doff32>>24]<<24);
 break; 
 case 0x7F000000:
  *doff32=(((*doff32&0xFEFEFE)+(fillcol&0xFEFEFE))>>1)+(ablend127[*doff32>>24]<<24);
 break;
 default:
  destcol=*doff32;
  cp=blend+(fillcol>>24<<16);
  *doff32=
    cp[(fillcol<<8&0xFF00)+(destcol&255)    ]
  +(cp[(fillcol&0xFF00)   +(destcol>>8&255) ]<<8)
  +(cp[(fillcol>>8&0xFF00)+(destcol>>16&255)]<<16)
  +(ablend[(fillcol>>24)+(destcol>>16&0xFF00)]<<24);
 };//switch
 //--------done plot pixel--------
b_t[b_n]=11; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

}//i

//no new nodes?
if (b_n==0){
memset(done,0,write_page->width*write_page->height);//cleanup
return;//finished!
}

//swap a & b arrays
sp=a_x; a_x=b_x; b_x=sp;
sp=a_y; a_y=b_y; b_y=sp;
cp=a_t; a_t=b_t; b_t=cp;
a_n=b_n;

goto nextpass;
}


//32-bit NO ALPHA BENDING
void sub_paint32x(long step,float x,float y,unsigned long fillcol,unsigned long bordercol,long passed){

//uses 2 buffers, a and b, and swaps between them for reading and creating
static unsigned long a_n=0;
static unsigned short *a_x=(unsigned short*)malloc(2*65536),*a_y=(unsigned short*)malloc(2*65536);
static unsigned char *a_t=(unsigned char*)malloc(65536);
static unsigned long b_n=0;
static unsigned short *b_x=(unsigned short*)malloc(2*65536),*b_y=(unsigned short*)malloc(2*65536);
static unsigned char *b_t=(unsigned char*)malloc(65536);
static unsigned char *done=(unsigned char*)calloc(640*480,1);
static long ix,iy,i,t,x2,y2;
static unsigned long offset;
static unsigned char *cp;
static unsigned short *sp;
//overrides
static long done_size=640*480;
static unsigned long *qbg_active_page_offset;//override
static long qbg_width,qbg_view_x1,qbg_view_y1,qbg_view_x2,qbg_view_y2;//override

if ((passed&1)==0) fillcol=write_page->color;
if ((passed&2)==0) bordercol=fillcol;

if (step){write_page->x+=x; write_page->y+=y;}else{write_page->x=x; write_page->y=y;}

if (write_page->clipping_or_scaling){
if (write_page->clipping_or_scaling==2){
ix=qbr_float_to_long(write_page->x*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
iy=qbr_float_to_long(write_page->y*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
}else{
ix=qbr_float_to_long(write_page->x)+write_page->view_offset_x; iy=qbr_float_to_long(write_page->y)+write_page->view_offset_y;
}
}else{
ix=qbr_float_to_long(write_page->x); iy=qbr_float_to_long(write_page->y);
}

//return if offscreen
if ((ix<write_page->view_x1)||(iy<write_page->view_y1)||(ix>write_page->view_x2)||(iy>write_page->view_y2)){
return;
}

//overrides
qbg_active_page_offset=write_page->offset32;
qbg_width=write_page->width;
qbg_view_x1=write_page->view_x1;
qbg_view_y1=write_page->view_y1;
qbg_view_x2=write_page->view_x2;
qbg_view_y2=write_page->view_y2;
i=write_page->width*write_page->height;
if (i>done_size){
free(done);
done=(unsigned char*)calloc(i,1);
}

//return if first point is the bordercolor
if (qbg_active_page_offset[iy*qbg_width+ix]==bordercol) return;

//create first node
a_x[0]=ix; a_y[0]=iy;
a_t[0]=15;
//types:
//&1=check left
//&2=check right
//&4=check above
//&8=check below

a_n=1;
qbg_active_page_offset[iy*qbg_width+ix]=fillcol;
done[iy*qbg_width+ix]=1;

nextpass:
b_n=0;
for (i=0;i<a_n;i++){
t=a_t[i]; ix=a_x[i]; iy=a_y[i];

//left
if (t&1){
x2=ix-1; y2=iy;
if (x2>=qbg_view_x1){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=13; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//right
if (t&2){
x2=ix+1; y2=iy;
if (x2<=qbg_view_x2){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=14; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//above
if (t&4){
x2=ix; y2=iy-1;
if (y2>=qbg_view_y1){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=7; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//below
if (t&8){
x2=ix; y2=iy+1;
if (y2<=qbg_view_y2){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=11; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

}//i

//no new nodes?
if (b_n==0){
memset(done,0,write_page->width*write_page->height);//cleanup
return;//finished!
}

//swap a & b arrays
sp=a_x; a_x=b_x; b_x=sp;
sp=a_y; a_y=b_y; b_y=sp;
cp=a_t; a_t=b_t; b_t=cp;
a_n=b_n;

goto nextpass;
}



//8-bit (default entry point)
void sub_paint(long step,float x,float y,unsigned long fillcol,unsigned long bordercol,qbs *backgroundstr,long passed){
if (new_error) return;
if (write_page->text){error(5); return;}
if (passed&4){error(5); return;}

if (write_page->bytes_per_pixel==4){
if (write_page->alpha_disabled){
sub_paint32x(step,x,y,fillcol,bordercol,passed);
return;
}else{
sub_paint32(step,x,y,fillcol,bordercol,passed);
return;
}
}

//uses 2 buffers, a and b, and swaps between them for reading and creating
static unsigned long a_n=0;
static unsigned short *a_x=(unsigned short*)malloc(2*65536),*a_y=(unsigned short*)malloc(2*65536);
static unsigned char *a_t=(unsigned char*)malloc(65536);
static unsigned long b_n=0;
static unsigned short *b_x=(unsigned short*)malloc(2*65536),*b_y=(unsigned short*)malloc(2*65536);
static unsigned char *b_t=(unsigned char*)malloc(65536);
static unsigned char *done=(unsigned char*)calloc(640*480,1);
static long ix,iy,i,t,x2,y2;
static unsigned long offset;
static unsigned char *cp;
static unsigned short *sp;
//overrides
static long done_size=640*480;
static unsigned char *qbg_active_page_offset;//override
static long qbg_width,qbg_view_x1,qbg_view_y1,qbg_view_x2,qbg_view_y2;//override

if ((passed&1)==0) fillcol=write_page->color;
if ((passed&2)==0) bordercol=fillcol;
fillcol&=write_page->mask;
bordercol&=write_page->mask;

if (step){write_page->x+=x; write_page->y+=y;}else{write_page->x=x; write_page->y=y;}

if (write_page->clipping_or_scaling){
if (write_page->clipping_or_scaling==2){
ix=qbr_float_to_long(write_page->x*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
iy=qbr_float_to_long(write_page->y*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
}else{
ix=qbr_float_to_long(write_page->x)+write_page->view_offset_x; iy=qbr_float_to_long(write_page->y)+write_page->view_offset_y;
}
}else{
ix=qbr_float_to_long(write_page->x); iy=qbr_float_to_long(write_page->y);
}

//return if offscreen
if ((ix<write_page->view_x1)||(iy<write_page->view_y1)||(ix>write_page->view_x2)||(iy>write_page->view_y2)){
return;
}

//overrides
qbg_active_page_offset=write_page->offset;
qbg_width=write_page->width;
qbg_view_x1=write_page->view_x1;
qbg_view_y1=write_page->view_y1;
qbg_view_x2=write_page->view_x2;
qbg_view_y2=write_page->view_y2;
i=write_page->width*write_page->height;
if (i>done_size){
free(done);
done=(unsigned char*)calloc(i,1);
}

//return if first point is the bordercolor
if (qbg_active_page_offset[iy*qbg_width+ix]==bordercol) return;

//create first node
a_x[0]=ix; a_y[0]=iy;
a_t[0]=15;
//types:
//&1=check left
//&2=check right
//&4=check above
//&8=check below

a_n=1;
qbg_active_page_offset[iy*qbg_width+ix]=fillcol;
done[iy*qbg_width+ix]=1;

nextpass:
b_n=0;
for (i=0;i<a_n;i++){
t=a_t[i]; ix=a_x[i]; iy=a_y[i];

//left
if (t&1){
x2=ix-1; y2=iy;
if (x2>=qbg_view_x1){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=13; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//right
if (t&2){
x2=ix+1; y2=iy;
if (x2<=qbg_view_x2){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=14; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//above
if (t&4){
x2=ix; y2=iy-1;
if (y2>=qbg_view_y1){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=7; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//below
if (t&8){
x2=ix; y2=iy+1;
if (y2<=qbg_view_y2){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=11; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

}//i

//no new nodes?
if (b_n==0){
memset(done,0,write_page->width*write_page->height);//cleanup
return;//finished!
}

//swap a & b arrays
sp=a_x; a_x=b_x; b_x=sp;
sp=a_y; a_y=b_y; b_y=sp;
cp=a_t; a_t=b_t; b_t=cp;
a_n=b_n;

goto nextpass;

}






void sub_paint(long step,float x,float y,qbs *fillstr,unsigned long bordercol,qbs *backgroundstr,long passed){
if (new_error) return;

//uses 2 buffers, a and b, and swaps between them for reading and creating
static unsigned long fillcol=0;//stub
static unsigned long a_n=0;
static unsigned short *a_x=(unsigned short*)malloc(2*65536),*a_y=(unsigned short*)malloc(2*65536);
static unsigned char *a_t=(unsigned char*)malloc(65536);
static unsigned long b_n=0;
static unsigned short *b_x=(unsigned short*)malloc(2*65536),*b_y=(unsigned short*)malloc(2*65536);
static unsigned char *b_t=(unsigned char*)malloc(65536);
static unsigned char *done=(unsigned char*)calloc(640*480,1);
static long ix,iy,i,t,x2,y2;
static unsigned long offset;
static unsigned char *cp;
static unsigned short *sp;
static unsigned long backgroundcol;

if (qbg_text_only){error(5); return;}
if ((passed&1)==0){error(5); return;}//must be called with this parameter!

//STEP 1: create the tile in a buffer (tile) using the source string
static unsigned char tilestr[256];
static unsigned char tile[8][64];
static long sx,sy;
static long bytesperrow;
static long row2offset;
static long row3offset;
static long row4offset;
static long byte;
static long bitvalue;
static long c;
if (fillstr->len==0){error(5); return;}
if (qbg_bits_per_pixel==4){
if (fillstr->len>256){error(5); return;}
}else{
if (fillstr->len>64){error(5); return;}
}
memset(&tilestr[0],0,256);
memcpy(&tilestr[0],fillstr->chr,fillstr->len);
sx=8; sy=fillstr->len; //defaults
if (qbg_bits_per_pixel==8) sx=1;
if (qbg_bits_per_pixel==4){
if (fillstr->len&3){
sy=(fillstr->len-(fillstr->len&3)+4)>>2;
}else{
sy=fillstr->len>>2;
}
bytesperrow=sx>>3; if (sx&7) bytesperrow++;
row2offset=bytesperrow;
row3offset=bytesperrow*2;
row4offset=bytesperrow*3;
}
if (qbg_bits_per_pixel==2) sx=4;
//use modified "PUT" routine to create the tile
cp=&tilestr[0];
{//layer
static long x,y;
for (y=0;y<sy;y++){
if (qbg_bits_per_pixel==4){
bitvalue=128;
byte=0;
}
for (x=0;x<sx;x++){
//get colour
if (qbg_bits_per_pixel==8){
 c=*cp;
 cp++;
}
if (qbg_bits_per_pixel==4){
byte=x>>3;
c=0;
if (cp[byte]&bitvalue) c|=1;
if (cp[row2offset+byte]&bitvalue) c|=2;
if (cp[row3offset+byte]&bitvalue) c|=4;
if (cp[row4offset+byte]&bitvalue) c|=8;
bitvalue>>=1; if (bitvalue==0) bitvalue=128;
}
if (qbg_bits_per_pixel==1){
 if (!(x&7)){
  byte=*cp;
  cp++;
 }
 c=(byte&128)>>7; byte<<=1;
}
if (qbg_bits_per_pixel==2){
 if (!(x&3)){
  byte=*cp;
  cp++;
 }
 c=(byte&192)>>6; byte<<=2;
}
//"pset" color
tile[x][y]=c;
}//x
if (qbg_bits_per_pixel==4) cp+=(bytesperrow*4);
if (qbg_bits_per_pixel==1){
 if (sx&7) cp++;
}
if (qbg_bits_per_pixel==2){
 if (sx&3) cp++;
}
}//y
}//unlayer
//tile created!

//STEP 2: establish border and background colors
if ((passed&2)==0) bordercol=qbg_color;
bordercol&=qbg_pixel_mask;

backgroundcol=0;//default
if (passed&4){
if (backgroundstr->len==0){error(5); return;}
if (backgroundstr->len>255){error(5); return;}
if (qbg_bits_per_pixel==1){
c=backgroundstr->chr[0];
if ((c>0)&&(c<255)) backgroundcol=-1;//unclear definition
if (c==255) backgroundcol=1;
}
if (qbg_bits_per_pixel==2){
backgroundcol=-1;//unclear definition
x2=backgroundstr->chr[0];
y2=x2&3;
x2>>=2; if ((x2&3)!=y2) goto uncleardef;
x2>>=2; if ((x2&3)!=y2) goto uncleardef;
x2>>=2; if ((x2&3)!=y2) goto uncleardef;
backgroundcol=y2;
}
if (qbg_bits_per_pixel==4){
backgroundcol=-1;//unclear definition
y2=0;
x2=4; if (backgroundstr->len<4) x2=backgroundstr->len;
c=0; memcpy(&c,backgroundstr->chr,x2);
x2=c&255; c>>=8; if ((x2!=0)&&(x2!=255)) goto uncleardef;
y2|=(x2&1);
x2=c&255; c>>=8; if ((x2!=0)&&(x2!=255)) goto uncleardef;
y2|=((x2&1)<<1);
x2=c&255; c>>=8; if ((x2!=0)&&(x2!=255)) goto uncleardef;
y2|=((x2&1)<<2);
x2=c&255; c>>=8; if ((x2!=0)&&(x2!=255)) goto uncleardef;
y2|=((x2&1)<<3);
backgroundcol=y2;
}
if (qbg_bits_per_pixel==8){
backgroundcol=backgroundstr->chr[0];
}
}
uncleardef:

//STEP 3: perform tile'd fill
if (step){qbg_x+=x; qbg_y+=y;}else{qbg_x=x; qbg_y=y;}
if (qbg_clipping_or_scaling){
if (qbg_clipping_or_scaling==2){
ix=qbr_float_to_long(qbg_x*qbg_scaling_x+qbg_scaling_offset_x)+qbg_view_offset_x;
iy=qbr_float_to_long(qbg_y*qbg_scaling_y+qbg_scaling_offset_y)+qbg_view_offset_y;
}else{
ix=qbr_float_to_long(qbg_x)+qbg_view_offset_x; iy=qbr_float_to_long(qbg_y)+qbg_view_offset_y;
}
}else{
ix=qbr_float_to_long(qbg_x); iy=qbr_float_to_long(qbg_y);
}

//return if offscreen
if ((ix<qbg_view_x1)||(iy<qbg_view_y1)||(ix>qbg_view_x2)||(iy>qbg_view_y2)){
return;
}

offset=iy*qbg_width+ix;

//return if first point is the bordercolor
if (qbg_active_page_offset[offset]==bordercol) return;

//return if first point is the same as the tile color used and is not the background color
fillcol=tile[ix%sx][iy%sy];
if ((fillcol==qbg_active_page_offset[offset])&&(fillcol!=backgroundcol)) return;
qbg_active_page_offset[offset]=fillcol;




//create first node
a_x[0]=ix; a_y[0]=iy;
a_t[0]=15;
//types:
//&1=check left
//&2=check right
//&4=check above
//&8=check below

a_n=1;
qbg_active_page_offset[iy*qbg_width+ix]=fillcol;
done[iy*qbg_width+ix]=1;

nextpass:
b_n=0;
for (i=0;i<a_n;i++){
t=a_t[i]; ix=a_x[i]; iy=a_y[i];

//left
if (t&1){
x2=ix-1; y2=iy;
if (x2>=qbg_view_x1){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
fillcol=tile[x2%sx][y2%sy];
//no tile check required when moving horizontally!
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=13; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//right
if (t&2){
x2=ix+1; y2=iy;
if (x2<=qbg_view_x2){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
fillcol=tile[x2%sx][y2%sy];
//no tile check required when moving horizontally!
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=14; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}}}}

//above
if (t&4){
x2=ix; y2=iy-1;
if (y2>=qbg_view_y1){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
fillcol=tile[x2%sx][y2%sy];
if ((fillcol!=qbg_active_page_offset[offset])||(fillcol==backgroundcol)){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=7; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}
}}}}

//below
if (t&8){
x2=ix; y2=iy+1;
if (y2<=qbg_view_y2){
offset=y2*qbg_width+x2;
if (!done[offset]){
done[offset]=1;
if (qbg_active_page_offset[offset]!=bordercol){
fillcol=tile[x2%sx][y2%sy];
if ((fillcol!=qbg_active_page_offset[offset])||(fillcol==backgroundcol)){
qbg_active_page_offset[offset]=fillcol;
b_t[b_n]=11; b_x[b_n]=x2; b_y[b_n]=y2; b_n++;//add new node
}
}}}}

}//i

//no new nodes?
if (b_n==0){
memset(done,0,qbg_width*qbg_height);//cleanup
return;//finished!
}

//swap a & b arrays
sp=a_x; a_x=b_x; b_x=sp;
sp=a_y; a_y=b_y; b_y=sp;
cp=a_t; a_t=b_t; b_t=cp;
a_n=b_n;

goto nextpass;

}












void sub_circle(long step,double x,double y,double r,unsigned long col,double start,double end,double aspect,long passed){
if (new_error) return;

//                                                                 &1         &2           &4         &8
//[{STEP}](?,?),?[,[?][,[?][,[?][,?]]]]



//data
static double pi= 3.1415926535897932,pi2=6.2831853071795865;
static long line_to_start,line_from_end;
static long ix,iy;//integer screen co-ordinates of circle's centre
static double xspan,yspan;
static double c;//circumference
static double px,py;
static double sinb,cosb;//second angle used in double-angle-formula
static long pixels;
static double tmp;
static long tmpi;
static long i;
static long exclusive;
static double arc1,arc2,arc3,arc4,arcinc;
static double px2,py2;
static long x2,y2;
static long x3,y3;
static long lastplotted_x2,lastplotted_y2;
static long lastchecked_x2,lastchecked_y2;

if (write_page->text){error(5); return;}

//lines to & from centre
if (!(passed&2)) start=0;
if (!(passed&4)) end=pi2;
line_to_start=0; if (start<0) {line_to_start=1; start=-start;}
line_from_end=0; if (end<0) {line_from_end=1; end=-end;}

//error checking
if (start>pi2){error(5); return;}
if (end>pi2){error(5); return;}

//when end<start, the arc of the circle that wouldn't have been drawn if start & end
//were swapped is drawn
exclusive=0;
if (end<start){
tmp=start; start=end; end=tmp;
tmpi=line_to_start; line_to_start=line_from_end; line_from_end=tmpi;
exclusive=1;
}

//calc. centre
if (step){x=write_page->x+x; y=write_page->y+y;}
write_page->x=x; write_page->y=y;//set graphics cursor position to circle's centre



r=x+r;//the differece between x & x+r in pixels will be the radius in pixels
//resolve coordinates (but keep as floats)
if (write_page->clipping_or_scaling){
if (write_page->clipping_or_scaling==2){
x=x*write_page->scaling_x+write_page->scaling_offset_x+write_page->view_offset_x;
y=y*write_page->scaling_y+write_page->scaling_offset_y+write_page->view_offset_y;
r=r*write_page->scaling_x+write_page->scaling_offset_x+write_page->view_offset_x;
}else{
x=x+write_page->view_offset_x;
y=y+write_page->view_offset_y;
r=r+write_page->view_offset_x;
}
}
if (x<0) ix=x-0.5; else ix=x+0.5;
if (y<0) iy=y-0.5; else iy=y+0.5;
r=fabs(r-x);//r is now a radius in pixels



//adjust vertical and horizontal span of the circle based on aspect ratio
xspan=r; yspan=r;
if (!(passed&8)) aspect=4.0*((double)write_page->height/(double)write_page->width)/3.0;
if (aspect>=0){
 if (aspect<1){
  //aspect: 0 to 1
  yspan*=aspect;
 }
 if (aspect>1){
  //aspect: 1 to infinity
  xspan/=aspect;
 }
}else{
 if (aspect>-1){
  //aspect: -1 to 0
  yspan*=(1+aspect);
 }
 //if aspect<-1 no change is required
}

//skip everything if none of the circle is inside current viwport
if ((x+xspan+0.5)<write_page->view_x1) return;
if ((y+yspan+0.5)<write_page->view_y1) return;
if ((x-xspan-0.5)>write_page->view_x2) return;
if ((y-yspan-0.5)>write_page->view_y2) return;

if (!(passed&1)) col=write_page->color;

//pre-set/pre-calculate values
c=pi2*r;
pixels=c/4.0+0.5;
arc1=0;
arc2=pi;
arc3=pi;
arc4=pi2;
arcinc=(pi/2)/(double)pixels;
sinb=sin(arcinc);
cosb=cos(arcinc);
lastplotted_x2=-1;
lastchecked_x2=-1;
i=0;

if (line_to_start){
px=cos(start); py=sin(start);
x2=px*xspan+0.5; y2=py*yspan-0.5;
fast_line(ix,iy,ix+x2,iy-y2,col);
}

px=1;
py=0;

drawcircle:
x2=px*xspan+0.5;
y2=py*yspan-0.5;

if (i==0) {lastchecked_x2=x2; lastchecked_y2=y2; goto plot;}

if ( (abs(x2-lastplotted_x2)>=2)||(abs(y2-lastplotted_y2)>=2) ){
plot:
if (exclusive){
if ((arc1<=start)||(arc1>=end)){pset_and_clip(ix+lastchecked_x2,iy+lastchecked_y2,col);}
if ((arc2<=start)||(arc2>=end)){pset_and_clip(ix-lastchecked_x2,iy+lastchecked_y2,col);}
if ((arc3<=start)||(arc3>=end)){pset_and_clip(ix-lastchecked_x2,iy-lastchecked_y2,col);}
if ((arc4<=start)||(arc4>=end)){pset_and_clip(ix+lastchecked_x2,iy-lastchecked_y2,col);}
}else{//inclusive
if ((arc1>=start)&&(arc1<=end)){pset_and_clip(ix+lastchecked_x2,iy+lastchecked_y2,col);}
if ((arc2>=start)&&(arc2<=end)){pset_and_clip(ix-lastchecked_x2,iy+lastchecked_y2,col);}
if ((arc3>=start)&&(arc3<=end)){pset_and_clip(ix-lastchecked_x2,iy-lastchecked_y2,col);}
if ((arc4>=start)&&(arc4<=end)){pset_and_clip(ix+lastchecked_x2,iy-lastchecked_y2,col);}
}
if (i>pixels) goto allplotted;
lastplotted_x2=lastchecked_x2; lastplotted_y2=lastchecked_y2;
}
lastchecked_x2=x2; lastchecked_y2=y2;

if (i<=pixels){
i++;
if (i>pixels) goto plot;
px2=px*cosb+py*sinb;
py=py*cosb-px*sinb;
px=px2;
if (i) {arc1+=arcinc; arc2-=arcinc; arc3+=arcinc; arc4-=arcinc;}
goto drawcircle;
}
allplotted:

if (line_from_end){
px=cos(end); py=sin(end);
x2=px*xspan+0.5; y2=py*yspan-0.5;
fast_line(ix,iy,ix+x2,iy-y2,col);
}

}//sub_circle

unsigned long point(long x,long y){//does not clip!
if (read_page->bytes_per_pixel==1){
return read_page->offset[y*read_page->width+x]&read_page->mask;
}else{
return read_page->offset32[y*read_page->width+x];
}
return NULL;
}




double func_point(float x,float y,long passed){
static long x2,y2,i;
if (!read_page->compatible_mode){error(5); return 0;}

//revise!
if (!passed){
i=qbr_float_to_long(x);
if ((i<0)||(i>3)){error(5); return 0;}
if (i==2) return qbg_x;
if (i==3) return qbg_y;
if (qbg_clipping_or_scaling==2){
x2=qbr_float_to_long(qbg_x*qbg_scaling_x+qbg_scaling_offset_x);
y2=qbr_float_to_long(qbg_y*qbg_scaling_y+qbg_scaling_offset_y);
}else{
x2=qbr_float_to_long(qbg_x); y2=qbr_float_to_long(qbg_y);
}
if (i==0) return x2;
return y2;
}

if (read_page->clipping_or_scaling){
 if (read_page->clipping_or_scaling==2){
 x2=qbr_float_to_long(x*read_page->scaling_x+read_page->scaling_offset_x)+read_page->view_offset_x;
 y2=qbr_float_to_long(y*read_page->scaling_y+read_page->scaling_offset_y)+read_page->view_offset_y;
 }else{
 x2=qbr_float_to_long(x)+read_page->view_offset_x; y2=qbr_float_to_long(y)+read_page->view_offset_y;
 }
}else{
x2=qbr_float_to_long(x); y2=qbr_float_to_long(y);
}
if (x2>=read_page->view_x1){ if (x2<=read_page->view_x2){
if (y2>=read_page->view_y1){ if (y2<=read_page->view_y2){
return point(x2,y2);
}}}}
return -1;
}








void qbg_pset(long step,float x,float y,unsigned long col,long passed){
if (new_error) return;



static long x2,y2;
if (!write_page->compatible_mode){error(5); return;}
//Special Format: [{STEP}](?,?),[?]
if (step){write_page->x+=x; write_page->y+=y;}else{write_page->x=x; write_page->y=y;}
if (!(passed&1)) col=write_page->color;
if (write_page->clipping_or_scaling){
if (write_page->clipping_or_scaling==2){
x2=qbr(write_page->x*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
y2=qbr(write_page->y*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
}else{
x2=qbr(write_page->x)+write_page->view_offset_x; y2=qbr(write_page->y)+write_page->view_offset_y;
}
if (x2>=write_page->view_x1){ if (x2<=write_page->view_x2){
if (y2>=write_page->view_y1){ if (y2<=write_page->view_y2){
 pset(x2,y2,col);
}}}}
return;
}else{
 x2=qbr(write_page->x); if (x2>=0){ if (x2<write_page->width){
 y2=qbr(write_page->y); if (y2>=0){ if (y2<write_page->height){
  pset(x2,y2,col);
 }}}}
}
return;
}

void sub_preset(long step,float x,float y,unsigned long col,long passed){
if (new_error) return;

if (!passed){
col=write_page->background_color;
passed=1;
}
qbg_pset(step,x,y,col,passed);
return;
}


long img_printchr=0;
long img_printchr_i;
long img_printchr_x;
long img_printchr_y;
char *img_printchr_offset;

void printchr(long character){
static unsigned long x,x2,y,y2,w,h,z,z2,z3,a,a2,a3,color,background_color,f;
static unsigned long *lp;
static unsigned char *cp;
static img_struct *im;
static SDL_Surface *ts;
static SDL_Color c,c2;

im=write_page;
color=im->color;
background_color=im->background_color;


if (im->text){
im->offset[(((im->cursor_y-1)*im->width+im->cursor_x-1))<<1]=character;
im->offset[((((im->cursor_y-1)*im->width+im->cursor_x-1))<<1)+1]=(color&0xF)+background_color*16+(color&16)*8;
return;
}

//precalculations
character&=255;
f=im->font;
x=fontwidth[f]; if (x) x*=(im->cursor_x-1); else x=im->cursor_x-1;
y=(im->cursor_y-1)*fontheight[f];
h=fontheight[f];
c.r=255; c.g=255; c.b=255; c.unused=0;//dummy values
c2.r=255; c2.g=255; c2.b=255; c2.unused=0;//dummy values


//if (mode==1) img[i].print_mode=3;//fill
//if (mode==2) img[i].print_mode=1;//keep
//if (mode==3) img[i].print_mode=2;//only

if (f>=32){//custom font

//8-bit / alpha-disabled 32-bit / dont-blend(alpha may still be applied)
if ((im->bytes_per_pixel==1)||((im->bytes_per_pixel==4)&&(im->alpha_disabled))||(fontflags[f]&8)){
ts=TTF_RenderText_Solid(font[f],(char*)&character,c);//8-bit, 0=clear, 1=text
if (ts==NULL) return;
w=ts->w;
if (x2=fontwidth[f]) if (x2!=w) w=x2;//render width too large!  
switch(im->print_mode){
case 3:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 if (*cp++) pset(x+x2,y+y2,color); else pset(x+x2,y+y2,background_color);
 }}
 break;
case 1:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 if (*cp++) pset(x+x2,y+y2,color);
 }}
 break;
case 2:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 if (!(*cp++)) pset(x+x2,y+y2,background_color);
 }}
break;
default:
break;
}//z
SDL_FreeSurface(ts);
return;
}//1-8 bit

//assume 32-bit blended
a=(color>>24)+1;
a2=(background_color>>24)+1;
z=color&0xFFFFFF;
z2=background_color&0xFFFFFF;
//8 bit, 0=background -> 255=foreground
ts=TTF_RenderText_Shaded(font[f],(char*)&character,c,c2);
if (ts==NULL) return;
w=ts->w;
if (x2=fontwidth[f]) if (x2!=w) w=x2;//render width too large!  
switch(im->print_mode){
case 3:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 z3=*cp++;
 if (z3!=255) pset(x+x2,y+y2,(((255-z3)*a2)>>8<<24)+z2);
 if (z3) pset(x+x2,y+y2,((z3*a)>>8<<24)+z);
 }}
break;
case 1:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 z3=*cp++;
 //if (z3!=255) pset(x+x2,y+y2,(((255-z3)*a2)>>8<<24)+z2);
 if (z3) pset(x+x2,y+y2,((z3*a)>>8<<24)+z);
 }}
break;
case 2:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 z3=*cp++;
 if (z3!=255) pset(x+x2,y+y2,(((255-z3)*a2)>>8<<24)+z2);
 //if (z3) pset(x+x2,y+y2,((z3*a)>>8<<24)+z);
 }}
break;
default:
break;
}
SDL_FreeSurface(ts);
return;

}//custom font

//default fonts
if (im->font==8) cp=&charset8x8[character][0][0];
if (im->font==14) cp=&charset8x16[character][1][0];
if (im->font==16) cp=&charset8x16[character][0][0];
switch(im->print_mode){
case 3:
 for (y2=0;y2<h;y2++){ for (x2=0;x2<8;x2++){
 if (*cp++) pset(x+x2,y+y2,color); else pset(x+x2,y+y2,background_color);
 }}
 break;
case 1:
 for (y2=0;y2<h;y2++){ for (x2=0;x2<8;x2++){
 if (*cp++) pset(x+x2,y+y2,color);
 }}
 break;
case 2:
 for (y2=0;y2<h;y2++){ for (x2=0;x2<8;x2++){
 if (!(*cp++)) pset(x+x2,y+y2,background_color);
 }}
 break;
default:
break;
}//z
return;

}//printchr


//prints a character at pixel offset x,y of image surface i
//returns the width in pixels of the character printed
//this is used for printing un-kerned characters
//cannot be used on a text surface!
long printchr2(long x,long y,unsigned long character,long i){
static unsigned long x2,y2,w,h,z,z2,z3,a,a2,a3,color,background_color,f;
static unsigned long *lp;
static unsigned char *cp;
static img_struct *im;
static SDL_Surface *ts;
static SDL_Color c,c2;

im=&img[i];
color=im->color;
background_color=im->background_color;

//precalculations
character&=255;
f=im->font;
h=fontheight[f];
c.r=255; c.g=255; c.b=255; c.unused=0;//dummy values
c2.r=255; c2.g=255; c2.b=255; c2.unused=0;//dummy values

if (f>=32){//custom font

//8-bit / alpha-disabled 32-bit / dont-blend(alpha may still be applied)
if ((im->bytes_per_pixel==1)||((im->bytes_per_pixel==4)&&(im->alpha_disabled))||(fontflags[f]&8)){
ts=TTF_RenderText_Solid(font[f],(char*)&character,c);//8-bit, 0=clear, 1=text
if (ts==NULL) return 0;
w=ts->w;
if (x2=fontwidth[f]) if (x2!=w) w=x2;//render width too large!  
switch(im->print_mode){
case 3:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 if (*cp++) pset_and_clip(x+x2,y+y2,color); else pset_and_clip(x+x2,y+y2,background_color);
 }}
 break;
case 1:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 if (*cp++) pset_and_clip(x+x2,y+y2,color);
 }}
 break;
case 2:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 if (!(*cp++)) pset_and_clip(x+x2,y+y2,background_color);
 }}
break;
default:
break;
}//z
SDL_FreeSurface(ts);
return w;
}//1-8 bit

//assume 32-bit blended
a=(color>>24)+1;
a2=(background_color>>24)+1;
z=color&0xFFFFFF;
z2=background_color&0xFFFFFF;
//8 bit, 0=background -> 255=foreground
ts=TTF_RenderText_Shaded(font[f],(char*)&character,c,c2);
if (ts==NULL) return 0;
w=ts->w;
if (x2=fontwidth[f]) if (x2!=w) w=x2;//render width too large!  
switch(im->print_mode){
case 3:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 z3=*cp++;
 if (z3!=255) pset_and_clip(x+x2,y+y2,(((255-z3)*a2)>>8<<24)+z2);
 if (z3) pset_and_clip(x+x2,y+y2,((z3*a)>>8<<24)+z);
 }}
break;
case 1:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 z3=*cp++;
 //if (z3!=255) pset_and_clip(x+x2,y+y2,(((255-z3)*a2)>>8<<24)+z2);
 if (z3) pset_and_clip(x+x2,y+y2,((z3*a)>>8<<24)+z);
 }}
break;
case 2:
 for (y2=0;y2<h;y2++){
 cp=((unsigned char*)ts->pixels)+y2*ts->pitch;
 for (x2=0;x2<w;x2++){
 z3=*cp++;
 if (z3!=255) pset_and_clip(x+x2,y+y2,(((255-z3)*a2)>>8<<24)+z2);
 //if (z3) pset_and_clip(x+x2,y+y2,((z3*a)>>8<<24)+z);
 }}
break;
default:
break;
}
SDL_FreeSurface(ts);
return w;
}//custom font

//default fonts
if (im->font==8) cp=&charset8x8[character][0][0];
if (im->font==14) cp=&charset8x16[character][1][0];
if (im->font==16) cp=&charset8x16[character][0][0];
switch(im->print_mode){
case 3:
 for (y2=0;y2<h;y2++){ for (x2=0;x2<8;x2++){
 if (*cp++) pset_and_clip(x+x2,y+y2,color); else pset_and_clip(x+x2,y+y2,background_color);
 }}
 break;
case 1:
 for (y2=0;y2<h;y2++){ for (x2=0;x2<8;x2++){
 if (*cp++) pset_and_clip(x+x2,y+y2,color);
 }}
 break;
case 2:
 for (y2=0;y2<h;y2++){ for (x2=0;x2<8;x2++){
 if (!(*cp++)) pset_and_clip(x+x2,y+y2,background_color);
 }}
 break;
default:
break;
}//z
return 8;

}//printchr2



long chrwidth(long character){
static unsigned long x;
static img_struct *im;
static SDL_Surface *ts;
static SDL_Color c;
im=write_page;
if (x=fontwidth[im->font]) return x;
character&=255;
//int minx,maxx,miny,maxy,advance;
//TTF_GlyphMetrics(font[im->font], character,  &minx,  &maxx,  &miny,  &maxy,  &advance);
c.r=255; c.g=255; c.b=255; c.unused=0;//dummy values
if (im->alpha_disabled||(fontflags[im->font]&8)||(im->bytes_per_pixel==1)){
ts=TTF_RenderText_Solid(font[im->font],(char*)&character,c);//8-bit, 0=clear, 1=text
}else{
ts=TTF_RenderText_Shaded(font[im->font],(char*)&character,c,c);//8-bit 0-255
}
if (ts==NULL) return 0;
x=ts->w;
SDL_FreeSurface(ts);
return x;
}//chrwidth


void newline(){
static unsigned long *lp;
static unsigned short *sp;
static long z,z2;

//move cursor to new line
write_page->cursor_y++; write_page->cursor_x=1;

//scroll up screen if necessary
if (write_page->cursor_y>write_page->bottom_row){

if (write_page->text){
//text
//move lines up
memmove(
 write_page->offset+(write_page->top_row-1)*2*write_page->width,
 write_page->offset+ write_page->top_row   *2*write_page->width,
 (write_page->bottom_row-write_page->top_row)*2*write_page->width
);
//erase bottom line
z2=(write_page->color&0xF)+(write_page->background_color&7)*16+(write_page->color&16)*8;
z2<<=8;
z2+=32;
sp=((unsigned short*)(write_page->offset+(write_page->bottom_row-1)*2*write_page->width));
z=write_page->width;
while(z--) *sp++=z2;
}else{
//graphics
//move lines up
memmove(
 write_page->offset+(write_page->top_row-1)*write_page->bytes_per_pixel*write_page->width*fontheight[write_page->font],
 write_page->offset+ write_page->top_row   *write_page->bytes_per_pixel*write_page->width*fontheight[write_page->font],
 (write_page->bottom_row-write_page->top_row)*write_page->bytes_per_pixel*write_page->width*fontheight[write_page->font]
);
//erase bottom line
if (write_page->bytes_per_pixel==1){
memset(write_page->offset+(write_page->bottom_row-1)*write_page->width*fontheight[write_page->font],write_page->background_color,write_page->width*fontheight[write_page->font]);
}else{
//assume 32-bit
z2=write_page->background_color;
lp=write_page->offset32+(write_page->bottom_row-1)*write_page->width*fontheight[write_page->font];
z=write_page->width*fontheight[write_page->font];
while(z--) *lp++=z2;
}
}//graphics
write_page->cursor_y=write_page->bottom_row;
}//scroll up

}//newline

void makefit(qbs *text){
static long w,x,x2,x3;
if (write_page->holding_cursor) return;
if (write_page->cursor_x!=1){//if already at left-most, nothing more can be done
if (write_page->text){
 if ((write_page->cursor_x+text->len-1)>write_page->width) newline();
}else{
 w=func__printwidth(text,NULL,NULL);
 x=fontwidth[write_page->font]; if (!x) x=1; x=x*(write_page->cursor_x-1);
 if ((x+w)>write_page->width) newline();
}
}
}


void tab(){
static long x,x2,w;

//tab() on a held-cursor only sets the cursor to the left hand position of the next line
if (write_page->holding_cursor){
newline(); write_page->holding_cursor=0;
return;
}

//text
if (write_page->text){
qbs_print(singlespace,0);
text:
if (write_page->cursor_x!=1){
if (((write_page->cursor_x-1)%14)||(write_page->cursor_x>(write_page->width-13))){
if (write_page->cursor_x<write_page->width){qbs_print(singlespace,0); goto text;}
}
}//!=1
return;
}

x=fontwidth[write_page->font]; 
if (!x){

//variable width
x=write_page->cursor_x-1;
x2=(x/112+1)*112;//next position
if (x2>=write_page->width){//it doesn't fit on line
//box fill x to end of line with background color
fast_boxfill(x,(write_page->cursor_y-1)*fontheight[write_page->font],write_page->width-1,write_page->cursor_y*fontheight[write_page->font]-1,write_page->background_color);
newline();
}else{//fits on line
//box fill x to x2-1 with background color
fast_boxfill(x,(write_page->cursor_y-1)*fontheight[write_page->font],x2-1,write_page->cursor_y*fontheight[write_page->font]-1,write_page->background_color);
write_page->cursor_x=x2;
}


}else{

//fixed width
w=write_page->width/x;

qbs_print(singlespace,0);
fixwid:
if (write_page->cursor_x!=1){
if (((write_page->cursor_x-1)%14)||(write_page->cursor_x>(w-13))){
if (write_page->cursor_x<w){qbs_print(singlespace,0); goto fixwid;}
}
}//!=1

}
return;
}

void qbs_print(qbs* str,long finish_on_new_line){
if (new_error) return;
long i,i2,entered_new_line,x,x2,y,y2,z,z2,w;
entered_new_line=0;
static unsigned long character;

/*
if (!str->len){
if (!newline) return;//no action required
if (write_page->holding_cursor){//extra CR required before return
write_page->holding_cursor=0;
i=-1;
write_page->cursor_x++;
goto print_unhold_cursor;
}
}

if (!str->len) goto null_length;

if (write_page->holding_cursor){
write_page->holding_cursor=0;
i=-1;
write_page->cursor_x++;
goto print_unhold_cursor;
}
*/

//holding cursor?
if (write_page->holding_cursor){
if (str->len){
write_page->holding_cursor=0;
newline();
}else{
//null length print string
if (finish_on_new_line) write_page->holding_cursor=0;//new line will be entered automatically
}
}


for (i=0;i<str->len;i++){
entered_new_line=0;//beginning a new line was the last action (so don't add a new one)
character=str->chr[i];

//###special characters

if (character==28){
//advance one cursor position
//can cursor advance?
if (write_page->cursor_y>=write_page->bottom_row){
 if (write_page->text){
  if (write_page->cursor_x>=write_page->width) goto skip;
 }else{
  if (fontwidth[write_page->font]){
   if (write_page->cursor_x>=(write_page->width/fontwidth[write_page->font])) goto skip;
  }else{
   if (write_page->cursor_x>=write_page->width) goto skip;
  }
 } 
}
write_page->cursor_x++;
 if (write_page->text){
  if (write_page->cursor_x>write_page->width){write_page->cursor_y++; write_page->cursor_x=1;}
 }else{
  if (fontwidth[write_page->font]){
   if (write_page->cursor_x>(write_page->width/fontwidth[write_page->font])){write_page->cursor_y++; write_page->cursor_x=1;}
  }else{
   if (write_page->cursor_x>write_page->width){write_page->cursor_y++; write_page->cursor_x=1;}
  }
 } 
goto skip;
}

if (character==29){
//go back one cursor position
//can cursor go back?
if ((write_page->cursor_y==write_page->top_row)||(write_page->cursor_y>write_page->bottom_row)){
if (write_page->cursor_x==1) goto skip;
}
write_page->cursor_x--;
if (write_page->cursor_x<1){
write_page->cursor_y--;
 if (write_page->text){
  write_page->cursor_x=write_page->width;
 }else{
  if (fontwidth[write_page->font]){
   write_page->cursor_x=write_page->width/fontwidth[write_page->font];
  }else{
   write_page->cursor_x=write_page->width;
  }
 } 
}
goto skip;
}

if (character==30){
//previous row, same column
//no change if cursor not within view print boundries
if ((write_page->cursor_y>write_page->top_row)&&(write_page->cursor_y<=write_page->bottom_row)){
write_page->cursor_y--;
}
goto skip;
}

if (character==31){
//next row, same column
//no change if cursor not within view print boundries
if ((write_page->cursor_y>=write_page->top_row)&&(write_page->cursor_y<write_page->bottom_row)){
write_page->cursor_y++;
}
goto skip;
}

if (character==12){
//clears text viewport
//clears bottom row
//moves cursor to top-left of text viewport
sub_cls(NULL,NULL,0);
goto skip;
}

if (character==11){
write_page->cursor_x=1; write_page->cursor_y=write_page->top_row;
goto skip;
}

if (character==9){
//moves to next multiple of 8 (always advances at least one space)
if (!fontwidth[write_page->font]){
 //variable width!
 x=write_page->cursor_x-1;
 x2=(x/64+1)*64;//next position
 if (x2>=write_page->width){//it doesn't fit on line
 //box fill x to end of line with background color
 fast_boxfill(x,(write_page->cursor_y-1)*fontheight[write_page->font],write_page->width-1,write_page->cursor_y*fontheight[write_page->font]-1,write_page->background_color);
 newline();
 entered_new_line=1;
 }else{//fits on line
 //box fill x to x2-1 with background color
 fast_boxfill(x,(write_page->cursor_y-1)*fontheight[write_page->font],x2-1,write_page->cursor_y*fontheight[write_page->font]-1,write_page->background_color);
 write_page->cursor_x=x2;
 }
 goto skip;
}else{
 if (write_page->cursor_x%8){//next cursor position not a multiple of 8
 i--;//more spaces will be required
 }
 character=32;//override character 9
}
}//9

if (character==7){
qb64_generatesound(783.99,0.2,0);
Sleep(250);
goto skip;
}

if ((character==10)||(character==13)){
newline();
//note: entered_new_line not set because these carriage returns compound on each other
goto skip;
}

//###check if character fits on line, if not move to next line
//(only applies to non-fixed width fonts)
if (!fontwidth[write_page->font]){//unpredictable width
w=chrwidth(character);
if ((write_page->cursor_x+w)>write_page->width){
newline();
//entered_new_line not set, a character will follow
}
}

//###print the character
printchr(character);

//###advance cursor
if (fontwidth[write_page->font]){
write_page->cursor_x++;
}else{
write_page->cursor_x+=w;
}

//###check if another character could fit at cursor_x's location
if (write_page->compatible_mode){//graphics
 x=fontwidth[write_page->font]; if (!x) x=1;
 x2=x*(write_page->cursor_x-1);
 if (x2>(write_page->width-x)){
  if (!finish_on_new_line){
  if (i==(str->len-1)){//last character
  //move horizontal cursor back to right-most valid position
  write_page->cursor_x=write_page->width/x;
  write_page->holding_cursor=1;
  goto held_cursor;
  }
  }
 newline();
 entered_new_line=1;
 }
}else{//text
 if (write_page->cursor_x>write_page->width){
  if (!finish_on_new_line){
  if (i==(str->len-1)){//last character
  write_page->cursor_x--;//move horizontal cursor back to right-most valid position
  write_page->holding_cursor=1;
  goto held_cursor;
  }
  }
 newline();
 entered_new_line=1;
 }
}
held_cursor:

skip:;

/*
tabbing1:


write_page->cursor_x++;



//hold cursor?
if (write_page->cursor_x>qbg_width_in_characters){//past last x position
if (!newline){//don't need a new line
if (i==(str->len-1)){//last character
write_page->cursor_x--;
write_page->holding_cursor=1;
goto hold_cursor;
}
}
}




qbs_print_skipchar:;

print_unhold_cursor:

if (write_page->cursor_x>qbg_width_in_characters){
qbs_print_newline:
newlineadded=1;

if (write_page->cursor_y==qbg_height_in_characters) write_page->cursor_y=qbg_bottom_row;

write_page->cursor_y++;
write_page->cursor_x=1;



if (write_page->cursor_y>qbg_bottom_row){
//move screen space within view print up 1 row
//if (qbg_mode==13){

///memmove(&cmem[655360+(qbg_top_row-1)*2560],&cmem[655360+qbg_top_row*2560],(qbg_bottom_row-qbg_top_row)*2560);
///memset(&cmem[655360+(qbg_bottom_row-1)*2560],0,2560);
if (qbg_text_only){

memmove(qbg_active_page_offset+(qbg_top_row-1)*qbg_width_in_characters*2,
        qbg_active_page_offset+(qbg_top_row)*qbg_width_in_characters*2,
        (qbg_bottom_row-qbg_top_row)*qbg_width_in_characters*2);
for (i2=0;i2<qbg_width_in_characters;i2++){
qbg_active_page_offset[(qbg_bottom_row-1)*qbg_width_in_characters*2+i2*2]=32;
qbg_active_page_offset[(qbg_bottom_row-1)*qbg_width_in_characters*2+i2*2+1]=7;

}

}else{
memmove(qbg_active_page_offset+(qbg_top_row-1)*qbg_bytes_per_pixel*qbg_width*qbg_character_height,
        qbg_active_page_offset+qbg_top_row*qbg_bytes_per_pixel*qbg_width*qbg_character_height,
        (qbg_bottom_row-qbg_top_row)*qbg_bytes_per_pixel*qbg_width*qbg_character_height);
memset(qbg_active_page_offset+(qbg_bottom_row-1)*qbg_bytes_per_pixel*qbg_width*qbg_character_height,0,qbg_bytes_per_pixel*qbg_width*qbg_character_height);
}




write_page->cursor_y=qbg_bottom_row;
}



}
*/

}//i

null_length:
if (finish_on_new_line&&(!entered_new_line)) newline();



/*
null_length:

//begin new line?
if (newline&&(!newlineadded)) {newline=0; goto qbs_print_newline;}

//hold cursor
hold_cursor:
*/

return;

}


long qbs_cleanup(unsigned long base,long passvalue){
while (qbs_tmp_list_nexti>base) { qbs_tmp_list_nexti--; if(qbs_tmp_list[qbs_tmp_list_nexti]!=0xFFFFFFFF)qbs_free((qbs*)qbs_tmp_list[qbs_tmp_list_nexti]); }//clear any temp. strings created
return passvalue;
}



void qbg_sub_window(long screen,float x1,float y1,float x2,float y2,long passed){
if (new_error) return;
static float i;
static float old_x,old_y;

if (write_page->text) goto qbg_sub_window_error;
if ((!passed)&&screen) goto qbg_sub_window_error;//SCREEEN passed without any other arguements!

//backup current qbg_x & qbg_y coordinates relative to viewport, not window
if (write_page->clipping_or_scaling==2){
old_x=write_page->x*write_page->scaling_x+write_page->scaling_offset_x;
old_y=write_page->y*write_page->scaling_y+write_page->scaling_offset_y;
}else{
old_x=write_page->x;
old_y=write_page->y;
}

if (passed){
if (x1==x2) goto qbg_sub_window_error;
if (y1==y2) goto qbg_sub_window_error;
//sort so x1 & y1 contain the lower values
if (x1>x2){i=x1; x1=x2; x2=i;}
if (y1>y2){i=y1; y1=y2; y2=i;}
if (!screen){
i=y1; y1=y2; y2=i;
}
//Note: Window's coordinates are not based on prev. WINDOW values
write_page->clipping_or_scaling=2;
write_page->scaling_x=((float)(write_page->view_x2-write_page->view_x1))/(x2-x1);
write_page->scaling_y=((float)(write_page->view_y2-write_page->view_y1))/(y2-y1);
write_page->scaling_offset_x=-x1*write_page->scaling_x; //scaling offset should be applied before scaling
write_page->scaling_offset_y=-y1*write_page->scaling_y;
if (!screen){
write_page->scaling_offset_y=-y2*write_page->scaling_y+(write_page->view_y2-write_page->view_y1);
}
write_page->window_x1=x1; write_page->window_x2=x2; write_page->window_y1=y1; write_page->window_y2=y2;


if (x1==0){ if (y1==0){ if (x2==(write_page->width-1)){ if (y2==(write_page->height-1)){
if ((write_page->scaling_x==1)&&(write_page->scaling_y==1)){
if ((write_page->scaling_offset_x==0)&&(write_page->scaling_offset_y==0)){
goto qbg_sub_window_restore_default;
}
}
}}}}

//adjust qbg_x & qbg_y according to new window
write_page->x=(old_x-write_page->scaling_offset_x)/write_page->scaling_x;
write_page->y=(old_y-write_page->scaling_offset_y)/write_page->scaling_y;

return;
}else{
//restore default WINDOW coordinates
qbg_sub_window_restore_default:
write_page->clipping_or_scaling=1;
write_page->scaling_x=1;
write_page->scaling_y=1;
write_page->scaling_offset_x=0;
write_page->scaling_offset_y=0;
write_page->window_x1=0; write_page->window_x2=write_page->width-1; write_page->window_y1=0; write_page->window_y2=write_page->height-1;
if (write_page->view_x1==0){ if (write_page->view_y1==0){ if (write_page->view_x2==(write_page->width-1)){ if (write_page->view_y2==(write_page->height-1)){
if (write_page->view_offset_x==0){ if (write_page->view_offset_y==0){
write_page->clipping_or_scaling=0;
}}
}}}}

//adjust qbg_x & qbg_y according to new window
write_page->x=old_x;
write_page->y=old_y;

return;
}
qbg_sub_window_error:
error(5);
return;
}



void qbg_sub_view_print(long topline,long bottomline,long passed){
if (new_error) return;

static long maxrows;
maxrows=write_page->height; if (!write_page->text) maxrows/=fontheight[write_page->font];

if (!passed){//topline and bottomline not passed
write_page->top_row=1; write_page->bottom_row=maxrows;
write_page->cursor_y=1; write_page->cursor_x=1;
write_page->holding_cursor=0;
return;
}

if (topline<=0) goto error;
if (topline>maxrows) goto error;
if (bottomline<topline) goto error;
if (bottomline>maxrows) goto error;

write_page->top_row=topline;
write_page->bottom_row=bottomline;
write_page->cursor_y=write_page->top_row;
write_page->cursor_x=1;
write_page->holding_cursor=0;
return;

error:
error(5);
return;
}

void qbg_sub_view(long coords_relative_to_screen,long x1,long y1,long x2,long y2,long fillcolor,long bordercolor,long passed){
if (new_error) return;

//format: [{SCREEN}][(?,?)-(?,?)],[?],[?]
//bordercolor draws a line AROUND THE OUTSIDE of the specified viewport
//the current WINDOW settings do not affect inputted x,y values
//the current VIEW settings do not affect inputted x,y values
//REMEMBER! Recalculate WINDOW values based on new viewport dimensions
long i;

//PRE-ERROR CHECKING
if (passed&1){
if (x1<0) goto error;
if (x1>=write_page->width) goto error;
if (y1<0) goto error;
if (y1>=write_page->height) goto error;
if (x2<0) goto error;
if (x2>=write_page->width) goto error;
if (y2<0) goto error;
if (y2>=write_page->height) goto error;
}else{
if (coords_relative_to_screen) goto error;
if (passed&2) goto error;
if (passed&4) goto error;
}

if (passed&1){
//force x1,y1 to be the top left corner
if (x2<x1){i=x1;x1=x2;x2=i;}
if (y2<y1){i=y1;y1=y2;y2=i;}

write_page->view_x1=x1; write_page->view_y1=y1; write_page->view_x2=x2; write_page->view_y2=y2;
if (coords_relative_to_screen==0){
write_page->view_offset_x=x1; write_page->view_offset_y=y1;
}else{
write_page->view_offset_x=0; write_page->view_offset_y=0;
}
if (!write_page->clipping_or_scaling) write_page->clipping_or_scaling=1;
}else{
//no argurments passed
write_page->view_x1=0; write_page->view_y1=0; write_page->view_x2=write_page->width-1; write_page->view_y2=write_page->height-1;
write_page->view_offset_x=0; write_page->view_offset_y=0;
if (write_page->clipping_or_scaling==1) write_page->clipping_or_scaling=0;
}

//recalculate window values based on new viewport (if necessary)
if (write_page->clipping_or_scaling==2){//WINDOW'ing in use
write_page->scaling_x=((float)(write_page->view_x2-write_page->view_x1))/(write_page->window_x2-write_page->window_x1);
write_page->scaling_y=((float)(write_page->view_y2-write_page->view_y1))/(write_page->window_y2-write_page->window_y1);
write_page->scaling_offset_x=-write_page->window_x1*write_page->scaling_x;
write_page->scaling_offset_y=-write_page->window_y1*write_page->scaling_y;
if (write_page->window_y2<write_page->window_y1) write_page->scaling_offset_y=-write_page->window_y2*write_page->scaling_y+write_page->view_y2;
}

if (passed&2){//fillcolor
qb32_boxfill(write_page->window_x1,write_page->window_y1,write_page->window_x2,write_page->window_y2,fillcolor);
}

if (passed&4){//bordercolor
static long bx,by;
by=write_page->view_y1-1;
if ((by>=0)&&(by<write_page->height)){
for (bx=write_page->view_x1-1;bx<=write_page->view_x2;bx++){
if ((bx>=0)&&(bx<write_page->width)){
pset(bx,by,bordercolor);
}}}
by=write_page->view_y2+1;
if ((by>=0)&&(by<write_page->height)){
for (bx=write_page->view_x1-1;bx<=write_page->view_x2;bx++){
if ((bx>=0)&&(bx<write_page->width)){
pset(bx,by,bordercolor);
}}}
bx=write_page->view_x1-1;
if ((bx>=0)&&(bx<write_page->width)){
for (by=write_page->view_y1-1;by<=write_page->view_y2;by++){
if ((by>=0)&&(by<write_page->height)){
pset(bx,by,bordercolor);
}}}
bx=write_page->view_x2+1;
if ((bx>=0)&&(bx<write_page->width)){
for (by=write_page->view_y1-1;by<=(write_page->view_y2+1);by++){
if ((by>=0)&&(by<write_page->height)){
pset(bx,by,bordercolor);
}}}
}

return;
error:
error(5);
return;
}


void sub_cls(long method,unsigned long use_color,long passed){
if (new_error) return;
static long characters,i;
static unsigned short *sp;
static unsigned short clearvalue;

//validate
if (passed&2){
if (write_page->bytes_per_pixel!=4){
if (use_color>write_page->mask) goto error;
}
}else{
use_color=write_page->background_color;
}

if (passed&1){
if ((method>2)||(method<0)) goto error;
}

//all CLS methods reset the cursor position
write_page->cursor_y=write_page->top_row;
write_page->cursor_x=1;

if (write_page->text){
//precalculate a (short) value which can be used to clear the screen
clearvalue=(write_page->color&0xF)+(use_color&7)*16+(write_page->color&16)*8;
clearvalue<<=8;
clearvalue+=32;
}

if ((passed&1)==0){//no method specified
//video mode: clear only graphics viewport
//text mode: clear text view port AND the bottom line
if (write_page->text){
 //text view port
 characters=write_page->width*(write_page->bottom_row-write_page->top_row+1);
 sp=(unsigned short*)&write_page->offset[(write_page->top_row-1)*write_page->width*2];
 for (i=0;i<characters;i++){sp[i]=clearvalue;}
 //bottom line
 characters=write_page->width;
 sp=(unsigned short*)&write_page->offset[(write_page->height-1)*write_page->width*2];
 for (i=0;i<characters;i++){sp[i]=clearvalue;}
 return;
}else{//graphics
 //graphics view port
 if (write_page->bytes_per_pixel==1){//8-bit
  if (write_page->clipping_or_scaling){
  qb32_boxfill(write_page->window_x1,write_page->window_y1,write_page->window_x2,write_page->window_y2,use_color);
  }else{//fast method (no clipping/scaling)
  memset(write_page->offset,use_color,write_page->width*write_page->height);
  }
 }else{//32-bit
  i=write_page->alpha_disabled; write_page->alpha_disabled=1;  
  if (write_page->clipping_or_scaling){
  qb32_boxfill(write_page->window_x1,write_page->window_y1,write_page->window_x2,write_page->window_y2,use_color);
  }else{//fast method (no clipping/scaling)
  fast_boxfill(0,0,write_page->width-1,write_page->height-1,use_color);
  }
  write_page->alpha_disabled=i;
 }
}

if (write_page->clipping_or_scaling==2){
write_page->x=((float)(write_page->view_x2-write_page->view_x1+1))/write_page->scaling_x/2.0f+write_page->scaling_offset_x;
write_page->y=((float)(write_page->view_y2-write_page->view_y1+1))/write_page->scaling_y/2.0f+write_page->scaling_offset_y;
}else{
write_page->x=((float)(write_page->view_x2-write_page->view_x1+1))/2.0f;
write_page->y=((float)(write_page->view_y2-write_page->view_y1+1))/2.0f;
}

return;
}

if (method==0){//clear everything
if (write_page->text){
 characters=write_page->height*write_page->width;
 sp=(unsigned short*)write_page->offset;
 for (i=0;i<characters;i++){sp[i]=clearvalue;}
 return;
}else{
 if (write_page->bytes_per_pixel==1){
 memset(write_page->offset,use_color,write_page->width*write_page->height);
 }else{ 
 i=write_page->alpha_disabled; write_page->alpha_disabled=1;  
 fast_boxfill(0,0,write_page->width-1,write_page->height-1,use_color);
 write_page->alpha_disabled=i;
 }
}

if (write_page->clipping_or_scaling==2){
write_page->x=((float)(write_page->view_x2-write_page->view_x1+1))/write_page->scaling_x/2.0f+write_page->scaling_offset_x;
write_page->y=((float)(write_page->view_y2-write_page->view_y1+1))/write_page->scaling_y/2.0f+write_page->scaling_offset_y;
}else{
write_page->x=((float)(write_page->view_x2-write_page->view_x1+1))/2.0f;
write_page->y=((float)(write_page->view_y2-write_page->view_y1+1))/2.0f;
}

return;
}

if (method==1){//ONLY clear the graphics viewport
if (write_page->text) return;
 //graphics view port
 if (write_page->bytes_per_pixel==1){//8-bit
  if (write_page->clipping_or_scaling){
  qb32_boxfill(write_page->window_x1,write_page->window_y1,write_page->window_x2,write_page->window_y2,use_color);
  }else{//fast method (no clipping/scaling)
  memset(write_page->offset,use_color,write_page->width*write_page->height);
  }
 }else{//32-bit
  i=write_page->alpha_disabled; write_page->alpha_disabled=1;  
  if (write_page->clipping_or_scaling){
  qb32_boxfill(write_page->window_x1,write_page->window_y1,write_page->window_x2,write_page->window_y2,use_color);
  }else{//fast method (no clipping/scaling)
  fast_boxfill(0,0,write_page->width-1,write_page->height-1,use_color);
  }
  write_page->alpha_disabled=i;
 }

if (write_page->clipping_or_scaling==2){
write_page->x=((float)(write_page->view_x2-write_page->view_x1+1))/write_page->scaling_x/2.0f+write_page->scaling_offset_x;
write_page->y=((float)(write_page->view_y2-write_page->view_y1+1))/write_page->scaling_y/2.0f+write_page->scaling_offset_y;
}else{
write_page->x=((float)(write_page->view_x2-write_page->view_x1+1))/2.0f;
write_page->y=((float)(write_page->view_y2-write_page->view_y1+1))/2.0f;
}

return;
}

if (method==2){//ONLY clear the VIEW PRINT range text viewport
if (write_page->text){
 //text viewport
 characters=write_page->width*(write_page->bottom_row-write_page->top_row+1);
 sp=(unsigned short*)&write_page->offset[(write_page->top_row-1)*write_page->width*2];
 for (i=0;i<characters;i++){sp[i]=clearvalue;}
 return;
}else{
 //text viewport
 if (write_page->bytes_per_pixel==1){//8-bit
  memset(&write_page->offset[write_page->width*fontheight[write_page->font]*(write_page->top_row-1)],use_color,write_page->width*fontheight[write_page->font]*(write_page->bottom_row-write_page->top_row+1));
 }else{//32-bit
  i=write_page->alpha_disabled; write_page->alpha_disabled=1;  
  fast_boxfill(0,fontheight[write_page->font]*(write_page->top_row-1),write_page->width-1,fontheight[write_page->font]*write_page->bottom_row-1,use_color);
  write_page->alpha_disabled=i;
 }
 return;
}
}

return;
error:
error(5);
return;
}



void qbg_sub_locate(long row,long column,long cursor,long start,long stop,long passed){
static long h,w,i;
if (new_error) return;

//calculate height & width in characters
if (write_page->compatible_mode){
h=write_page->height/fontheight[write_page->font];
if (fontwidth[write_page->font]){
 w=write_page->width/fontwidth[write_page->font];
}else{
 w=write_page->width;
}
}else{
h=write_page->height;
w=write_page->width;
}

//PRE-ERROR CHECKING
if (passed&1){
if (row<write_page->top_row) goto error;
if ((row!=h)&&(row>write_page->bottom_row)) goto error;
}
if (passed&2){
if (column<1) goto error;
if (column>w) goto error;
}
if (passed&4){
if (cursor<0) goto error;
if (cursor>1) goto error;
}
if (passed&8){
if (start<0) goto error;
if (start>31) goto error;
if (stop<0) goto error;
if (stop>31) goto error;
}

if (passed&1) {write_page->cursor_y=row; write_page->holding_cursor=0;}
if (passed&2) {write_page->cursor_x=column; write_page->holding_cursor=0;}

if (passed&4){
if (cursor) cursor=1;
write_page->cursor_show=cursor;
 if (write_page->flags&IMG_SCREEN){//page-linked attribute
 for (i=0;i<pages;i++){
 if (page[i]) img[i].cursor_show=cursor;
 }
 }//IMG_SCREEN
}//passed&4

if (passed&8){
write_page->cursor_firstvalue=start;
write_page->cursor_lastvalue=stop;
 if (write_page->flags&IMG_SCREEN){//page-linked attribute
 for (i=0;i<pages;i++){
 if (page[i]){
 img[i].cursor_firstvalue=start;
 img[i].cursor_lastvalue=stop;
 }
 }//i
 }//IMG_SCREEN
}

return;

error:
error(5);
return;
}









//input helper functions:
uint64 hexoct2uint64_value;
long hexoct2uint64(qbs* h){
//returns 0=failed
//        1=HEX value (default if unspecified)
//        2=OCT value
static long i,i2;
static uint64 result;
result=0;
static long type;
type=0;
hexoct2uint64_value=0;
if (!h->len) return 1;
if (h->chr[0]!=38) return 0;//not "&"
if (h->len==1) return 1;//& received, but awaiting further input
i=h->chr[1];
if ((i==72)||(i==104)) type=1;//"H"or"h"
if ((i==79)||(i==111)) type=2;//"O"or"o"
if (!type) return 0;
if (h->len==2) return type;

if (type==1){
if (h->len>18) return 0;//larger than int64
for (i=2;i<h->len;i++){
result<<=4;
i2=h->chr[i];
//          0  -      9             A  -      F             a  -      f
if ( ((i2>=48)&&(i2<=57)) || ((i2>=65)&&(i2<=70)) || ((i2>=97)&&(i2<=102)) ){
if (i2>=97) i2-=32;
if (i2>=65) i2-=7;
i2-=48;
//i2 is now a values between 0 and 15
result+=i2;
}else return 0;//invalid character
}//i
hexoct2uint64_value=result;
return 1;
}//type==1

if (type==2){
//unsigned _int64 max=18446744073709551615 (decimal, 20 chars)
//                   =1777777777777777777777 (octal, 22 chars)
//                   =FFFFFFFFFFFFFFFF (hex, 16 chars)
if (h->len>24) return 0;//larger than int64
if (h->len==24){
if ((h->chr[2]!=48)&&(h->chr[2]!=49)) return 0;//larger than int64
}
for (i=2;i<h->len;i++){
result<<=3;
i2=h->chr[i];
if ((i2>=48)&&(i2<=55)){//0-7
i2-=48;
result+=i2;
}else return 0;//invalid character
}//i
hexoct2uint64_value=result;
return 2;
}//type==2

}



//input method (complex, calls other qbs functions)
const char *uint64_max[] =    {"18446744073709551615"};
const char *int64_max[] =     {"9223372036854775807"};
const char *int64_max_neg[] = {"9223372036854775808"};
const char *single_max[] = {"3402823"};
const char *single_max_neg[] = {"1401298"};
const char *double_max[] = {"17976931"};
const char *double_max_neg[] = {"4940656"};
unsigned char significant_digits[1024];
long num_significant_digits;

extern void *qbs_input_variableoffsets[257];
extern long qbs_input_variabletypes[257];
qbs *qbs_input_arguements[257];
long cursor_show_last;


void qbs_input(long numvariables,unsigned char newline){
if (new_error) return;

//duplicate dest image so any changes can be reverted
static long dest_image,dest_image_temp;
dest_image=func__copyimage(NULL,NULL);
if (dest_image==-1) error(257);//out of memory
dest_image_temp=func__copyimage(NULL,NULL);
if (dest_image_temp==-1) error(257);//out of memory
static long dest_image_cursor_x,dest_image_cursor_y;
dest_image_cursor_x=write_page->cursor_x;
dest_image_cursor_y=write_page->cursor_y;

unsigned long qbs_tmp_base=qbs_tmp_list_nexti;

static long lineinput;
lineinput=0;
if (qbs_input_variabletypes[1]&ISSTRING){
if (qbs_input_variabletypes[1]&512){
qbs_input_variabletypes[1]=-512;
lineinput=1;
}}

cursor_show_last=write_page->cursor_show;
write_page->cursor_show=1;

long i,i2,i3,i4,i5,i6;
long addspaces;
addspaces=0;
qbs* inpstr=qbs_new(0,0);//not temp so must be freed
qbs* inpstr2=qbs_new(0,0);//not temp so must be freed
qbs* key=qbs_new(0,0);//not temp so must be freed
qbs* c=qbs_new(1,0);//not temp so must be freed

for (i=1;i<=numvariables;i++) qbs_input_arguements[i]=qbs_new(0,0);

//init all passed variables to 0 or ""
for (i=1;i<=numvariables;i++){

if (qbs_input_variabletypes[i]&ISSTRING){//STRING
if (((qbs*)qbs_input_variableoffsets[i])->fixed){
memset(((qbs*)qbs_input_variableoffsets[i])->chr,32,((qbs*)qbs_input_variableoffsets[i])->len);
}else{
((qbs*)qbs_input_variableoffsets[i])->len=0;
}
}

if ((qbs_input_variabletypes[i]&ISOFFSETINBITS)==0){//reg. numeric variable
memset(qbs_input_variableoffsets[i],0,(qbs_input_variabletypes[i]&511)>>3);
}

//bit referenced?

}//i




qbs_input_next:

long argn,firstchr,toomany;
toomany=0;
argn=1;
i=0;
i2=0;
qbs_input_arguements[1]->len=0;
firstchr=1;
qbs_input_sep_arg:

if (i<inpstr->len){

if (inpstr->chr[i]==44){//","
if (i2!=1){//not in the middle of a string
if (!lineinput){
i2=0;
argn=argn+1;
if (argn>numvariables){toomany=1; goto qbs_input_sep_arg_done;}
qbs_input_arguements[argn]->len=0;
firstchr=1;
goto qbs_input_next_arg;
}
}
}

if (inpstr->chr[i]==34){//"
if (firstchr){
if (!lineinput){
i2=1;//requires closure
firstchr=0;
goto qbs_input_next_arg;
}
}
if (i2==1){
i2=2;
goto qbs_input_next_arg;
}
}

if (i2==2){
goto backspace;//INVALID! Cannot have any characters after a closed "..."
}

c->chr[0]=inpstr->chr[i];
qbs_set(qbs_input_arguements[argn],qbs_add(qbs_input_arguements[argn],c));

firstchr=0;
qbs_input_next_arg:;
i++;
goto qbs_input_sep_arg;
}
qbs_input_sep_arg_done:
if (toomany) goto backspace;

//validate current arguements
//ASSUME LEADING & TRALING SPACES REMOVED!
unsigned char valid;
unsigned char neg;
long completewith;
long l;
unsigned char *cp,*cp2;
uint64 max,max_neg,multiple,value;
uint64 hexvalue;

completewith=-1;
valid=1;
l=qbs_input_arguements[argn]->len;
cp=qbs_input_arguements[argn]->chr;
neg=0;

if ((qbs_input_variabletypes[argn]&ISSTRING)==0){
if ((qbs_input_variabletypes[argn]&ISFLOAT)==0){
if ((qbs_input_variabletypes[argn]&511)<=32){//cannot handle INTEGER64 variables using this method!
int64 finalvalue;
//it's an integer variable!
finalvalue=0;
if (l==0){completewith=48; goto typechecked_integer;}
//calculate max & max_neg (i4 used to store number of bits)
i4=qbs_input_variabletypes[argn]&511;
max=1;
max<<=i4;
max--;

//check for hex/oct
if (i3=hexoct2uint64(qbs_input_arguements[argn])){
hexvalue=hexoct2uint64_value;
if (hexvalue>max){valid=0; goto typechecked;}
//i. check max num of "digits" required to represent a value, if more exist cull excess
//ii. set completewith value (if necessary)
if (i3==1){
 value=max;
 i=0;
 for (i2=1;i2<=16;i2++){
 if (value&0xF) i=i2;
 value>>=4;
 }
 if (l>(2+i)){valid=0; goto typechecked;}
 if (l==1) completewith=72;//"H"
 if (l==2) completewith=48;//"0"
}
if (i3==2){
 value=max;
 i=0;
 for (i2=1;i2<=22;i2++){
 if (value&0x7) i=i2;
 value>>=3;
 }
 if (l>(2+i)){valid=0; goto typechecked;}
 if (l==1) completewith=111;//"O"
 if (l==2) completewith=48;//"0"
}
finalvalue=hexvalue;
goto typechecked_integer;
}

//max currently contains the largest UNSIGNED value possible, adjust as necessary
if (qbs_input_variabletypes[argn]&ISUNSIGNED){ 
max_neg=0;
}else{
max>>=1;
max_neg=max+1;
}
//check for - sign
i2=0;
 if ((qbs_input_variabletypes[argn]&ISUNSIGNED)==0){ 
 if (cp[i2]==45){//"-"
 if (l==1) {completewith=48; goto typechecked_integer;}
 i2++; neg=1;
 }
 }
//after a leading 0 no other digits are possible, return an error if this is the case
if (cp[i2]==48){
if (l>(i2+1)){valid=0; goto typechecked;}
}
//scan the "number"...
multiple=1;
value=0;
for (i=l-1;i>=i2;i--){
i3=cp[i]-48;
if ((i3>=0)&&(i3<=9)){
value+=multiple*i3;
 if (qbs_input_variabletypes[argn]&ISUNSIGNED){ 
 if (value>max){valid=0; goto typechecked;}
 }else{
 if (neg){
 if (value>max_neg){valid=0; goto typechecked;}
 }else{
 if (value>max){valid=0; goto typechecked;}
 }
 }
}else{valid=0; goto typechecked;}
multiple*=10;
}//next i
if (neg) finalvalue=-value; else finalvalue=value;
typechecked_integer:
//set variable to finalvalue
if ((qbs_input_variabletypes[argn]&ISOFFSETINBITS)==0){//reg. numeric variable
memcpy(qbs_input_variableoffsets[argn],&finalvalue,(qbs_input_variabletypes[argn]&511)>>3);
}
goto typechecked;
}
}
}

if (qbs_input_variabletypes[argn]&ISSTRING){
if (((qbs*)qbs_input_variableoffsets[argn])->fixed){
if (l>((qbs*)qbs_input_variableoffsets[argn])->len) {valid=0; goto typechecked;}
}
qbs_set((qbs*)qbs_input_variableoffsets[argn],qbs_input_arguements[argn]);
goto typechecked;
}

//INTEGER64 type
//int64 range:          –9223372036854775808 to  9223372036854775807
//uint64 range: 0                    to 18446744073709551615
if ((qbs_input_variabletypes[argn]&ISSTRING)==0){
if ((qbs_input_variabletypes[argn]&ISFLOAT)==0){
if ((qbs_input_variabletypes[argn]&511)==64){
if (l==0){completewith=48; *(int64*)qbs_input_variableoffsets[argn]=0; goto typechecked;}

//check for hex/oct
if (i3=hexoct2uint64(qbs_input_arguements[argn])){
hexvalue=hexoct2uint64_value;
if (hexvalue>max){valid=0; goto typechecked;}
//set completewith value (if necessary)
if (i3==1) if (l==1) completewith=72;//"H"
if (i3==2) if (l==1) completewith=111;//"O"
if (l==2) completewith=48;//"0"
*(uint64*)qbs_input_variableoffsets[argn]=hexvalue;
goto typechecked;
}

//check for - sign
i2=0;
 if ((qbs_input_variabletypes[argn]&ISUNSIGNED)==0){ 
 if (cp[i2]==45){//"-"
 if (l==1) {completewith=48; *(int64*)qbs_input_variableoffsets[argn]=0; goto typechecked;}
 i2++; neg=1;
 }
 }
//after a leading 0 no other digits are possible, return an error if this is the case
if (cp[i2]==48){
if (l>(i2+1)){valid=0; goto typechecked;}
}
//count how many digits are in the number
i4=0;
for (i=l-1;i>=i2;i--){
i3=cp[i]-48;
if ((i3<0)||(i3>9)) {valid=0; goto typechecked;}
i4++;
}//i
if (qbs_input_variabletypes[argn]&ISUNSIGNED){
if (i4<20) goto typechecked_int64;
if (i4>20) {valid=0; goto typechecked;}
cp2=(unsigned char*)uint64_max[0];
}else{
if (i4<19) goto typechecked_int64;
if (i4>19) {valid=0; goto typechecked;}
if (neg) cp2=(unsigned char*)int64_max_neg[0]; else cp2=(unsigned char*)int64_max[0];
}
//number of digits valid, but exact value requires checking
cp=qbs_input_arguements[argn]->chr;
for (i=0;i<i4;i++){
if (cp[i+i2]<cp2[i]) goto typechecked_int64;
if (cp[i+i2]>cp2[i]) {valid=0; goto typechecked;}
}
typechecked_int64:
//add character 0 to end to make it a null terminated string
c->chr[0]=0; qbs_set(qbs_input_arguements[argn],qbs_add(qbs_input_arguements[argn],c));
if (qbs_input_variabletypes[argn]&ISUNSIGNED){
#ifdef QB64_WINDOWS
 sscanf((char*)qbs_input_arguements[argn]->chr,"%I64u",(uint64*)qbs_input_variableoffsets[argn]);
#else
 sscanf((char*)qbs_input_arguements[argn]->chr,"%ull",(uint64*)qbs_input_variableoffsets[argn]);
#endif
}else{
#ifdef QB64_WINDOWS
 sscanf((char*)qbs_input_arguements[argn]->chr,"%I64i",(int64*)qbs_input_variableoffsets[argn]);
#else
 sscanf((char*)qbs_input_arguements[argn]->chr,"%ll",(int64*)qbs_input_variableoffsets[argn]);
#endif
}
goto typechecked;
}
}
}

//check ISFLOAT type?
//[-]9999[.]9999[E/D][+/-]99999
if (qbs_input_variabletypes[argn]&ISFLOAT){
static long digits_before_point;
static long digits_after_point;
static long zeros_after_point;
static long neg_power;
digits_before_point=0;
digits_after_point=0;
neg_power=0;
value=0;
zeros_after_point=0;
num_significant_digits=0;

//set variable to 0
if ((qbs_input_variabletypes[argn]&511)==32) *(float*)qbs_input_variableoffsets[argn]=0;
if ((qbs_input_variabletypes[argn]&511)==64) *(double*)qbs_input_variableoffsets[argn]=0;
if ((qbs_input_variabletypes[argn]&511)==256) *(long double*)qbs_input_variableoffsets[argn]=0;

//begin with a generic assessment, regardless of whether it is single, double or float
if (l==0){completewith=48; goto typechecked;}

//check for hex/oct
if (i3=hexoct2uint64(qbs_input_arguements[argn])){
hexvalue=hexoct2uint64_value;
//set completewith value (if necessary)
if (i3==1) if (l==1) completewith=72;//"H"
if (i3==2) if (l==1) completewith=111;//"O"
if (l==2) completewith=48;//"0"
//nb. because VC6 didn't support...
//error C2520: conversion from uint64 to double not implemented, use signed int64
//I've implemented a work-around so correct values will be returned
static int64 transfer;
transfer=0x7FFFFFFF;
transfer<<=32;
transfer|=0xFFFFFFFF;
while(hexvalue>transfer){
hexvalue-=transfer;
if ((qbs_input_variabletypes[argn]&511)==32) *(float*)qbs_input_variableoffsets[argn]+=transfer;
if ((qbs_input_variabletypes[argn]&511)==64) *(double*)qbs_input_variableoffsets[argn]+=transfer;
if ((qbs_input_variabletypes[argn]&511)==256) *(long double*)qbs_input_variableoffsets[argn]+=transfer;
}
transfer=hexvalue;
if ((qbs_input_variabletypes[argn]&511)==32) *(float*)qbs_input_variableoffsets[argn]+=transfer;
if ((qbs_input_variabletypes[argn]&511)==64) *(double*)qbs_input_variableoffsets[argn]+=transfer;
if ((qbs_input_variabletypes[argn]&511)==256) *(long double*)qbs_input_variableoffsets[argn]+=transfer;
goto typechecked;
}

//check for - sign
i2=0;
 if (cp[i2]==45){//"-"
 if (l==1) {completewith=48; goto typechecked;}
 i2++; neg=1;
 }
//if it starts with 0, it may only have one leading 0
if (cp[i2]==48){
if (l>(i2+1)){
i2++;
if (cp[i2]==46) goto decimal_point;
valid=0; goto typechecked;//expected a decimal point
//nb. of course, user could have typed D or E BUT there is no point
//    calculating 0 to the power of anything!
}else goto typechecked;//validate, as no other data is required
}
//scan digits before decimal place
for (i=i2;i<l;i++){
i3=cp[i];
if ((i3==68)||(i3==(68+32))||(i3==69)||(i3==(69+32))){//d,D,e,E?
if (i==i2){valid=0; goto typechecked;}//cannot begin with d,D,e,E!
i2=i;
goto exponent;
}
if (i3==46){i2=i; goto decimal_point;}//nb. it can begin with a decimal point!
i3-=48;
if ((i3<0)||(i3>9)){valid=0; goto typechecked;}
digits_before_point++;
//nb. because leading 0 is handled differently, all digits are significant
significant_digits[num_significant_digits]=i3+48; num_significant_digits++;
}
goto assess_float;
////////////////////////////////
decimal_point:;
i4=1;
if (i2==(l-1)) {completewith=48; goto assess_float;}
i2++;
for (i=i2;i<l;i++){
i3=cp[i];
if ((i3==68)||(i3==(68+32))||(i3==69)||(i3==(69+32))){//d,D,e,E?
if (num_significant_digits){
if (i==i2){valid=0; goto typechecked;}//cannot begin with d,D,e,E just after a decimal point!
i2=i;
goto exponent;
}
}
i3-=48;
if ((i3<0)||(i3>9)){valid=0; goto typechecked;}
if (i3) i4=0;
if (i4) zeros_after_point++;
digits_after_point++;
if ((num_significant_digits)||i3){
significant_digits[num_significant_digits]=i3+48; num_significant_digits++;
}
}//i
goto assess_float;
////////////////////////////////
exponent:;
//ban d/D for SINGLE precision input
if ((qbs_input_variabletypes[argn]&511)==32){//SINGLE
i3=cp[i2];
if ((i3==68)||(i3==(68+32))){//d/D
valid=0; goto typechecked;
}
}
//correct "D" notation for c++ scanf
i3=cp[i2];
if ((i3==68)||(i3==(68+32))){//d/D
cp[i2]=69;//"E"
}
if (i2==(l-1)) {completewith=48; goto assess_float;}
i2++;
//check for optional + or -
i3=cp[i2];
if (i3==45){//"-"
if (i2==(l-1)) {completewith=48; goto assess_float;}
neg_power=1;
i2++;
}
if (i3==43){//"+"
if (i2==(l-1)) {completewith=48; goto assess_float;}
i2++;
}
//nothing valid after a leading 0
if (cp[i2]==48){//0
if (l>(i2+1)) {valid=0; goto typechecked;}
}
multiple=1;
value=0;
for (i=l-1;i>=i2;i--){
i3=cp[i]-48;
if ((i3>=0)&&(i3<=9)){
value+=multiple*i3;
}else{
valid=0; goto typechecked;
}
multiple*=10;
}//i
//////////////////////////
assess_float:;
//nb. 0.???? means digits_before_point==0

if ((qbs_input_variabletypes[argn]&511)==32){//SINGLE
//QB:           ±3.402823    E+38 to ±1.401298    E-45
//WIKIPEDIA:    ±3.4028234   E+38 to ?
//OTHER SOURCE: ±3.402823466 E+38 to ±1.175494351 E-38
if (neg_power) value=-value;
//special case->single 0 after point
if ((zeros_after_point==1)&&(digits_after_point==1)){
digits_after_point=0;
zeros_after_point=0;
}
//upper overflow check
//i. check that value doesn't consist solely of 0's
if (zeros_after_point>43){valid=0; goto typechecked;}//cannot go any further without reversal by exponent
if ((digits_before_point==0)&&(digits_after_point==zeros_after_point)) goto nooverflow_float;
//ii. calculate the position of the first WHOLE digit (in i)
i=digits_before_point;
if (!i) i=-zeros_after_point;
/*EXAMPLES:
1.0			i=1
12.0		i=2
0.1			i=0
0.01		i=-1
*/
i=i+value;//apply exponent
if (i>39){valid=0; goto typechecked;}
//nb. the above blocks the ability to type a long-long number and use a neg exponent
//    to validate it
//********IMPORTANT: if i==39 then the first 7 digits MUST be scanned!!!
if (i==39){
cp2=(unsigned char*)single_max[0];
i2=num_significant_digits;
if (i2>7) i2=7;
for (i3=0;i3<i2;i3++){
if (significant_digits[i3]>*cp2){valid=0; goto typechecked;}
if (significant_digits[i3]<*cp2) break;
cp2++;
}
}
//check for pointless levels of precision (eg. 1.21351273512653625116212!)
if (digits_after_point){
if (digits_before_point){
if ((digits_after_point+digits_before_point)>8){valid=0; goto typechecked;}
}else{
if ((digits_after_point-zeros_after_point)>8){valid=0; goto typechecked;}
}
}
//check for "under-flow"
if (i<-44){valid=0; goto typechecked;}
//********IMPORTANT: if i==-44 then the first 7 digits MUST be scanned!!!
if (i==-44){
cp2=(unsigned char*)single_max_neg[0];
i2=num_significant_digits;
if (i2>7) i2=7;
for (i3=0;i3<i2;i3++){
if (significant_digits[i3]<*cp2){valid=0; goto typechecked;}
if (significant_digits[i3]>*cp2) break;
cp2++;
}
}
nooverflow_float:;
c->chr[0]=0; qbs_set(qbs_input_arguements[argn],qbs_add(qbs_input_arguements[argn],c));
sscanf((char*)qbs_input_arguements[argn]->chr,"%f",(float*)qbs_input_variableoffsets[argn]);
goto typechecked;
}

if ((qbs_input_variabletypes[argn]&511)==64){//DOUBLE
//QB: Double (15-digit) precision ±1.7976931 D+308 to ±4.940656 D-324
//WIKIPEDIA:    ±1.7976931348623157 D+308 to ???
//OTHER SOURCE: ±1.7976931348623157 D+308 to ±2.2250738585072014E-308



if (neg_power) value=-value;
//special case->single 0 after point
if ((zeros_after_point==1)&&(digits_after_point==1)){
digits_after_point=0;
zeros_after_point=0;
}
//upper overflow check
//i. check that value doesn't consist solely of 0's
if (zeros_after_point>322){valid=0; goto typechecked;}//cannot go any further without reversal by exponent
if ((digits_before_point==0)&&(digits_after_point==zeros_after_point)) goto nooverflow_double;
//ii. calculate the position of the first WHOLE digit (in i)
i=digits_before_point;
if (!i) i=-zeros_after_point;
i=i+value;//apply exponent
if (i>309){valid=0; goto typechecked;}
//nb. the above blocks the ability to type a long-long number and use a neg exponent
//    to validate it
//********IMPORTANT: if i==309 then the first 8 digits MUST be scanned!!!
if (i==309){
cp2=(unsigned char*)double_max[0];
i2=num_significant_digits;
if (i2>8) i2=8;
for (i3=0;i3<i2;i3++){
if (significant_digits[i3]>*cp2){valid=0; goto typechecked;}
if (significant_digits[i3]<*cp2) break;
cp2++;
}
}
//check for pointless levels of precision (eg. 1.21351273512653625116212!)
if (digits_after_point){
if (digits_before_point){
if ((digits_after_point+digits_before_point)>16){valid=0; goto typechecked;}
}else{
if ((digits_after_point-zeros_after_point)>16){valid=0; goto typechecked;}
}
}
//check for "under-flow"
if (i<-323){valid=0; goto typechecked;}
//********IMPORTANT: if i==-323 then the first 7 digits MUST be scanned!!!
if (i==-323){
cp2=(unsigned char*)double_max_neg[0];
i2=num_significant_digits;
if (i2>7) i2=7;
for (i3=0;i3<i2;i3++){
if (significant_digits[i3]<*cp2){valid=0; goto typechecked;}
if (significant_digits[i3]>*cp2) break;
cp2++;
}
}
nooverflow_double:;
c->chr[0]=0; qbs_set(qbs_input_arguements[argn],qbs_add(qbs_input_arguements[argn],c));
sscanf((char*)qbs_input_arguements[argn]->chr,"%lf",(double*)qbs_input_variableoffsets[argn]);
goto typechecked;
}

if ((qbs_input_variabletypes[argn]&511)==256){//FLOAT
//at present, there is no defined limit for FLOAT type numbers, so no restrictions
//are applied!
c->chr[0]=0; qbs_set(qbs_input_arguements[argn],qbs_add(qbs_input_arguements[argn],c));

//sscanf((char*)qbs_input_arguements[argn]->chr,"%lf",(long double*)qbs_input_variableoffsets[argn]);
static double sscanf_fix;
sscanf((char*)qbs_input_arguements[argn]->chr,"%lf",&sscanf_fix);
*(long double*)qbs_input_variableoffsets[argn]=sscanf_fix;

}

}//ISFLOAT

//undefined/uncheckable types fall through as valid!
typechecked:;
if (!valid) goto backspace;



qbs_set(inpstr2,inpstr);


//input a key



qbs_input_invalidinput:

static long showing_cursor;
showing_cursor=0;

qbs_input_wait_for_key:

//toggle box cursor
if (!write_page->text){
i=1;
if ((write_page->font>=32)||(write_page->compatible_mode==256)||(write_page->compatible_mode==32)){
if (SDL_GetTicks()&512) i=0;
}
if (i!=showing_cursor){
showing_cursor^=1;
static long x,y,x2,y2,fx,fy,alpha,cw;
static unsigned long c;
alpha=write_page->alpha_disabled; write_page->alpha_disabled=1;
fy=fontheight[write_page->font];
fx=fontwidth[write_page->font]; if (!fx) fx=1;
cw=fx; if ((write_page->font>=32)||(write_page->compatible_mode==256)||(write_page->compatible_mode==32)) cw=1;
y2=(write_page->cursor_y-1)*fy;
for (y=0;y<fy;y++){
x2=(write_page->cursor_x-1)*fx;
for (x=0;x<cw;x++){
pset (x2,y2,point(x2,y2)^write_page->color);
x2++;
}
y2++;
}
write_page->alpha_disabled=alpha;
}
}//!write_page->text

if (addspaces){
addspaces--;
c->chr[0]=32; qbs_set(key,c);
}else{
SDL_Delay(10);
qbs_set(key,qbs_inkey());
qbs_cleanup(qbs_tmp_base,0);
}
if (stop_program) return;
if (key->len!=1) goto qbs_input_wait_for_key;

//remove box cursor
if (!write_page->text){
if (showing_cursor){
showing_cursor=0;
static long x,y,x2,y2,fx,fy,cw,alpha;
static unsigned long c;
alpha=write_page->alpha_disabled; write_page->alpha_disabled=1;
fy=fontheight[write_page->font];
fx=fontwidth[write_page->font]; if (!fx) fx=1;
cw=fx; if ((write_page->font>=32)||(write_page->compatible_mode==256)||(write_page->compatible_mode==32)) cw=1;
y2=(write_page->cursor_y-1)*fy;
for (y=0;y<fy;y++){
x2=(write_page->cursor_x-1)*fx;
for (x=0;x<cw;x++){
pset (x2,y2,point(x2,y2)^write_page->color);
x2++;
}
y2++;
}
write_page->alpha_disabled=alpha;
}
}//!write_page->text

//input should disallow certain characters
if (key->chr[0]==7) {qbs_print(key,0); goto qbs_input_next;}//beep!
if (key->chr[0]==10) goto qbs_input_next;//linefeed
if (key->chr[0]==9){//tab
i=8-(inpstr2->len&7);
addspaces=i;
goto qbs_input_next;
}
//other ASCII chars that cannot be printed
if (key->chr[0]==11) goto qbs_input_next;
if (key->chr[0]==12) goto qbs_input_next;
if (key->chr[0]==28) goto qbs_input_next;
if (key->chr[0]==29) goto qbs_input_next;
if (key->chr[0]==30) goto qbs_input_next;
if (key->chr[0]==31) goto qbs_input_next;

if (key->chr[0]==13){
//assume input is valid

//autofinish (if necessary)

//assume all parts entered

for (i=1;i<=numvariables;i++){
qbs_free(qbs_input_arguements[i]);
}

if (newline){
c->len=0;
qbs_print(c,1);
}
qbs_free(c);
qbs_free(inpstr);
qbs_free(inpstr2);
qbs_free(key);

write_page->cursor_show=cursor_show_last;

sub__freeimage(dest_image,1); sub__freeimage(dest_image_temp,1);

return;
}

if (key->chr[0]==8){//backspace
backspace:
if (!inpstr->len) goto qbs_input_invalidinput;
inpstr->len--;
i2=func__dest();//backup current dest
sub_pcopy(dest_image,dest_image_temp);//copy original background to temp
//write characters to temp
sub__dest(dest_image_temp);
write_page->cursor_x=dest_image_cursor_x; write_page->cursor_y=dest_image_cursor_y;
for (i=0;i<inpstr->len;i++){key->chr[0]=inpstr->chr[i]; qbs_print(key,0);}
sub__dest(i2);
//copy temp to dest
sub_pcopy(dest_image_temp,i2);
//update cursor
write_page->cursor_x=img[-dest_image_temp].cursor_x; write_page->cursor_y=img[-dest_image_temp].cursor_y;
goto qbs_input_next;
}

if (inpstr2->len>=255) goto qbs_input_next;

//affect inpstr2 with key
qbs_set(inpstr2,qbs_add(inpstr2,key));

//perform actual update
qbs_print(key,0);

qbs_set(inpstr,inpstr2);

goto qbs_input_next;

}//qbs_input

const char *func_val_max[]={"1797693134862315"};
double func_val(qbs *s){
static unsigned char *val_max;
val_max=(unsigned char*)func_val_max[0];
static long i,i2,step,c,num_significant_digits,most_significant_digit_position;
static long num_exponent_digits;
static long negate,negate_exponent;
static unsigned char significant_digits[16];
static int64 exponent_value;
static unsigned char built_number[32];
static double return_value;
static int64 value;
static int64 hex_value;
static long hex_digits;
if (!s->len) return 0;
value=0;
negate_exponent=0;
num_exponent_digits=0;
num_significant_digits=0;
most_significant_digit_position=0;
step=0;
exponent_value=0;
negate=0;

i=0;
for (i=0;i<s->len;i++){
c=s->chr[i];

if ((c==32)||(c==9)) goto whitespace;

if (c==38){//&
if (step==0) goto hex;
goto finish;
}

if (c==45){//-
if (step==0){negate=1; step=1; goto checked;}
if (step==3){negate_exponent=1; step=4; goto checked;}
goto finish;
}

if (c==43){//+
if (step==0){step=1; goto checked;}
if (step==3){step=4; goto checked;}
goto finish;
}

if ((c>=48)&&(c<=57)){//0-9

if (step<=1){//before decimal point
step=1;
if ((num_significant_digits)||(c>48)){
most_significant_digit_position++;
if (num_significant_digits>=16) goto checked;
significant_digits[num_significant_digits]=c;
num_significant_digits++;
value=value*10+c-48;
}
}

if (step==2){//after decimal point
if ((num_significant_digits==0)&&(c==48)) most_significant_digit_position--;
if ((num_significant_digits)||(c>48)){
if (num_significant_digits>=16) goto checked;
significant_digits[num_significant_digits]=c;
num_significant_digits++;
}
}

if (step>=3){//exponent
step=4;
if ((num_exponent_digits)||(c>48)){
if (num_exponent_digits>=18) goto finish;
exponent_value*=10; exponent_value=exponent_value+c-48;//precalculate
num_exponent_digits++;
}
}

goto checked;
}

if (c==46){//.
if (step>1) goto finish;
step=2; goto checked;
}

if ((c==68)||(c==69)||(c==100)||(c==101)){//D,E,d,e
if (step>2) goto finish;
step=3; goto checked;
}

goto finish;//invalid character
checked:
whitespace:;
}
finish:;

if (num_significant_digits==0) return 0;

if (exponent_value==0){
if (num_significant_digits==most_significant_digit_position){
if (negate) value=-value;
return value;
}
}

if (negate_exponent) exponent_value=-exponent_value;
i=0;
if (negate) {built_number[i]=45; i++;}//-
if (num_significant_digits){
 for (i2=0;i2<num_significant_digits;i2++){
 if (i2==1){built_number[i]=46; i++;}
 built_number[i]=significant_digits[i2]; i++;
 }
 built_number[i]=69; i++;//E
 //adjust exponent value appropriately
 //if most_significant_digit_position=1, then no change need occur
 exponent_value=exponent_value+most_significant_digit_position-1;
 //range check exponent value & return an error if necessary
 if (exponent_value>308){error(6); return 0;}
 if (exponent_value<-324)return 0;
 //range check significant digits (if necessary)
 //LIMIT FROM QB4.5 ---> PRINT VAL("1.797693134862315D+308"), the largest possible value
 if (exponent_value==308){
 for (i2=0;i2<num_significant_digits;i2++){
 if (significant_digits[i2]>val_max[i2]){error(6); return 0;}
 if (significant_digits[i2]<val_max[i2]) break;
 }
 }
 //add exponent's value
 #ifdef QB64_WINDOWS
  i2=sprintf((char*)&built_number[i],"%I64i",exponent_value);
 #else
  i2=sprintf((char*)&built_number[i],"%ll",exponent_value);
 #endif
 i=i+i2;
}else{
 built_number[i]=48; i++;//0
}
built_number[i]=0;//NULL terminate
sscanf((char*)&built_number[0],"%lf",&return_value);
return return_value;

hex://hex/oct
if (i>=(s->len-2)) return 0;
c=s->chr[i+1];
if ((c==79)||(c==111)){//"O"or"o"
 hex_digits=0;
 hex_value=0;
 for (i=i+2;i<s->len;i++){
 c=s->chr[i];
 if ((c>=48)&&(c<=55)){//0-7
 c-=48;
 hex_value<<=3;
 hex_value|=c;
 if (hex_digits||c) hex_digits++; 
  if (hex_digits>=22){
  if ((hex_digits>22)||(s->chr[i-21]>49)){error(6); return 0;}
  }
 }else break;
 }//i
 return hex_value;
}
if ((c==72)||(c==104)){//"H"or"h"
 hex_digits=0;
 hex_value=0;
 for (i=i+2;i<s->len;i++){
 c=s->chr[i];
 if ( ((c>=48)&&(c<=57)) || ((c>=65)&&(c<=70)) || ((c>=97)&&(c<=102)) ){//0-9 or A-F or a-f
 if ((c>=48)&&(c<=57)) c-=48;
 if ((c>=65)&&(c<=70)) c-=55;
 if ((c>=97)&&(c<=102)) c-=87;
 hex_value<<=4;
 hex_value|=c;
 if (hex_digits||c) hex_digits++;
 if (hex_digits>16) {error(6); return 0;}
 }else break;
 }//i
 return hex_value;
}
return 0;//& followied by unknown
}






long unsupported_port_accessed=0;

long H3C8_palette_register_index=0;
long H3C9_next=0;
long palette_register_index=0;

void sub_out(long port,long data){
if (new_error) return;

unsupported_port_accessed=0;
port=port&65535;
data=data&255;
if (port==968){//&H3C8, set palette register index
H3C8_palette_register_index=data;
goto done;
}
if (port==969){//&H3C9, set palette color
data=data&63;
if (write_page->pal){//avoid NULL pointer
if (H3C9_next==0){//red
write_page->pal[H3C8_palette_register_index]&=0x00FFFF;
write_page->pal[H3C8_palette_register_index]+=(qbr((double)data*4.063492f-0.4999999f)<<16);
}
if (H3C9_next==1){//green
write_page->pal[H3C8_palette_register_index]&=0xFF00FF;
write_page->pal[H3C8_palette_register_index]+=(qbr((double)data*4.063492f-0.4999999f)<<8);
}
if (H3C9_next==2){//blue
write_page->pal[H3C8_palette_register_index]&=0xFFFF00;
write_page->pal[H3C8_palette_register_index]+=qbr((double)data*4.063492f-0.4999999f);
}
}
H3C9_next=H3C9_next+1;
if (H3C9_next==3){
H3C9_next=0;
H3C8_palette_register_index=H3C8_palette_register_index+1;
H3C8_palette_register_index&=0xFF;
}
goto done;
}

unsupported_port_accessed=1;
done:
return;
error:
error(5);
}

unsigned long rnd_seed=327680;
void sub_randomize (double seed,long passed){
if (new_error) return;

if (passed){
//Dim As Uinteger m = cptr(Uinteger Ptr, @n)[1]
static unsigned long m;
m=((unsigned long*)&seed)[1];
//m Xor= (m Shr 16)
m^=(m>>16);
//rnd_seed = (m And &hffff) Shl 8 Or (rnd_seed And &hff)
rnd_seed=((m&0xffff)<<8)|(rnd_seed&0xff);
return;
}
qbs_print(qbs_new_txt("Random-number seed (-32768 to 32767)? "),0);
static short integerseed;
qbs_input_variabletypes[1]=16;//id.t=16 'a signed 16 bit integer
qbs_input_variableoffsets[1]=&integerseed;
qbs_input(1,1);
//rnd_seed = (m And &hffff) Shl 8 Or (rnd_seed And &hff) 'nb. same as above
rnd_seed=((integerseed&0xffff)<<8)|(rnd_seed&0xff);
return;
}
float func_rnd(float n,long passed){
if (new_error) return 0;

static unsigned long m;
if (!passed) n=1.0f;
if (n!=0.0){
if (n<0.0){
m=*((unsigned long*)&n);
rnd_seed=(m&0xFFFFFF)+((m&0xFF000000)>>24);
}
rnd_seed=(rnd_seed*16598013+12820163)&0xFFFFFF;
}     
return (double)rnd_seed/0x1000000;
}

float func_timer(){

static unsigned long x;
static double d;
x=SDL_GetTicks();
x-=sdl_firsttimervalue;
x+=qb64_firsttimervalue;
//make timer value loop after midnight
//note: there are 86400000 milliseconds in 24hrs(1 day)
func_timer_fixoffset:
if (x>=86400000){
x-=86400000;
goto func_timer_fixoffset;
}
d=x;
//reduce accuracy to 18.2 "ticks" per second
d*=18.2;
d/=1000.0;
d=(unsigned long)d;//round
d/=18.2;
return (float)d;
}

void sub_sound(double frequency,double lengthinclockticks){
sndsetup();
if (new_error) return;
//note: there are 18.2 clock ticks per second
if ((frequency<37.0)&&(frequency!=0)) goto error;
if (frequency>32767.0) goto error;
if (lengthinclockticks<0.0) goto error;
if (lengthinclockticks>65535.0) goto error;
if (lengthinclockticks==0.0) return;
qb64_generatesound(frequency,lengthinclockticks/18.2,1);
return;
error:
error(5);
}

//FILE ACCESS SUBS/FUNCTIONS

//device handles
struct device_handle_struct
{
unsigned long open;//1 or 0
fstream fh;
long pos;
long eof_reached;
long read_access;
long write_access;
long type;
long record_length;
uint64 column;//used for tab'ing in OUTPUT modes (starts at 0!)
};
unsigned long last_device_handle=1023;
device_handle_struct device_handle[1024];






void sub_open(qbs *name,long type,long access,long sharing,long i,long record_length,long passed){
static ofstream ofh;
static ifstream ifh;


if (new_error) return;



//?[{FOR RANDOM|FOR BINARY|FOR INPUT|FOR OUTPUT|FOR APPEND}]
//1 2
//[{ACCESS READ WRITE|ACCESS READ|ACCESS WRITE}]
//  3
//[{SHARED|LOCK READ WRITE|LOCK READ|LOCK WRITE}]{AS}[#]?[{LEN =}?]
//  4                                                   5        6[1]
static unsigned long x,x2,x3;
static qbs *tqbs=NULL;
if (!tqbs) tqbs=qbs_new(0,0);
static qbs *nullt=NULL;
if (!nullt) nullt=qbs_new(1,0);
nullt->chr[0]=0;



//check range of i
if ((i<1)||(i>last_device_handle)){error(52); return;}//Bad file name or number
//check if already open
if (device_handle[i].open){error(55); return;}//File already open

if (type<=1){//RANDOM (default)
//note: default record len=128
device_handle[i].record_length=128;
if (passed){
if ((record_length==0)||(record_length<-1)){error(5); return;}//Illegal function call
if (record_length==-1) record_length=128;
device_handle[i].record_length=record_length;
}
qbs_set(tqbs,qbs_add(name,nullt));//prepare null-terminated filename
device_handle[i].read_access=1; device_handle[i].write_access=1;
if (access==2) device_handle[i].write_access=0;
if (access==3) device_handle[i].read_access=0;
//file exists?
ifh.open((const char*)tqbs->chr,ios::in);
if (ifh.is_open()){
ifh.close();
}else{
ifh.clear();
//create file
ofh.open((const char*)tqbs->chr,ios::out);
if (ofh.is_open()==NULL){ofh.clear(); error(64); return;}//Bad file name
ofh.close();
}
device_handle[i].fh.open((const char*)tqbs->chr,ios::binary|ios::out|ios::in);
if (device_handle[i].fh.is_open()==NULL){device_handle[i].fh.clear(); error(64); return;}//Bad file name
device_handle[i].open=1;
device_handle[i].pos=0;
device_handle[i].eof_reached=0;
device_handle[i].type=1;
return;
}

if (type==2){//BINARY
//note: recordlength is ignored (as in QB4.5) for BINARY
qbs_set(tqbs,qbs_add(name,nullt));//prepare null-terminated filename
device_handle[i].read_access=1; device_handle[i].write_access=1;
if (access==2) device_handle[i].write_access=0;
if (access==3) device_handle[i].read_access=0;
//file exists?
ifh.open((const char*)tqbs->chr,ios::in);
if (ifh.is_open()){
ifh.close();
}else{
ifh.clear();
//create file
ofh.open((const char*)tqbs->chr,ios::out);
if (ofh.is_open()==NULL){ofh.clear(); error(64); return;}//Bad file name
ofh.close();
}
device_handle[i].fh.open((const char*)tqbs->chr,ios::binary|ios::out|ios::in);
if (device_handle[i].fh.is_open()==NULL){device_handle[i].fh.clear(); error(64); return;}//Bad file name
device_handle[i].open=1;
device_handle[i].pos=0;
device_handle[i].eof_reached=0;
device_handle[i].type=2;
return;
}

if (type==5){//APPEND
qbs_set(tqbs,qbs_add(name,nullt));//prepare null-terminated filename
device_handle[i].read_access=0;
device_handle[i].write_access=1;
if (access&&(access!=3)) error(54);//Bad file mode (must have write not read access)
ifh.open((const char*)tqbs->chr,ios::in);
if (ifh.is_open()){
ifh.close();
}else{
ifh.clear();
//create the file
ofh.open((const char*)tqbs->chr,ios::out);
if (ofh.is_open()==NULL){ofh.clear(); error(64); return;}//cannot create file!
ofh.close();
}
device_handle[i].fh.open((const char*)tqbs->chr,ios::binary|ios::out|ios::in);
if (device_handle[i].fh.is_open()==NULL){device_handle[i].fh.clear(); error(64); return;}//cannot open file!
device_handle[i].fh.seekp(0,ios::end);
device_handle[i].open=1;
device_handle[i].pos=device_handle[i].fh.tellp();
device_handle[i].eof_reached=0;
device_handle[i].type=4;//OUTPUT type
device_handle[i].column=0;
return;
}

if (type==4){//OUTPUT
qbs_set(tqbs,qbs_add(name,nullt));//prepare null-terminated filename
device_handle[i].read_access=0;
device_handle[i].write_access=1;
if (access&&(access!=3)) error(54);//Bad file mode (must have write not read access)
device_handle[i].fh.open((const char*)tqbs->chr,ios::binary|ios::out);
if (device_handle[i].fh.is_open()==NULL){device_handle[i].fh.clear(); error(64); return;}//cannot open file!
device_handle[i].open=1;
device_handle[i].pos=0;
device_handle[i].eof_reached=0;
device_handle[i].type=4;
device_handle[i].column=0;
return;
}

if (type==3){//INPUT
qbs_set(tqbs,qbs_add(name,nullt));//prepare null-terminated filename
device_handle[i].read_access=1;
device_handle[i].write_access=0;
if (access&&(access!=2)) error(54);//Bad file mode (must have read not write access)
device_handle[i].fh.open((const char*)tqbs->chr,ios::binary|ios::in);
if (device_handle[i].fh.is_open()==NULL){device_handle[i].fh.clear(); error(64); return;}//cannot open file!
device_handle[i].open=1;
device_handle[i].pos=0;
device_handle[i].eof_reached=0;
device_handle[i].type=3;
return;
}


exit(89);//unknown command
}

void sub_close(long i2,long passed){
if (new_error) return;

static long i;
if (passed){
 i=i2;
 if (device_handle[i].open){
 if (device_handle[i].fh.is_open()) device_handle[i].fh.close();
 device_handle[i].open=0;
 }
}else{
 for (i=1;i<=last_device_handle;i++){
  if (device_handle[i].open){
  if (device_handle[i].fh.is_open()) device_handle[i].fh.close();
  device_handle[i].open=0;
  }
 }
}
}//close





long file_input_chr(long i){
//returns -1 if eof reached
static unsigned char c;
if (device_handle[i].eof_reached){
device_handle[i].fh.seekg(device_handle[i].pos,ios::beg);
if (!device_handle[i].fh.eof()) device_handle[i].eof_reached=0; else {device_handle[i].fh.clear(); return -1;}
}
device_handle[i].fh.read((char*)&c,1);
if (!device_handle[i].fh.gcount()){
device_handle[i].fh.clear();
device_handle[i].eof_reached=1;
return -1;
}else{
//CHR$(26)?
if (c==26){
device_handle[i].fh.seekg(device_handle[i].pos,ios::beg);//go back 1 byte
return -1;
}
device_handle[i].pos++;
}
return c;
}

void file_input_nextitem(long i,long lastc){
//this may require reversing a bit too!
long c,nextc;
c=lastc;
nextchr:
if (c==-1) return;
if (c==32){
nextc=file_input_chr(i);
if ( (nextc!=32)&&(nextc!=44)&&(nextc!=10)&&(nextc!=13) ){
device_handle[i].pos--; device_handle[i].fh.seekg(device_handle[i].pos,ios::beg); device_handle[i].eof_reached=0;
return;
}else{
c=nextc;
goto nextchr;
}
}
if (c==44) return;//,
if (c==10){//lf 
nextc=file_input_chr(i);
if (nextc==13) return;
//backtrack
device_handle[i].pos--; device_handle[i].fh.seekg(device_handle[i].pos,ios::beg); device_handle[i].eof_reached=0;
return;
}
if (c==13){//lf 
nextc=file_input_chr(i);
if (nextc==10) return;
//backtrack
device_handle[i].pos--; device_handle[i].fh.seekg(device_handle[i].pos,ios::beg); device_handle[i].eof_reached=0;
return;
}
c=file_input_chr(i);
goto nextchr;
}


char sub_file_print_spaces[32];
void sub_file_print(long i,qbs *str,long extraspace,long tab,long newline){
if (new_error) return;
//handle to open file?
if (i<1){error(52); return;}//Bad file name or number
if (i>last_device_handle){error(52); return;}//Bad file name or number
if (!device_handle[i].open){error(52); return;}//Bad file name or number
//open for output?
if (device_handle[i].type!=4){error(54); return;}//Bad file mode

//assume pos is not past eof!
device_handle[i].fh.write((char*)str->chr,str->len);
device_handle[i].column+=str->len;

//add extra spaces as needed
static long nspaces,x;
static short cr_lf=13+10*256; 
nspaces=0;
if (extraspace){
nspaces++;
device_handle[i].column++;
}
if (tab){
//a space MUST be added
nspaces++;
device_handle[i].column++;
x=device_handle[i].column%14;
if (x!=0){
x=14-x;
nspaces+=x;
device_handle[i].column+=x;
}
}
if (nspaces){
device_handle[i].fh.write(&sub_file_print_spaces[0],nspaces);
}
if (newline){
device_handle[i].fh.write((char*)&cr_lf,2);
device_handle[i].column=0;
}
}

//number limits
const char *range_int64_max[]={"9223372036854775807"};//19 digits
const char *range_int64_neg_max[]={"9223372036854775808"};//19 digits
const char *range_uint64_max[]={"18446744073709551615"};//20 digits
const char *range_float_max[]=    {"17976931348623157"};//17 digits
                          
//universal number representation
unsigned short n_digits;
unsigned char n_digit[256];
int64 n_exp;//if 0, there is one digit in front of the decimal place
unsigned char n_neg;//if 1, the number is negative
unsigned char n_hex;//if 1, the digits are in hexidecimal and n_exp should be ignored
                    //if 2, the digits are in octal and n_exp should be ignored
                    //(consider revising variable name n_hex)

long n_roundincrement(){
static long i,i2,i3;
if (n_digits==0) return 0;
if (n_digits>(n_exp+1)){//numbers exist after the decimal point
i=n_digit[n_exp+1]-48;
if (i>=5) return 1;
}
return 0;
}

long double n_float_value;
long n_float(){
//return value: Bit 0=successful
//data
static unsigned char built[32];
static int64 value;
uint64 uvalue;
static long i,i2,i3;
static unsigned char *max;
max=(unsigned char*)range_float_max[0];
n_float_value=0; value=0; uvalue=0;
if (n_digits==0) return 1;
//hex?
if (n_hex==1){
if (n_digits>16) return 0;
for (i=0;i<n_digits;i++){
 i2=n_digit[i];
 if ((i2>=48)&&(i2<=57)) i2-=48;
 if ((i2>=65)&&(i2<=70)) i2-=55;
 if ((i2>=97)&&(i2<=102)) i2-=87;
 value<<=4;
 value|=i2;
}
n_float_value=value;
return 1;
}
//oct?
if (n_hex==2){
if (n_digits>=22){
if ((n_digits>22)||(n_digit[0]>49)) return 0;
}
for (i=0;i<n_digits;i++){
 i2=n_digit[i]-48;
 value<<=3;
 value|=i2;
}
n_float_value=value;
return 1;
}

//max range check (+-1.7976931348623157E308)
if (n_exp>308)return 0;//overflow
if (n_exp==308){
 i2=n_digits; if (i2>17) i2=17;
 for (i=0;i<i2;i++){
  if (n_digit[i]>max[i]) return 0;//overflow
  if (n_digit[i]<max[i]) break;
 }
}
//too close to 0?
if (n_exp<-324) return 1;
//read & return value (via C++ function)
//build number
i=0;
if (n_neg){built[i]=45; i++;}//-
built[i]=n_digit[0]; i++;
built[i]=46; i++;//.
if (n_digits==1){
built[i]=48; i++;//0
}else{
 i3=n_digits; if (i3>17) i3=17;
 for (i2=1;i2<i3;i2++){
 built[i]=n_digit[i2]; i++;
 }
}
built[i]=69; i++;//E
#ifdef QB64_WINDOWS
i2=sprintf((char*)&built[i],"%I64i",n_exp);
#else
i2=sprintf((char*)&built[i],"%ll",n_exp);
#endif
i=i+i2;

static double sscanf_fix;
sscanf((char*)&built[0],"%lf",&sscanf_fix);
n_float_value=sscanf_fix;

return 1;
}

int64 n_int64_value;
long n_int64(){
//return value: Bit 0=successful
//data
static int64 value;
uint64 uvalue;
static long i,i2;
static unsigned char *max; static unsigned char *neg_max;
static int64 v0=build_int64(0x80000000,0x00000000);
static int64 v1=build_int64(0x7FFFFFFF,0xFFFFFFFF);
max=(unsigned char*)range_int64_max[0]; neg_max=(unsigned char*)range_int64_neg_max[0];
n_int64_value=0; value=0; uvalue=0;
if (n_digits==0) return 1;
//hex
if (n_hex==1){
if (n_digits>16) return 0;
for (i=0;i<n_digits;i++){
 i2=n_digit[i];
 if ((i2>=48)&&(i2<=57)) i2-=48;
 if ((i2>=65)&&(i2<=70)) i2-=55;
 if ((i2>=97)&&(i2<=102)) i2-=87;
 value<<=4;
 value|=i2;
}
n_int64_value=value;
return 1;
}
//oct
if (n_hex==2){
if (n_digits>=22){
if ((n_digits>22)||(n_digit[0]>49)) return 0;
}
for (i=0;i<n_digits;i++){
 i2=n_digit[i]-48;
 value<<=3;
 value|=i2;
}
n_int64_value=value;
return 1;
}

//range check: int64 (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807)
if (n_exp>18)return 0;//overflow
if (n_exp==18){
i2=n_digits; if (i2>19) i2=19;//only scan integeral digits
 for (i=0;i<i2;i++){
  if (n_neg){
  if (n_digit[i]>neg_max[i]) return 0;//overflow
  if (n_digit[i]<neg_max[i]) break;
  }else{
  if (n_digit[i]>max[i]) return 0;//overflow
  if (n_digit[i]<max[i]) break;
  }
 }
}
//calculate integeral value
i2=n_digits; if (i2>(n_exp+1)) i2=n_exp+1;
for (i=0;i<(n_exp+1);i++){
uvalue*=10;
if (i<i2) uvalue=uvalue+(n_digit[i]-48);
}
if (n_neg){
value=-uvalue;
}else{
value=uvalue;
}
//apply rounding
if (n_roundincrement()){
if (n_neg){
if (value==v0) return 0;
value--;
}else{
if (value==v1) return 0;
value++;
}
}
//return value
n_int64_value=value;
return 1;
}

uint64 n_uint64_value;
long n_uint64(){
//return value: Bit 0=successful
//data
static int64 value;
uint64 uvalue;
static long i,i2;
static unsigned char *max;
static int64 v0=build_uint64(0xFFFFFFFF,0xFFFFFFFF);
max=(unsigned char*)range_uint64_max[0];
n_uint64_value=0; value=0; uvalue=0;
if (n_digits==0) return 1;
//hex
if (n_hex==1){
if (n_digits>16) return 0;
for (i=0;i<n_digits;i++){
 i2=n_digit[i];
 if ((i2>=48)&&(i2<=57)) i2-=48;
 if ((i2>=65)&&(i2<=70)) i2-=55;
 if ((i2>=97)&&(i2<=102)) i2-=87;
 uvalue<<=4;
 uvalue|=i2;
}
n_uint64_value=uvalue;
return 1;
}
//oct
if (n_hex==2){
if (n_digits>=22){
if ((n_digits>22)||(n_digit[0]>49)) return 0;
}
for (i=0;i<n_digits;i++){
 i2=n_digit[i]-48;
 uvalue<<=3;
 uvalue|=i2;
}
n_uint64_value=uvalue;
return 1;
}

//negative?
if (n_neg){
if (n_exp>=0) return 0;//cannot return a negative number!
}
//range check: int64 (0 to 18446744073709551615)
if (n_exp>19)return 0;//overflow
if (n_exp==19){
i2=n_digits; if (i2>20) i2=20;//only scan integeral digits
 for (i=0;i<i2;i++){ 
 if (n_digit[i]>max[i]) return 0;//overflow
 if (n_digit[i]<max[i]) break; 
 }
}
//calculate integeral value
i2=n_digits; if (i2>(n_exp+1)) i2=n_exp+1;
for (i=0;i<(n_exp+1);i++){
uvalue*=10;
if (i<i2) uvalue=uvalue+(n_digit[i]-48);
}
//apply rounding
if (n_roundincrement()){
if (n_neg){
return 0;
}else{
if (uvalue==v0) return 0;
uvalue++;
}
}
//return value
n_uint64_value=uvalue;
return 1;
}

long n_inputnumberfromdata(unsigned char *data,unsigned long *data_offset,unsigned long data_size){
//return values:
//0=success!
//1=Overflow
//2=Out of DATA
//3=Syntax error
//note: when read fails the data_offset MUST be restored to its old position

//data
static long i,i2;
static long step,c;
static long exponent_digits;
static unsigned char negate_exponent;
static int64 exponent_value;
static long return_value;

return_value=1;//overflow (default)
step=0;
negate_exponent=0;
exponent_value=0;
exponent_digits=0;

//prepare universal number representation
n_digits=0; n_exp=0; n_neg=0; n_hex=0;

//Out of DATA?
if (*data_offset>=data_size) return 2;

//read character
c=data[*data_offset];

//hex/oct
if (c==38){//&
(*data_offset)++; if (*data_offset>=data_size) goto gotnumber;
c=data[*data_offset];
if (c==44){(*data_offset)++; goto gotnumber;}
if ((c==72)||(c==104)){//"H"or"h"
 nexthexchr:
 (*data_offset)++; if (*data_offset>=data_size) goto gotnumber;
 c=data[*data_offset];
 if (c==44){(*data_offset)++; goto gotnumber;}
 if ( ((c>=48)&&(c<=57)) || ((c>=65)&&(c<=70)) || ((c>=97)&&(c<=102)) ){//0-9 or A-F or a-f
 if (n_digits==256) return 1;//Overflow
 n_digit[n_digits]=c;
 n_digits++;
 n_hex=1;
 goto nexthexchr;
 }
 return 3;//Syntax error
}
if ((c==79)||(c==111)){//"O"or"o"
 nexthexchr2:
 (*data_offset)++; if (*data_offset>=data_size) goto gotnumber;
 c=data[*data_offset];
 if (c==44){(*data_offset)++; goto gotnumber;}
 if ((c>=48)&&(c<=55)){//0-7
 if (n_digits==256) return 1;//Overflow
 n_digit[n_digits]=c;
 n_digits++;
 n_hex=2;
 goto nexthexchr2;
 }
 return 3;//Syntax error
}
return 3;//Syntax error
}//&

readnextchr:
if (c==44){(*data_offset)++; goto gotnumber;}

if (c==45){//-
if (step==0){n_neg=1; step=1; goto nextchr;}
if (step==3){negate_exponent=1; step=4; goto nextchr;}
return 3;//Syntax error
}

if (c==43){//+
if (step==0){step=1; goto nextchr;}
if (step==3){step=4; goto nextchr;}
return 3;//Syntax error
}

if ((c>=48)&&(c<=57)){//0-9

if (step<=1){//before decimal point
step=1;
if (n_digits||(c>48)){
if (n_digits) n_exp++;
if (n_digits==256) return 1;//Overflow
n_digit[n_digits]=c;
n_digits++;
}
}

if (step==2){//after decimal point
if ((n_digits==0)&&(c==48)) n_exp--;
if ((n_digits)||(c>48)){
if (n_digits==256) return 1;//Overflow
n_digit[n_digits]=c;
n_digits++;
}
}

if (step>=3){//exponent
step=4;
if ((exponent_digits)||(c>48)){
if (exponent_digits==18) return 1;//Overflow
exponent_value*=10;
exponent_value=exponent_value+(c-48);
exponent_digits++;
}
}

goto nextchr;
}

if (c==46){//.
if (step>1) return 3;//Syntax error
if (n_digits==0) n_exp=-1;
step=2; goto nextchr;
}

if ((c==68)||(c==69)||(c==100)||(c==101)){//D,E,d,e
if (step>2) return 3;//Syntax error
step=3; goto nextchr;
}

return 3;//Syntax error
nextchr:
(*data_offset)++; if (*data_offset>=data_size) goto gotnumber;
c=data[*data_offset];
goto readnextchr;

gotnumber:;
if (negate_exponent) n_exp-=exponent_value; else n_exp+=exponent_value;//complete exponent
if (n_digits==0) {n_exp=0; n_neg=0;}//clarify number
return 0;//success
}




















long n_inputnumberfromfile(long fileno){
//return values:
//0=success
//1=overflow
//2=eof

//data
static long i,i2;
static long step,c;
static long exponent_digits;
static unsigned char negate_exponent;
static int64 exponent_value;
static long return_value;

return_value=1;//overflow (default)
step=0;
negate_exponent=0;
exponent_value=0;
exponent_digits=0;

//prepare universal number representation
n_digits=0; n_exp=0; n_neg=0; n_hex=0;

//skip any leading spaces
do{
c=file_input_chr(fileno);
if (c==-1){return_value=2; goto error;}//input past end of file
}while(c==32);

//hex/oct
if (c==38){//&
c=file_input_chr(fileno);
if (c==-1) goto gotnumber;
if ((c==72)||(c==104)){//"H"or"h"
 nexthexchr:
 c=file_input_chr(fileno);
 if ( ((c>=48)&&(c<=57)) || ((c>=65)&&(c<=70)) || ((c>=97)&&(c<=102)) ){//0-9 or A-F or a-f
 if (n_digits==256) goto error;//overflow
 n_digit[n_digits]=c;
 n_digits++;
 n_hex=1;
 goto nexthexchr;
 }
 goto gotnumber;
}
if ((c==79)||(c==111)){//"O"or"o"
 nexthexchr2:
 c=file_input_chr(fileno);
 if ((c>=48)&&(c<=55)){//0-7
 if (n_digits==256) goto error;//overflow
 n_digit[n_digits]=c;
 n_digits++;
 n_hex=2;
 goto nexthexchr2;
 }
 goto gotnumber;
}
goto gotnumber;
}//&

readnextchr:
if (c==-1) goto gotnumber;

if (c==45){//-
if (step==0){n_neg=1; step=1; goto nextchr;}
if (step==3){negate_exponent=1; step=4; goto nextchr;}
goto gotnumber;
}

if (c==43){//+
if (step==0){step=1; goto nextchr;}
if (step==3){step=4; goto nextchr;}
goto gotnumber;
}

if ((c>=48)&&(c<=57)){//0-9

if (step<=1){//before decimal point
step=1;
if (n_digits||(c>48)){
if (n_digits) n_exp++;
if (n_digits==256) goto error;//overflow
n_digit[n_digits]=c;
n_digits++;
}
}

if (step==2){//after decimal point
if ((n_digits==0)&&(c==48)) n_exp--;
if ((n_digits)||(c>48)){
if (n_digits==256) goto error;//overflow
n_digit[n_digits]=c;
n_digits++;
}
}

if (step>=3){//exponent
step=4;
if ((exponent_digits)||(c>48)){
if (exponent_digits==18) goto error;//overflow
exponent_value*=10;
exponent_value=exponent_value+(c-48);
exponent_digits++;
}
}

goto nextchr;
}

if (c==46){//.
if (step>1) goto gotnumber;
if (n_digits==0) n_exp=-1;
step=2; goto nextchr;
}

if ((c==68)||(c==69)||(c==100)||(c==101)){//D,E,d,e
if (step>2) goto gotnumber;
step=3; goto nextchr;
}

goto gotnumber;//invalid character
nextchr:
c=file_input_chr(fileno);
goto readnextchr;

gotnumber:;
if (negate_exponent) n_exp-=exponent_value; else n_exp+=exponent_value;//complete exponent
if (n_digits==0) {n_exp=0; n_neg=0;}//clarify number
file_input_nextitem(fileno,c);
return 0;//success

error:
file_input_nextitem(fileno,c);
return return_value;
}

void sub_file_line_input_string(long fileno,qbs *deststr){
if (new_error) return;
static qbs *str,*character;
long c,nextc;
long inspeechmarks;
str=qbs_new(0,0);
c=file_input_chr(fileno);
if (c==-1){
qbs_set(deststr,str);
qbs_free(str);
error(62);//input past end of file
return;
}
character=qbs_new(1,0);
nextchr:
if (c==-1) goto gotstr;
if (c==10) goto gotstr;
if (c==13) goto gotstr;
character->chr[0]=c; qbs_set(str,qbs_add(str,character));
c=file_input_chr(fileno);
goto nextchr;
gotstr:
nextstr:
//scan until next item (or eof) reached
if (c==-1) goto returnstr;
if (c==10){//lf 
nextc=file_input_chr(fileno);
if (nextc==13) goto returnstr;
//backtrack
device_handle[fileno].pos--; device_handle[fileno].fh.seekg(device_handle[fileno].pos,ios::beg); device_handle[fileno].eof_reached=0;
goto returnstr;
}
if (c==13){//cr
nextc=file_input_chr(fileno);
if (nextc==10) goto returnstr;
//backtrack
device_handle[fileno].pos--; device_handle[fileno].fh.seekg(device_handle[fileno].pos,ios::beg); device_handle[fileno].eof_reached=0;
goto returnstr;
}
c=file_input_chr(fileno);
goto nextstr;
//return string
returnstr:
qbs_set(deststr,str);
qbs_free(str);
qbs_free(character);
return;
}

void sub_file_input_string(long fileno,qbs *deststr){
if (new_error) return;
static qbs *str,*character;
long c,nextc;
long inspeechmarks;
str=qbs_new(0,0);
//skip whitespace (spaces & tabs)
do{
c=file_input_chr(fileno);
if (c==-1){
qbs_set(deststr,str);
qbs_free(str);
error(62);//input past end of file
return;
}
}while((c==32)||(c==9));
//quoted string?
inspeechmarks=0;
if (c==34){//"
inspeechmarks=1;
c=file_input_chr(fileno);
}
//read string
character=qbs_new(1,0);
nextchr:
if (c==-1) goto gotstr;
if (inspeechmarks){
if (c==34) goto gotstr;//"
}else{
if (c==44) goto gotstr;//,
if (c==10) goto gotstr;
if (c==13) goto gotstr;
}
character->chr[0]=c; qbs_set(str,qbs_add(str,character));
c=file_input_chr(fileno);
goto nextchr;
gotstr:
//cull trailing whitespace
if (!inspeechmarks){
cullstr:
if (str->len){
if ((str->chr[str->len-1]==32)||(str->chr[str->len-1]==9)){str->len--; goto cullstr;}
}
}
nextstr:
//scan until next item (or eof) reached
if (c==-1) goto returnstr;
if (c==44) goto returnstr;
if (c==10){//lf 
nextc=file_input_chr(fileno);
if (nextc==13) goto returnstr;
//backtrack
device_handle[fileno].pos--; device_handle[fileno].fh.seekg(device_handle[fileno].pos,ios::beg); device_handle[fileno].eof_reached=0;
goto returnstr;
}
if (c==13){//cr
nextc=file_input_chr(fileno);
if (nextc==10) goto returnstr;
//backtrack
device_handle[fileno].pos--; device_handle[fileno].fh.seekg(device_handle[fileno].pos,ios::beg); device_handle[fileno].eof_reached=0;
goto returnstr;
}
c=file_input_chr(fileno);
goto nextstr;
//return string
returnstr:
qbs_set(deststr,str);
qbs_free(str);
qbs_free(character);
return;
}

int64 func_file_input_int64(long fileno){
if (new_error) return 0;
static long i;
i=n_inputnumberfromfile(fileno);
if (i==1){error(6); return 0;}//overflow
if (i==2){error(62); return 0;}//input past end of file
if (n_int64()) return n_int64_value;
error(6);//overflow
return 0;
}

uint64 func_file_input_uint64(long fileno){
if (new_error) return 0;
static long i;
i=n_inputnumberfromfile(fileno);
if (i==1){error(6); return 0;}//overflow
if (i==2){error(62); return 0;}//input past end of file
if (n_uint64()) return n_uint64_value;
error(6);//overflow
return 0;
}




void sub_read_string(unsigned char *data,unsigned long *data_offset,unsigned long data_size,qbs *deststr){
if (new_error) return;
static qbs *str,*character;
static long c,inspeechmarks;
str=qbs_new(0,0);
character=qbs_new(1,0);
inspeechmarks=0;

if (*data_offset>=data_size){error(4); goto gotstr;}//Out of DATA

c=data[*data_offset];
nextchr:

if (c==44){//,
if (!inspeechmarks){
(*data_offset)++;
goto gotstr;
}
}

if (c==34){//"
if (!str->len) {inspeechmarks=1; goto skipchr;}
if (inspeechmarks) {inspeechmarks=0; goto skipchr;}
}

character->chr[0]=c; qbs_set(str,qbs_add(str,character));
skipchr:

(*data_offset)++; if (*data_offset>=data_size) goto gotstr;
c=data[*data_offset];
goto nextchr;

gotstr:
qbs_set(deststr,str);
qbs_free(str);
qbs_free(character);
return;
}

long double func_read_float(unsigned char *data,unsigned long *data_offset,unsigned long data_size,long typ){
if (new_error) return 0;
static long i;
static int64 maxval,minval;
static int64 value;
static unsigned long old_data_offset;
old_data_offset=*data_offset;
i=n_inputnumberfromdata(data,data_offset,data_size);


//return values:
//0=success!
//1=Overflow
//2=Out of DATA
//3=Syntax error
//note: when read fails the data_offset MUST be restored to its old position
if (i==1){//Overflow
goto overflow;
}
if (i==2){//Out of DATA
error(4);
return 0;
}
if (i==3){//Syntax error
*data_offset=old_data_offset;
error(2); 
return 0;
}

if (!n_float()) goto overflow; //returns n_float_value



//range test & return value
if (typ&ISFLOAT){


 if ((typ&511)>=64) return n_float_value;
 if (n_float_value>3.402823466E+38) goto overflow;
return n_float_value;
}else{




 if (n_float_value<(-(9.2233720368547758E+18)))goto overflow;//too low to store!
 if (n_float_value>   9.2233720368547758E+18)  goto overflow;//too high to store!
 value=qbr(n_float_value);
 if (typ&ISUNSIGNED){
 maxval=(1<<(typ&511))-1;
 minval=0;
 }else{
 maxval=(1<<((typ&511)-1))-1;
 minval=-(1<<((typ&511)-1));
 }
 if ((value>maxval)||(value<minval)) goto overflow;

//value=458758;


 return value;
}

overflow:
*data_offset=old_data_offset;
error(6); 
return 0;
}//func_file_input_float




































long double func_file_input_float(long fileno,long typ){
if (new_error) return 0;
static long i;
static int64 maxval,minval;
static int64 value;
i=n_inputnumberfromfile(fileno);
if (i==1){error(6); return 0;}//overflow
if (i==2){error(62); return 0;}//input past end of file
if (!n_float()){error(6); return 0;} //returns n_float_value
//range test & return value
if (typ&ISFLOAT){
 if ((typ&511)>=64) return n_float_value;
 if (n_float_value>3.402823466E+38){error(6); return 0;}
 return n_float_value;
}else{
 if (n_float_value<(-(9.2233720368547758E+18))){error(6); return 0;}//too low to store!
 if (n_float_value>   9.2233720368547758E+18)  {error(6); return 0;}//too high to store!
 value=qbr(n_float_value);
 if (typ&ISUNSIGNED){
 maxval=(1<<(typ&511))-1;
 minval=0;
 }else{
 maxval=(1<<((typ&511)-1))-1;
 minval=-(1<<((typ&511)-1));
 }
 if ((value>maxval)||(value<minval)){error(6); return 0;}
 return value;
}
}//func_file_input_float







//revise following!
void *byte_element(uint64 offset,unsigned long length){
//add structure to xms stack (byte_element structures are never stored in cmem!)
void *p;
if ((mem_static_pointer+=12)<mem_static_limit) p=mem_static_pointer-12; else p=mem_static_malloc(12);
*((uint64*)p)=offset;
((unsigned long*)p)[2]=length;
return p;
}

void sub_get(long i,long offset,void *element,long passed){
if (new_error) return;
static byte_element_struct *ele;
static long x;
if ((i<1)||(i>last_device_handle)){error(52); return;}
if (!device_handle[i].open){error(52); return;}
if (device_handle[i].type>2){error(54); return;}
if (!device_handle[i].read_access){error(75); return;}

ele=(byte_element_struct*)element;

if (device_handle[i].type==1){
if (ele->length>device_handle[i].record_length){error(59); return;}//Bad record length
if (passed){
offset--;
if (offset<0){error(63); return;}//Bad record number
offset*=device_handle[i].record_length;
offset++;
}
}

if (passed){
 offset--;
 if (offset<0){error(63); return;} 
 if (offset==device_handle[i].pos){
  if (device_handle[i].eof_reached){
   device_handle[i].fh.seekg(offset,ios::beg);
   if (device_handle[i].fh.eof()) device_handle[i].fh.clear(); else device_handle[i].eof_reached=0;   
  }  
 }else{
  device_handle[i].fh.seekg(offset,ios::beg);
  if (device_handle[i].fh.eof()){device_handle[i].eof_reached=1; device_handle[i].fh.clear();} else device_handle[i].eof_reached=0;  
  device_handle[i].pos=offset;
 } 
}else{
 if (device_handle[i].eof_reached){
  device_handle[i].fh.seekg(device_handle[i].pos,ios::beg);
  if (device_handle[i].fh.eof()) device_handle[i].fh.clear(); else device_handle[i].eof_reached=0;  
 }
}

device_handle[i].pos+=ele->length;

if (ele->length){
if (device_handle[i].eof_reached){
 memset((void*)(ele->offset),0,ele->length);
}else{
 device_handle[i].fh.read((char*)(ele->offset),ele->length);
 x=device_handle[i].fh.gcount();
 if (x<ele->length){
 device_handle[i].fh.clear();
 device_handle[i].eof_reached=1;
 memset((void*)(ele->offset+x),0,ele->length-x); 
 }
}
}
if (device_handle[i].type==2) return;//BINARY

//RANDOM
if (ele->length<device_handle[i].record_length){
x=device_handle[i].record_length-ele->length;
device_handle[i].fh.seekg(device_handle[i].pos+x,ios::beg);
if (device_handle[i].fh.eof()){
device_handle[i].fh.clear(); device_handle[i].eof_reached=1;
device_handle[i].fh.seekg(0,ios::end);
}
device_handle[i].pos+=x;
}

}//end (get)

void sub_get2(long i,long offset,qbs *str,long passed){
if (new_error) return;
static long x;
if ((i<1)||(i>last_device_handle)){error(52); return;}
if (!device_handle[i].open){error(52); return;}
if (device_handle[i].type>2){error(54); return;}
//BINARY
if (device_handle[i].type==2){
if (!device_handle[i].read_access){error(75); return;}
sub_get(i,offset,byte_element((uint64)str->chr,str->len),passed);
return;
}
//RANDOM (copied and modified from sub_get)
if (!device_handle[i].read_access){str->len=0; error(75); return;}
if (device_handle[i].record_length<2){error(59); return;}//Bad record length

if (passed){
 offset--;
 if (offset<0){str->len=0; error(63); return;}//Bad record number
 offset*=device_handle[i].record_length;
 if (offset==device_handle[i].pos){
  if (device_handle[i].eof_reached){
   device_handle[i].fh.seekg(offset,ios::beg);
   if (device_handle[i].fh.eof()) device_handle[i].fh.clear(); else device_handle[i].eof_reached=0;
  }
 }else{
  device_handle[i].fh.seekg(offset,ios::beg);
  if (device_handle[i].fh.eof()) device_handle[i].eof_reached=1;
  device_handle[i].fh.clear();
  device_handle[i].pos=offset;
 }
}else{
 if (device_handle[i].eof_reached){
  device_handle[i].fh.seekg(device_handle[i].pos,ios::beg);
  if (device_handle[i].fh.eof()) device_handle[i].fh.clear(); else device_handle[i].eof_reached=0;
 }
}

//read the entire record
static char *data;
static long l,x2;
data=(char*)malloc(device_handle[i].record_length);
device_handle[i].pos+=device_handle[i].record_length;
device_handle[i].fh.read(data,device_handle[i].record_length);
x=device_handle[i].fh.gcount();
if (x<device_handle[i].record_length){
device_handle[i].fh.clear();
device_handle[i].eof_reached=1;
memset((void*)(data+x),0,device_handle[i].record_length-x);
}

x2=2;//offset where actual string data will be read from
l=((unsigned short*)data)[0];
if (l&32768){
 if (device_handle[i].record_length<4){
 //record length is too small to read the length data
 //move read position back to previous location
 device_handle[i].pos-=device_handle[i].record_length;
 device_handle[i].fh.seekg(device_handle[i].pos,ios::beg);
 if (device_handle[i].fh.eof()){device_handle[i].fh.clear(); device_handle[i].eof_reached=1;}
  free(data);
 str->len=0; error(59); return;//Bad record length
 }
x2=4;
l=((((unsigned short*)data)[1])<<15)+l-32768;
}
 if ((device_handle[i].record_length-x2)<l){
 //record_length cannot fit the string's data
 //move read position back to previous location
 device_handle[i].pos-=device_handle[i].record_length;
 device_handle[i].fh.seekg(device_handle[i].pos,ios::beg);
 if (device_handle[i].fh.eof()){device_handle[i].fh.clear(); device_handle[i].eof_reached=1;}
 free(data);
 str->len=0; error(59); return;//Bad record length
 }
//success
qbs_set(str,qbs_new_txt_len(data+x2,l));
free(data);
}

void sub_put(long i,long offset,void *element,long passed){
if (new_error) return;
static byte_element_struct *ele;
static char *nullchrs;
static long x;
if ((i<1)||(i>last_device_handle)){error(52); return;}
if (!device_handle[i].open){error(52); return;}
if (device_handle[i].type>2){error(54); return;}
if (!device_handle[i].write_access){error(75); return;}

ele=(byte_element_struct*)element;

if (device_handle[i].type==1){
if (ele->length>device_handle[i].record_length){error(59); return;}//Bad record length
if (passed){
offset--;
if (offset<0){error(63); return;}//Bad record number
offset*=device_handle[i].record_length;
offset++;
}
}

if (passed){
 offset--;
 if (offset<0){error(63); return;} 
 if (offset==device_handle[i].pos){
  if (device_handle[i].eof_reached){
  device_handle[i].fh.seekp(offset,ios::beg);
  if (device_handle[i].fh.eof()){
  device_handle[i].fh.clear();
  //extend file
  device_handle[i].fh.seekp(0,ios::end);
  x=device_handle[i].fh.tellp();
  x=offset-x;
  nullchrs=(char*)calloc(x,1);
  device_handle[i].fh.write(nullchrs,x);
  free(nullchrs);
  device_handle[i].eof_reached=0;
  }
  }
 }else{
  //need to set new offset
  device_handle[i].fh.seekp(offset,ios::beg);
  if (device_handle[i].fh.eof()){
   device_handle[i].fh.clear();
  //extend file
  device_handle[i].fh.seekp(0,ios::end);
  x=device_handle[i].fh.tellp();
  x=offset-x;
  nullchrs=(char*)calloc(x,1);
  device_handle[i].fh.write(nullchrs,x);
  free(nullchrs);
  device_handle[i].eof_reached=0;
  } 
  device_handle[i].pos=offset;
 }
}else{
 if (device_handle[i].eof_reached){
 device_handle[i].fh.seekp(device_handle[i].pos,ios::beg);
 if (device_handle[i].fh.eof()){
  device_handle[i].fh.clear();
 //extend file
 device_handle[i].fh.seekp(0,ios::end);
 x=device_handle[i].fh.tellp();
 x=device_handle[i].pos-x;
 nullchrs=(char*)calloc(x,1);
 device_handle[i].fh.write(nullchrs,x);
 free(nullchrs);
 device_handle[i].eof_reached=0;
 }
 }
}

if (ele->length){
device_handle[i].fh.write((char*)(ele->offset),ele->length);
}
device_handle[i].pos+=ele->length;

if (device_handle[i].type==1){
if (ele->length<device_handle[i].record_length){
x=device_handle[i].record_length-ele->length;
device_handle[i].fh.seekp(device_handle[i].pos+x,ios::beg);
if (device_handle[i].fh.eof()){
device_handle[i].fh.clear();
device_handle[i].eof_reached=1;
}
if (device_handle[i].eof_reached){
device_handle[i].fh.seekp(device_handle[i].pos,ios::beg);
}
device_handle[i].pos+=x;
}
}

}

//put2 adds a 2-4 byte length descriptor to the data
//(used to PUT variable length strings in RANDOM mode)
void sub_put2(long i,long offset,void *element,long passed){
if (new_error) return;
static byte_element_struct *ele;
static long l,x;
static unsigned char *data;
if ((i<1)||(i>last_device_handle)){error(52); return;}
if (!device_handle[i].open){error(52); return;}
if (device_handle[i].type>2){error(54); return;}
if (!device_handle[i].write_access){error(75); return;}
if (device_handle[i].type==1){
ele=(byte_element_struct*)element;
l=ele->length;
//[15][1][[16]]
if (l>32767){
data=(unsigned char*)malloc(l+4);
memcpy(&data[4],(char*)(ele->offset),l);
((unsigned short*)data)[0]=(l&32767)+32768;
x=l; x>>=15;
((unsigned short*)data)[1]=x;
ele->length+=4;
}else{
data=(unsigned char*)malloc(l+2);
memcpy(&data[2],(char*)(ele->offset),l);
((unsigned short*)data)[0]=l;
ele->length+=2;
}
ele->offset=(uint64)&data[0];
sub_put(i,offset,element,passed);
free(data);
}else{
sub_put(i,offset,element,passed);
}
}






void sub_graphics_get(long step1,float x1f,float y1f,long step2,float x2f,float y2f,void *element,unsigned long mask,long passed){
if (new_error) return;

if (read_page->bytes_per_pixel!=1){error(5); return;}

static long x1,y1,x2,y2;

//change coordinates according to step
if (step1){x1f=read_page->x+x1f; y1f=read_page->y+y1f;}
read_page->x=x1f; read_page->y=y1f;
if (step2){x2f=read_page->x+x2f; y2f=read_page->y+y2f;}
read_page->x=x2f; read_page->y=y2f;

//resolve coordinates
if (read_page->clipping_or_scaling){
if (read_page->clipping_or_scaling==2){
x1=qbr_float_to_long(x1f*read_page->scaling_x+read_page->scaling_offset_x)+read_page->view_offset_x;
y1=qbr_float_to_long(y1f*read_page->scaling_y+read_page->scaling_offset_y)+read_page->view_offset_y;
x2=qbr_float_to_long(x2f*read_page->scaling_x+read_page->scaling_offset_x)+read_page->view_offset_x;
y2=qbr_float_to_long(y2f*read_page->scaling_y+read_page->scaling_offset_y)+read_page->view_offset_y;
}else{
x1=qbr_float_to_long(x1f)+read_page->view_offset_x; y1=qbr_float_to_long(y1f)+read_page->view_offset_y;
x2=qbr_float_to_long(x2f)+read_page->view_offset_x; y2=qbr_float_to_long(y2f)+read_page->view_offset_y;
}
}else{
x1=qbr_float_to_long(x1f); y1=qbr_float_to_long(y1f);
x2=qbr_float_to_long(x2f); y2=qbr_float_to_long(y2f);
}

//assume enough space exists to store the image

static byte_element_struct *ele;
ele=(byte_element_struct*)element;



static long x,y;
static long sx,sy,c,px,py;


//boundry checking (if no mask colour was passed)
if ((passed&1)==0){
if ((x1<0)||(x1>=read_page->width)||(x2<0)||(x2>=read_page->width)||(y1<0)||(y1>=read_page->height)||(y2<0)||(y2>=read_page->height)){
error(5);
return;
}
}



sx=x2-x1+1; sy=y2-y1+1;
if (sx<0) sx=0;
if (sy<0) sy=0;

//check required size here

static unsigned char *cp;


static long sizeinbytes;
static long byte;
static long bitvalue;
static long bytesperrow;
static long row2offset;
static long row3offset;
static long row4offset;

//header
((unsigned short*)ele->offset)[0]=sx;
((unsigned short*)ele->offset)[1]=sy;
cp=(unsigned char*)(ele->offset+4);
if (read_page->bits_per_pixel==8){
((unsigned short*)ele->offset)[0]*=8;
}

if (read_page->bits_per_pixel==1){
mask&=1;
byte=0;
}

if (read_page->bits_per_pixel==2){
((unsigned short*)ele->offset)[0]*=2;
mask&=3;
byte=0;
}


if (read_page->bits_per_pixel==4){
mask&=15;
bytesperrow=sx>>3; if (sx&7) bytesperrow++;
sizeinbytes=bytesperrow*sy;
memset(cp,0,sizeinbytes);
row2offset=bytesperrow;
row3offset=bytesperrow*2;
row4offset=bytesperrow*3;



}




for (y=0;y<sy;y++){
py=y1+y;

if (read_page->bits_per_pixel==4){
bitvalue=128;
byte=0;
}

for (x=0;x<sx;x++){
px=x1+x;
if ((px>=0)&&(px<read_page->width)&&(py>=0)&&(py<read_page->height)){
c=read_page->offset[py*read_page->width+px];
}else{
c=mask;
}

if (read_page->bits_per_pixel==8){
*cp=c;
cp++;
}

if (read_page->bits_per_pixel==1){
 byte>>=1;
 if (c) byte|=128;
 if ((x&7)==7){
  *cp=byte;  
  cp++;
  byte=0;
 }
}//read_page->bits_per_pixel==1


if (read_page->bits_per_pixel==2){
 byte<<=2;
 byte|=c;
 if ((x&3)==3){
  *cp=byte;
  cp++;
  byte=0;
 }
}//read_page->bits_per_pixel==2





if (read_page->bits_per_pixel==4){
byte=x>>3;
if (c&1) cp[byte]|=bitvalue;
if (c&2) cp[row2offset+byte]|=bitvalue;
if (c&4) cp[row3offset+byte]|=bitvalue;
if (c&8) cp[row4offset+byte]|=bitvalue;
bitvalue>>=1; if (bitvalue==0) bitvalue=128;
}

}//x

if (read_page->bits_per_pixel==4) cp+=(bytesperrow*4);

if (read_page->bits_per_pixel==1){
 if (x&7){
  *cp=byte;  
  cp++;
  byte=0;
 }
}


if (read_page->bits_per_pixel==2){
 if (sx&3){
  x=(4-(sx&3))*2;
  byte<<=x;
  *cp=byte;  
  cp++;
  byte=0;
 }
}

}//y



}


void sub_graphics_put(long step,float x1f,float y1f,void *element,long clip,long option,unsigned long mask,long passed){
//                                                                                                     &1                    
if (new_error) return;

if (write_page->bytes_per_pixel!=1){error(5); return;}

static long x1,y1;
//change coordinates according to step
if (step){x1f=write_page->x+x1f; y1f=write_page->y+y1f;}
//resolve coordinates
if (write_page->clipping_or_scaling){
if (write_page->clipping_or_scaling==2){
x1=qbr_float_to_long(x1f*write_page->scaling_x+write_page->scaling_offset_x)+write_page->view_offset_x;
y1=qbr_float_to_long(y1f*write_page->scaling_y+write_page->scaling_offset_y)+write_page->view_offset_y;
}else{
x1=qbr_float_to_long(x1f)+write_page->view_offset_x; y1=qbr_float_to_long(y1f)+write_page->view_offset_y;
}
}else{
x1=qbr_float_to_long(x1f); y1=qbr_float_to_long(y1f);
}
//_x=x2f; _y=y2f; ***need to adjust graphics cursor correctly***

static byte_element_struct *ele;
ele=(byte_element_struct*)element;
static long x,y;
static long sx,sy,c,px,py;
static unsigned char *cp;
static long *lp;

sx=((unsigned short*)ele->offset)[0];
sy=((unsigned short*)ele->offset)[1];
cp=(unsigned char*)(ele->offset+4);
lp=(long*)cp;


static long sizeinbytes;
static long byte;
static long bitvalue;
static long bytesperrow;
static long row2offset;
static long row3offset;
static long row4offset;

static long longval;

if (write_page->bits_per_pixel==8){
mask&=255;
//create error if not divisible by 8!
sx>>=3;
}

if (write_page->bits_per_pixel==1){
mask&=1;
}

if (write_page->bits_per_pixel==2){
mask&=3;
sx>>=1;
}


if (write_page->bits_per_pixel==4){
mask&=15;
bytesperrow=sx>>3; if (sx&7) bytesperrow++;
row2offset=bytesperrow;
row3offset=bytesperrow*2;
row4offset=bytesperrow*3;
}


for (y=0;y<sy;y++){
py=y1+y;

if (write_page->bits_per_pixel==4){
bitvalue=128;
byte=0;
}

for (x=0;x<sx;x++){
px=x1+x;


//get colour
if (write_page->bits_per_pixel==8){
 c=*cp;
 cp++;
}

if (write_page->bits_per_pixel==4){
byte=x>>3;
c=0;
if (cp[byte]&bitvalue) c|=1;
if (cp[row2offset+byte]&bitvalue) c|=2;
if (cp[row3offset+byte]&bitvalue) c|=4;
if (cp[row4offset+byte]&bitvalue) c|=8;
bitvalue>>=1; if (bitvalue==0) bitvalue=128;
}


if (write_page->bits_per_pixel==1){
 if (!(x&7)){
  byte=*cp;
  cp++;
 }
 c=(byte&128)>>7; byte<<=1;
}

if (write_page->bits_per_pixel==2){
 if (!(x&3)){
  byte=*cp;
  cp++;
 }
 c=(byte&192)>>6; byte<<=2;
}


if ((px>=0)&&(px<write_page->width)&&(py>=0)&&(py<write_page->height)){

//check color
if (passed){
if (c==mask) goto maskpixel;
}


//"pset" color

//PUT[{STEP}](?,?),?[,[{PSET|PRESET|AND|OR|XOR}][,[?]]]
//apply option

if (option==1){
write_page->offset[py*write_page->width+px]=c;
}
if (option==2){
//PRESET=bitwise NOT
write_page->offset[py*write_page->width+px]=(~c)&write_page->mask;
}
if (option==3){
write_page->offset[py*write_page->width+px]&=c;
}
if (option==4){
write_page->offset[py*write_page->width+px]|=c;
}
if ((option==5)||(option==0)){
write_page->offset[py*write_page->width+px]^=c;
}






}
maskpixel:;


}//x


if (write_page->bits_per_pixel==4) cp+=(bytesperrow*4);

//if (_bits_per_pixel==1){
// if (sx&7) cp++;
//}

//if (_bits_per_pixel==2){
// if (sx&3) cp++;
//}


}//y

}

void sub_date(qbs* date){
if (new_error) return;
return;//stub
}

qbs *func_date(){
//mm-dd-yyyy
//0123456789
static time_t qb64_tm_val;
static tm *qb64_tm;
//struct tm {
//        int tm_sec;     /* seconds after the minute - [0,59] */
//        int tm_min;     /* minutes after the hour - [0,59] */
//        int tm_hour;    /* hours since midnight - [0,23] */
//        int tm_mday;    /* day of the month - [1,31] */
//        int tm_mon;     /* months since January - [0,11] */
//        int tm_year;    /* years since 1900 */
//        int tm_wday;    /* days since Sunday - [0,6] */
//        int tm_yday;    /* days since January 1 - [0,365] */
//        int tm_isdst;   /* daylight savings time flag */
//        };
static long x,x2,i;
static qbs *str;
str=qbs_new(10,1);
str->chr[2]=45; str->chr[5]=45;//-
time(&qb64_tm_val); qb64_tm=localtime(&qb64_tm_val);
x=qb64_tm->tm_mon; x++; 
i=0; str->chr[i]=x/10+48; str->chr[i+1]=x%10+48;
x=qb64_tm->tm_mday;
i=3; str->chr[i]=x/10+48; str->chr[i+1]=x%10+48;
x=qb64_tm->tm_year; x+=1900;
i=6;
x2=x/1000; x=x-x2*1000; str->chr[i]=x2+48; i++;
x2=x/100; x=x-x2*100; str->chr[i]=x2+48; i++;
x2=x/10; x=x-x2*10; str->chr[i]=x2+48; i++;
str->chr[i]=x+48;
return str;
}








void sub_time(qbs* str){
if (new_error) return;
return;//stub
}

qbs *func_time(){
//23:59:59 (hh:mm:ss)
//01234567
static time_t qb64_tm_val;
static tm *qb64_tm;
//struct tm {
//        int tm_sec;     /* seconds after the minute - [0,59] */
//        int tm_min;     /* minutes after the hour - [0,59] */
//        int tm_hour;    /* hours since midnight - [0,23] */
//        int tm_mday;    /* day of the month - [1,31] */
//        int tm_mon;     /* months since January - [0,11] */
//        int tm_year;    /* years since 1900 */
//        int tm_wday;    /* days since Sunday - [0,6] */
//        int tm_yday;    /* days since January 1 - [0,365] */
//        int tm_isdst;   /* daylight savings time flag */
//        };
static long x,x2,i;
static qbs *str;
str=qbs_new(8,1);
str->chr[2]=58; str->chr[5]=58;//:
time(&qb64_tm_val); qb64_tm=localtime(&qb64_tm_val);
x=qb64_tm->tm_hour;
i=0; str->chr[i]=x/10+48; str->chr[i+1]=x%10+48;
x=qb64_tm->tm_min;
i=3; str->chr[i]=x/10+48; str->chr[i+1]=x%10+48;
x=qb64_tm->tm_sec;
i=6; str->chr[i]=x/10+48; str->chr[i+1]=x%10+48;
return str;
}


long func_csrlin(){
return write_page->cursor_y;
}
long func_pos(long ignore){
return write_page->cursor_x;
}



double func_log(double value){
if (value<=0){error(5);return 0;}
return log(value);
}

//FIX
double func_fix_double(double value){
if (value<0) return ceil(value); else return floor(value);
}
long double func_fix_float(long double value){
if (value<0) return ceil(value); else return floor(value);
}

//EXP
double func_exp_single(double value){
if (value<=88.02969){
return exp(value);
}
error(6); return 0;
}
long double func_exp_float(long double value){
if (value<=709.782712893){
return exp(value);
}
error(6); return 0;
}

long sleep_break=0;
void sub_sleep(long seconds,long passed){
if (new_error) return;
static uint64 start;
static uint64 milliseconds;
static uint64 stop;
static uint64 now;
sleep_break=0;

if (!passed) seconds=0;
if (seconds<=0){
while ((!sleep_break)&&(!stop_program)){
SDL_Delay(32);
}
return;
}

start=SDL_GetTicks();
milliseconds=seconds;
milliseconds*=1000;
stop=start+milliseconds;
if (stop>4294966295){error(6); return;}//cannot process wait time correctly!
wait:
if (sleep_break||stop_program) return;
now=SDL_GetTicks();
if (now<(stop-64)){//more than 64 milliseconds remain!
SDL_Delay(32);
goto wait;
}
if (now<stop) goto wait;
}

//if max_characters is 0 func_hex decides max_caharacters
qbs *func_oct(uint64 value,long sig_bits){
static qbs *str;
static long i,i2,leadingspace,max_characters;
static uint64 value2;
static uint64 v0=build_uint64(0xFFFFFFFF,0x80000000);
static uint64 v1=build_uint64(0xFFFFFFFF,0xFFFF8000);

if (sig_bits<=0){
i=-sig_bits;
sig_bits=64;//maximum
if (i<=32){
value2=value&v0;
if ((value2==v0)||(value2==0)) sig_bits=32;
}
if (i<=16){
value2=value&v1;
if ((value2==v1)||(value2==0)) sig_bits=16;
}
}
max_characters=(sig_bits+2)/3;
str=qbs_new(max_characters,1);
//set non-significant bits to 0
value<<=(64-sig_bits);
value>>=(64-sig_bits);
leadingspace=0;
for (i=1;i<=max_characters;i++){
i2=value&7;
if (i2) leadingspace=0; else leadingspace++;
i2+=48;
str->chr[max_characters-i]=i2;
value>>=3;
}
if (leadingspace){
memmove(str->chr,str->chr+leadingspace,max_characters-leadingspace);
str->len=max_characters-leadingspace;
}
return str;
}

//performs overflow check before calling func_oct
qbs *func_oct_float(long double value){
static int64 ivalue;
static qbs *str;
if ((value>9.2233720368547758E16)||(value<-9.2233720368547758E16)){
str=qbs_new(0,1);
error(6);//Overflow
return str;
}
if (value<0) ivalue=value-0.5; else ivalue=value+0.5;
return func_oct(ivalue,-32);
}













//if max_characters is 0 func_hex decides max_caharacters
qbs *func_hex(uint64 value,long max_characters){
static qbs *str;
static long i,i2,leadingspace;
static uint64 value2;
static uint64 v0=build_uint64(0xFFFFFFFF,0x80000000);
static uint64 v1=build_uint64(0xFFFFFFFF,0xFFFF8000);

if (max_characters<=0){
i=-max_characters;
max_characters=16;//maximum
if (i<=8){
value2=value&v0;
if ((value2==v0)||(value2==0)) max_characters=8;
}
if (i<=4){
value2=value&v1;
if ((value2==v1)||(value2==0)) max_characters=4;
}
}
str=qbs_new(max_characters,1);
leadingspace=0;
for (i=1;i<=max_characters;i++){
i2=value&15;
if (i2) leadingspace=0; else leadingspace++;
if (i2>9) i2+=55; else i2+=48;
str->chr[max_characters-i]=i2;
value>>=4;
}
if (leadingspace){
memmove(str->chr,str->chr+leadingspace,max_characters-leadingspace);
str->len=max_characters-leadingspace;
}
return str;
}

//performs overflow check before calling func_hex
qbs *func_hex_float(long double value){
static int64 ivalue;
static qbs *str;
if ((value>9.2233720368547758E16)||(value<-9.2233720368547758E16)){
str=qbs_new(0,1);
error(6);//Overflow
return str;
}
if (value<0) ivalue=value-0.5; else ivalue=value+0.5;
return func_hex(ivalue,-8);
}

long func_lbound(long *array,long index,long num_indexes){
if ((index<1)||(index>num_indexes)){
error(9); return 0;
}
index=num_indexes-index+1;
return array[4*index];
}

long func_ubound(long *array,long index,long num_indexes){
if ((index<1)||(index>num_indexes)){
error(9); return 0;
}
index=num_indexes-index+1;
return array[4*index]+array[4*index+1]-1;
}

static unsigned char port60h_event[256];
static long port60h_events=0;


long func_inp(long port){
static long value;
unsupported_port_accessed=0;
if ((port>65535)||(port<-65536)){
error(6); return 0;//Overflow
}
port&=0xFFFF;

/*
3dAh (R):  Input Status #1 Register
bit   0  Either Vertical or Horizontal Retrace active if set
      1  Light Pen has triggered if set
      2  Light Pen switch is open if set
      3  Vertical Retrace in progress if set
    4-5  Shows two of the 6 color outputs, depending on 3C0h index 12h.
          Attr: Bit 4-5:   Out bit 4  Out bit 5
                   0          Blue       Red
                   1        I Blue       Green
                   2        I Red      I Green
*/
if (port==0x3DA){
value=0;
if (vertical_retrace_happened||vertical_retrace_in_progress){
vertical_retrace_happened=0;
value|=8;
}
return value;
}

if (port==0x60){
//return last scancode event
if (port60h_events){
value=port60h_event[0];
if (port60h_events>1) memmove(port60h_event,port60h_event+1,255);
port60h_events--;
return value;
}else{
return port60h_event[0];
}

}



unsupported_port_accessed=1;
return 0;//unknown port!
}

void sub_wait(long port,long andexpression,long xorexpression,long passed){
if (new_error) return;
//1. read value from port
//2. value^=xorexpression (if passed!)
//3. value^=andexpression
//IMPORTANT: Wait returns immediately if given port is unsupported by QB64 so program
//           can continue
static long value;

//error & range checking
if ((port>65535)||(port<-65536)){
error(6); return;//Overflow
}
port&=0xFFFF;
if ((andexpression<-32768)||(andexpression>65535)){
error(6); return;//Overflow
}
andexpression&=0xFF;
if (passed){
if ((xorexpression<-32768)||(xorexpression>65535)){
error(6); return;//Overflow
}
}
xorexpression&=0xFF;

wait:
value=func_inp(port);
if (passed) value^=xorexpression;
value&=andexpression;
if (value||unsupported_port_accessed||stop_program) return;
Sleep(1);
goto wait;
}

extern long tab_spc_cr_size; //=1;//default
qbs *func_tab(long pos){
//returns a string to advance to the horizontal position "pos" on either
//the current line or the next line.
static long w,div;
//calculate width in spaces & current position
if (write_page->text){
w=write_page->width;
div=1;
}else{
 if (fontwidth[write_page->font]){
 w=write_page->width/fontwidth[write_page->font];
 div=1;
 }else{
 //w=func__printwidth(singlespace,NULL,0);
 w=write_page->width;
 div=func__printwidth(singlespace,NULL,0);
 }
}
static qbs *tqbs;
if ((pos<-32768)||(pos>32767)){
tqbs=qbs_new(0,1);
error(7); return tqbs;//Overflow
}
if (pos>w) pos%=w;
if (pos<1) pos=1;
static long size,spaces,cr;
size=0; spaces=0; cr=0;
if (write_page->cursor_x>pos){
cr=1;
size=tab_spc_cr_size;
spaces=pos/div; if (pos%div) spaces++;
spaces--;//don't put a space on the dest position
size+=spaces;
}else{
spaces=(pos-write_page->cursor_x)/div; if ((pos-write_page->cursor_x)%div) spaces++;
size=spaces;
}
//build custom string
tqbs=qbs_new(size,1);
if (cr){
tqbs->chr[0]=13; if (tab_spc_cr_size==2) tqbs->chr[1]=10;
memset(&tqbs->chr[tab_spc_cr_size],32,spaces);
}else{
memset(tqbs->chr,32,spaces);
}
return tqbs;
}

qbs *func_spc(long spaces){
static qbs *tqbs;
if ((spaces<-32768)||(spaces>32767)){
tqbs=qbs_new(0,1);
error(7); return tqbs;//Overflow
}

tqbs=qbs_new(0,1); return tqbs;//********STUB********

if (spaces<0) spaces=0;
spaces%=qbg_width_in_characters;
//cr required?
if ((qbg_cursor_x+spaces)>qbg_width_in_characters){
spaces=spaces-(qbg_width_in_characters-qbg_cursor_x)-1;
if (spaces<0) spaces=0;//safeguard for when cursor_x=qbg_width_in_characters+1
tqbs=qbs_new(tab_spc_cr_size+spaces,1);
tqbs->chr[0]=13; if (tab_spc_cr_size==2) tqbs->chr[1]=10;
memset(&tqbs->chr[tab_spc_cr_size],32,spaces);
}else{
tqbs=qbs_new(spaces,1);
memset(tqbs->chr,32,spaces);
}
return tqbs;
}

float func_pmap(float val,long option){
static long x,y;
if (new_error) return 0;
if (!write_page->text){
//note: for QBASIC/4.5/7.1 compatibility clipping_or_scaling check is skipped
if (option==0){
x=qbr_float_to_long(val*write_page->scaling_x+write_page->scaling_offset_x);
return x;
}
if (option==1){
y=qbr_float_to_long(val*write_page->scaling_y+write_page->scaling_offset_y);
return y;
}
if (option==2){
return (((double)qbr_float_to_long(val))-write_page->scaling_offset_x)/write_page->scaling_x;
}
if (option==3){
return (((double)qbr_float_to_long(val))-write_page->scaling_offset_y)/write_page->scaling_y;
}
}//!write_page->text
error(5);
return 0;
}



unsigned long func_screen(long y,long x,long returncol,long passed){

static unsigned char chr[65536];
static long x2,y2,x3,y3,i,i2,i3;
static unsigned long col,firstcol;
unsigned char *cp;

if (!passed) returncol=0;

if (read_page->text){
//on screen?
if ((y<1)||(y>read_page->height)){error(5); return 0;}
if ((x<1)||(x>read_page->width)){error(5); return 0;}
if (returncol){
return read_page->offset[((y-1)*read_page->width+x-1)*2+1];
}
return read_page->offset[((y-1)*read_page->width+x-1)*2];
}

//only 8x8,8x14,8x16 supported
if ((read_page->font!=8)&&(read_page->font!=14)&&(read_page->font!=16)){error(5); return 0;}

//on screen?
x2=read_page->width/fontwidth[read_page->font];
y2=read_page->height/fontheight[read_page->font];
if ((y<1)||(y>y2)){error(5); return 0;}
if ((x<1)||(x>x2)){error(5); return 0;}

//create "signature" of character on screen
x--; y--;
i=0;
i3=1;
y3=y*fontheight[read_page->font];
for (y2=0;y2<fontheight[read_page->font];y2++){
x3=x*fontwidth[read_page->font];
for (x2=0;x2<fontwidth[read_page->font];x2++){
col=point(x3,y3);
if (col){
if (i3){i3=0;firstcol=col;}
col=255;
}
chr[i]=col;
i++;
x3++;
}
y3++;
}

if (i3){//assume SPACE, no non-zero pixels were found
if (returncol) return 1;
return 32;
}

i3=i;//number of bytes per character "signature"

//compare signature with all ascii characters
for (i=0;i<=255;i++){
if (read_page->font==8) cp=&charset8x8[i][0][0];
if (read_page->font==14) cp=&charset8x16[i][1][0];
if (read_page->font==16) cp=&charset8x16[i][0][0];
i2=memcmp(cp,chr,i3);
if (!i2){//identical!
if (returncol) return firstcol;
return i;
}
}

//no match found
if (returncol) return 0;
return 32;
}

void sub_bsave(qbs *filename,long offset,long size){
if (new_error) return;
static ofstream fh;

static qbs *tqbs=NULL;
if (!tqbs) tqbs=qbs_new(0,0);
static qbs *nullt=NULL;
if (!nullt) nullt=qbs_new(1,0);

static long x;
nullt->chr[0]=0;
if ((offset<-65536)||(offset>65535)){error(6); return;}//Overflow
offset&=0xFFFF;
//note: QB64 allows a maximum of 65536 bytes, QB only allows 65535
if ((size<-65536)||(size>65536)){error(6); return;}//Overflow
if (size!=65536) size&=0xFFFF;
qbs_set(tqbs,qbs_add(filename,nullt));//prepare null-terminated filename
fh.open((const char*)tqbs->chr,ios::binary|ios::out);
if (fh.is_open()==NULL){error(64); return;}//Bad file name
x=253; fh.write((char*)&x,1);//signature byte
x=(defseg-&cmem[0])/16; fh.write((char*)&x,2);//segment
x=offset; fh.write((char*)&x,2);//offset
x=size; if (x>65535) x=0;//if filesize>65542 then size=filesize-7
fh.write((char*)&x,2);//size
fh.write((char*)(defseg+offset),size);//data
fh.close();
}

void sub_bload(qbs *filename,long offset,long passed){
if (new_error) return;
static unsigned char header[7];
static ifstream fh;
static qbs *tqbs=NULL;
if (!tqbs) tqbs=qbs_new(0,0);
static qbs *nullt=NULL;
if (!nullt) nullt=qbs_new(1,0);


static long x,file_seg,file_off,file_size;
nullt->chr[0]=0;
if (passed){
if ((offset<-65536)||(offset>65535)){error(6); return;}//Overflow
offset&=0xFFFF;
}
qbs_set(tqbs,qbs_add(filename,nullt));//prepare null-terminated filename
fh.open((const char*)tqbs->chr,ios::binary|ios::in);
if (fh.is_open()==NULL){error(53); return;}//File not found
fh.read((char*)header,7); if (fh.gcount()!=7) goto error;
if (header[0]!=253) goto error;
file_seg=header[1]+header[2]*256;
file_off=header[3]+header[4]*256;
file_size=header[5]+header[6]*256;
if (file_size==0){
fh.seekg(0,ios::end);
file_size=fh.tellg();
fh.seekg(7,ios::beg);
file_size-=7;
if (file_size<65536) file_size=0;
}
if (passed){
fh.read((char*)(defseg+offset),file_size);
}else{
fh.read((char*)(&cmem[0]+file_seg*16+file_off),file_size);
}
if (fh.gcount()!=file_size) goto error;
fh.close();
return;
error:
fh.close();
error(54);//Bad file mode
}

long func_lof(long i){
static long lof,pos;
if (i<1){error(52); return 0;}//Bad file name or number
if (i>last_device_handle){error(52); return 0;}//Bad file name or number
if (!device_handle[i].open){error(52); return 0;}//Bad file name or number
if (device_handle[i].read_access){
pos=device_handle[i].fh.tellg();
device_handle[i].fh.seekg(0,ios::end);
lof=device_handle[i].fh.tellg();
device_handle[i].fh.seekg(pos,ios::beg);
}else{
pos=device_handle[i].fh.tellp();
device_handle[i].fh.seekp(0,ios::end);
lof=device_handle[i].fh.tellp();
device_handle[i].fh.seekp(pos,ios::beg);
}
return lof;
}

long func_eof(long i){
static long pos,lof;
if (i<1){error(52); return 0;}//Bad file name or number
if (i>last_device_handle){error(52); return 0;}//Bad file name or number
if (!device_handle[i].open){error(52); return 0;}//Bad file name or number
lof=func_lof(i);
if (device_handle[i].pos>=lof){
return -1;
}
return 0;
}

void sub_seek(long i,long pos){
if (new_error) return;
if (i<1){error(52); return;}//Bad file name or number
if (i>last_device_handle){error(52); return;}//Bad file name or number
if (!device_handle[i].open){error(52); return;}//Bad file name or number

if (device_handle[i].type==1){//RANDOM access?
pos--;
if (pos<0){error(63); return;}//Bad record number
pos*=device_handle[i].record_length;
pos++;
}

pos--;
if (pos<0){error(63); return;}//Bad record number
device_handle[i].pos=pos;
if (device_handle[i].read_access){
device_handle[i].fh.seekg(pos,ios::beg);
}else{
device_handle[i].fh.seekp(pos,ios::beg);
}
if (device_handle[i].fh.eof()){
device_handle[i].fh.clear();
device_handle[i].eof_reached=1;
}
}

long func_seek(long i){
static long lof,pos;
if (i<1){error(52); return 0;}//Bad file name or number
if (i>last_device_handle){error(52); return 0;}//Bad file name or number
if (!device_handle[i].open){error(52); return 0;}//Bad file name or number
if (device_handle[i].type==1) return device_handle[i].pos/device_handle[i].record_length+1;//RANDOM access?
return device_handle[i].pos+1;
}

long func_loc(long i){
static long lof,pos;
if (i<1){error(52); return 0;}//Bad file name or number
if (i>last_device_handle){error(52); return 0;}//Bad file name or number
if (!device_handle[i].open){error(52); return 0;}//Bad file name or number
if (device_handle[i].type==1){//RANDOM
return device_handle[i].pos/device_handle[i].record_length;
}
if (device_handle[i].type==2){//BINARY
return device_handle[i].pos;
}
//APPEND/OUTPUT/INPUT
pos=device_handle[i].pos;
if (pos==0) return 1;
pos--;
pos=pos/128;
pos++;
return pos;
}

qbs *func_input(long n,long i,long passed){
if (new_error) return qbs_new(0,1);
static qbs *str,*str2;
static long x,c;
if (n<0) str=qbs_new(0,1); else str=qbs_new(n,1);
if (passed){
//file
if (i<1){error(52); return str;}//Bad file name or number
if (i>last_device_handle){error(52); return str;}//Bad file name or number
if (!device_handle[i].open){error(52); return str;}//Bad file name or number
//OUTPUT/APPEND modes ->error
if (!device_handle[i].read_access){error(62); return str;}//Input past end of file
if (n<0){error(52); return str;}
if (n==0) return str;
//INPUT -> Input past end of file at EOF or CHR$(26)
//         unlike BINARY, partial strings cannot be returned
//         (use input_file_chr and modify it to support CHR$(26)
if (device_handle[i].type==3){
x=0;
do{
c=file_input_chr(i);
if (c==-1){error(62); return str;}//Input past end of file
str->chr[x]=c;
x++;
}while(x<n);
return str;
}
//BINARY -> If past EOF, returns a NULL length string!
//          or as much of the data that was valid as possible
//          Seek POS is only advanced as far as was read!
//          Binary can read past CHR$(26)
//          (simply call C's read statement and manage via .eof
if (device_handle[i].type==2){
//still at eof?
if (device_handle[i].eof_reached){
device_handle[i].fh.seekg(device_handle[i].pos,ios::beg);
if (device_handle[i].fh.eof()){
device_handle[i].fh.clear();
str->len=0;
return str;
}
device_handle[i].eof_reached=0;
}
//read n bytes
device_handle[i].fh.read((char*)str->chr,n);
x=device_handle[i].fh.gcount();
device_handle[i].pos+=x;
if (x!=n){
device_handle[i].fh.clear();
device_handle[i].eof_reached=1;
str->len=x;
}
return str;
}
//RANDOM -> check help when implementing this
//...
}else{
//keyboard/piped
//      For extended-two-byte character codes, only the first, CHR$(0), is returned and counts a 1 byte
if (n<0){error(52); return str;}
if (n==0) return str;
x=0;
waitforinput:
str2=qbs_inkey();
if (str2->len){
str->chr[x]=str2->chr[0];
x++;
}
qbs_free(str2);
if (stop_program) return str;
if (x<n){SDL_Delay(1); goto waitforinput;}
return str;
}
}

double func_sqr(double value){
if (value<0){error(5); return 0;}
return sqrt(value);
}

void sub_beep(){
sndsetup();
qb64_generatesound(783.99,0.2,0);
Sleep(250);
}

#define SND_CAPABILITY_SYNC 1
#define SND_CAPABILITY_VOL 2
#define SND_CAPABILITY_PAUSE 4
#define SND_CAPABILITY_LEN 8
#define SND_CAPABILITY_SETPOS 16

struct snd_struct{
unsigned long handle;
Mix_Chunk *sample;
unsigned char free;
unsigned char playing;
unsigned char paused;
float volume;
float volume_mult1;
float posx;
float posy;
float posz;
unsigned long start_time;
unsigned long paused_time;
unsigned char looping;
unsigned long limit;
double len;
unsigned char capability;
};
snd_struct snd[AUDIO_CHANNELS];
Mix_Music *stream=NULL;
long stream_free=0;
long stream_loaded=0;
long stream_playing=0;
unsigned long snd_free_handle=2;
long stream_type=0;
long stream_paused=0;
float stream_volume=1;
float stream_volume_mult1=1;
unsigned long stream_start_time=0;
unsigned long stream_paused_time=0;
double stream_setpos=0;
long stream_looping=0;
double stream_limit=0;
long stream_limited=0;
double stream_len=-1;
unsigned char stream_capability;

void snd_check(){
sndsetup();
static long i,i2,i3;
//check stream
if (stream_loaded){
if (stream_free){
//still playing?
if (stream_playing&&(!stream_looping)) if (!Mix_PlayingMusic()) stream_playing=0;
if (!stream_playing){
Mix_FreeMusic(stream);
stream=NULL;
}
}
}
//check samples
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle){
if (snd[i].free){
//still playing?
if (snd[i].playing&&(!snd[i].looping)) if (!Mix_Playing(i)) snd[i].playing=0;
if (!snd[i].playing){
snd[i].handle=0;
//free sample data if unrequired by any other sample
i3=1;
for (i2=0;i2<AUDIO_CHANNELS;i2++){
if (snd[i2].handle){
if (snd[i2].sample==snd[i].sample){i3=0; break;}
}}
if (i3) Mix_FreeChunk(snd[i].sample);
}//!stream_playing
}//free
}//handle
}//i
}//snd_check





unsigned long func__sndraw(unsigned char* data,unsigned long bytes){
sndsetup();
//***unavailable to QB64 user***
static long i,i2,i3;
//find available index
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==0){
memset(&snd[i],0,sizeof(snd_struct));//clear structure
Mix_Volume(i,128);//restore full volume
snd[i].volume=1;
snd[i].volume_mult1=1;
snd[i].sample=Mix_QuickLoad_RAW((Uint8*)data,bytes);
//length_in_sec=size_in_bytes/samples_per_sec/bytes_per_sample/channels
snd[i].len=((double)snd[i].sample->alen)/22050.0/2.0/2.0;
snd[i].handle=snd_free_handle; snd_free_handle++; if (snd_free_handle==0) snd_free_handle=2;
snd[i].capability=SND_CAPABILITY_SYNC;
return snd[i].handle;
}
}
return 0;//no free indexes
}







unsigned long func__sndopen(qbs* filename,qbs* requirements,long passed){
sndsetup();
if (new_error) return 0;

static qbs *s1=NULL;
if (!s1) s1=qbs_new(0,0);
static qbs *req=NULL;
if (!req) req=qbs_new(0,0);
static qbs *s3=NULL;
if (!s3) s3=qbs_new(0,0);

static unsigned char r[32];
static long i,i2,i3;
//check requirements
memset(r,0,32);
if (passed){
if (requirements->len){
i=1;
qbs_set(req,qbs_ucase(requirements));//convert tmp str to perm str
nextrequirement:
i2=func_instr(i,req,qbs_new_txt(","),1);
if (i2){
qbs_set(s1,func_mid(req,i,i2-i,1));
}else{
qbs_set(s1,func_mid(req,i,req->len-i+1,1));
}
if (qbs_equal(s1,qbs_new_txt("SYNC"))){r[0]++; goto valid;}
if (qbs_equal(s1,qbs_new_txt("VOL"))){r[1]++; goto valid;}
if (qbs_equal(s1,qbs_new_txt("PAUSE"))){r[2]++; goto valid;}
if (qbs_equal(s1,qbs_new_txt("LEN"))){r[3]++; goto valid;}
if (qbs_equal(s1,qbs_new_txt("SETPOS"))){r[4]++; goto valid;}
error(5); return 0;//invalid requirements
valid:
if (i2){i=i2+1; goto nextrequirement;}
for (i=0;i<32;i++) if (r[i]>1){error(5); return 0;}//cannot define requirements twice
}//->len
}//passed
qbs_set(s1,qbs_add(filename,qbs_new_txt_len("\0",1)));//s1=filename+CHR$(0)
if (r[0]){//"SYNC"
if (r[4]) return 0;//"SETPOS" unsupported
//find available index
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==0){
memset(&snd[i],0,sizeof(snd_struct));//clear structure
Mix_Volume(i,128);//restore full volume
snd[i].volume=1;
snd[i].volume_mult1=1;
snd[i].sample=Mix_LoadWAV((char*)s1->chr);
if (snd[i].sample==NULL) return 0;
//length_in_sec=size_in_bytes/samples_per_sec/bytes_per_sample/channels
snd[i].len=((double)snd[i].sample->alen)/22050.0/2.0/2.0;
snd[i].handle=snd_free_handle; snd_free_handle++; if (snd_free_handle==0) snd_free_handle=2;
snd[i].capability=SND_CAPABILITY_SYNC+r[1]*SND_CAPABILITY_VOL+r[2]*SND_CAPABILITY_PAUSE+r[3]*SND_CAPABILITY_LEN;
return snd[i].handle;
}
}
return 0;//no free indexes
}else{//not "SYNC"
//stream
	//dealloc current stream?
	if (stream_loaded){
	if (!stream_free){error(5); return 0;}
  Mix_FreeMusic(stream);
	stream_loaded=0;
	}
stream=NULL;

//check requirements
stream_len=-1;
if (r[3]){//"LEN"
//detect length (if possible)
static Mix_Chunk *tmpsample;
tmpsample=Mix_LoadWAV((char*)s1->chr);
if (tmpsample){
stream_len=((double)tmpsample->alen)/22050.0/2.0/2.0;
Mix_FreeChunk(tmpsample);
}else{
return 0;//capability unavailable
}
}

stream=Mix_LoadMUS((char*)s1->chr);
if (!stream) return 0;
Mix_VolumeMusic(128);//restore full volume
stream_volume=1;
stream_volume_mult1=1;
stream_paused=0;
stream_setpos=0;
stream_looping=0;
stream_free=0;
//check requirements?
stream_type=0;
i=Mix_GetMusicType(stream);
if (i==6) stream_type=1;//mp3

if (r[4]){//"SETPOS"
if (stream_type!=1){
 Mix_FreeMusic(stream);
 return 0;//capability unavailable
}
}

stream_capability=r[1]*SND_CAPABILITY_VOL+r[2]*SND_CAPABILITY_PAUSE+r[3]*SND_CAPABILITY_LEN+r[4]*SND_CAPABILITY_SETPOS;
stream_loaded=1;
return 1;
}
}

double func__sndlen(unsigned long handle){
sndsetup();
static long i;
if (handle==0) return 0;//default response
if (handle==1){
if (stream_len==-1){error(5); return 0;}
if (!(stream_capability&SND_CAPABILITY_LEN)){error(5); return 0;}//unrequested capability
return stream_len;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return 0;}
if (!(snd[i].capability&SND_CAPABILITY_LEN)){error(5); return 0;}//unrequested capability
return snd[i].len;
}
}//i
error(5); return 0;//invalid handle
}

void sub__sndlimit(unsigned long handle,double limit){
sndsetup();
if (new_error) return;
//limit=0 means no limit
static long i;
if (handle==0) return;//default response
if (handle==1){
if (!stream_loaded){error(5); return;}
if (stream_free){error(5); return;}
stream_limit=limit;
return;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return;}
snd[i].limit=limit*1000;
return;
}
}//i
error(5); return;//invalid handle
}

void sub__sndstop(unsigned long handle){
sndsetup();
if (new_error) return;
static long i;
if (handle==0) return;//default response
if (handle==1){
if (!stream_loaded){error(5); return;}
if (stream_free){error(5); return;}
Mix_HaltMusic();//stop music
stream_paused=0;
stream_playing=0;
stream_setpos=0;
stream_looping=0;
return;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return;}
Mix_HaltChannel(i);
snd[i].playing=0;
snd[i].paused=0;
snd[i].looping=0;
return;
}
}//i
error(5); return;//invalid handle
}


long sndloop_call=0;

void sub__sndsetpos(unsigned long handle,double sec){
sndsetup();
if (new_error) return;
static unsigned long ms;
if (sec<0){error(5); return;}//must be >=0
if (handle==0) return;//default response
if (handle==1){
if (!stream_loaded){error(5); return;}
if (stream_free){error(5); return;}
if (stream_type!=1){error(5); return;}//only mp3 supports setpos
if (!(stream_capability&SND_CAPABILITY_SETPOS)){error(5); return;}//unrequested capability
if (stream_looping){error(5); return;}//setpos unavailable while looping
//still playing?
if (stream_playing&&(!stream_looping)) if (!Mix_PlayingMusic()) stream_playing=0;
if (stream_playing){//could also be paused
Sleep(100);//without this, music sometimes causes a GPF!
Mix_RewindMusic();
Sleep(100);//without this, music sometimes causes a GPF!
Mix_SetMusicPosition(sec);
ms=sec*1000.0+0.5;
stream_start_time=SDL_GetTicks()-ms;
if (stream_paused) stream_paused_time=SDL_GetTicks();
}else{
//music not playing, buffer the request
stream_setpos=sec;
}
return;
}
error(5); return;//samples do not support this function in sdl_mixer
}

double func__sndgetpos(unsigned long handle){
sndsetup();
static long i;
if (handle==0) return 0;//default response
if (handle==1){
if (!stream_loaded){error(5); return 0;}
if (stream_free){error(5); return 0;}
//still playing?
if (stream_playing&&(!stream_looping)) if (!Mix_PlayingMusic()) stream_playing=0;
if (!stream_playing) return 0;
if (stream_paused) return(((double)(stream_paused_time-stream_start_time))/1000.0);
return(((double)(SDL_GetTicks()-stream_start_time))/1000.0);
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return 0;}
//still playing?
if (snd[i].playing&&(!snd[i].looping)) if (!Mix_Playing(i)) snd[i].playing=0;
if (!snd[i].playing) return 0;
if (snd[i].paused) return(((double)(snd[i].paused_time-snd[i].start_time))/1000.0);
return(((double)(SDL_GetTicks()-snd[i].start_time))/1000.0);
}
}//i
error(5); return 0;//invalid handle
}

void sub__sndbal(unsigned long handle,double x,double y,double z,long passed){
sndsetup();
if (new_error) return;
//any optional parameter not passed is assumed to be 0 (which is what NULL equates to)
static long i,v,i2,i3;
static double d,d2,d3;
if (handle==0) return;//default response
if (handle==1){
if (!stream_loaded){error(5); return;}
if (stream_free){error(5); return;}
if (!(stream_capability&SND_CAPABILITY_VOL)){error(5); return;}//unrequested capability
//unsupported, but emulate distance by using a volume change
d=sqrt(x*x+y*y+z*z);

if (d<1) d=0;
if (d>1000) d=1000;
d=1000-d;
d=d/1000.0;
stream_volume_mult1=d;

v=stream_volume*129; if (v==129) v=128;
Mix_VolumeMusic(v*stream_volume_mult1);
return;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return;}
if (!(snd[i].capability&SND_CAPABILITY_VOL)){error(5); return;}//unrequested capability
if ((x==0)&&(z==0)) z=1;
snd[i].posx=x; snd[i].posy=y; snd[i].posz=z;
d=atan2(x,z)*57.295779513;
if (d<0) d=360+d;
i2=d+0.5; if (i2==360) i2=0;//angle
d2=sqrt(x*x+y*y+z*z);//distance
if (d2<1) d2=1;
if (d2>999.9) d2=999.9;
i3=d2/3.90625;
Mix_SetPosition(i,i2,i3);
return;
}
}//i
error(5); return;//invalid handle
}

void sub__sndplay(unsigned long handle){
sndsetup();
if (new_error) return;
static long i;
static unsigned long ms;
if (handle==0) return;//default response
	//stream?
	if (handle==1){
	if (!stream_loaded){error(5); return;}
	if (stream_free){error(5); return;}
	if (stream_paused){	
  Mix_ResumeMusic();	
  stream_start_time=stream_start_time+(SDL_GetTicks()-stream_paused_time);
  stream_paused=0;
	return;
	}
  stream_looping=0;
  Mix_HaltMusic();//in case music is already playing
	if (sndloop_call){
	if (stream_limit){error(5); return;}//limit invalid
  if (Mix_PlayMusic(stream,-1)==-1) return;
	stream_limited=0;
	stream_looping=1;
  }else{
	if (Mix_PlayMusic(stream,0)==-1) return;
	if (stream_limit) stream_limited=1; else stream_limited=0;
  }
  stream_start_time=SDL_GetTicks();
	if (stream_setpos){
	Sleep(100);	
	Mix_SetMusicPosition(stream_setpos); 
  ms=stream_setpos*1000.0+0.5;
	stream_start_time=SDL_GetTicks()-ms;  
  stream_setpos=0; 
  }
	stream_playing=1;
  return;
	}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return;}
if (snd[i].paused){
Mix_Resume(i);
//augment start_time
snd[i].start_time=snd[i].start_time+(SDL_GetTicks()-snd[i].paused_time);
snd[i].paused=0;
return;
}
snd[i].looping=0;
Mix_HaltChannel(i);//in case sound is already playing
//remind sdl_mixer of sound's position?
if (snd[i].posx||snd[i].posy||snd[i].posz) sub__sndbal(handle,snd[i].posx,snd[i].posy,snd[i].posz,-1);
if (sndloop_call){
	if (snd[i].limit){
	error(5); return;//limit invalid
	}else{
	if(Mix_PlayChannel(i,snd[i].sample,-1)==-1) return;	
	}
snd[i].looping=1;
}else{
	if (snd[i].limit){
	if(Mix_PlayChannelTimed(i,snd[i].sample,0,snd[i].limit)==-1) return;
	}else{
	if(Mix_PlayChannel(i,snd[i].sample,0)==-1) return;
	}
}
snd[i].start_time=SDL_GetTicks();
snd[i].playing=1;
return;
}
}//i
error(5); return;//invalid handle
}

void sub__sndloop(unsigned long handle){
sndsetup();
if (new_error) return;
static long i;
if (handle==0) return;//default response
if (handle==1){
if (!stream_loaded){error(5); return;}
if (stream_free){error(5); return;}
sub__sndstop(handle);
stream_setpos=0;
sndloop_call=1; sub__sndplay(handle); sndloop_call=0;
return;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return;}
sub__sndstop(handle);
sndloop_call=1; sub__sndplay(handle); sndloop_call=0;
return;
}
}//i
error(5); return;//invalid handle
}

unsigned long func__sndcopy(unsigned long handle){
sndsetup();
if (new_error) return 0;
static long i,i2;
if (handle==0) return 0;//default response
if (handle==1){error(5); return 0;}//cannot copy a stream
//find handle
for (i2=0;i2<AUDIO_CHANNELS;i2++){
if (snd[i2].handle==handle){
	if (snd[i2].free){error(5); return 0;}
  if (!(snd[i2].capability&SND_CAPABILITY_SYNC)){error(5); return 0;}//unrequested capability
  //find free index
  for (i=0;i<AUDIO_CHANNELS;i++){
	if (snd[i].handle==0){
	memset(&snd[i],0,sizeof(snd_struct));//clear structure
  Mix_Volume(i,128);//restore full volume
	snd[i].volume=1;
  snd[i].volume_mult1=1;
  snd[i].sample=snd[i2].sample;
	//...
  snd[i].handle=snd_free_handle; snd_free_handle++; if (snd_free_handle==0) snd_free_handle=2;
	return snd[i].handle;
	}
	}
	return 0;//no free indexes
}
}//i2
error(5); return 0;//invalid handle
}

long sub__sndvol_error;
void sub__sndvol(unsigned long handle,float volume){
sndsetup();
if (new_error) return;
static long i,v;
sub__sndvol_error=1;
if ((volume<0)||(volume>1)){error(5); return;}
v=volume*129; if (v==129) v=128;
if (handle==0) {sub__sndvol_error=0;return;}//default response
if (handle==1){
if (!stream_loaded){error(5); return;}//invalid handle
if (stream_free){error(5); return;}
if (!(stream_capability&SND_CAPABILITY_VOL)){error(5); return;}//unrequested capability
Mix_VolumeMusic((float)v*stream_volume_mult1);
stream_volume=volume;
sub__sndvol_error=0;
return;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return;}
if (!(snd[i].capability&SND_CAPABILITY_VOL)){error(5); return;}//unrequested capability
Mix_Volume(i,(float)v*snd[i].volume_mult1);
snd[i].volume=volume;
sub__sndvol_error=0;
return;
}
}//i
error(5); return;//invalid handle
}

void sub__sndpause(unsigned long handle){
sndsetup();
if (new_error) return;
static long i;
if (handle==0) return;//default response
if (handle==1){
if (!stream_loaded){error(5); return;}
if (stream_free){error(5); return;}
if (!(stream_capability&SND_CAPABILITY_PAUSE)){error(5); return;}//unrequested capability
if (stream_paused) return;
//still playing?
if (stream_playing&&(!stream_looping)) if (!Mix_PlayingMusic()) stream_playing=0;
if (!stream_playing) return;
Mix_PauseMusic();
stream_paused_time=SDL_GetTicks();
stream_paused=1;
return;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return;}
if (!(snd[i].capability&SND_CAPABILITY_PAUSE)){error(5); return;}//unrequested capability
if (snd[i].paused) return;
//still playing?
if (snd[i].playing&&(!snd[i].looping)) if (!Mix_Playing(i)) snd[i].playing=0;
if (!snd[i].playing) return;
Mix_Pause(i);
snd[i].paused_time=SDL_GetTicks();
snd[i].paused=1;
return;
}
}//i
error(5); return;//invalid handle
}

long func__sndpaused(unsigned long handle){
sndsetup();
static long i;
if (handle==0) return 0;//default response
if (handle==1){
if (!stream_loaded){error(5); return 0;}
if (stream_free){error(5); return 0;}
if (!(stream_capability&SND_CAPABILITY_PAUSE)){error(5); return 0;}//unrequested capability
if (stream_paused) return -1;
return 0;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return 0;}
if (!(snd[i].capability&SND_CAPABILITY_PAUSE)){error(5); return 0;}//unrequested capability
if (snd[i].paused) return -1;
return 0;
}
}//i
error(5); return 0;//invalid handle
}

long func__sndplaying(unsigned long handle){
sndsetup();
static long i;
if (handle==0) return 0;//default response
if (handle==1){
if (!stream_loaded){error(5); return 0;}
if (stream_free){error(5); return 0;}
//still playing?
if (stream_playing&&(!stream_looping)) if (!Mix_PlayingMusic()) stream_playing=0;
if (stream_paused) return 0;
if (stream_playing) return -1;
return 0;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return 0;}
//still playing?
if (snd[i].playing&&(!snd[i].looping)) if (!Mix_Playing(i)) snd[i].playing=0;
if (snd[i].paused) return 0;
if (snd[i].playing) return -1;
return 0;
}
}//i
error(5); return 0;//invalid handle
}

void sub__sndclose(unsigned long handle){
sndsetup();
if (new_error) return;
static long i;
if (handle==0) return;//default response
if (handle==1){
if (!stream_loaded){error(5); return;}//invalid handle
if (stream_free){error(5); return;}//freed already
stream_free=1;
snd_check();
return;
}
//find handle
for (i=0;i<AUDIO_CHANNELS;i++){
if (snd[i].handle==handle){
if (snd[i].free){error(5); return;}//freed already
snd[i].free=1;
snd_check();
return;
}
}//i
error(5); return;//invalid handle
}

//"macros"
void sub__sndplayfile(qbs *filename,long sync,double volume,long passed){
sndsetup();
if (new_error) return;
static unsigned long handle;
static long setvolume;
static qbs *syncstr=NULL;
if (!syncstr) syncstr=qbs_new(0,0);
setvolume=0;
if (passed&2){
if ((volume<0)||(volume>1)){error(5); return;}
if (volume!=1) setvolume=1;
}
if ((!setvolume)&&(!sync)) syncstr->len=0;
if ((setvolume)&&(!sync)) qbs_set(syncstr,qbs_new_txt("VOL"));
if ((!setvolume)&&(sync)) qbs_set(syncstr,qbs_new_txt("SYNC"));
if ((setvolume)&&(sync)) qbs_set(syncstr,qbs_new_txt("SYNC,VOL"));
if (syncstr->len){
handle=func__sndopen(filename,syncstr,1);
}else{
handle=func__sndopen(filename,NULL,0);
}
if (handle==0) return;
if (setvolume) sub__sndvol(handle,volume);
sub__sndplay(handle);
sub__sndclose(handle);
}

void sub__sndplaycopy(unsigned long handle,double volume,long passed){
sndsetup();
if (new_error) return;
unsigned long handle2;
handle2=func__sndcopy(handle);
if (!handle2) return;//an error has already happened
if (passed){
sub__sndvol(handle2,volume);
if (sub__sndvol_error){
sub__sndclose(handle2);
return;
}
}
sub__sndplay(handle2);
sub__sndclose(handle2);
}


qbs *func_command_str=NULL;
qbs *func_command(){
static qbs *tqbs;
tqbs=qbs_new(func_command_str->len,1);
memcpy(tqbs->chr,func_command_str->chr,func_command_str->len);
return tqbs;
}

long shell_call_in_progress=0;
long shell_restore_full_screen=0;

void sub_shell(qbs *str,long passed){
if (new_error) return;
 //exit full screen mode if necessary
 static long full_screen_mode;
 full_screen_mode=full_screen;
 if (full_screen_mode){
 full_screen_set=0;
 do{
 Sleep(0);
 }while(full_screen);
 }//full_screen_mode
static qbs *strz=NULL;
static long i;
if (!strz) strz=qbs_new(0,0);
if (passed) if (str->len==0) passed=0;//"" means launch a CONSOLE
if (passed){

#ifdef QB64_WINDOWS

static STARTUPINFO si;
static PROCESS_INFORMATION pi;
//attempt: CMD.EXE
qbs_set(strz,qbs_add(qbs_new_txt("cmd.exe /c "),str));
qbs_set(strz,qbs_add(strz,qbs_new_txt_len("\0",1)));
ZeroMemory( &si, sizeof(si) ); si.cb = sizeof(si); ZeroMemory( &pi, sizeof(pi) );
if(CreateProcess(
        NULL,           // No module name (use command line)
        (char*)&strz->chr[0], // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        DETACHED_PROCESS, // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
){
shell_call_in_progress=1;
// Wait until child process exits.
WaitForSingleObject( pi.hProcess, INFINITE );
// Close process and thread handles. 
CloseHandle( pi.hProcess );
CloseHandle( pi.hThread );
shell_call_in_progress=0;
goto shell_complete;
}
//attempt: command.com
qbs_set(strz,qbs_add(qbs_new_txt("command.com /c "),str));
qbs_set(strz,qbs_add(strz,qbs_new_txt_len("\0",1)));
ZeroMemory( &si, sizeof(si) ); si.cb = sizeof(si); ZeroMemory( &pi, sizeof(pi) );
if(CreateProcess(
        NULL,           // No module name (use command line)
        (char*)&strz->chr[0], // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        DETACHED_PROCESS, // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
){
shell_call_in_progress=1;
// Wait until child process exits.
WaitForSingleObject( pi.hProcess, INFINITE );
// Close process and thread handles. 
CloseHandle( pi.hProcess );
CloseHandle( pi.hThread );
shell_call_in_progress=0;
goto shell_complete;
}
//attempt: direct
qbs_set(strz,str);
qbs_set(strz,qbs_add(strz,qbs_new_txt_len("\0",1)));
ZeroMemory( &si, sizeof(si) ); si.cb = sizeof(si); ZeroMemory( &pi, sizeof(pi) );
if(CreateProcess(
        NULL,           // No module name (use command line)
        (char*)&strz->chr[0], // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        DETACHED_PROCESS, // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
){
shell_call_in_progress=1;
// Wait until child process exits.
WaitForSingleObject( pi.hProcess, INFINITE );
// Close process and thread handles. 
CloseHandle( pi.hProcess );
CloseHandle( pi.hThread );
shell_call_in_progress=0;
goto shell_complete;
}

#else

qbs_set(strz,qbs_add(str,qbs_new_txt_len("\0",1)));
shell_call_in_progress=1;
system((char*)strz->chr);
shell_call_in_progress=0;

#endif

}else{

#ifdef QB64_WINDOWS
//SHELL (with no parameters)
AllocConsole();
qbs_set(strz,qbs_new_txt_len("COMMAND\0",8));
shell_call_in_progress=1;
system((char*)strz->chr);
shell_call_in_progress=0;
FreeConsole();
#endif

}

shell_complete:
 //reenter full screen mode if necessary
 if (full_screen_mode){
 full_screen_set=full_screen_mode;
 do{
 Sleep(0);
 }while(!full_screen);
 }//full_screen_mode
}

//note: called using SHELL _HIDE option
void sub_shell2(qbs *str){
if (new_error) return;
static long i;
static qbs *strz=NULL;
if (!strz) strz=qbs_new(0,0);
if (!str->len){error(5); return;}//cannot launch a hidden console

#ifdef QB64_WINDOWS

static STARTUPINFO si;
static PROCESS_INFORMATION pi;
//attempt: CMD.EXE
qbs_set(strz,qbs_add(qbs_new_txt("cmd.exe /c "),str));
qbs_set(strz,qbs_add(strz,qbs_new_txt_len("\0",1)));
ZeroMemory( &si, sizeof(si) ); si.cb = sizeof(si); ZeroMemory( &pi, sizeof(pi) );
if(CreateProcess(
        NULL,           // No module name (use command line)
        (char*)&strz->chr[0], // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW, // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
){
shell_call_in_progress=1;
// Wait until child process exits.
WaitForSingleObject( pi.hProcess, INFINITE );
// Close process and thread handles. 
CloseHandle( pi.hProcess );
CloseHandle( pi.hThread );
shell_call_in_progress=0;
return;
}
//attempt: command.com
qbs_set(strz,qbs_add(qbs_new_txt("command.com /c "),str));
qbs_set(strz,qbs_add(strz,qbs_new_txt_len("\0",1)));
ZeroMemory( &si, sizeof(si) ); si.cb = sizeof(si); ZeroMemory( &pi, sizeof(pi) );
if(CreateProcess(
        NULL,           // No module name (use command line)
        (char*)&strz->chr[0], // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW, // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
){
shell_call_in_progress=1;
// Wait until child process exits.
WaitForSingleObject( pi.hProcess, INFINITE );
// Close process and thread handles. 
CloseHandle( pi.hProcess );
CloseHandle( pi.hThread );
shell_call_in_progress=0;
return;
}
//attempt: direct
qbs_set(strz,str);
qbs_set(strz,qbs_add(strz,qbs_new_txt_len("\0",1)));
ZeroMemory( &si, sizeof(si) ); si.cb = sizeof(si); ZeroMemory( &pi, sizeof(pi) );
if(CreateProcess(
        NULL,           // No module name (use command line)
        (char*)&strz->chr[0], // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW, // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
){
shell_call_in_progress=1;
// Wait until child process exits.
WaitForSingleObject( pi.hProcess, INFINITE );
// Close process and thread handles. 
CloseHandle( pi.hProcess );
CloseHandle( pi.hThread );
shell_call_in_progress=0;
return;
}

#else

qbs_set(strz,qbs_add(str,qbs_new_txt_len("\0",1)));
shell_call_in_progress=1;
system((char*)strz->chr);
shell_call_in_progress=0;
return;

#endif

}//shell2

void sub_kill(qbs *str){
if (new_error) return;
static qbs *strz=NULL;
if (!strz) strz=qbs_new(0,0);
qbs_set(strz,qbs_add(str,qbs_new_txt_len("\0",1)));
static long i;
if (remove((char*)strz->chr)){
i=errno;
if (i==ENOENT){error(53); return;}//file not found
if (i==EACCES){error(75); return;}//path/file access error
error(64);//bad file name (assumed)
}
}

void sub_name(qbs *oldname,qbs *newname){
if (new_error) return;
static qbs *strz=NULL;
if (!strz) strz=qbs_new(0,0);
static qbs *strz2=NULL;
if (!strz2) strz2=qbs_new(0,0);
static long i;
qbs_set(strz,qbs_add(oldname,qbs_new_txt_len("\0",1)));
qbs_set(strz2,qbs_add(newname,qbs_new_txt_len("\0",1)));
if (rename((char*)strz,(char*)strz2)){
i=errno;
if (i==ENOENT){error(53); return;}//file not found
if (i==EINVAL){error(64); return;}//bad file name
if (i==EACCES){error(75); return;}//path/file access error
error(5);//Illegal function call (assumed)
}
}

void sub_chdir(qbs *str){
if (new_error) return;
static qbs *strz=NULL;
if (!strz) strz=qbs_new(0,0);
qbs_set(strz,qbs_add(str,qbs_new_txt_len("\0",1)));
if (chdir((char*)strz->chr)==-1){
//assume errno==ENOENT
error(76);//path not found
}
}

void sub_mkdir(qbs *str){
if (new_error) return;
static qbs *strz=NULL;
if (!strz) strz=qbs_new(0,0);
qbs_set(strz,qbs_add(str,qbs_new_txt_len("\0",1)));
#ifdef QB64_LINUX
 if (mkdir((char*)strz->chr,0770)==-1){
#else
 if (mkdir((char*)strz->chr)==-1){
#endif
if (errno==EEXIST){error(75); return;}//path/file access error
//assume errno==ENOENT
error(76);//path not found
}

}

void sub_rmdir(qbs *str){
if (new_error) return;
static qbs *strz=NULL;
if (!strz) strz=qbs_new(0,0);
qbs_set(strz,qbs_add(str,qbs_new_txt_len("\0",1)));
if (rmdir((char*)strz->chr)==-1){
if (errno==ENOTEMPTY){error(75); return;}//path/file access error
//assume errno==ENOENT
error(76);//path not found
}
}

//Use FindFirstFile to creates a directory listing for QB FILES command

double pow2(double x,double y){
if (x<0){
if (y!=floor(y)){error(5); return 0;}
}
return pow(x,y);
}

long func_freefile(){
static long i;
for (i=1;i<=last_device_handle;i++){
if (!device_handle[i].open) return i;
}
error(5);
return 0;
}

void sub__mousehide(){
mouse_hideshow_called=1;
static int x,y;
SDL_GetMouseState(&x,&y);
SDL_WarpMouse(x,y);
SDL_ShowCursor(0);
SDL_WarpMouse(x,y);
}
void sub__mouseshow(){
mouse_hideshow_called=1;
static int x,y;
SDL_GetMouseState(&x,&y);
SDL_WarpMouse(x,y);
SDL_ShowCursor(1);
SDL_WarpMouse(x,y);
}

float func__mousex(){
static long x,x2;
static float f;
x=mouse_messages[current_mouse_message].x;
x-=x_offset;
if (x<0) x=0;
x/=x_scale;
x2=display_page->width; if (display_page->text) x2*=fontwidth[display_page->font];
if (x>=x2) x=x2-1;
if (display_page->text){
f=x;
x2=fontwidth[display_page->font];
x=x/x2+1;
f=f/(float)x2+0.5f;
 //if cint(f)<>x then adjust f so it does
 x2=qbr_float_to_long(f);
 if (x2>x) f-=0.001f;
 if (x2<x) f+=0.001f;
return f;
}
return x;
}

float func__mousey(){
static long y,y2;
static float f;
y=mouse_messages[current_mouse_message].y;
y-=y_offset;
if (y<0) y=0;
y/=y_scale;
y2=display_page->height; if (display_page->text) y2*=fontheight[display_page->font];
if (y>=y2) y=y2-1;
if (display_page->text){
f=y;
y2=fontheight[display_page->font];
y=y/y2+1;
f=f/(float)y2+0.5f;
 //if cint(f)<>y then adjust f so it does
 y2=qbr_float_to_long(f);
 if (y2>y) f-=0.001f;
 if (y2<y) f+=0.001f;
return f;
}
return y;
}

long func__mouseinput(){
if (current_mouse_message==last_mouse_message) return 0;
current_mouse_message++; if (current_mouse_message>1023) current_mouse_message=0;
return -1;
}

long func__mousebutton(long i){
if (i<1){error(5); return 0;}
if (i>3) return 0;//current SDL only supports 3 mouse buttons!
//swap indexes 2&3
if (i==2){
i=3;
}else{
if (i==3) i=2;
}
if (mouse_messages[current_mouse_message].buttons&(1<<(i-1))) return -1;
return 0;
}



uint16 call_absolute_offsets[256];
void call_absolute(long args,uint16 offset){
memset(&cpu,0,sizeof(cpu_struct));//flush cpu
cpu.cs=((defseg-cmem)>>4); cpu.ip=offset;
cpu.ss=0xFFFF; cpu.sp=0;//sp "loops" to <65536 after first push
cpu.ds=80;
//push (near) arg offsets
static long i;
for (i=0;i<args;i++){
cpu.sp-=2; *(uint16*)(cmem+cpu.ss*16+cpu.sp)=call_absolute_offsets[i];
}
//push ret segment, then push ret offset (both 0xFFFF to return control to QB64)
cpu.sp-=4; *(uint32*)(cmem+cpu.ss*16+cpu.sp)=0xFFFFFFFF;
cpu_call();
}

void call_interrupt(long i){

if (i==0x33){
if (cpu.ax==0){
cpu.ax=0xFFFF;//mouse installed
cpu.bx=2;
return;
}

if (cpu.ax==1){sub__mouseshow(); return;}
if (cpu.ax==2){sub__mousehide(); return;}
if (cpu.ax==3){
//return the current mouse status
cpu.bx=mouse_messages[last_mouse_message].buttons&1;
if (mouse_messages[last_mouse_message].buttons&4) cpu.bx+=2;
//assume screen 0
cpu.cx=mouse_messages[last_mouse_message].x;
cpu.dx=mouse_messages[last_mouse_message].y/2;
return;
}

if (cpu.ax==7){//horizontal min/max
return;
}
if (cpu.ax==8){//vertical min/max
return;
}


//MessageBox(NULL,"Unknown MOUSE Sub-function","Call Interrupt Error",MB_OK);
//exit(cpu.ax);


return;
}

}


//unsigned char *soundwave(double frequency,double length,double volume,double fadein,double fadeout,unsigned char *data);
//unsigned char *soundwavesilence(double length,unsigned char *data);


/*
Formats:
A[#|+|-][0-64]
   0-64 is like temp. Lnumber, 0 is whatever the current default is




*/
void sub_play(qbs *str){
sndsetup();
static unsigned char *b,*wave,*wave2,*wave3;
static double d;
static long i,bytes_left,a,x,x2,x3,x4,x5,wave_bytes,wave_base;
static long o=4;
static double t=120;//quarter notes per minute (120/60=2 per second)
static double l=4;
static double pause=1.0/8.0;//ML 0.0, MN 1.0/8.0, MS 1.0/4.0
static double length,length2;//derived from l and t
static double frequency;
static double mb=0;
static double v=50;

static long n;//the semitone-intervaled note to be played
static long n_changed;//+,#,- applied?
static int64 number;
static long number_entered;
static long followup;//1=play note
static long playit;
static unsigned long handle=NULL;
static long fullstops=0;
b=str->chr;
bytes_left=str->len;
wave=NULL;
wave_bytes=0;
n_changed=0;
n=0;
number_entered=0;
number=0;
followup=0;
length=1.0/(t/60.0)*(4.0/l);
playit=0;
wave_base=0;//point at which new sounds will be inserted

next_byte:
if ((bytes_left--)||followup){

if (bytes_left<0){i=32; goto follow_up;}

i=*b++;
if (i==32) goto next_byte;
if (i>=97&&i<=122) a=i-32; else a=i;

if (i==61){//= (+VARPTR$)
if (fullstops){error(5); return;}
if (number_entered){error(5); return;}
number_entered=2;
//VARPTR$ reference
/*
'BYTE=1
'INTEGER=2
'STRING=3 SUB-STRINGS must use "X"+VARPTR$(string$)
'SINGLE=4
'INT64=5
'FLOAT=6
'DOUBLE=8
'LONG=20
'BIT=64+n
*/
if (bytes_left<3){error(5); return;}
i=*b++; bytes_left--;//read type byte
x=*(unsigned short*)b; b+=2; bytes_left-=2;//read offset within DBLOCK
//note: allowable _BIT type variables in VARPTR$ are all at a byte offset and are all
//      padded until the next byte
d=0;
switch(i){
case 1:
d=*(char*)(dblock+x);
break;
case (1+128):
d=*(unsigned char*)(dblock+x);
break;
case 2:
d=*(short*)(dblock+x);
break;
case (2+128):
d=*(unsigned short*)(dblock+x);
break;
case 4:
d=*(float*)(dblock+x);
break;
case 5:
d=*(int64*)(dblock+x);
break;
case (5+128):
d=*(int64*)(dblock+x); //unsigned conversion is unsupported!
break;
case 6:
d=*(long double*)(dblock+x);
break;
case 8:
d=*(double*)(dblock+x);
break;
case 20:
d=*(long*)(dblock+x);
break;
case (20+128):
d=*(unsigned long*)(dblock+x);
break;
default:
//bit type?
if ((i&64)==0){error(5); return;}
x2=i&63;
if (x2>56){error(5); return;}//valid number of bits?
//create a mask
static int64 i64num,mask,i64x;
mask=(1<<x2)-1;
i64num=(*(int64*)(dblock+x))&mask;
//signed?
if (i&128){
mask=1<<(x2-1);
if (i64num&mask){//top bit on?
mask=-1; mask<<=x2; i64num+=mask;
}
}//signed
d=i64num;
}
if (d>2147483647.0||d<-2147483648.0){error(5); return;}//out of range value!
number=qbr_double_to_long(d);
goto next_byte;
}

//read in a number
if ((i>=48)&&(i<=57)){
if (fullstops||(number_entered==2)){error(5); return;}
if (!number_entered){number=0; number_entered=1;}
number=number*10+i-48;
goto next_byte;
}

//read fullstops
if (i==46){
if (followup!=7&&followup!=1&&followup!=4){error(5); return;}
fullstops++;
goto next_byte;
}

follow_up:

if (followup==8){//V...
if (!number_entered){error(5); return;}
number_entered=0;
if (number>100){error(5); return;}
v=number;
followup=0; if (bytes_left<0) goto done;
}//8

if (followup==7){//P...
if (number_entered){
number_entered=0;
if (number<1||number>64){error(5); return;}
length2=1.0/(t/60.0)*(4.0/((double)number));
}else{
length2=length;
}
d=length2; for (x=1;x<=fullstops;x++){d/=2.0; length2=length2+d;} fullstops=0;

soundwave_bytes=wavesize(length2);
if (!wave){
 //create buffer
 wave=(unsigned char*)calloc(soundwave_bytes,1); wave_bytes=soundwave_bytes;
 wave_base=0;
}else{
 //increase buffer?
 if ((wave_base+soundwave_bytes)>wave_bytes){
 wave=(unsigned char*)realloc(wave,wave_base+soundwave_bytes);
 memset(wave+wave_base,0,wave_base+soundwave_bytes-wave_bytes);
 wave_bytes=wave_base+soundwave_bytes;
 }
}
if (i!=44){
wave_base+=soundwave_bytes;
}

playit=1;
followup=0;
if (i==44) goto next_byte;
if (bytes_left<0) goto done;
}//7

if (followup==6){//T...
if (!number_entered){error(5); return;}
number_entered=0;
if (number<32||number>255){error(5); return;}
t=number;
length=1.0/(t/60.0)*(4.0/l);
followup=0; if (bytes_left<0) goto done;
}//6

if (followup==5){//M...
if (number_entered){error(5); return;}
switch(a){
case 76://L
pause=0;
break;
case 78://N
pause=1.0/8.0;
break;
case 83://S
pause=1.0/4.0;
break;

case 66://MB
if (!mb){
mb=1;
if (playit){
 handle=func__sndraw(wave,wave_bytes);
 if (handle){
  sndqueue[sndqueue_next]=handle;
  sndqueue_next++; if (sndqueue_next>sndqueue_lastindex) sndqueue_next=0;
 }else{free(wave);}
 wave=(unsigned char*)calloc(4,1); handle=func__sndraw(wave,4);
 if (handle){
  sndqueue_wait=sndqueue_next; suspend_program|=2; qbevent=1;
  sndqueue[sndqueue_next]=handle;
  sndqueue_next++; if (sndqueue_next>sndqueue_lastindex) sndqueue_next=0;
 }else{free(wave);}
}
playit=0;
wave=NULL;
}
break;
case 70://MF
if (mb){
mb=0;
//preceding MB content incorporated into MF block
}
break;
default:
error(5); return;
}
followup=0; goto next_byte;
}//5

if (followup==4){//N...
if (!number_entered){error(5); return;}
number_entered=0;
if (number>84){error(5); return;}
n=-33+number;
goto followup1;
followup=0; if (bytes_left<0) goto done;
}//4

if (followup==3){//O...
if (!number_entered){error(5); return;}
number_entered=0;
if (number>6){error(5); return;}
o=number;
followup=0; if (bytes_left<0) goto done;
}//3

if (followup==2){//L...
if (!number_entered){error(5); return;}
number_entered=0;
if (number<1||number>64){error(5); return;}
l=number;
length=1.0/(t/60.0)*(4.0/l);
followup=0; if (bytes_left<0) goto done;
}//2

if (followup==1){//A-G...
if (i==45){//-
 if (n_changed||number_entered){error(5); return;}
 n_changed=1; n--;
 goto next_byte;
}
if (i==43||i==35){//+,#
 if (n_changed||number_entered){error(5); return;}
 n_changed=1; n++;
goto next_byte;
}
followup1:
if (number_entered){
 number_entered=0;
 if (number<0||number>64){error(5); return;}
 if (!number) length2=length; else length2=1.0/(t/60.0)*(4.0/((double)number));
}else{
 length2=length;
}//number_entered
d=length2; for (x=1;x<=fullstops;x++){d/=2.0; length2=length2+d;} fullstops=0;
//frequency=(2^(note/12))*440
frequency=pow(2.0,((double)n)/12.0)*440.0;

//create wave
wave2=soundwave(frequency,length2*(1.0-pause),v/100.0,NULL,NULL);
if (pause>0){
wave2=(unsigned char*)realloc(wave2,soundwave_bytes+wavesize(length2*pause));
memset(wave2+soundwave_bytes,0,wavesize(length2*pause));
soundwave_bytes+=wavesize(length2*pause);
}

if (!wave){
 //adopt buffer
 wave=wave2; wave_bytes=soundwave_bytes;
 wave_base=0;
}else{
 //mix required?
 if (wave_base==wave_bytes) x=0; else x=1;
 //increase buffer?
 if ((wave_base+soundwave_bytes)>wave_bytes){
 wave=(unsigned char*)realloc(wave,wave_base+soundwave_bytes);
 memset(wave+wave_base,0,wave_base+soundwave_bytes-wave_bytes);
 wave_bytes=wave_base+soundwave_bytes;
 }
 //mix or copy
 if (x){
  //mix
  static short *sp,*sp2;
  sp=(short*)(wave+wave_base);
  sp2=(short*)wave2;
  x2=soundwave_bytes/2;
  for (x=0;x<x2;x++){
  x3=*sp2++;
  x4=*sp;
  x4+=x3;
  if (x4>32767) x4=32767;
  if (x4<-32767) x4=-32767;
  *sp++=x4;
  }//x 
 }else{
  //copy
  memcpy(wave+wave_base,wave2,soundwave_bytes);  
 }//x
 free(wave2);
}
if (i!=44){
wave_base+=soundwave_bytes;
}

playit=1;
n_changed=0;
followup=0; 
if (i==44) goto next_byte;
if (bytes_left<0) goto done;
}//1

if (a>=65&&a<=71){
//modify a to represent a semitonal note (n) interval
switch(a){
//[c][ ][d][ ][e][f][ ][g][ ][a][ ][b]
// 0  1  2  3  4  5  6  7  8  9  0  1
case 65: n=9; break;
case 66: n=11; break;
case 67: n=0; break;
case 68: n=2; break;
case 69: n=4; break;
case 70: n=5; break;
case 71: n=7; break;
}
n=n+(o-2)*12-9;
followup=1;
goto next_byte;
}//a

if (a==76){//L
followup=2;
goto next_byte;
}

if (a==77){//M
followup=5;
goto next_byte;
}

if (a==78){//N
followup=4;
goto next_byte;
}

if (a==79){//O
followup=3;
goto next_byte;
}

if (a==84){//T
followup=6;
goto next_byte;
}

if (a==60){//<
o--; if (o<0) o=0;
goto next_byte;
}

if (a==62){//>
o++; if (o>6) o=6;
goto next_byte;
}

if (a==80){//P
followup=7;
goto next_byte;
}

if (a==86){//V
followup=8;
goto next_byte;
}

error(5); return;
}//bytes_left
done:
if (number_entered||followup){error(5); return;}//unhandled data




if (playit){
handle=func__sndraw(wave,wave_bytes);
if (handle){
sndqueue[sndqueue_next]=handle;
sndqueue_next++; if (sndqueue_next>sndqueue_lastindex) sndqueue_next=0;
}

//creating a "blocking" sound object
if (!mb){
wave=(unsigned char*)calloc(4,1); handle=func__sndraw(wave,4);
if (handle){
sndqueue_wait=sndqueue_next; suspend_program|=2; qbevent=1;
sndqueue[sndqueue_next]=handle;
sndqueue_next++; if (sndqueue_next>sndqueue_lastindex) sndqueue_next=0;
}else{free(wave);}

}//!mb




}//playit

}



//2D PROTOTYPE QB64<->C CALLS

//Creating/destroying an image surface:

long func__newimage(long x,long y,long bpp,long passed){
static long i;
if (new_error) return 0;
if (x<=0||y<=0){error(5); return 0;}
if (!passed){
bpp=write_page->compatible_mode;
}else{
i=0;
if (bpp>=0&&bpp<=2) i=1;
if (bpp>=7&&bpp<=13) i=1;
if (bpp==256) i=1;
if (bpp==32) i=1;
if (!i){error(5); return 0;}
}
i=imgnew(x,y,bpp);
if (!i) return -1;
if (!passed){
//adopt palette
if (write_page->pal){
memcpy(img[i].pal,write_page->pal,1024);
}
//adopt font
sub__font(write_page->font,-i,1);
//adopt colors
img[i].color=write_page->color;
img[i].background_color=write_page->background_color;
//adopt transparent color
img[i].transparent_color=write_page->transparent_color;
//adopt blend state
img[i].alpha_disabled=write_page->alpha_disabled;
//adopt print mode
img[i].print_mode=write_page->print_mode;
}
return -i;
}

long func__loadimage(qbs *f,long bpp,long passed){
static qbs *tqbs=NULL,*nullt=NULL;
static long i;
if (new_error) return 0;
//validate bpp
if (passed){
if ((bpp!=32)&&(bpp!=256)){error(5); return 0;}
}else{
if (write_page->text){error(5); return 0;}
bpp=-1;
}
if (!f->len) return -1;//return invalid handle if null length string
if (!tqbs) tqbs=qbs_new(0,0);
if (!nullt){nullt=qbs_new(1,0); nullt->chr[0]=0;}
qbs_set(tqbs,qbs_add(f,nullt));
i=imgload((char*)tqbs->chr,bpp);
if (!i) return -1;//failed
return -i;
}

long func__copyimage(long i,long passed){
static long i2,bytes;
static img_struct *s,*d;
if (new_error) return 0;
if (passed){
  if (i>=0){//validate i
  validatepage(i); i=page[i];
  }else{
  i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
 }else{
 i=write_page_index;
 }
s=&img[i];
//duplicate structure
i2=newimg();
d=&img[i2];
memcpy(d,s,sizeof(img_struct));
//duplicate pixel data
bytes=d->width*d->height*d->bytes_per_pixel;
d->offset=(unsigned char*)malloc(bytes);
if (!d->offset){freeimg(i2); return -1;}
memcpy(d->offset,s->offset,bytes);
d->flags|=IMG_FREEMEM;
//duplicate palette
if (d->pal){
d->pal=(unsigned long*)malloc(1024);
if (!d->pal){free(d->offset); freeimg(i2); return -1;}
memcpy(d->pal,s->pal,1024);
d->flags|=IMG_FREEPAL;
}
//adjust flags
if (d->flags&IMG_SCREEN)d->flags^=IMG_SCREEN;
//return new handle
return -i2;
}

void sub__freeimage(long i,long passed){
if (new_error) return;
 if (passed){
  if (i>=0){//validate i
  error(5); return;//The SCREEN's pages cannot be freed!
  }else{
  i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;}
 }
 }else{
 i=write_page_index;
 }
if (img[i].flags&IMG_SCREEN){error(5); return;}//The SCREEN's pages cannot be freed!
if (write_page_index==i) sub__dest(-display_page_index);
if (read_page_index==i) sub__source(-display_page_index);
if (img[i].flags&IMG_FREEMEM) free(img[i].offset);//free pixel data
if (img[i].flags&IMG_FREEPAL) free(img[i].pal);//free palette
freeimg(i);
}

//Selecting images:

void sub__source(long i){ 
if (new_error) return;
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;} 
 }
read_page_index=i; read_page=&img[i];
}

void sub__dest(long i){ 
if (new_error) return;
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;} 
 }
write_page_index=i; write_page=&img[i];
}

long func__source(){
return -read_page_index;
}

long func__dest(){
return -write_page_index;
}

long func__display(){
return -display_page_index;
}

//Changing the settings of an image surface:

void sub__blend(long i,long passed){
if (new_error) return;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;} 
 }
}else{
i=write_page_index;
}
if (img[i].bytes_per_pixel!=4){error(5); return;}
img[i].alpha_disabled=0;
}

void sub__dontblend(long i,long passed){
if (new_error) return;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;} 
 }
}else{
i=write_page_index;
}
if (img[i].bytes_per_pixel!=4) return;
img[i].alpha_disabled=1;
}


void sub__clearcolor(long none,unsigned long c,long i,long passed){
//-->                                        1      2
//_IMGHIDECOLOR "[{_NONE}][?][,?]"
if (new_error) return;
static img_struct *im;
static long z;
static unsigned long *lp,*last;
static unsigned char b_max,b_min,g_max,g_min,r_max,r_min;
static unsigned char *cp,*clast,v;
if (passed&2){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;} 
 }
}else{
i=write_page_index;
}
im=&img[i];
if (im->text){
if (none&&(!(passed&1))) return;
error(5); return;
}
//palette?
if (im->pal){
if (passed&2){error(5); return;}//invalid options
if (none){
if (passed&1){error(5); return;}//invalid options
im->transparent_color=-1;
return;
}
if (!(passed&1)){error(5); return;}//invalid options
if (c>255){error(5); return;}//invalid color
im->transparent_color=c;
return;
}
//32-bit (but alpha is ignored in this case)
if (none){
if (passed&1){error(5); return;}//invalid options
return;//no action
}
if (!(passed&1)){error(5); return;}//invalid options
c&=0xFFFFFF;
last=im->offset32+im->width*im->height;
for (lp=im->offset32;lp<last;lp++){
if ((*lp&0xFFFFFF)==c) *lp=c;
}
return;
}

//Changing/Using an image surface:

//_PUT "[(?,?)[-(?,?)]][,[?][,[?][,[(?,?)[-(?,?)]]]]]"
//(defined elsewhere)

//_IMGALPHA "?[,[?[{TO}?]][,?]]"
void sub__setalpha(long a,unsigned long c,unsigned long c2,long i,long passed){
//-->                                1               2              4
static img_struct *im;
static long z;
static unsigned long *lp,*last;
static unsigned char b_max,b_min,g_max,g_min,r_max,r_min,a_max,a_min;
static unsigned char *cp,*clast,v;
if (new_error) return;
if (passed&4){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;} 
 }
}else{
i=write_page_index;
}
im=&img[i];
if (im->pal){error(5); return;}//does not work on paletted images!
if (a<0||a>255){error(5); return;}//invalid range
if (passed&2){
//ranged
if (c==c2) goto uniquerange;
b_min=c&0xFF;  g_min=c>>8&0xFF;  r_min=c>>16&0xFF; a_min=c>>24&0xFF;
b_max=c2&0xFF; g_max=c2>>8&0xFF; r_max=c2>>16&0xFF; a_max=c2>>24&0xFF;
if (b_min>b_max) swap(b_min,b_max);
if (g_min>g_max) swap(g_min,g_max);
if (r_min>r_max) swap(r_min,r_max);
if (a_min>a_max) swap(a_min,a_max);
cp=im->offset;
z=im->width*im->height;
setalpha:
if (z--){
v=*cp; if (v<=b_max&&v>=b_min){
v=*(cp+1); if (v<=g_max&&v>=g_min){
v=*(cp+2); if (v<=r_max&&v>=r_min){
v=*(cp+3); if (v<=a_max&&v>=a_min){
*(cp+3)=a;
}}}}
cp+=4;
goto setalpha;
}
return;
}
if (passed&1){
uniquerange:
//alpha of c=a
c2=a<<24;
lp=im->offset32-1;
last=im->offset32+im->width*im->height-1;
while (lp<last){
if (*++lp==c){
*lp=(*lp&0xFFFFFF)|c2;
}
}
return;
}
//all alpha=a
cp=im->offset-1;
clast=im->offset+im->width*im->height*4-4;
while (cp<clast){*(cp+=4)=a;}
return;
}

//Finding infomation about an image surface:

long func__width(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
return img[i].width;
}

long func__height(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
return img[i].height;
}

long func__pixelsize(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
i=img[i].compatible_mode;
if (i==32) return 4;
if (!i) return 0;
return 1;
}

long func__clearcolor(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
if (img[i].text) return -1;
if (img[i].compatible_mode==32) return 0;
return img[i].transparent_color;
}

long func__blend(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
if (img[i].compatible_mode==32){
if (!img[i].alpha_disabled) return -1;
}
return 0;
}

unsigned long func__defaultcolor(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
return img[i].color;
}

unsigned long func__backgroundcolor(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
return img[i].background_color;
}

//Working with 256 color palettes:

unsigned long func__palettecolor(long n,long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
if (!img[i].pal){error(5); return 0;}
if (n<0||n>255){error(5); return 0;}//out of range
return img[i].pal[n]|0xFF000000;
}

void sub__palettecolor(long n,unsigned long c,long i,long passed){
if (new_error) return;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;}
 }
}else{
i=write_page_index;
}
if (!img[i].pal){error(5); return;}
if (n<0||n>255){error(5); return;}//out of range
img[i].pal[n]=c;
}

void sub__copypalette(long i,long i2,long passed){
if (new_error) return;
if (passed&1){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;}
 }
}else{
i=read_page_index;
}
if (!img[i].pal){error(5); return;}
swap(i,i2);
if (passed&2){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;}
 }
}else{
i=write_page_index;
}
if (!img[i].pal){error(5); return;}
swap(i,i2);
memcpy(img[i2].pal,img[i].pal,1024);
}

void sub__printstring(long step,double f_x,double f_y,qbs* text,long i,long passed){
if (new_error) return;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;}
 }
}else{
i=write_page_index;
}
static img_struct *im;
im=&img[i];
if (im->text){error(5); return;}//graphics modes only
if (!text->len) return;
//default is no kerning, assume kerning is off
static long x,y,i2,w;
x=f_x; y=f_y;
for (i2=0;i2<text->len;i2++){
w=printchr2(x,y,text->chr[i2],i);
x=x+w;
}
}

long func__printwidth(qbs* text,long i,long passed){
if (new_error) return 0;
static long i2,w2;
static unsigned long character;
static unsigned long x2,w,f;
static SDL_Surface *ts;
static SDL_Color c,c2;
static img_struct *im;

if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
im=&img[i];
if (im->text){error(5); return 0;}//graphics modes only
if (!text->len) return 0;

//default is no kerning, assume kerning is off

w2=0;
for (i2=0;i2<text->len;i2++){
character=text->chr[i2];

//precalculations
character&=255;
f=im->font;
c.r=255; c.g=255; c.b=255; c.unused=0;//dummy values
c2.r=255; c2.g=255; c2.b=255; c2.unused=0;//dummy values

if (f>=32){//custom font

//8-bit / alpha-disabled 32-bit / dont-blend(alpha may still be applied)
if ((im->bytes_per_pixel==1)||((im->bytes_per_pixel==4)&&(im->alpha_disabled))||(fontflags[f]&8)){
ts=TTF_RenderText_Solid(font[f],(char*)&character,c);//8-bit, 0=clear, 1=text
if (ts==NULL) return 0;
w=ts->w;
if (x2=fontwidth[f]) if (x2!=w) w=x2;//render width too large!  
SDL_FreeSurface(ts);
w2+=w;
goto nexti2;
}//1-8 bit

//assume 32-bit blended
//8 bit, 0=background -> 255=foreground
ts=TTF_RenderText_Shaded(font[f],(char*)&character,c,c2);
if (ts==NULL) return 0;
w=ts->w;
if (x2=fontwidth[f]) if (x2!=w) w=x2;//render width too large!  
SDL_FreeSurface(ts);
w2+=w;
goto nexti2;

}//custom font

//default fonts
w2+=8;

nexti2:;
}//i2

return w2;
}//printwidth


long func__loadfont(qbs *filename,double size,qbs *requirements,long passed){
//f=_FONTLOAD(ttf_filename$,height[,"bold,italic,underline,monospace,dontblend"])
//1 bold TTF_STYLE_BOLD
//2 italic TTF_STYLE_ITALIC
//4 underline TTF_STYLE_UNDERLINE
//8 dontblend (blending is the default in 32-bit alpha-enabled modes)
//16 monospace
if (new_error) return 0;
static qbs *s1=NULL;
if (!s1) s1=qbs_new(0,0);
static qbs *req=NULL;
if (!req) req=qbs_new(0,0);
static qbs *s3=NULL;
if (!s3) s3=qbs_new(0,0);
static unsigned char r[32];
static long i,i2,i3;
//validate
if (size<0.001){error(5); return 0;}
if (size>=1000.5) return 0;
memset(r,0,32);
if (passed){
if (requirements->len){
i=1;
qbs_set(req,qbs_ucase(requirements));//convert tmp str to perm str
nextrequirement:
i2=func_instr(i,req,qbs_new_txt(","),1);
if (i2){
qbs_set(s1,func_mid(req,i,i2-i,1));
}else{
qbs_set(s1,func_mid(req,i,req->len-i+1,1));
}
if (qbs_equal(s1,qbs_new_txt("BOLD"))){r[0]++; goto valid;}
if (qbs_equal(s1,qbs_new_txt("ITALIC"))){r[1]++; goto valid;}
if (qbs_equal(s1,qbs_new_txt("UNDERLINE"))){r[2]++; goto valid;}
if (qbs_equal(s1,qbs_new_txt("DONTBLEND"))){r[3]++; goto valid;}
if (qbs_equal(s1,qbs_new_txt("MONOSPACE"))){r[4]++; goto valid;}
error(5); return 0;//invalid requirements
valid:
if (i2){i=i2+1; goto nextrequirement;}
for (i=0;i<32;i++) if (r[i]>1){error(5); return 0;}//cannot define requirements twice
}//->len
}//passed
qbs_set(s1,qbs_add(filename,qbs_new_txt_len("\0",1)));//s1=filename+CHR$(0)
i=r[0]+(r[1]<<1)+(r[2]<<2)+(r[3]<<3)+(r[4]<<4);
i=fontopen((char*)filename->chr,size,i);
if (!i) return -1;
return i;
}

void sub__font(long f,long i,long passed){
//_FONT "?[,?]"
static long i2;
static img_struct *im;
if (new_error) return;
if (passed&1){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;}
 }
}else{
i=write_page_index;
}
im=&img[i];
//validate f
i2=0;
if (f==8) i2=1;
if (f==14) i2=1;
if (f==16) i2=1;
if (f>=32&&f<=lastfont){
if (font[f]) i2=1;
}
if (!i2){error(258); return;}
if (im->text&&((fontflags[f]&16)==0)){error(5); return;}//only monospace fonts can be used on text surfaces
im->font=f;
im->cursor_x=1; im->cursor_y=1;
im->top_row=1;
if (im->compatible_mode) im->bottom_row=im->height/fontheight[f]; else im->bottom_row=im->height;
im->bottom_row--; if (im->bottom_row<=0) im->bottom_row=1;
return;
}

long func__fontwidth(long f,long passed){
static long i2;
if (new_error) return 0;
if (passed){
//validate f
i2=0;
if (f==8) i2=1;
if (f==14) i2=1;
if (f==16) i2=1;
if (f>=32&&f<=lastfont){
if (font[f]) i2=1;
}
if (!i2){error(258); return 0;}
}else{
f=write_page->font;
}
return fontwidth[f];
}

long func__fontheight(long f,long passed){
static long i2;
if (new_error) return 0;
if (passed){
//validate f
i2=0;
if (f==8) i2=1;
if (f==14) i2=1;
if (f==16) i2=1;
if (f>=32&&f<=lastfont){
if (font[f]) i2=1;
}
if (!i2){error(258); return 0;}
}else{
f=write_page->font;
}
return fontheight[f];
}

long func__font(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
return img[i].font;
}

void sub__freefont(long f){
if (new_error) return;
static long i,i2;
//validate f (cannot remove QBASIC built in fonts!)
i2=0;
if (f>=32&&f<=lastfont){
if (font[f]) i2=1;
}
if (!i2){error(258); return;}
//check all surfaces, no surface can be using the font
for (i=1;i<nextimg;i++){
if (img[i].valid){
if (img[i].font==f){error(5); return;}
}
}
//remove font
TTF_CloseFont(font[f]);
font[f]=NULL;
}

void sub__printmode(long mode,long i,long passed){
if (new_error) return;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return;} if (!img[i].valid){error(258); return;}
 }
}else{
i=write_page_index;
}
if (img[i].text){
if (mode!=1){error(5); return;}
}
if (mode==1) img[i].print_mode=3;//fill
if (mode==2) img[i].print_mode=1;//keep
if (mode==3) img[i].print_mode=2;//only
}

long func__printmode(long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
}else{
i=write_page_index;
}
return img[i].print_mode;
}


unsigned long matchcol(long r,long g,long b){
static long v,v2,n,n2,best,c;
static long *p;
p=(long*)write_page->pal;
if (write_page->text) n2=16; else n2=write_page->mask+1;
v=1000;
best=0;
for (n=0;n<n2;n++){
c=*p++;
v2=abs(b-(c&0xFF))+abs(g-(c>>8&0xFF))+abs(r-(c>>16&0xFF));
if (v2<v){
if (!v2) return n;//perfect match
v=v2;
best=n;
}
}//n
return best;
}

unsigned long matchcol(long r,long g,long b,long i){
static long v,v2,n,n2,best,c;
static long *p;
p=(long*)img[i].pal;
if (img[i].text) n2=16; else n2=img[i].mask+1;
v=1000;
best=0;
for (n=0;n<n2;n++){
c=*p++;
v2=abs(b-(c&0xFF))+abs(g-(c>>8&0xFF))+abs(r-(c>>16&0xFF));
if (v2<v){
if (!v2) return n;//perfect match
v=v2;
best=n;
}
}//n
return best;
}

unsigned long func__rgb(long r,long g,long b,long i,long passed){
if (new_error) return 0;
if (r<0) r=0;
if (r>255) r=255;
if (g<0) g=0;
if (g>255) g=255;
if (b<0) b=0;
if (b>255) b=255;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
 if (img[i].bytes_per_pixel==4){
 return (r<<16)+(g<<8)+b|0xFF000000;
 }else{//==4
 return matchcol(r,g,b,i);
 }//==4
}else{
 if (write_page->bytes_per_pixel==4){
 return (r<<16)+(g<<8)+b|0xFF000000;
 }else{//==4
 return matchcol(r,g,b);
 }//==4
}//passed
}//rgb

unsigned long func__rgba(long r,long g,long b,long a,long i,long passed){
if (new_error) return 0;
if (r<0) r=0;
if (r>255) r=255;
if (g<0) g=0;
if (g>255) g=255;
if (b<0) b=0;
if (b>255) b=255;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
 if (img[i].bytes_per_pixel==4){
 return (a<<24)+(r<<16)+(g<<8)+b;
 }else{//==4
 //error(5); return 0;
 if ((!a)&&(img[i].transparent_color!=-1)) return img[i].transparent_color;
 return matchcol(r,g,b,i);
 }//==4
}else{
 if (write_page->bytes_per_pixel==4){
 return (a<<24)+(r<<16)+(g<<8)+b;
 }else{//==4
 //error(5); return 0;
 if ((!a)&&(write_page->transparent_color!=-1)) return write_page->transparent_color;
 return matchcol(r,g,b);
 }//==4
}//passed
}//rgba

long func__alpha(unsigned long col,long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
 if (img[i].bytes_per_pixel==4){
 return col>>24;
 }else{//==4
 //error(5); return 0; 
 if ((col<0)||(col>(img[i].mask))){error(5); return 0;} 
 if (img[i].transparent_color==col) return 0;
 return 255;
 }//==4
}else{
 if (write_page->bytes_per_pixel==4){
 return col>>24;
 }else{//==4
 //error(5); return 0; 
 if ((col<0)||(col>(write_page->mask))){error(5); return 0;} 
 if (write_page->transparent_color==col) return 0;
 return 255;
 }//==4
}//passed
}

long func__red(unsigned long col,long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
 if (img[i].bytes_per_pixel==4){
 return col>>16&0xFF;
 }else{//==4
 if ((col<0)||(col>(img[i].mask))){error(5); return 0;}
 return img[i].pal[col]>>16&0xFF;
 }//==4
}else{
 if (write_page->bytes_per_pixel==4){
 return col>>16&0xFF;
 }else{//==4
 if ((col<0)||(col>(write_page->mask))){error(5); return 0;}
 return write_page->pal[col]>>16&0xFF;
 }//==4
}//passed
}

long func__green(unsigned long col,long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
 if (img[i].bytes_per_pixel==4){
 return col>>8&0xFF;
 }else{//==4
 if ((col<0)||(col>(img[i].mask))){error(5); return 0;}
 return img[i].pal[col]>>8&0xFF;
 }//==4
}else{
 if (write_page->bytes_per_pixel==4){
 return col>>8&0xFF;
 }else{//==4
 if ((col<0)||(col>(write_page->mask))){error(5); return 0;}
 return write_page->pal[col]>>8&0xFF;
 }//==4
}//passed
}

long func__blue(unsigned long col,long i,long passed){
if (new_error) return 0;
if (passed){
 if (i>=0){//validate i
 validatepage(i); i=page[i];
 }else{
 i=-i; if (i>=nextimg){error(258); return 0;} if (!img[i].valid){error(258); return 0;}
 }
 if (img[i].bytes_per_pixel==4){
 return col&0xFF;
 }else{//==4
 if ((col<0)||(col>(img[i].mask))){error(5); return 0;}
 return img[i].pal[col]&0xFF;
 }//==4
}else{
 if (write_page->bytes_per_pixel==4){
 return col&0xFF;
 }else{//==4
 if ((col<0)||(col>(write_page->mask))){error(5); return 0;}
 return write_page->pal[col]&0xFF;
 }//==4
}//passed
}

void sub_end(){
//1. set the display page as the destination page
sub__dest(func__display());
//2. VIEW PRINT bottomline,bottomline
static long y;
if (write_page->text){
 y=write_page->height;
}else{
 y=write_page->height/fontheight[write_page->font];
}
qbg_sub_view_print(y,y,1|2);
//3. PRINT 'clears the line without having to worry about its contents/size
qbs_print(nothingstring,1);
//4. PRINT "Press any key to continue"
qbs_print(qbs_new_txt("Press any key to continue"),0);
//5. Clear any buffered keypresses
static unsigned long qbs_tmp_base;
qbs_tmp_base=qbs_tmp_list_nexti;
while(qbs_cleanup(qbs_tmp_base,qbs_notequal(qbs_inkey(),qbs_new_txt("")))){
SDL_Delay(0);
}
//6. Wait for a new keypress
do{
SDL_Delay(0);
if (stop_program) end();
}while(qbs_cleanup(qbs_tmp_base,qbs_equal(qbs_inkey(),qbs_new_txt(""))));
//7. end program
close_program=1;
end();
exit(0);//<-- should never happen
}

unsigned char pu_dig[1024];//digits (left justified)
long pu_ndig;//number of digits
long pu_dp;//decimal place modifier
//note: if dp=0, the number is an integer and can be read as is
//      if dp=1 the number is itself*10
//      if dp=-1 the number is itself/10
long pu_neg;
unsigned char pu_buf[1024];//a buffer for preprocessing
unsigned char pu_exp_char=69; //"E"

long print_using(qbs *f, long s2, qbs *dest, qbs* pu_str){
//type: 1=numeric, 2=string
if (new_error) return 0;
static long x,x2,s,stage,len,chrsleft,z,z2,z3,i,i2,i3,type;
static unsigned char c;
static long leading_plus,dollar_sign,asterisk_spaces,digits_before_point,commas;
static long decimal_point,digits_after_point,trailing_plus,exponent_digits, trailing_minus;
static long cant_fit,extra_sign_space,rounded,digits_and_commas_before_point,leading_zero;
static qbs *qbs1=NULL;
if (qbs1==NULL) qbs1=qbs_new(1,0);

if (pu_str) type=2; else type=1;



s=s2;
len=f->len;

scan:
rounded=0;
rounded_repass:

x=s-1; //subtract one to counter pre-increment later
leading_plus=0; dollar_sign=0; asterisk_spaces=0; digits_before_point=0; commas=0;
decimal_point=0; digits_after_point=0; trailing_plus=0; exponent_digits=0; trailing_minus=0;
digits_and_commas_before_point=0; leading_zero=0;
stage=0;

nextchar:
x++;
if (x<len){
c=f->chr[x];
chrsleft=len-x;

if ((stage>=2)&&(stage<=4)){

if (c==43){//+
trailing_plus=1; x++; goto numeric_spacer;
}

if (c==45){//-
trailing_minus=1; x++; goto numeric_spacer;
}

}//stage>=2 & stage<=4

if ((stage>=2)&&(stage<=3)){

if (chrsleft>=5){
if ((c==94)&&(f->chr[x+1]==94)&&(f->chr[x+2]==94)&&(f->chr[x+3]==94)&&(f->chr[x+4]==94)){//^^^^^
exponent_digits=3; stage=4; x+=4; goto nextchar;
}
}//5

if (chrsleft>=4){
if ((c==94)&&(f->chr[x+1]==94)&&(f->chr[x+2]==94)&&(f->chr[x+3]==94)){//^^^^
exponent_digits=2; stage=4; x+=3; goto nextchar;
}
}//4

}//stage>=2 & stage<=3

if (stage==3){

if (c==35){//#
digits_after_point++; goto nextchar;
}

}//stage==3

if (stage==2){

if (c==44){//,
commas=1; digits_before_point++; goto nextchar;
}

}//stage==2

if (stage<=2){

if (c==35){//#
digits_before_point++; stage=2; goto nextchar;
}

if (c==46){//.
decimal_point=1; stage=3; goto nextchar;
}

}//stage<=2

if (stage<=1){

if (chrsleft>=3){
if ((c==42)&&(f->chr[x+1]==42)&&(f->chr[x+2]==36)){//**$
asterisk_spaces=1; digits_before_point=2; dollar_sign=1; stage=2; x+=2; goto nextchar;
}
}//3

if (chrsleft>=2){
if ((c==42)&&(f->chr[x+1]==42)){//**
asterisk_spaces=1; digits_before_point=2; stage=2; x++; goto nextchar;
}
if ((c==36)&&(f->chr[x+1]==36)){//$$
dollar_sign=1; digits_before_point=1; stage=2; x++; goto nextchar;
}
}//2

}//stage 1

if (stage==0){

if (c==43){//+
leading_plus=1; stage=1; goto nextchar;
}

}//stage 0

//spacer/end encountered
}//x<len
numeric_spacer:

//valid numeric format?
if (stage<=1) goto invalid_numeric_format;
if ((digits_before_point==0)&&(digits_after_point==0)) goto invalid_numeric_format;

if (type==0) return s; //s is the beginning of a new format but item has already been added to dest
if (type==2){//expected string format, not numeric format
error(13);//type mismatch
return 0;
}

//reduce digits before point appropriatly
extra_sign_space=0;
if (exponent_digits){
 if ((leading_plus==0)&&(trailing_plus==0)&&(trailing_minus==0)){digits_before_point--; extra_sign_space=1;}
}else{
 //the following don't occur if using an exponent
 if (pu_neg){
 if ((leading_plus==0)&&(trailing_plus==0)&&(trailing_minus==0)){digits_before_point--; extra_sign_space=1;}
 }
 if (commas){
 digits_and_commas_before_point=i;
 i=digits_before_point/4;//for every 4 digits, one digit will be used up by a comma
 digits_before_point-=i;
 }
}

//will number fit? if it can't then adjustments will be made
cant_fit=0;
if (exponent_digits){
 //give back extra_sign_space?
 if (extra_sign_space){
 if (!pu_neg){
 if (digits_before_point<=0){
 extra_sign_space=0;
 digits_before_point++;//will become 0 or 1
  //force 0 in recovered digit?
  if ((digits_before_point==1)&&(digits_after_point>0)){
  digits_before_point--;
  extra_sign_space=2;//2=put 0 instead of blank space
  }
 }
 }
 }
 if ((digits_before_point==0)&&(digits_after_point==0)){
 cant_fit=1;
 digits_before_point=1;//give back removed (for extra sign space) digit
 }
 //but does the exponent fit?
 z2=pu_ndig+pu_dp-1;//calc exponent of most significant digit
                //1.0  = 0
                //10.0 = 1
                //0.1  = -1
 //calc exponent of format's most significant position
 if (digits_before_point) z3=digits_before_point-1; else z3=-1;
 z=z2-z3;//combine to calculate actual exponent which will be "printed" 
 z3=abs(z);
 z2=sprintf((char*)pu_buf,"%u",z3);//use pu_buf to convert exponent to a string
 if (z2>exponent_digits){cant_fit=1; exponent_digits=z2;}
}else{
 z=pu_ndig+pu_dp;//calc number of digits required before decimal places
 if (digits_before_point<z){
 digits_before_point=z; cant_fit=1;
 if (commas) digits_and_commas_before_point=digits_before_point+(digits_before_point-1)/3;
 }
}

static long buf_size;//buf_size is an estimation of size required
static unsigned char *cp,*buf=NULL;
static long count;
if (buf) free(buf);
buf_size=256;//256 bytes to account for calc overflow (such as exponent digits)
buf_size+=9;//%(1)+-(1)$(1)???.(1)???exponent(5)
buf_size+=digits_before_point;
if (commas) buf_size+=((digits_before_point/3)+2);
buf_size+=digits_after_point;
buf=(unsigned char*)malloc(buf_size);
cp=buf;
count=0;//char count
i=0;

if (asterisk_spaces) asterisk_spaces=42; else asterisk_spaces=32;//chraracter to fill blanks with

if (cant_fit) {*cp++=37; count++;}//%

//leading +/-
if (leading_plus){
if (pu_neg) *cp++=45; else *cp++=43;
count++;
}

if (exponent_digits){
 //add $?
 if (dollar_sign) {*cp++=36; count++;}//$
 //add - sign? (as sign space was not specified)
 if (extra_sign_space){
 if (pu_neg){
 *cp++=45;
 }else{
 if (extra_sign_space==2) *cp++=48; else *cp++=32;
 }
 count++;
 }
 //add digits left of decimal point
 for (z3=0;z3<digits_before_point;z3++){
 if (i<pu_ndig) *cp++=pu_dig[i++]; else *cp++=48;
 count++;
 }
 //add decimal point
 if (decimal_point){*cp++=46; count++;}
 //add digits right of decimal point
 for (z3=0;z3<digits_after_point;z3++){
 if (i<pu_ndig) *cp++=pu_dig[i++]; else *cp++=48;
 count++;
 }
 //round last digit (requires a repass)
 if (!rounded){
 if (i<pu_ndig){
 if (pu_dig[i]>=53){//>="5" (round 5 up)
 z=i-1;
  //round up pu (by adding 1 from digit at character position z)
  //note: pu_dig is rebuilt one character to the right so highest digit can flow over into next character
  rounded=1;
  memmove(&pu_dig[1],&pu_dig[0],pu_ndig); pu_dig[0]=48; z++;
  puround2:
  pu_dig[z]++;
  if (pu_dig[z]>57) {pu_dig[z]=48; z--; goto puround2;}
  if (pu_dig[0]!=48){//was extra character position necessary?
  pu_ndig++; //note: pu_dp does not require any changes  
  }else{
  memmove(&pu_dig[0],&pu_dig[1],pu_ndig);
  }
  goto rounded_repass;
 }
 }
 }
 //add exponent...
 *cp++=pu_exp_char; count++; //add exponent D/E/F (set and restored by calling function as necessary)
 if (z>=0) {*cp++=43; count++;} else {*cp++=45; count++;} //+/- exponent's sign
 //add exponent's leading 0s (if any)
 for (z3=0;z3<(exponent_digits-z2);z3++){
 *cp++=48; count++;
 }
 //add exponent's value
 for (z3=0;z3<z2;z3++){
 *cp++=pu_buf[z3]; count++;
 }
}else{
 //"print" everything before the point
 //calculate digit spaces before the point in number
 if (!commas) digits_and_commas_before_point=digits_before_point;
 z=pu_ndig+pu_dp;//num of character whole portion of number requires
 if (commas) z=z+(z-1)/3;//including appropriate amount of commas
 if (z<0) z=0; 
 z2=digits_and_commas_before_point-z;
 if ((z==0)&&(z2>0)){leading_zero=1; z2--;}//change .1 to 0.1 if possible
 for (z3=0;z3<z2;z3++){*cp++=asterisk_spaces; count++;}
 //add - sign? (as sign space was not specified)
 if (extra_sign_space){*cp++=45; count++;}
 //add $?
 if (dollar_sign){*cp++=36; count++;}//$ 
 //leading 0?
 if (leading_zero){*cp++=48; count++;}//0
 //add digits left of decimal point
 for (z3=0;z3<z;z3++){
 if ((commas!=0)&&(((z-z3)&3)==0)){
 *cp++=44;
 }else{
 if (i<pu_ndig) *cp++=pu_dig[i++]; else *cp++=48;
 }
 count++;
 } 
 //add decimal point
 if (decimal_point){*cp++=46; count++;}
 //add digits right of decimal point
 for (z3=0;z3<digits_after_point;z3++){
 if (i<pu_ndig) *cp++=pu_dig[i++]; else *cp++=48;
 count++;
 }
 //round last digit (requires a repass)
 if (!rounded){
 if (i<pu_ndig){
 if (pu_dig[i]>=53){//>="5" (round 5 up)
 z=i-1;
  //round up pu (by adding 1 from digit at character position z)
  //note: pu_dig is rebuilt one character to the right so highest digit can flow over into next character
  rounded=1;  
  memmove(&pu_dig[1],&pu_dig[0],pu_ndig); pu_dig[0]=48; z++;
  puround1: 
  pu_dig[z]++;
  if (pu_dig[z]>57) {pu_dig[z]=48; z--; goto puround1;}
  if (pu_dig[0]!=48){//was extra character position necessary?
  pu_ndig++; //note: pu_dp does not require any changes  
  }else{
  memmove(&pu_dig[0],&pu_dig[1],pu_ndig);  
  }
  goto rounded_repass;
 } 
 }
 }
}//exponent_digits

//add trailing sign?
//trailing +/-
if (trailing_plus){
if (pu_neg) *cp++=45; else *cp++=43;
count++;
}
//trailing -
if (trailing_minus){
if (pu_neg) *cp++=45; else *cp++=32;
count++;
}

qbs_set(dest,qbs_add(dest,qbs_new_txt_len((char*)buf,count)));

s=x;
type=0;//passed type added
if (s>=len) return 0;//end of format line encountered and passed item added
goto scan;

invalid_numeric_format:
//string format
static long string_size;

x=s;
if (x<len){
c=f->chr[x];
string_size=0;//invalid
if (c==38) string_size=-1; //"&" (all of string)
if (c==33) string_size=1; //"!" (first character only)
if (c==92){ //"\" first n characters
 z=1;
 x++;
 get_str_fmt:
 if (x>=len) goto invalid_string_format;
 c=f->chr[x];
 z++;
 if (c==32){x++; goto get_str_fmt;}
 if (c!=92) goto invalid_string_format;
 string_size=z;
}//c==47
if (string_size){
 if (type==0) return s; //s is the beginning of a new format but item has already been added to dest
 if (type==1){//expected numeric format, not string format
 error(13);//type mismatch
 return 0;
 }
 if (string_size!=-1){
 s+=string_size;
  for (z=0;z<string_size;z++){
  if (z<pu_str->len) qbs1->chr[0]=pu_str->chr[z]; else qbs1->chr[0]=32;
  qbs_set(dest,qbs_add(dest,qbs1));
  }//z 
 }else{
 qbs_set(dest,qbs_add(dest,pu_str));
 s++;
 }
 type=0;//passed type added
 if (s>=len) return 0;//end of format line encountered and passed item added
 goto scan;
}//string_size
}//x<len
invalid_string_format:

//add literal?
if ((f->chr[s]==95)&&(s<(len-1))){//trailing single _ in format is treated as a literal _
s++;
}
//add non-format character
qbs1->chr[0]=f->chr[s]; qbs_set(dest,qbs_add(dest,qbs1));

s++;
if (s>=len){
s=0;
if (type==0) return s;//end of format line encountered and passed item added
 //illegal format? (format has been passed from start (s2=0) to end and has no numeric/string identifiers
 if (s2==0){
 error(5);//illegal function call
 return 0;
 }
}
goto scan;

return 0;
}

long print_using_integer64(qbs* format, int64 value, long start, qbs *output){
if (new_error) return 0;
#ifdef QB64_WINDOWS
 pu_ndig=sprintf((char*)pu_buf,"% I64i",value);
#else
 pu_ndig=sprintf((char*)pu_buf,"% ll",value);
#endif
if (pu_buf[0]==45) pu_neg=1; else pu_neg=0;
pu_ndig--;//remove sign
memcpy(pu_dig,&pu_buf[1],pu_ndig);
pu_dp=0;
start=print_using(format,start,output,NULL);
return start;
}

long print_using_uinteger64(qbs* format, uint64 value, long start, qbs *output){
if (new_error) return 0;
#ifdef QB64_WINDOWS
 pu_ndig=sprintf((char*)pu_dig,"%I64u",value);
#else
 pu_ndig=sprintf((char*)pu_dig,"%ull",value);
#endif
pu_neg=0;
pu_dp=0;
start=print_using(format,start,output,NULL);
return start;
}

long print_using_single(qbs* format, float value, long start, qbs *output){
if (new_error) return 0;
static long i,len,neg_exp;
static unsigned char c;
static int64 exp;
len=sprintf((char*)&pu_buf,"% .255E",value);//256 character limit ([1].[255])
pu_dp=0;
pu_ndig=0;
//1. sign
if (pu_buf[0]==45) pu_neg=1; else pu_neg=0;
i=1;
//2. digits before decimal place
getdigits:
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if ((c>=48)&&(c<=57)){
pu_dig[pu_ndig++]=c;
i++;
goto getdigits;
}
//3. decimal place
if (c!=46){error(5); return 0;}//expected decimal point
i++;
//4. digits after decimal place
getdigits2:
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if ((c>=48)&&(c<=57)){
pu_dig[pu_ndig++]=c;
pu_dp--;
i++;
goto getdigits2;
}
//assume random character signifying an exponent
i++;
//optional exponent sign
neg_exp=0;
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if (c==45){neg_exp=1; i++;}//-
if (c==43) i++;//+
//assume remaining characters are an exponent
exp=0;
getdigits3:
if (i<len){
c=pu_buf[i];
if ((c<48)||(c>57)){error(5); return 0;}
exp=exp*10;
exp=exp+c-48;
i++;
goto getdigits3;
}
if (neg_exp) exp=-exp;
pu_dp+=exp;
start=print_using(format,start,output,NULL);
return start;
}

long print_using_double(qbs* format, double value, long start, qbs *output){
if (new_error) return 0;
static long i,len,neg_exp;
static unsigned char c;
static int64 exp;
len=sprintf((char*)&pu_buf,"% .255E",value);//256 character limit ([1].[255])
pu_dp=0;
pu_ndig=0;
//1. sign
if (pu_buf[0]==45) pu_neg=1; else pu_neg=0;
i=1;
//2. digits before decimal place
getdigits:
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if ((c>=48)&&(c<=57)){
pu_dig[pu_ndig++]=c;
i++;
goto getdigits;
}
//3. decimal place
if (c!=46){error(5); return 0;}//expected decimal point
i++;
//4. digits after decimal place
getdigits2:
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if ((c>=48)&&(c<=57)){
pu_dig[pu_ndig++]=c;
pu_dp--;
i++;
goto getdigits2;
}
//assume random character signifying an exponent
i++;
//optional exponent sign
neg_exp=0;
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if (c==45){neg_exp=1; i++;}//-
if (c==43) i++;//+
//assume remaining characters are an exponent
exp=0;
getdigits3:
if (i<len){
c=pu_buf[i];
if ((c<48)||(c>57)){error(5); return 0;}
exp=exp*10;
exp=exp+c-48;
i++;
goto getdigits3;
}
if (neg_exp) exp=-exp;
pu_dp+=exp;
pu_exp_char=68; //"D"
start=print_using(format,start,output,NULL);
pu_exp_char=69; //"E"
return start;
}

//WARNING: DUE TO MINGW NOT SUPPORTING PRINTF LONG DOUBLE, VALUES ARE CONVERTED TO A DOUBLE
//         BUT PRINTED AS IF THEY WERE A LONG DOUBLE
long print_using_float(qbs* format, double value, long start, qbs *output){
if (new_error) return 0;
static long i,len,neg_exp;
static unsigned char c;
static int64 exp;
len=sprintf((char*)&pu_buf,"% .255E",value);//256 character limit ([1].[255])
pu_dp=0;
pu_ndig=0;
//1. sign
if (pu_buf[0]==45) pu_neg=1; else pu_neg=0;
i=1;
//2. digits before decimal place
getdigits:
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if ((c>=48)&&(c<=57)){
pu_dig[pu_ndig++]=c;
i++;
goto getdigits;
}
//3. decimal place
if (c!=46){error(5); return 0;}//expected decimal point
i++;
//4. digits after decimal place
getdigits2:
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if ((c>=48)&&(c<=57)){
pu_dig[pu_ndig++]=c;
pu_dp--;
i++;
goto getdigits2;
}
//assume random character signifying an exponent
i++;
//optional exponent sign
neg_exp=0;
if (i>=len){error(5); return 0;}
c=pu_buf[i];
if (c==45){neg_exp=1; i++;}//-
if (c==43) i++;//+
//assume remaining characters are an exponent
exp=0;
getdigits3:
if (i<len){
c=pu_buf[i];
if ((c<48)||(c>57)){error(5); return 0;}
exp=exp*10;
exp=exp+c-48;
i++;
goto getdigits3;
}
if (neg_exp) exp=-exp;
pu_dp+=exp;
pu_exp_char=70; //"F"
start=print_using(format,start,output,NULL);
pu_exp_char=69; //"E"
return start;
}






int main( int argc, char* argv[] )
{

static long i,i2,i3,i4;
static unsigned char c,c2,c3,c4;
static long x,x2,x3,x4;
static long y,y2,y3,y4;
static long z,z2,z3,z4;
static float f,f2,f3,f4;
static unsigned char *cp,*cp2,*cp3,*cp4;


memset(font,0,sizeof(font));

fontwidth[8]=8; fontwidth[14]=8; fontwidth[16]=8;
fontheight[8]=8; fontheight[14]=14; fontheight[16]=16;
fontflags[8]=16; fontflags[14]=16; fontflags[16]=16;//monospace flag
fontwidth[8+1]=8*2; fontwidth[14+1]=8*2; fontwidth[16+1]=8*2;
fontheight[8+1]=8; fontheight[14+1]=14; fontheight[16+1]=16;
fontflags[8+1]=16; fontflags[14+1]=16; fontflags[16+1]=16;//monospace flag

memset(img,0,IMG_BUFFERSIZE*sizeof(img_struct));
x=newimg();//reserve index 0
img[x].valid=0;
x=newimg();//reserve index 1
img[x].valid=0;

cp=blend;
for (i=0;i<256;i++){//source alpha
for (x2=0;x2<256;x2++){//source
for (x3=0;x3<256;x3++){//dest
f=i;
f2=x2;
f3=x3;
f/=255.0;//0.0-1.0
*cp++=qbr_float_to_long((f*f2)+((1.0-f)*f3));//CINT(0.0-255.0)
}}}


/*
"60%+60%=84%" formula
imagine a 60% opaque lens, you can see 40% of whats behind
now put another 60% opaque lens on top of it
you can now see 40% of the previous lens of which 40% is of the original scene
40% of 40% is 16%
100%-16%=84%
 V1=60, V2=60
 v1=V1/100, v2=V2/100
 iv1=1-v1, iv2=1-v2
 iv3=iv1*iv2
 v3=1-iv3
 V3=v3*100
*/
cp=ablend;
for (i=0;i<256;i++){//first alpha value
for (i2=0;i2<256;i2++){//second alpha value
f=i; f2=i2;
f/=255.0; f2/=255.0;
f=1.0-f; f2=1.0-f2;
f3=f*f2;
z=qbr_float_to_long((1.0-f3)*255.0);
*cp++=z;
}}
ablend127=ablend+(127<<8);
ablend128=ablend+(128<<8);




/*
i=50;
i2=50;
cp=ablend;
exit (cp[(i2<<8)+i]);

//alpha test
i=128;
x=255;
y=255;
cp=blend+(i<<16);
exit (cp[(x<<8)+y]);
*/







memset(&cpu,0,sizeof(cpu_struct));

//unsigned char *asmcodep=(unsigned char*)&asmcode[0];
//memcpy(&cmem[0],asmcodep,sizeof(asmcode));
reg8[0]=&cpu.al;
reg8[1]=&cpu.cl;
reg8[2]=&cpu.dl;
reg8[3]=&cpu.bl;
reg8[4]=&cpu.ah;
reg8[5]=&cpu.ch;
reg8[6]=&cpu.dh;
reg8[7]=&cpu.bh;

reg16[0]=&cpu.ax;
reg16[1]=&cpu.cx;
reg16[2]=&cpu.dx;
reg16[3]=&cpu.bx;
reg16[4]=&cpu.sp;
reg16[5]=&cpu.bp;
reg16[6]=&cpu.si;
reg16[7]=&cpu.di;

reg32[0]=&cpu.eax;
reg32[1]=&cpu.ecx;
reg32[2]=&cpu.edx;
reg32[3]=&cpu.ebx;
reg32[4]=&cpu.esp;
reg32[5]=&cpu.ebp;
reg32[6]=&cpu.esi;
reg32[7]=&cpu.edi;

segreg[0]=&cpu.es;
segreg[1]=&cpu.cs;
segreg[2]=&cpu.ss;
segreg[3]=&cpu.ds;
segreg[4]=&cpu.fs;
segreg[5]=&cpu.gs;






//cpu.dh=5;
//cpu_call();

//cpu.cs=0; cpu.ip=0;
//need some instructions!
//cpu_call();

//WINDOWS SPECIFIC CONTENT
FreeConsole();
//END WINDOWS SPECIFIC CONTENT

//#ifndef _TM_DEFINED
//struct tm {
//        int tm_sec;     /* seconds after the minute - [0,59] */
//        int tm_min;     /* minutes after the hour - [0,59] */
//        int tm_hour;    /* hours since midnight - [0,23] */
//        int tm_mday;    /* day of the month - [1,31] */
//        int tm_mon;     /* months since January - [0,11] */
//        int tm_year;    /* years since 1900 */
//        int tm_wday;    /* days since Sunday - [0,6] */
//        int tm_yday;    /* days since January 1 - [0,365] */
//        int tm_isdst;   /* daylight savings time flag */
//        };
tm *qb64_tm;
time_t qb64_tm_val;
time_t qb64_tm_val_old;


//call both timing routines as close as possible to each other to maximize accuracy
//wait for second "hand" to "tick over"/move
time(&qb64_tm_val_old);

do{
time(&qb64_tm_val);
}while(qb64_tm_val==qb64_tm_val_old);

sdl_firsttimervalue=SDL_GetTicks();

qb64_tm=localtime(&qb64_tm_val);

//calculate localtime as milliseconds past midnight
//TODO: WHEN TIMER IS CALLED IT MUST SOMETIMES FORCE THE VALUE TO LOOP
qb64_firsttimervalue=qb64_tm->tm_hour*3600+qb64_tm->tm_min*60+qb64_tm->tm_sec;
qb64_firsttimervalue*=1000;






for (i=0;i<=last_device_handle;i++) device_handle[i].open=0;

for (i=0;i<32;i++) sub_file_print_spaces[i]=32;


port60h_event[0]=128;


mem_static_size=1048576;//1MEG
mem_static=(unsigned char*)malloc(mem_static_size);
mem_static_pointer=mem_static;
mem_static_limit=mem_static+mem_static_size;

memset(&cmem[0],0,sizeof(cmem));










memset(&keyon[0],0,sizeof(keyon));

dblock=(unsigned long)&cmem+1280;//0:500h

//define "nothing"
cmem_sp-=8; nothingvalue=(uint64*)(dblock+cmem_sp);
nothingvalue=0;
nothingstring=qbs_new_cmem(0,0);
singlespace=qbs_new_cmem(1,0);
singlespace->chr[0]=32;

i=argc;
if (i>1){
//calculate required size of COMMAND$ string
i2=0;
for (i=1;i<argc;i++){
i2+=strlen(argv[i]);
if (i!=1) i2++;//for a space
}
//create COMMAND$ string
func_command_str=qbs_new(i2,0);
//build COMMAND$ string
i3=0;
for (i=1;i<argc;i++){
if (i!=1){func_command_str->chr[i3]=32; i3++;}
memcpy(&func_command_str->chr[i3],argv[i],strlen(argv[i])); i3+=strlen(argv[i]);
}
}else{
func_command_str=qbs_new(0,0);
}


//init SDL
if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_CDROM|SDL_INIT_TIMER) < 0 ) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(100);
    }
atexit(SDL_Quit);



//init truetype .ttf/.fon font library
if (TTF_Init()==-1) exit(7000);
atexit(TTF_Quit);



modes=SDL_ListModes(NULL,SDL_FULLSCREEN|SDL_HWSURFACE);
if(modes){
if(modes==(SDL_Rect **)-1){
anymode=1;
}else{
for(i=0;modes[i];++i){
nmodes++;
//cout<<i; cout<<",";
//cout<<modes[i]->w; cout<<",";
//cout<<modes[i]->h;
//cout<<endl;
}
}
}//modes

memset(snd,0,sizeof(snd));

//set key repeat rate
SDL_EnableKeyRepeat(500,30); key_repeat_on=1;
//safer to use default key repeat rates which aren't dissimilar to those used above


//enable unicode
SDL_EnableUNICODE(1);

//init fake keyb. cyclic buffer
cmem[0x41a]=30; cmem[0x41b]=0; //head
cmem[0x41c]=30; cmem[0x41d]=0; //tail

SDL_WM_SetIcon(SDL_LoadBMP("./data/qb64icon.bmp"), NULL);
SDL_WM_SetCaption("Untitled",NULL);

unsigned long *pixel;

int int_x,int_y;
SDL_GetMouseState(&int_x,&int_y);
mouse_messages[0].x=int_x;
mouse_messages[0].y=int_y;
mouse_messages[0].buttons=0;

//SDL_Cursor * mycursor=SDL_CreateCursor
		//(Uint8 *data, Uint8 *mask, int w, int h, int hot_x, int hot_y);
SDL_Cursor * mycursor=init_system_cursor(arrow);
SDL_SetCursor(mycursor);

ifstream fh;


//pre-check for required files
fh.open ("./data/qb64icon.bmp",ios::binary|ios::out|ios::in);
if (fh.is_open()){
fh.close();
}else{MessageBox(NULL,"QB64ICON.BMP","Data File Missing!",MB_OK); exit(1);}


//load the default 256 color palette
fh.open ("./data/qb64.pal",ios::binary|ios::out|ios::in);
if (fh.is_open()){
fh.read((char*)&palette_256,256*4);
fh.close();
}else{MessageBox(NULL,"QB64.PAL","Data File Missing!",MB_OK); exit(1);}
for(i=0;i<256;i++) palette_256[i]|=0xFF000000;

fh.open ("./data/qb64ega.pal",ios::binary|ios::out|ios::in);
if (fh.is_open()){
fh.read((char*)&palette_64,64*4);
fh.close();
}else{MessageBox(NULL,"QB64EGA.PAL","Data File Missing!",MB_OK); exit(1);}
for(i=0;i<64;i++) palette_64[i]|=0xFF000000;

//manually set screen 10 palette
pal_mode10[0][0]=0;
pal_mode10[0][1]=0;
pal_mode10[0][2]=0;
pal_mode10[0][3]=0x808080;
pal_mode10[0][4]=0x808080;
pal_mode10[0][5]=0x808080;
pal_mode10[0][6]=0xFFFFFF;
pal_mode10[0][7]=0xFFFFFF;
pal_mode10[0][8]=0xFFFFFF;
pal_mode10[1][0]=0;
pal_mode10[1][1]=0x808080;
pal_mode10[1][2]=0xFFFFFF;
pal_mode10[1][3]=0;
pal_mode10[1][4]=0x808080;
pal_mode10[1][5]=0xFFFFFF;
pal_mode10[1][6]=0;
pal_mode10[1][7]=0x808080;
pal_mode10[1][8]=0xFFFFFF;

//load the 8x8 character set
fh.open ("./data/charset8.raw",ios::binary|ios::out|ios::in);
if (fh.is_open()){
fh.read((char*)&charset8x8,256*8*8);
fh.close();
}else{MessageBox(NULL,"CHARSET8.RAW","Data File Missing!",MB_OK); exit(1);}

//load the 8x16 character set
fh.open ("./data/chrset16.raw",ios::binary|ios::out|ios::in);
if (fh.is_open()){
fh.read((char*)&charset8x16,256*16*8);
fh.close();
}else{MessageBox(NULL,"CHRSET16.RAW","Data File Missing!",MB_OK); exit(1);}


//get 32bit argb pixel format
ts=SDL_CreateRGBSurface(NULL,8,8,32,0x00FF0000,0x0000FF00,0x000000FF,0xFF000000);
pixelformat32=*ts->format;
SDL_FreeSurface(ts);
ts=SDL_CreateRGBSurface(NULL,8,8,8,NULL,NULL,NULL,NULL);
pixelformat8=*ts->format;
SDL_FreeSurface(ts);

qbg_screen(0,NULL,NULL,NULL,NULL,1);

    thread = SDL_CreateThread (QBMAIN, NULL  );
    if ( thread == NULL ) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        return 0;
    }

lock_display_required=1;



long update=0;//0=update input,1=update display

static unsigned char *pixeldata=(unsigned char*)malloc(1);
static long pixeldatasize=1;
static long paldata[256];

main_loop:

if (shell_call_in_progress){
if (shell_call_in_progress!=-1){
shell_call_in_progress=-1;
if (key_repeat_on){SDL_EnableKeyRepeat(0,0); key_repeat_on=0;}
goto update_display_only;
}
Sleep(64);
goto main_loop;
}

Sleep(15);
vertical_retrace_happened=1; vertical_retrace_in_progress=1;
Sleep(1);

if (close_program) goto end_program;

update^=1;//toggle update

//buffered sound/play notes
//(safe even if sndsetup not called)
if (sndqueue_first!=sndqueue_next){
if (sndqueue_played==0){
 sndqueue_played=1;
 if (sndqueue_first==sndqueue_wait){suspend_program^=2; sndqueue_wait=-1;}
 sub__sndplay(sndqueue[sndqueue_first]);
 sndqueue_first++;
}else{
 i=sndqueue_first-1; if (i==-1) i=sndqueue_lastindex;
 if (!func__sndplaying(sndqueue[i])){
 sub__sndclose(sndqueue[i]);
 if (sndqueue_first==sndqueue_wait){suspend_program^=2; sndqueue_wait=-1;}
 sub__sndplay(sndqueue[sndqueue_first]);
 sndqueue_first++; if (sndqueue_first>sndqueue_lastindex) sndqueue_first=0;
 }
}
}



//update display
if (update==1){
update_display_only:

frame++;//~32 fps



if (lock_display==1){lock_display=2; Sleep(0);}
if (lock_display==0){


//validate display_page
if (!display_page) goto display_page_invalid;



//check what is possible in full screen
x=display_page->width; y=display_page->height;

if (display_page->compatible_mode==0){
x=display_page->width*fontwidth[display_page->font]; y=display_page->height*fontheight[display_page->font];
}

//check for y-stretch flag?

if (x<=512&&y<=384){
x*=2; y*=2;
}

static long mode_square,mode_stretch;

//find best fullscreen mode(s) (eg. square/"1:1", stretched)
mode_square=-1;
mode_stretch=-1;
z2=0x7FFFFFFF; z3=0x7FFFFFFF;
for (i=0;i<nmodes;i++){
if (modes[i]->w>=640){
if (modes[i]->w>=x&&modes[i]->h>=y){
z=modes[i]->w-x+modes[i]->h-y;
 //square
 if (z<=z2){
 //is it square?
 if (modes[i]->w/4==modes[i]->h/3){
 mode_square=i; z2=z;
 }//square
 }//z2<=z
 //stretch
 if (z<=z3){
 mode_stretch=i; z3=z;
 }//z<=z3
}//>x,>y
}//ignore?
}//i

static long full_screen_change;
full_screen_change=0;

if (full_screen_set!=-1){
if (full_screen_set==0) {full_screen_set=-1; goto full_screen0;}
if (full_screen_set==1) {full_screen_set=-1; goto full_screen1;}
if (full_screen_set==2) {full_screen_set=-1; goto full_screen2;}
}

if (full_screen_toggle){
full_screen_toggle--;

if (full_screen==0){
full_screen1:
if (mode_stretch>-1){//can be displayed
full_screen=1;
 full_screen_change=1;
 screen_last_valid=0;
 if (!mouse_hideshow_called) SDL_ShowCursor(0);
}
goto full_screen_toggle_done;
}

if (full_screen==1){
full_screen2:
if (mode_square>-1&&mode_square!=mode_stretch){//usable 1:1 mode exists (that isn't same as stretched mode)
full_screen=2;
 full_screen_change=1;
 screen_last_valid=0;
 if (!mouse_hideshow_called) SDL_ShowCursor(0);
goto full_screen_toggle_done;
}
}
//back to windowed mode
full_screen0:
full_screen=0;
 full_screen_change=1;
 screen_last_valid=0;
 if (!mouse_hideshow_called) SDL_ShowCursor(1);
}
full_screen_toggle_done:

//set the screen mode if necessary

x_offset=0; y_offset=0;
x_scale=1; y_scale=1;

x=display_page->width; y=display_page->height;
if (display_page->compatible_mode==0){
x=display_page->width*fontwidth[display_page->font]; y=display_page->height*fontheight[display_page->font];
}

//check for y-stretch flag?

if (full_screen){

//double res. of "small" surfaces to avoid video driver incompatibilities
if (x<=512&&y<=384){
x*=2; y*=2;
y_scale*=2; x_scale*=2;
}

x2=x; y2=y;
if (full_screen==1){
x=modes[mode_stretch]->w; y=modes[mode_stretch]->h;
}
if (full_screen==2){
x=modes[mode_square]->w; y=modes[mode_square]->h;
}

//calculate offsets
x_offset=(x-x2)/2; y_offset=(y-y2)/2;

}//full_screen



//adjust monitor resolution
if ((x!=x_monitor)||(y!=y_monitor)||full_screen_change){
x_monitor=x; y_monitor=y;
if (full_screen) z=SDL_FULLSCREEN; else z=0;
display_surface=SDL_SetVideoMode(x,y,32,z);
display_surface_offset=(unsigned long*)display_surface->pixels;
pixel=(unsigned long*)display_surface->pixels;//***remove later***
}
if (!screen_last_valid){
memset(display_surface->pixels,0,x_monitor*y_monitor*4);
}

//update the screen data

if (!display_page->compatible_mode){//text

//removed following line on 22/1/09, questionable performace gain vs visual loss
//if (frame&1) goto screen_refresh_unrequired;//~16fps for text mode

static long show_flashing_last=0;
static long show_cursor_last=0;
static long check_last;
static unsigned char *cp,*cp2,*cp_last;
static unsigned long *lp;
static long cx,cy;
static long cx_last=-1,cy_last=-1;
static long show_cursor;
static long show_flashing;
static unsigned char chr,col,chr_last,col_last;
static long screen_changed;
static long qbg_y_offset;


static long f,f_pitch,f_width,f_height;//font info
f=display_page->font; f_width=fontwidth[f]; f_height=fontheight[f];

//realloc buffer if necessary
i=display_page->width*display_page->height*2;
if (screen_last_size!=i){
free(screen_last);
screen_last=(unsigned char*)malloc(i);
screen_last_size=i;
}

check_last=screen_last_valid;
if (!check_last){
 //set pal_last (no prev pal was avilable to compare to)
 memcpy(&pal_last[0],display_page->pal,256*4);
}else{
 //if palette has changed, update pal_last and draw all characters
 if (memcmp(&pal_last[0],display_page->pal,256*4)){
 memcpy(&pal_last[0],display_page->pal,256*4);
 check_last=0;
 }
}

//calculate cursor position (base 0)
cx=display_page->cursor_x-1; cy=display_page->cursor_y-1;

//show cursor?
if (frame&4) show_cursor=1; else show_cursor=0;

//show flashing?
if (frame&8) show_flashing=1; else show_flashing=0;

qbg_y_offset=y_offset*x_monitor+x_offset;//the screen base offset
cp=display_page->offset;//read from
cp_last=screen_last;//written to for future comparisons

//outer loop
y2=0;
for (y=0;y<display_page->height;y++){
x2=0;
for (x=0;x<display_page->width;x++){

chr=*cp; cp++; col=*cp; cp++;

//can be skipped?
chr_last=*cp_last; cp_last++; col_last=*cp_last; cp_last++;

if (check_last){
if (chr==chr_last){//same character
if (col==col_last){//same colours
if (col&128) if (show_flashing!=show_flashing_last) goto cantskip;//same flash
if (x==cx) if (y==cy) if (show_cursor!=show_cursor_last) goto cantskip;//same cursor
if (x==cx_last){ if (y==cy_last){
if ((cx!=cx_last)||(cy!=cy_last)) goto cantskip;//fixup old cursor's location
}}
goto skip;
}}}
cantskip:
screen_changed=1;
cp_last-=2; *cp_last=chr; cp_last++; *cp_last=col; cp_last++;

//set cp2 to the character's data
z2=0;//double-width
if (f>=32){//custom font
static SDL_Surface *ts=NULL;
static SDL_Color c;
c.r=255; c.g=255; c.b=255; c.unused=0;//dummy values
if (ts) SDL_FreeSurface(ts);//free previously created buffer
z=chr;
ts=TTF_RenderText_Solid(font[f],(char*)&z,c);//8-bit, 0=clear, 1=text
cp2=(unsigned char*)ts->pixels;
f_pitch=ts->pitch-f_width;
}else{//default font
f_pitch=0;
if (f==8) cp2=&charset8x8[chr][0][0];
if (f==14) cp2=&charset8x16[chr][1][0];
if (f==16) cp2=&charset8x16[chr][0][0];
if (f==(8+1)) {cp2=&charset8x8[chr][0][0]; z2=1;}
if (f==(14+1)) {cp2=&charset8x16[chr][1][0]; z2=1;}
if (f==(16+1)) {cp2=&charset8x16[chr][0][0]; z2=1;}
}
c=col&0xF;//foreground col
c2=(col>>4)&7;//background col
c3=col>>7;//flashing?
if (c3&&show_flashing) c=c2;
i2=display_page->pal[c];
i3=display_page->pal[c2];
lp=display_surface_offset+qbg_y_offset+y2*x_monitor+x2;
z=x_monitor-fontwidth[display_page->font];

//inner loop
for (y3=0;y3<f_height;y3++){
for (x3=0;x3<f_width;x3++){
if (*cp2) *lp=i2; else *lp=i3;
if (z2){
if (x3&z2) cp2++;
}else{
cp2++;
}
lp++;
}
lp+=z;
cp2+=f_pitch;
}//y3,x3

//draw cursor
if (display_page->cursor_show&&show_cursor&&(cx==x)&&(cy==y)){
static long v1,v2;
static unsigned char from_bottom;//bottom is the 2nd bottom scanline in width ?x25
static unsigned char half_cursor;//if set, overrides all following values
static unsigned char size;//if 0, no cursor is drawn, if 255, from begin to bottom
static unsigned char begin;//only relevant if from_bottom was not specified
v1=display_page->cursor_firstvalue;
v2=display_page->cursor_lastvalue;
from_bottom=0;
half_cursor=0;
size=0;
begin=0;
//RULE: IF V2=0, NOTHING (UNLESS V1=0)
if (v2==0){
if (v1==0){size=1; goto cursor_created;}
goto nocursor;//no cursor!
}
//RULE: IF V2<V1, FROM V2 TO BOTTOM
if (v2<v1){begin=v2; size=255; goto cursor_created;}
//RULE: IF V1=V2, SINGLE SCANLINE AT V1 (OR BOTTOM IF V1>=4)
if (v1==v2){
if (v1<=3){begin=v1; size=1; goto cursor_created;}
from_bottom=1; size=1; goto cursor_created;
}
//NOTE: V2 MUST BE LARGER THAN V1!
//RULE: IF V1>=3, CALC. DIFFERENCE BETWEEN V1 & V2
//                IF DIFF=1, 2 SCANLINES AT BOTTOM
//                IF DIFF=2, 3 SCANLINES AT BOTTOM
//                OTHERWISE HALF CURSOR
if (v1>=3){
if ((v2-v1)==1){from_bottom=1; size=2; goto cursor_created;}
if ((v2-v1)==2){from_bottom=1; size=3; goto cursor_created;}
half_cursor=1; goto cursor_created;
}
//RULE: IF V1<=1, IF V2<=3 FROM V1 TO V3 ELSE FROM V1 TO BOTTOM
if (v1<=1){
if (v2<=3){begin=v1;size=v2-v1+1; goto cursor_created;} 
begin=v1;size=255; goto cursor_created;
}
//RULE: IF V1=2, IF V2=3, 2 TO 3
//               IF V2=4, 3 SCANLINES AT BOTTOM
//               IF V2>=5, FROM 2 TO BOTTOM
//(assume V1=2)
if (v2==3){begin=2;size=2; goto cursor_created;}
if (v2==4){from_bottom=1; size=3; goto cursor_created;}
begin=2;size=255;
cursor_created:
static long cw,ch;
cw=fontwidth[display_page->font]; ch=fontheight[display_page->font];
if (half_cursor){
//half cursor
y3=ch-1;
size=ch/2;
c=col&0xF;//foreground col
i2=display_page->pal[c];
draw_half_curs:
lp=display_surface_offset+qbg_y_offset+(y2+y3)*x_monitor+x2;
for (x3=0;x3<cw;x3++){
*lp=i2;
lp++;
}
y3--;
size--;
if (size) goto draw_half_curs;
}else{
if (from_bottom){
//cursor from bottom
y3=ch-1;
if (y3==15) y3=14;//leave bottom line blank in 8x16 char set
c=col&0xF;//foreground col
i2=display_page->pal[c];
draw_curs_from_bottom:
lp=display_surface_offset+qbg_y_offset+(y2+y3)*x_monitor+x2;
for (x3=0;x3<cw;x3++){
*lp=i2;
lp++;
}
y3--;
size--;
if (size) goto draw_curs_from_bottom;
}else{
//cursor from begin using size
if (begin<ch){
y3=begin;
c=col&0xF;//foreground col
i2=display_page->pal[c];
if (size==255) size=ch-begin;
draw_curs_from_begin:
lp=display_surface_offset+qbg_y_offset+(y2+y3)*x_monitor+x2;
for (x3=0;x3<cw;x3++){
*lp=i2;
lp++;
}
y3++;
size--;
if (size) goto draw_curs_from_begin;
}
}
}
}//draw cursor?
nocursor:

//outer loop
skip:
x2=x2+fontwidth[display_page->font];
}
y2=y2+fontheight[display_page->font];
}

show_flashing_last=show_flashing;
show_cursor_last=show_cursor;
cx_last=cx;
cy_last=cy;
screen_last_valid=1;
if (!screen_changed) goto screen_refresh_unrequired;
goto screen_refreshed;

}//text

if (display_page->bits_per_pixel==32){
//scaled refresh
if (x_scale==2){
static unsigned long *lp;
static unsigned long *lp2;
static unsigned long c;
lp=display_page->offset32;
lp2=display_surface_offset+x_monitor*y_offset+x_offset;
x2=display_page->width;
z=x2*8;
y2=display_page->height;
for (y=0;y<y2;y++){
for (x=0;x<x2;x++){
c=*lp++; *lp2++=c; *lp2++=c;
}
lp2+=(x_monitor-(x2*2));
if (y_scale==2){memcpy(lp2,lp2-x_monitor,z);lp2+=x_monitor;}
}
goto screen_refreshed;
}//x_scale==2
if (x_scale==1){
static unsigned long *lp;
static unsigned long *lp2;
lp=display_page->offset32;
lp2=display_surface_offset+x_monitor*y_offset+x_offset;
x2=display_page->width;
z=x2*4;
y2=display_page->height;
for (y=0;y<y2;y++){
memcpy(lp2,lp,z);
lp2+=x_monitor;
if (y_scale==2){memcpy(lp2,lp,z);lp2+=x_monitor;}
lp+=x2;
}
goto screen_refreshed;
}//x_scale==1
exit(48341);
}//32

//assume <=256 colors using palette

/*
//update SCREEN 10 palette
if (qbg_mode==10){
//pal_mode10[0-1][0-8]
i2=SDL_GetTicks()&512;
if (i2) i2=1;
for (i=0;i<=3;i++){
pal[i]=pal_mode10[i2][qbg_color_assign[i]];
}
}
*/

i=display_page->width*display_page->height;
i2=1<<display_page->bits_per_pixel;//unique colors
//data changed?
if (i!=pixeldatasize){
free(pixeldata);
pixeldata=(unsigned char*)malloc(i);
pixeldatasize=i;
goto update_display;
}
if (memcmp(pixeldata,display_page->offset,i)) goto update_display;
//palette changed?
if (memcmp(paldata,display_page->pal,i2*4)) goto update_display;
//force update because of mode change?
if (!screen_last_valid) goto update_display;
goto screen_refresh_unrequired;//no need to update display
update_display:
memcpy(pixeldata,display_page->offset,i);
memcpy(paldata,display_page->pal,i2*4);

if (x_scale==2){
static unsigned char *cp;
static unsigned long *lp2;
static unsigned long c;
cp=pixeldata;
lp2=display_surface_offset+x_monitor*y_offset+x_offset;
x2=display_page->width;
y2=display_page->height;
z=x2*8;
for (y=0;y<y2;y++){
for (x=0;x<x2;x++){
c=paldata[*cp++]; *lp2++=c; *lp2++=c;
}//x
lp2+=(x_monitor-(x2*2));
if (y_scale==2){memcpy(lp2,lp2-x_monitor,z);lp2+=x_monitor;}
}//y
goto screen_refreshed;
}

if (x_scale==1){
static unsigned char *cp;
static unsigned long *lp2;
static unsigned long c;
cp=pixeldata;
lp2=display_surface_offset+x_monitor*y_offset+x_offset;
x2=display_page->width;
y2=display_page->height;
z=x2*4;
for (y=0;y<y2;y++){
for (x=0;x<x2;x++){
*lp2++=paldata[*cp++];
}//x
lp2+=(x_monitor-x2);
if (y_scale==2){memcpy(lp2,lp2-x_monitor,z);lp2+=x_monitor;}
}//y
goto screen_refreshed;
}

error(4621);

screen_refreshed:
SDL_UpdateRect(display_surface,0,0,0,0);

screen_last_valid=1;
display_page_invalid:

/*
static long qbg_y_offset;//obselete?
qbg_y_offset=0;

//Video mode

x_offset=0; y_offset=0;
x_scale=1; y_scale=1;

if (qbg_text_only){
qbg_width=qbg_width_in_characters*qbg_character_width;
qbg_height=qbg_height_in_characters*qbg_character_height;
}
x=qbg_width; y=qbg_height;

//SCREEN 2/8 custom resolution
if (x==640&&y==200){
y_scale=2;
x=640; y=400;
}

x_monitor=x; y_monitor=y;

if (full_screen){

//fix 320x200 full screen resolution (SDL bug/incompatibility in low res. modes)
if (x==320&&y==200){
x_scale=2; y_scale=2;
x=640; y=400;
x_monitor=x; y_monitor=y;
}


//Check against available resolutions by priorities
//1. Exact match?
for (i=0;i<nmodes;i++){
if (modes[i]->w==x&&modes[i]->h==y) goto modeok;
}
//2. Same x, larger y?
y2=1000000;
for (i=0;i<nmodes;i++){
if (modes[i]->w==x){
if (modes[i]->h>y){
if (modes[i]->h<y2) y2=modes[i]->h;
}
}
}
if (y2!=1000000){
y_monitor=y2;
y_offset=(y_monitor-y)/2;
goto modeok;
}

//No mode seemed appropriate! Continue with default window resolution.

}//fullscreen
modeok:

if ((qbg_program_window_width!=x_monitor)||(qbg_program_window_height!=y_monitor)||(full_screen_in_use!=full_screen)){
qbg_program_window_width=x_monitor; qbg_program_window_height=y_monitor;


screen = SDL_SetVideoMode(qbg_program_window_width, qbg_program_window_height, 32, SDL_SWSURFACE|(SDL_FULLSCREEN*full_screen));

pixel=(unsigned long*)screen->pixels;
full_screen_in_use=full_screen;
}
if (!screen_last_valid){
memset(pixel,0,qbg_program_window_width*qbg_program_window_height);
}

if (qbg_text_only){
//if (frame&1) goto screen_refresh_unrequired;//~16fps for text mode
static long show_flashing_last=0;
static long show_cursor_last=0;
static long check_last;
static unsigned char *cp,*cp2,*cp_last;
static unsigned long *lp;
static long cx,cy;
static long cx_last=-1,cy_last=-1;
static long show_cursor;
static long show_flashing;
static unsigned char chr,col,chr_last,col_last;
static long screen_changed;

//realloc buffer if necessary
i=qbg_height_in_characters*qbg_width_in_characters*2;
if (screen_last_size!=i){
free(screen_last);
screen_last=(unsigned char*)malloc(i);
screen_last_size=i;
}

check_last=screen_last_valid;
if (!check_last){
 //set pal_last
 memcpy(&pal_last[0],&pal[0],256*4);
}else{
 //if palette has changed, update pal_last and draw all characters
 if (memcmp(&pal_last[0],&pal[0],256*4)){
 memcpy(&pal_last[0],&pal[0],256*4);
 check_last=0;
 }
}

//calculate cursor position (base 0)
cx=qbg_cursor_x; cy=qbg_cursor_y;
if (qbg_visual_page!=qbg_active_page){
cx=qbg_cursor_x_previous[qbg_visual_page]; cy=qbg_cursor_y_previous[qbg_visual_page];
}
cx--; cy--;

//show cursor?
if (frame&4) show_cursor=1; else show_cursor=0;

//show flashing?
if (frame&8) show_flashing=1; else show_flashing=0;

qbg_y_offset=(qbg_width*x_scale*y_offset);
cp=qbg_visual_page_offset;
cp_last=screen_last;

//outer loop
y2=0;
for (y=0;y<qbg_height_in_characters;y++){
x2=0;
for (x=0;x<qbg_width_in_characters;x++){

chr=*cp; cp++; col=*cp; cp++;

//can be skipped?
chr_last=*cp_last; cp_last++; col_last=*cp_last; cp_last++;
if (check_last){
if (chr==chr_last){//same character
if (col==col_last){//same colours
if (col&128) if (show_flashing!=show_flashing_last) goto cantskip;//same flash
if (x==cx) if (y==cy) if (show_cursor!=show_cursor_last) goto cantskip;//same cursor
if (x==cx_last){ if (y==cy_last){
if ((cx!=cx_last)||(cy!=cy_last)) goto cantskip;//fixup old cursor's location
}}
goto skip;
}}}
cantskip:
screen_changed=1;
cp_last-=2; *cp_last=chr; cp_last++; *cp_last=col; cp_last++;

//draw the character
if (qbg_character_height==8){
cp2=&charset8x8[chr][0][0];
}else{
if (qbg_character_height==16){
cp2=&charset8x16[chr][0][0];
}else{//assume 14
cp2=&charset8x16[chr][1][0];
}
}
if (qbg_character_width==16) z2=1; else z2=0;
//set the correct color
c=col&0xF;//foreground col
c2=(col>>4)&7;//background col
c3=col>>7;//flashing?
if (c3&&show_flashing) c=c2;
i2=pal[c];
i3=pal[c2];
lp=pixel+y2*qbg_width_in_characters*qbg_character_width+x2+qbg_y_offset;
z=qbg_width_in_characters*qbg_character_width-qbg_character_width;

//inner loop
for (y3=0;y3<qbg_character_height;y3++){
for (x3=0;x3<qbg_character_width;x3++){
if (*cp2) *lp=i2; else *lp=i3;
if (z2){
if (x3&z2) cp2++;
}else{
cp2++;
}
lp++;
}
lp+=z;
}//y3,x3



//draw cursor?
if (qbg_cursor_show&&show_cursor&&(cx==x)&&(cy==y)){
static long v1,v2;
static unsigned char from_bottom;//bottom is the 2nd bottom scanline in width ?x25
static unsigned char half_cursor;//if set, overrides all following values
static unsigned char size;//if 0, no cursor is drawn, if 255, from begin to bottom
static unsigned char begin;//only relevant if from_bottom was not specified
v1=qbg_cursor_firstvalue;
v2=qbg_cursor_lastvalue;
from_bottom=0;
half_cursor=0;
size=0;
begin=0;
//RULE: IF V2=0, NOTHING (UNLESS V1=0)
if (v2==0){
if (v1==0){size=1; goto cursor_created;}
goto nocursor;//no cursor!
}
//RULE: IF V2<V1, FROM V2 TO BOTTOM
if (v2<v1){begin=v2; size=255; goto cursor_created;}
//RULE: IF V1=V2, SINGLE SCANLINE AT V1 (OR BOTTOM IF V1>=4)
if (v1==v2){
if (v1<=3){begin=v1; size=1; goto cursor_created;}
from_bottom=1; size=1; goto cursor_created;
}
//NOTE: V2 MUST BE LARGER THAN V1!
//RULE: IF V1>=3, CALC. DIFFERENCE BETWEEN V1 & V2
//                IF DIFF=1, 2 SCANLINES AT BOTTOM
//                IF DIFF=2, 3 SCANLINES AT BOTTOM
//                OTHERWISE HALF CURSOR
if (v1>=3){
if ((v2-v1)==1){from_bottom=1; size=2; goto cursor_created;}
if ((v2-v1)==2){from_bottom=1; size=3; goto cursor_created;}
half_cursor=1; goto cursor_created;
}
//RULE: IF V1<=1, IF V2<=3 FROM V1 TO V3 ELSE FROM V1 TO BOTTOM
if (v1<=1){
if (v2<=3){begin=v1;size=v2-v1+1; goto cursor_created;} 
begin=v1;size=255; goto cursor_created;
}
//RULE: IF V1=2, IF V2=3, 2 TO 3
//               IF V2=4, 3 SCANLINES AT BOTTOM
//               IF V2>=5, FROM 2 TO BOTTOM
//(assume V1=2)
if (v2==3){begin=2;size=2; goto cursor_created;}
if (v2==4){from_bottom=1; size=3; goto cursor_created;}
begin=2;size=255;
cursor_created:

if (half_cursor){
//half cursor
y3=qbg_character_height-1;
size=qbg_character_height/2;
c=col&0xF;//foreground col
i2=pal[c];
draw_half_curs:
lp=pixel+(y2+y3)*qbg_width_in_characters*qbg_character_width+x2+qbg_y_offset;
for (x3=0;x3<qbg_character_width;x3++){
*lp=i2;
lp++;
}
y3--;
size--;
if (size) goto draw_half_curs;
}else{
if (from_bottom){
//cursor from bottom
y3=qbg_character_height-1;
if (y3==15) y3=14;//leave bottom line blank in 8x16 char set
c=col&0xF;//foreground col
i2=pal[c];
draw_curs_from_bottom:
lp=pixel+(y2+y3)*qbg_width_in_characters*qbg_character_width+x2+qbg_y_offset;
for (x3=0;x3<qbg_character_width;x3++){
*lp=i2;
lp++;
}
y3--;
size--;
if (size) goto draw_curs_from_bottom;
}else{
//cursor from begin using size
if (begin<qbg_character_height){
y3=begin;
c=col&0xF;//foreground col
i2=pal[c];
if (size==255) size=qbg_character_height-begin;
draw_curs_from_begin:
lp=pixel+(y2+y3)*qbg_width_in_characters*qbg_character_width+x2+qbg_y_offset;
for (x3=0;x3<qbg_character_width;x3++){
*lp=i2;
lp++;
}
y3++;
size--;
if (size) goto draw_curs_from_begin;
}
}
}
}//draw cursor?
nocursor:



//outer loop
skip:
x2=x2+qbg_character_width;
}
y2=y2+qbg_character_height;
}

show_flashing_last=show_flashing;
show_cursor_last=show_cursor;
cx_last=cx;
cy_last=cy;
screen_last_valid=1;

if (!screen_changed) goto screen_refresh_unrequired;
goto screen_refreshed;
}//text only









//graphical modes

//update SCREEN 10 palette
if (qbg_mode==10){
//pal_mode10[0-1][0-8]
i2=SDL_GetTicks()&512;
if (i2) i2=1;
for (i=0;i<=3;i++){
pal[i]=pal_mode10[i2][qbg_color_assign[i]];
}
}

i=qbg_width*qbg_height;
i2=1<<qbg_bits_per_pixel;//i2 is number of colors

//data changed?
if (i!=pixeldatasize){
free(pixeldata);
pixeldata=(unsigned char*)malloc(i);
pixeldatasize=i;
goto update_display;
}
if (memcmp(pixeldata,qbg_visual_page_offset,i)) goto update_display;

//palette changed?
if (memcmp(paldata,pal,i2*4)) goto update_display;

//force update because of mode change?
if (!screen_last_valid) goto update_display;

goto screen_refresh_unrequired;//no need to update display

update_display:

memcpy(pixeldata,qbg_visual_page_offset,i);
memcpy(paldata,pal,i2*4);
screen_last_valid=1;

//scaled refresh

if (x_scale==2){
static unsigned char *cp;
static unsigned long *lp;
static unsigned long c;
cp=pixeldata;
lp=pixel+(qbg_width*x_scale*y_offset);
for (y=0;y<qbg_height;y++){
for (x=0;x<qbg_width;x++){
c=paldata[*cp++]; *lp++=c; *lp++=c;
}//x
 if (y_scale==2){
 memcpy(lp,lp-qbg_width*2,qbg_width<<3);
 lp+=(qbg_width*2);
 }
}//y
goto screen_refreshed;
}

if (x_scale==1){
static unsigned char *cp;
static unsigned long *lp;
static unsigned long c;
cp=pixeldata;
lp=pixel+(qbg_width*x_scale*y_offset);
for (y=0;y<qbg_height;y++){
for (x=0;x<qbg_width;x++){
*lp++=paldata[*cp++];
}//x
 if (y_scale==2){
 memcpy(lp,lp-qbg_width,qbg_width<<2);
 lp+=qbg_width;
 }
}//y
goto screen_refreshed;
}


/*





//full screen 320x200 resolution correction
if (full_screen){
if (qbg_width==320&&qbg_height==200){
static unsigned char *cp;
static unsigned long *lp,*lp2;
static unsigned long c;
cp=qbg_visual_page_offset;
lp=pixel;
lp2=pixel+640;
for (y=0;y<200;y++){
for (x=0;x<320;x++){
c=pal[*cp++];
*lp++=c; *lp++=c;
*lp2++=c; *lp2++=c;
}
lp+=640; lp2+=640;
}
goto screen_refreshed;
}
}

//SCREEN 2/8 double y-axis
if (qbg_width==640&&qbg_height==200){
unsigned char *cp;
unsigned long *lp;
cp=qbg_visual_page_offset;
lp=pixel;
i2=qbg_program_window_width*qbg_program_window_height;
i3=0;
for (i=0;i<i2;i++){
*lp=pal[*cp];
i3++;
if (i3==640){cp-=640;i3=-640;}
cp++;
lp++;
}
goto screen_refreshed;
}

if (qbg_bytes_per_pixel==1){
static unsigned char *cp;
static unsigned long *lp;
cp=pixeldata;
lp=pixel+qbg_y_offset*qbg_width;
i2=qbg_width*qbg_height;

for (i=0;i<i2;i++){
*lp=pal[*cp];
cp++;
lp++;
}


}
*/

//screen_refreshed:
//SDL_UpdateRect(screen, 0, 0, 0, 0);
//SDL_UpdateRect(screen_surface,0,0,0,0);

//update display surface
//memcpy(display_page_surface->pixels,display_page->offset,1024*768*4);



//SDL_UpdateRect(display_page_surface,0,0,0,0);
screen_refresh_unrequired:;


}//lock_display==0

if (lock_display==1){lock_display=2; Sleep(0);}





}//update==1

vertical_retrace_in_progress=0;

if (shell_call_in_progress) goto main_loop;

if (update==0){

//correct SDL key repeat error after losing input focus while a key is held
//occurs when switching to another window, SHELL-ing, etc.
//SDL_APPMOUSEFOCUS	The application has mouse focus.
//SDL_APPINPUTFOCUS	The application has keyboard focus
//SDL_APPACTIVE	The application is visible
static long state;
state=SDL_GetAppState();
if ( (!(state&SDL_APPINPUTFOCUS)) || (!(state&SDL_APPACTIVE)) ){//lost focus
if (key_repeat_on){SDL_EnableKeyRepeat(0,0); key_repeat_on=0;}
}else{
//active
if (!key_repeat_on){SDL_EnableKeyRepeat(500,30); key_repeat_on=1;}
}

//update input
static long scancode;
static const unsigned char sdlk_2_scancode[] = {
0,0,0,0,0,0,0,0,14,15,0,0,0,28,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,57,0,0,0,0,0,0,40,0,0,0,0,51,12,52,53,11,2,3,4,5,6,7,8,9,10,0,39,0,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,43,27,0,0,41,30,48,46,32,18,33,34,35,23,36,37,38,50,49,24,25,16,19,31,20,22,47,17,45,21,44,0,0,0,0,83,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,82,79,80,81,75,76,77,71,72,73,83,53,55,74,78,28,0,72,80,77,75,82,71,79,73,81,59,60,61,62,63,64,65,66,67,68,133,134,0,0,0,0,0,0,69,58,70,54,42,29,29,56,56,0,0,91,92,0,0,0,0,55,197,93,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static SDL_Event event;

    while ( SDL_PollEvent(&event) ) {

asciicode_force=0;
        switch (event.type) {
/*
case SDL_ACTIVEEVENT:
 #ifdef QB64_WINDOWS
 if (shell_call_in_progress){
 if (event.active.gain==0){//lost focus
 if (full_screen){ 
 SDL_WM_IconifyWindow();
 }//full screen
 }//lost focus
 }//shell
 #endif
break;
*/

case SDL_KEYUP:
if (event.key.keysym.sym){//ignore NULL (unknown key)
if (event.key.keysym.sym==SDLK_PRINT) goto ignore_sdl_keyup;
if (event.key.keysym.sym==SDLK_SYSREQ) goto ignore_sdl_keyup;

sdl_shiftstate=event.key.keysym.mod;//update sdl shiftstate
if (event.key.keysym.sym<65536){
scancode=-1; if (event.key.keysym.sym<1024) scancode=sdlk_2_scancode[event.key.keysym.sym];
keyon[event.key.keysym.sym]=0;
//prepare virtual locks for future toggle
if (event.key.keysym.sym==SDLK_SCROLLOCK) sdl_scroll_lock_prepared=1;
if (event.key.keysym.sym==SDLK_INSERT) sdl_insert_prepared=1;
if (port60h_events==256){memmove(port60h_event,port60h_event+1,255); port60h_events=255;}
port60h_event[port60h_events]=scancode+128;
port60h_events++;
}
 if ((event.key.keysym.sym==SDLK_LALT)||(event.key.keysym.sym==SDLK_RALT)){
 if (asciicode_reading){
 asciicode_reading=0; 
 asciicode_force=1;
 event.key.keysym.sym=(SDLKey)65536;//unknown sym
 event.key.keysym.unicode=asciicode_value&255;
 asciicode_value=0;
 goto pushkey; 
 }
 }
}
ignore_sdl_keyup:;
break;

case SDL_KEYDOWN:
if (event.key.keysym.sym){//ignore 0 (unknown key)

sdl_shiftstate=event.key.keysym.mod;//update sdl shiftstate
pushkey:
sleep_break=1;
static unsigned long key;
key=event.key.keysym.sym;
/*
(&H417 in QB)
40:0017   2  Keyboard status bits; see Keyboard Shift Status Flags
40:0019   1  Current (accumulating) value of Alt+numpad pseudo-key input;
             normally 0.  When [Alt] is released, value is stored in
             keyboard buffer at 001e.
40:001a   2  Addr of keyboard buffer head (keystroke at that addr is next)
40:001c   2  Address of keyboard buffer tail
***range from 30 to 60 even number only***
***if head=tail then no data is available***
40:001e  32  Keyboard buffer.  BIOS stores keystrokes here (head and tail
             point to addresses from 041eH to 043dH inclusive).
***first byte is ASCII CHR or 0***
***second byte is the scancode***
*/
//"If the high 9 bits of the character are 0, then this maps to the equivalent ASCII character"
if (event.key.keysym.sym<65536){
scancode=-1; if (event.key.keysym.sym<1024) scancode=sdlk_2_scancode[event.key.keysym.sym];
keyon[event.key.keysym.sym]=1;
if (port60h_events==256){memmove(port60h_event,port60h_event+1,255); port60h_events=255;}
port60h_event[port60h_events]=scancode;
port60h_events++;
}



//update virtual locks
if ((event.key.keysym.sym==SDLK_SCROLLOCK)&&(sdl_scroll_lock_prepared)){sdl_scroll_lock=(sdl_scroll_lock+1)&1; sdl_scroll_lock_prepared=0;}
if ((event.key.keysym.sym==SDLK_INSERT)&&(sdl_insert_prepared)){sdl_insert=(sdl_insert+1)&1; sdl_insert_prepared=0;}
//ALT-ENTER handling
if (keyon[SDLK_RALT]||keyon[SDLK_LALT]){
if ((event.key.keysym.sym==SDLK_KP_ENTER)||(event.key.keysym.sym==SDLK_RETURN)){
full_screen_toggle++;
goto silent_key;
}
}
//CTRL-BREAK handling
if (keyon[SDLK_RCTRL]||keyon[SDLK_LCTRL]){
if ((event.key.keysym.sym==SDLK_BREAK)||(event.key.keysym.sym==SDLK_PAUSE)||(event.key.keysym.sym==302)){//scroll lock can occur instead of break!
goto end_program;
}
}
/* Skip special keys
  SDLK_NUMLOCK		= 300,
	SDLK_CAPSLOCK		= 301,
	SDLK_SCROLLOCK		= 302,
	SDLK_RSHIFT		= 303,
	SDLK_LSHIFT		= 304,
	SDLK_RCTRL		= 305,
	SDLK_LCTRL		= 306,
	SDLK_RALT		= 307,
  SDLK_LALT		= 308,
*/


if ((key>=300)&&(key<=308)){
goto silent_key;
}


//ALT+number(ascii-code)
//note: "Compose" an ASCII code for digits typed on the numeric keypad while ALT is held down. The code is the number (modulo 256).
if ((event.key.keysym.sym==SDLK_LALT)||(event.key.keysym.sym==SDLK_RALT)){
asciicode_value=0;
asciicode_reading=0;
}
/* Numeric keypad
	SDLK_KP0		= 256,
	SDLK_KP1		= 257,
	SDLK_KP2		= 258,
	SDLK_KP3		= 259,
	SDLK_KP4		= 260,
	SDLK_KP5		= 261,
	SDLK_KP6		= 262,
	SDLK_KP7		= 263,
	SDLK_KP8		= 264,
	SDLK_KP9		= 265,*/
if (keyon[SDLK_RALT]||keyon[SDLK_LALT]){
if ((event.key.keysym.sym>=256)&&(event.key.keysym.sym<=265)){
asciicode_value*=10;
asciicode_value+=(event.key.keysym.sym-256);
asciicode_reading=1;
goto silent_key;
}
}



//BREAK handling
if ((event.key.keysym.sym==SDLK_BREAK)||(event.key.keysym.sym==SDLK_PAUSE)){
suspend_program|=1;
qbevent=1;
goto silent_key;
}else{
if (suspend_program&1){
suspend_program^=1;
goto silent_key;
}
}

if (asciicode_force) goto asciicode_forced;
if ((event.key.keysym.unicode&0xFF80)==0){
asciicode_forced:
c=event.key.keysym.unicode&0xFF;
if (c){

//patch to force Linux to recognise [Delete] key
//(was returning ASCII character 127)
#ifdef QB64_LINUX
if (c==127){
scancode=83;
goto forcescancodekey;
}
#endif

//add to cyclic buffer
i=cmem[0x41a];
i2=cmem[0x41c];
i3=i2+2;
if (i3==62) i3=30;
if (i!=i3){//fits in buffer
if (c==9){
if (sdl_shiftstate&(KMOD_LSHIFT|KMOD_RSHIFT)){x=15; goto specialscancode;}
}
if (sdl_shiftstate&(KMOD_LALT|KMOD_RALT)){
if (event.key.keysym.sym==SDLK_KP_PERIOD) goto silent_key;
if (event.key.keysym.sym==SDLK_KP_DIVIDE){x=164; goto specialscancode;}
if (event.key.keysym.sym==SDLK_KP_MULTIPLY){x=55; goto specialscancode;}
if (event.key.keysym.sym==SDLK_KP_MINUS){x=74; goto specialscancode;}
if (event.key.keysym.sym==SDLK_KP_PLUS){x=78; goto specialscancode;}
if (scancode==15){x=165; goto specialscancode;}//Tab
if ((scancode>=51)&&(scancode<=53)){x=scancode; goto specialscancode;}// , . /
if (scancode==14){x=14; goto specialscancode;}//Backspace
if (scancode==1){x=1; goto specialscancode;}//ESC
if (scancode==133){x=139; goto specialscancode;}//F11
if (scancode==134){x=140; goto specialscancode;}//F12
if ((scancode>=16)&&(scancode<=50)){
x=scancode; goto specialscancode;
}
if ((scancode>=2)&&(scancode<=13)){
x=scancode-2+120; goto specialscancode;
}
}
cmem[0x400+i2]=c;
cmem[0x400+i2+1]=scancode;
cmem[0x41c]=i3;//fix tail location
goto gotkey;
}
}//c!=0
}else{
//printf("An International Character.\n");
goto gotkey;
}

//ASCII translation not possible... pass CHR$(0)+scan code
if (event.key.keysym.sym==SDLK_SYSREQ) goto silent_key;
forcescancodekey:
i=cmem[0x41a];
i2=cmem[0x41c];
i3=i2+2;
if (i3==62) i3=30;
if (i!=i3){//buffer is not full
x=scancode;
//prioritise alt(most)->ctrl->shift(least)
if (sdl_shiftstate&(KMOD_LALT|KMOD_RALT)){
//alt held
if (event.key.keysym.sym==SDLK_KP_PERIOD) goto silent_key;
if (event.key.keysym.sym==SDLK_KP_DIVIDE){x=164; goto specialscancode;}
if (event.key.keysym.sym==SDLK_KP_MULTIPLY){x=55; goto specialscancode;}
if (event.key.keysym.sym==SDLK_KP_MINUS){x=74; goto specialscancode;}
if (event.key.keysym.sym==SDLK_KP_PLUS){x=78; goto specialscancode;}
if (scancode==15){x=165; goto specialscancode;}//Tab
if ((scancode>=51)&&(scancode<=53)){x=scancode; goto specialscancode;}// , . /
if (scancode==14){x=14; goto specialscancode;}//Backspace
if (scancode==1){x=1; goto specialscancode;}//ESC
if (scancode==133){x=139; goto specialscancode;}//F11
if (scancode==134){x=140; goto specialscancode;}//F12
if ((scancode>=2)&&(scancode<=13)){
x=scancode-2+120; goto specialscancode;
}
if ((scancode>=16)&&(scancode<=50)){
x=scancode; goto specialscancode;
}
//review following?:
if ((x>=59)&&(x<=68)){x+=45; goto specialscancode;}
if ((x>=71)&&(x<=83)){x=x-71+151; goto specialscancode;}
}else{//alt not held
if (sdl_shiftstate&(KMOD_LCTRL|KMOD_RCTRL)){
//ctrl held
if (event.key.keysym.sym==SDLK_PRINT){x=148; goto specialscancode;}//Prt-Scrn
if (event.key.keysym.sym==SDLK_KP_DIVIDE){x=149; goto specialscancode;}
if (event.key.keysym.sym==SDLK_KP_MULTIPLY){x=150; goto specialscancode;}
if (scancode==41) goto silent_key;//~ or `
if (scancode==2) goto silent_key;//1
if ((scancode>=4)&&(scancode<=6)) goto silent_key;//3,4,5
if (scancode==7){//6
cmem[0x400+i2]=30; cmem[0x400+i2+1]=scancode; cmem[0x41c]=i3; goto gotkey;
}
if ((scancode>=8)&&(scancode<=11)) goto silent_key;//7,8,9,0
if (scancode==12){//-
cmem[0x400+i2]=31; cmem[0x400+i2+1]=scancode; cmem[0x41c]=i3; goto gotkey;
}
if (scancode==13) goto silent_key;//= or +
if (scancode==15){x=148; goto specialscancode;}
if (scancode==39) goto silent_key;//; or :
if (scancode==40) goto silent_key;//' or "
if ((scancode>=51)&&(scancode<=53)) goto silent_key;//,,<,.,>,/,?
if (scancode==69) goto silent_key;//num-lock
if (scancode==133){x=137; goto specialscancode;}
if (scancode==134){x=138; goto specialscancode;}
if (scancode==1){//ESC (*cannot be tested in windows)
cmem[0x400+i2]=27; cmem[0x400+i2+1]=scancode; cmem[0x41c]=i3; goto gotkey;
}
//review following?:
if ((x>=59)&&(x<=68)){x+=35; goto specialscancode;}
if (x==55){x=114; goto specialscancode;}
if (x==75){x=115; goto specialscancode;}
if (x==77){x=116; goto specialscancode;}
if (x==79){x=117; goto specialscancode;}
if (x==81){x=118; goto specialscancode;}
if (x==71){x=119; goto specialscancode;}
if (x==73){x=132; goto specialscancode;}
if (x==72){x=141; goto specialscancode;}
if (x==74){x=142; goto specialscancode;}
if (x==76){x=143; goto specialscancode;}
if (x==78){x=144; goto specialscancode;}
if (x==80){x=145; goto specialscancode;}
if (x==82){x=146; goto specialscancode;}
if (x==83){x=147; goto specialscancode;}
}else{//ctrl not held
if (sdl_shiftstate&(KMOD_LSHIFT|KMOD_RSHIFT)){
//shift held
if (scancode==133){x=135; goto specialscancode;}//F11
if (scancode==134){x=136; goto specialscancode;}//F12
if ((event.key.keysym.sym>=SDLK_KP0)&&(event.key.keysym.sym<=SDLK_KP9)){//SHIFT+numpad->number's ASCII value
cmem[0x400+i2]=event.key.keysym.sym-SDLK_KP0+48;
cmem[0x400+i2+1]=scancode;//(unused)
cmem[0x41c]=i3;//fix tail location
goto gotkey;
}
if ((event.key.keysym.sym==SDLK_KP_PERIOD)){//SHIFT+numpad .->"."
cmem[0x400+i2]=46;
cmem[0x400+i2+1]=scancode; cmem[0x41c]=i3; goto gotkey;
}
//review following?:
if ((x>=59)&&(x<=68)){x+=25; goto specialscancode;}
}//shift held
}//alt not held
}//ctrl not held
if (event.key.keysym.sym==SDLK_PRINT) goto silent_key;
specialscancode:



cmem[0x400+i2]=0;
cmem[0x400+i2+1]=x;
cmem[0x41c]=i3;//fix tail location
goto gotkey;
}
gotkey:;
silent_key:;
}
ignore_key:;
break;

case SDL_MOUSEMOTION:
last_mouse_message++; if (last_mouse_message>1023) last_mouse_message=0;
//delete oldest message if too many exist in the queue
if (last_mouse_message==current_mouse_message){
current_mouse_message++; if (current_mouse_message>1023) current_mouse_message=0;
}
mouse_messages[last_mouse_message].x=event.motion.x;
mouse_messages[last_mouse_message].y=event.motion.y;
mouse_messages[last_mouse_message].buttons=event.motion.state;
break;

case SDL_MOUSEBUTTONUP:
static unsigned long oldbuttonstate;
oldbuttonstate=mouse_messages[last_mouse_message].buttons;
last_mouse_message++; if (last_mouse_message>1023) last_mouse_message=0;
//delete oldest message if too many exist in the queue
if (last_mouse_message==current_mouse_message){
current_mouse_message++; if (current_mouse_message>1023) current_mouse_message=0;
}
mouse_messages[last_mouse_message].x=event.button.x;
mouse_messages[last_mouse_message].y=event.button.y;
oldbuttonstate=oldbuttonstate^(1<<(event.button.button-1));
mouse_messages[last_mouse_message].buttons=oldbuttonstate;
break;

case SDL_MOUSEBUTTONDOWN:
static unsigned long oldbuttonstate2;
oldbuttonstate2=mouse_messages[last_mouse_message].buttons;
last_mouse_message++; if (last_mouse_message>1023) last_mouse_message=0;
//delete oldest message if too many exist in the queue
if (last_mouse_message==current_mouse_message){
current_mouse_message++; if (current_mouse_message>1023) current_mouse_message=0;
}
mouse_messages[last_mouse_message].x=event.button.x;
mouse_messages[last_mouse_message].y=event.button.y;
oldbuttonstate2=oldbuttonstate2|(1<<(event.button.button-1));
mouse_messages[last_mouse_message].buttons=oldbuttonstate2;
break;

case SDL_QUIT:
goto end_program;
}
}

//update shift status bits
/*
0:417h                   Shift Status
               7 6 5 4 3 2 1 0
               x . . . . . . .      Insert locked
               . x . . . . . .      Caps Lock locked
               . . x . . . . .      Num Lock locked
               . . . x . . . .      Scroll Lock locked
               . . . . x . . .      Alt key is pressed
               . . . . . x . .      Ctrl key is pressed
               . . . . . . x .      Left Shift key is pressed
               . . . . . . . x      Right Shift key is pressed
*/
x=0;
if (keyon[SDLK_RSHIFT]) x|=1;
if (keyon[SDLK_LSHIFT]) x|=2;
if (keyon[SDLK_LCTRL]||keyon[SDLK_RCTRL]) x|=4;
if (keyon[SDLK_LALT]||keyon[SDLK_RALT]) x|=8;
//note: scroll lock state is emulated (off by default)
if (sdl_scroll_lock) x|=16;
if (sdl_shiftstate&KMOD_NUM) x|=32;
if (sdl_shiftstate&KMOD_CAPS) x|=64;
//note: insert state is emulated (off by default)
if (sdl_insert) x|=128;
cmem[0x417]=x;
/*
typedef enum {
	KMOD_NONE  = 0x0000,
	KMOD_LSHIFT= 0x0001,
	KMOD_RSHIFT= 0x0002,
	KMOD_LCTRL = 0x0040,
	KMOD_RCTRL = 0x0080,
	KMOD_LALT  = 0x0100,
	KMOD_RALT  = 0x0200,
	KMOD_LMETA = 0x0400,
	KMOD_RMETA = 0x0800,
	KMOD_NUM   = 0x1000,
	KMOD_CAPS  = 0x2000,
	KMOD_MODE  = 0x4000,
	KMOD_RESERVED = 0x8000
} SDLMod;
*/

/*
0:418h                   Extended Shift Status
 (interpret the work pressed as "being held down")
               7 6 5 4 3 2 1 0
               x . . . . . . .      Ins key is pressed
               . x . . . . . .      Caps Lock key is pressed (returns same as caps for 417!!!, not held status)
               . . x . . . . .      Num Lock key is pressed (same as 417!)
               . . . x . . . .      Scroll Lock key is pressed
               . . . . x . . .      Pause key locked (always 0)
               . . . . . x . .      SysReq key is pressed (always 0)
               . . . . . . x .      Left Alt key is pressed (helps to distinguish)
               . . . . . . . x      Left Ctrl key is pressed
*/
x=0;
if (keyon[SDLK_LCTRL]) x|=1;
if (keyon[SDLK_LALT]) x|=2;
if (keyon[SDLK_SYSREQ]) x|=4;
if (keyon[SDLK_BREAK]||keyon[SDLK_PAUSE]) x|=8;
if (keyon[SDLK_SCROLLOCK]) x|=16;
if (keyon[SDLK_NUMLOCK]) x|=32;
if (keyon[SDLK_CAPSLOCK]) x|=64;
if (keyon[SDLK_INSERT]) x|=128;
cmem[0x418]=x;
/*
0:496h                   Keyboard Status and Type Flags
    This byte holds keyboard status information.
                      Keyboard Status Information
            7 6 5 4 3 2 1 0
            x . . . . . . .       Read ID in progress (always 0)
            . x . . . . . .       Last character was first ID character (always 0)
            . . x . . . . .       Force Num Lock if read ID and KBX (always 0)
            . . . x . . . .       101/102-key keyboard installed (always 1)
            . . . . x . . .       Right Alt key is pressed
            . . . . . x . .       Right Ctrl key is pressed
            . . . . . . x .       Last code was E0 Hidden Code (always 0)
            . . . . . . . x       last code was E1 Hidden Code (always 0)
*/
x=0;
if (keyon[SDLK_RCTRL]) x|=4;
if (keyon[SDLK_RALT]) x|=8;
x|=16;
cmem[0x496]=x;

if (shell_call_in_progress) goto main_loop;



//safe even if sndsetup not called
static double pos;
if (stream_limited){
if (stream_loaded){
if (stream_playing){//worth checking?
pos=func__sndgetpos(1);
if (pos>=stream_limit){
sub__sndstop(1);
stream_limited=0;
}}}}

}//update==0

goto main_loop;

end_program:
stop_program=1;
qbevent=1;
SDL_WaitThread(thread, NULL);
SDL_ShowCursor(0);
SDL_FreeCursor(mycursor);


exit(0);
}