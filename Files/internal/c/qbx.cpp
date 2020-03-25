//definitions common to qbxlib & qbx-qbmain
#include "common.cpp"

//extern functions
extern int MessageBox(int p1,char* p2,char* p3,int p4);
extern uint32 _lrotl(uint32 word,uint32 shift);
extern int64 qbr(long double f);
extern uint64 qbr_longdouble_to_uint64(long double f);
extern long qbr_float_to_long(float f);
extern long qbr_double_to_long(double f);
extern int64 qbr(long double f);
extern uint64 qbr_longdouble_to_uint64(long double f);
extern long qbr_float_to_long(float f);
extern long qbr_double_to_long(double f);
extern void restorepalette(img_struct* im);
extern void pset(long x,long y,unsigned long col);
extern unsigned long newimg();
extern long freeimg(unsigned long i);
extern void imgrevert(long i);
extern long imgframe(unsigned char *o,long x,long y,long bpp);
extern long imgnew(long x,long y,long bpp);
extern long imgload(char *filename,long bpp);
extern void sub__putimage(long dstep1,double f_dx1,double f_dy1,long dstep2,double f_dx2,double f_dy2,long src,long dst,long sstep1,double f_sx1,double f_sy1,long sstep2,double f_sx2,double f_sy2,long passed);
extern long fontopen(char *name,double d_height,long flags);
extern long selectfont(long f,img_struct *im);
extern void sndsetup();
extern uint32 sib();
extern uint32 sib_mod0();
extern uint8 *rm8();
extern uint16 *rm16();
extern uint32 *rm32();
extern void cpu_call();
extern int64 build_int64(unsigned long val2,unsigned long val1);
extern uint64 build_uint64(unsigned long val2,unsigned long val1);
extern void fix_error();
extern void error(long error_number);
extern double get_error_erl();
extern unsigned long get_error_err();
extern void end();
extern unsigned char *mem_static_malloc(unsigned long size);
extern void mem_static_restore(unsigned char* restore_point);
extern unsigned char *cmem_dynamic_malloc(unsigned long size);
extern void cmem_dynamic_free(unsigned char *block);
extern void sub_defseg(long segment,long passed);
extern long func_peek(long offset);
extern void sub_poke(long offset,long value);
extern long array_check(unsigned long index,unsigned long limit);
extern void more_return_points();
extern void qb64_generatesound(double f,double l,unsigned char wait);
extern unsigned char *soundwave(double frequency,double length,double volume,double fadein,double fadeout);
extern long wavesize(double length);
extern qbs *qbs_new_descriptor();
extern void qbs_free_descriptor(qbs *str);
extern void qbs_free(qbs *str);
extern void qbs_cmem_concat_list();
extern void qbs_concat_list();
extern void qbs_tmp_concat_list();
extern void qbs_concat(unsigned long bytesrequired);
extern void qbs_concat_cmem(unsigned long bytesrequired);
extern qbs *qbs_new_cmem(long size,unsigned char tmp);
extern qbs *qbs_new_txt(const char *txt);
extern qbs *qbs_new_txt_len(const char *txt,long len);
extern qbs *qbs_new_fixed(unsigned char *offset,unsigned long size,unsigned char tmp);
extern qbs *qbs_new(long size,unsigned char tmp);
extern qbs *qbs_set(qbs *deststr,qbs *srcstr);
extern qbs *qbs_add(qbs *str1,qbs *str2);
extern qbs *qbs_ucase(qbs *str);
extern qbs *qbs_lcase(qbs *str);
extern qbs *func_chr(long value);
extern qbs *func_varptr_helper(unsigned char type,unsigned short offset);
extern qbs *qbs_left(qbs *str,long l);
extern qbs *qbs_right(qbs *str,long l);
extern qbs *func_mksmbf(float val);
extern qbs *func_mkdmbf(double val);
extern float func_cvsmbf(qbs *str);
extern double func_cvdmbf(qbs *str);
extern qbs *bit2string(unsigned long bsize,int64 v);
extern qbs *ubit2string(unsigned long bsize,uint64 v);
extern uint64 string2ubit(qbs*str,unsigned long bsize);
extern int64 string2bit(qbs*str,unsigned long bsize);
extern void sub_lset(qbs *dest,qbs *source);
extern void sub_rset(qbs *dest,qbs *source);
extern qbs *func_space(long spaces);
extern qbs *func_string(long characters,long asciivalue);
extern long func_instr(long start,qbs *str,qbs *substr,long passed);
extern void sub_mid(qbs *dest,long start,long l,qbs* src,long passed);
extern qbs *func_mid(qbs *str,long start,long l,long passed);
extern qbs *qbs_ltrim(qbs *str);
extern qbs *qbs_rtrim(qbs *str);
extern qbs *qbs_inkey();
extern qbs *qbs_str(int64 value);
extern qbs *qbs_str(long value);
extern qbs *qbs_str(short value);
extern qbs *qbs_str(char value);
extern qbs *qbs_str(uint64 value);
extern qbs *qbs_str(unsigned long value);
extern qbs *qbs_str(unsigned short value);
extern qbs *qbs_str(unsigned char value);
extern qbs *qbs_str(float value);
extern qbs *qbs_str(double value);
extern qbs *qbs_str(long double value);
extern long qbs_equal(qbs *str1,qbs *str2);
extern long qbs_notequal(qbs *str1,qbs *str2);
extern long qbs_greaterthan(qbs *str1,qbs *str2);
extern long qbs_lessthan(qbs *str1,qbs *str2);
extern long qbs_lessorequal(qbs *str1,qbs *str2);
extern long qbs_greaterorequal(qbs *str1,qbs *str2);
extern long qbs_asc(qbs *str);
extern long qbs_len(qbs *str);
extern void lineclip(long x1,long y1,long x2,long y2,long xmin,long ymin,long xmax,long ymax);
extern void qbg_palette(unsigned long attribute,unsigned long col,long passed);
extern void qbg_sub_color(unsigned long col1,unsigned long col2,unsigned long bordercolor,long passed);
extern void defaultcolors();
extern void validatepage(long n);
extern void qbg_screen(long mode,long color_switch,long active_page,long visual_page,long refresh,long passed);
extern void sub_pcopy(long src,long dst);
extern void qbsub_width(long option,long value1,long value2,long passed);
extern void pset(long x,long y,unsigned long col);
extern void pset_and_clip(long x,long y,unsigned long col);
extern void qb32_boxfill(float x1f,float y1f,float x2f,float y2f,unsigned long col);
extern void fast_boxfill(long x1,long y1,long x2,long y2,unsigned long col);
extern void fast_line(long x1,long y1,long x2,long y2,unsigned long col);
extern void qb32_line(float x1f,float y1f,float x2f,float y2f,unsigned long col,unsigned long style);
extern void sub_line(long step1,float x1,float y1,long step2,float x2,float y2,unsigned long col,long bf,unsigned long style,long passed);
extern void sub_paint32(long step,float x,float y,unsigned long fillcol,unsigned long bordercol,long passed);
extern void sub_paint32x(long step,float x,float y,unsigned long fillcol,unsigned long bordercol,long passed);
extern void sub_paint(long step,float x,float y,unsigned long fillcol,unsigned long bordercol,qbs *backgroundstr,long passed);
extern void sub_paint(long step,float x,float y,qbs *fillstr,unsigned long bordercol,qbs *backgroundstr,long passed);
extern void sub_circle(long step,double x,double y,double r,unsigned long col,double start,double end,double aspect,long passed);
extern unsigned long point(long x,long y);
extern double func_point(float x,float y,long passed);
extern void qbg_pset(long step,float x,float y,unsigned long col,long passed);
extern void sub_preset(long step,float x,float y,unsigned long col,long passed);
extern void printchr(long character);
extern long printchr2(long x,long y,unsigned long character,long i);
extern long chrwidth(long character);
extern void newline();
extern void makefit(qbs *text);
extern void tab();
extern void qbs_print(qbs* str,long finish_on_new_line);
extern long qbs_cleanup(unsigned long base,long passvalue);
extern void qbg_sub_window(long screen,float x1,float y1,float x2,float y2,long passed);
extern void qbg_sub_view_print(long topline,long bottomline,long passed);
extern void qbg_sub_view(long coords_relative_to_screen,long x1,long y1,long x2,long y2,long fillcolor,long bordercolor,long passed);
extern void sub_cls(long method,unsigned long use_color,long passed);
extern void qbg_sub_locate(long row,long column,long cursor,long start,long stop,long passed);
extern long hexoct2uint64(qbs* h);
extern void qbs_input(long numvariables,unsigned char newline);
extern double func_val(qbs *s);
extern void sub_out(long port,long data);
extern void sub_randomize (double seed,long passed);
extern float func_rnd(float n,long passed);
extern float func_timer();
extern void sub_sound(double frequency,double lengthinclockticks);
extern double func_abs(double d);
extern long double func_abs(long double d);
extern float func_abs(float d);
extern void sub_open(qbs *name,long type,long access,long sharing,long i,long record_length,long passed);
extern void sub_close(long i2,long passed);
extern long file_input_chr(long i);
extern void file_input_nextitem(long i,long lastc);
extern void sub_file_print(long i,qbs *str,long extraspace,long tab,long newline);
extern long n_roundincrement();
extern long n_float();
extern long n_int64();
extern long n_uint64();
extern long n_inputnumberfromdata(unsigned char *data,unsigned long *data_offset,unsigned long data_size);
extern long n_inputnumberfromfile(long fileno);
extern void sub_file_line_input_string(long fileno,qbs *deststr);
extern void sub_file_input_string(long fileno,qbs *deststr);
extern int64 func_file_input_int64(long fileno);
extern uint64 func_file_input_uint64(long fileno);
extern void sub_read_string(unsigned char *data,unsigned long *data_offset,unsigned long data_size,qbs *deststr);
extern long double func_read_float(unsigned char *data,unsigned long *data_offset,unsigned long data_size,long typ);
extern long double func_file_input_float(long fileno,long typ);
extern void *byte_element(uint64 offset,unsigned long length);
extern void sub_get(long i,long offset,void *element,long passed);
extern void sub_get2(long i,long offset,qbs *str,long passed);
extern void sub_put(long i,long offset,void *element,long passed);
extern void sub_put2(long i,long offset,void *element,long passed);
extern void sub_graphics_get(long step1,float x1f,float y1f,long step2,float x2f,float y2f,void *element,unsigned long mask,long passed);
extern void sub_graphics_put(long step,float x1f,float y1f,void *element,long clip,long option,unsigned long mask,long passed);
extern void sub_date(qbs* date);
extern qbs *func_date();
extern void sub_time(qbs* str);
extern qbs *func_time();
extern long func_csrlin();
extern long func_pos(long ignore);
extern double func_log(double value);
extern double func_csng_float(long double value);
extern double func_csng_double(double value);
extern double func_cdbl_float(long double value);
extern short func_cint_double(double value);
extern short func_cint_float(long double value);
extern short func_cint_long(long value);
extern short func_cint_ulong(unsigned long value);
extern short func_cint_int64(int64 value);
extern short func_cint_uint64(uint64 value);
extern long func_clng_double(double value);
extern long func_clng_float(long double value);
extern long func_clng_ulong(unsigned long value);
extern long func_clng_int64(int64 value);
extern long func_clng_uint64(uint64 value);
extern int64 func_round_double(double value);
extern int64 func_round_float(long double value);
extern double func_fix_double(double value);
extern long double func_fix_float(long double value);
extern double func_exp_single(double value);
extern long double func_exp_float(long double value);
extern void sub_sleep(long seconds,long passed);
extern qbs *func_oct(uint64 value,long sig_bits);
extern qbs *func_oct_float(long double value);
extern qbs *func_hex(uint64 value,long max_characters);
extern qbs *func_hex_float(long double value);
extern long func_lbound(long *array,long index,long num_indexes);
extern long func_ubound(long *array,long index,long num_indexes);
extern long func_sgn(unsigned char v);
extern long func_sgn(char v);
extern long func_sgn(unsigned short v);
extern long func_sgn(short v);
extern long func_sgn(unsigned long v);
extern long func_sgn(long v);
extern long func_sgn(uint64 v);
extern long func_sgn(int64 v);
extern long func_sgn(float v);
extern long func_sgn(double v);
extern long func_sgn(long double v);
extern long func_inp(long port);
extern void sub_wait(long port,long andexpression,long xorexpression,long passed);
extern qbs *func_tab(long pos);
extern qbs *func_spc(long spaces);
extern float func_pmap(float val,long option);
extern unsigned long func_screen(long y,long x,long returncol,long passed);
extern void sub_bsave(qbs *filename,long offset,long size);
extern void sub_bload(qbs *filename,long offset,long passed);
extern long func_lof(long i);
extern long func_eof(long i);
extern void sub_seek(long i,long pos);
extern long func_seek(long i);
extern long func_loc(long i);
extern qbs *func_input(long n,long i,long passed);
extern double func_sqr(double value);
extern void sub_beep();
extern void snd_check();
extern unsigned long func__sndraw(unsigned char* data,unsigned long bytes);
extern unsigned long func__sndopen(qbs* filename,qbs* requirements,long passed);
extern double func__sndlen(unsigned long handle);
extern void sub__sndlimit(unsigned long handle,double limit);
extern void sub__sndstop(unsigned long handle);
extern void sub__sndsetpos(unsigned long handle,double sec);
extern double func__sndgetpos(unsigned long handle);
extern void sub__sndbal(unsigned long handle,double x,double y,double z,long passed);
extern void sub__sndplay(unsigned long handle);
extern void sub__sndloop(unsigned long handle);
extern unsigned long func__sndcopy(unsigned long handle);
extern void sub__sndvol(unsigned long handle,float volume);
extern void sub__sndpause(unsigned long handle);
extern long func__sndpaused(unsigned long handle);
extern long func__sndplaying(unsigned long handle);
extern void sub__sndclose(unsigned long handle);
extern void sub__sndplayfile(qbs *filename,long sync,double volume,long passed);
extern void sub__sndplaycopy(unsigned long handle,double volume,long passed);
extern qbs *func_command();
extern void sub_shell(qbs *str,long passed);
extern void sub_shell2(qbs *str);
extern void sub_kill(qbs *str);
extern void sub_name(qbs *oldname,qbs *newname);
extern void sub_chdir(qbs *str);
extern void sub_mkdir(qbs *str);
extern void sub_rmdir(qbs *str);
extern double pow2(double x,double y);
extern long func_freefile();
extern void sub__mousehide();
extern void sub__mouseshow();
extern float func__mousex();
extern float func__mousey();
extern long func__mouseinput();
extern long func__mousebutton(long i);
extern void call_absolute(long args,uint16 offset);
extern void call_interrupt(long i);
extern void sub_play(qbs *str);
extern long func__newimage(long x,long y,long bpp,long passed);
extern long func__loadimage(qbs *f,long bpp,long passed);
extern long func__copyimage(long i,long passed);
extern void sub__freeimage(long i,long passed);
extern void sub__source(long i);
extern void sub__dest(long i);
extern long func__source();
extern long func__dest();
extern long func__display();
extern void sub__blend(long i,long passed);
extern void sub__dontblend(long i,long passed);
extern void sub__clearcolor(long none,unsigned long c,long i,long passed);
extern void sub__setalpha(long a,unsigned long c,unsigned long c2,long i,long passed);
extern long func__width(long i,long passed);
extern long func__height(long i,long passed);
extern long func__pixelsize(long i,long passed);
extern long func__clearcolor(long i,long passed);
extern long func__blend(long i,long passed);
extern unsigned long func__defaultcolor(long i,long passed);
extern unsigned long func__backgroundcolor(long i,long passed);
extern unsigned long func__palettecolor(long n,long i,long passed);
extern void sub__palettecolor(long n,unsigned long c,long i,long passed);
extern void sub__copypalette(long i,long i2,long passed);
extern void sub__printstring(long step,double f_x,double f_y,qbs* text,long i,long passed);
extern long func__printwidth(qbs* text,long i,long passed);
extern long func__loadfont(qbs *filename,double size,qbs *requirements,long passed);
extern void sub__font(long f,long i,long passed);
extern long func__fontwidth(long f,long passed);
extern long func__fontheight(long f,long passed);
extern long func__font(long i,long passed);
extern void sub__freefont(long f);
extern void sub__printmode(long mode,long i,long passed);
extern long func__printmode(long i,long passed);
extern unsigned long func__rgb32(long r,long g,long b);
extern unsigned long func__rgba32(long r,long g,long b,long a);
extern long func__alpha32(unsigned long col);
extern long func__red32(unsigned long col);
extern long func__green32(unsigned long col);
extern long func__blue32(unsigned long col);
extern unsigned long matchcol(long r,long g,long b);
extern unsigned long matchcol(long r,long g,long b,long i);
extern unsigned long func__rgb(long r,long g,long b,long i,long passed);
extern unsigned long func__rgba(long r,long g,long b,long a,long i,long passed);
extern long func__alpha(unsigned long col,long i,long passed);
extern long func__red(unsigned long col,long i,long passed);
extern long func__green(unsigned long col,long i,long passed);
extern long func__blue(unsigned long col,long i,long passed);
extern void sub_end();
extern long print_using(qbs *f, long s2, qbs *dest, qbs* pu_str);
extern long print_using_integer64(qbs* format, int64 value, long start, qbs *output);
extern long print_using_uinteger64(qbs* format, uint64 value, long start, qbs *output);
extern long print_using_single(qbs* format, float value, long start, qbs *output);
extern long print_using_double(qbs* format, double value, long start, qbs *output);
extern long print_using_float(qbs* format, double value, long start, qbs *output);
extern qbs *b2string(char v);
extern qbs *ub2string(char v);
extern qbs *i2string(short v);
extern qbs *ui2string(short v);
extern qbs *l2string(long v);
extern qbs *ul2string(unsigned long v);
extern qbs *i642string(int64 v);
extern qbs *i642string(uint64 v);
extern qbs *s2string(float v);
extern qbs *d2string(double v);
extern qbs *f2string(long double v);
extern char string2b(qbs*str);
extern unsigned char string2ub(qbs*str);
extern short string2i(qbs*str);
extern unsigned short string2ui(qbs*str);
extern long string2l(qbs*str);
extern unsigned long string2ul(qbs*str);
extern int64 string2i64(qbs*str);
extern uint64 string2ui64(qbs*str);
extern float string2s(qbs*str);
extern double string2d(qbs*str);
extern long double string2f(qbs*str);

//shared global variables
unsigned long qbs_cmem_sp=256;
unsigned long cmem_sp=65536;
unsigned long dblock;//32bit offset of dblock
unsigned char close_program=0;
long tab_spc_cr_size=1;//default
uint64 *nothingvalue;
unsigned long error_err=0;
double error_erl=0;
unsigned long qbs_tmp_list_nexti=1;
unsigned long error_occurred=0;
unsigned long new_error=0;
qbs* nothingstring;
unsigned char qbevent=0;
unsigned char suspend_program=0;
unsigned char stop_program=0;
unsigned long error_retry=0;
unsigned char cmem[1114099];//16*65535+65535+3 (enough for highest referencable dword in conv memory)
unsigned char *cmem_static_pointer=&cmem[0]+1280+65536;
unsigned char *cmem_dynamic_base=&cmem[0]+655360;
unsigned char *mem_static;
unsigned char *mem_static_pointer;
unsigned char *mem_static_limit;
double last_line=0;
unsigned long error_goto_line=0;
unsigned long error_handling=0;
unsigned long next_return_point=0;
unsigned long *return_point=(unsigned long*)malloc(4*16384);
unsigned long return_points=16384;
void *qbs_input_variableoffsets[257];
long qbs_input_variabletypes[257];

//qbmain specific global variables
char g_tmp_char;
unsigned char g_tmp_uchar;
short g_tmp_short;
unsigned short g_tmp_ushort;
long g_tmp_long;
unsigned long g_tmp_ulong;
int64 g_tmp_int64;
uint64 g_tmp_uint64;
float g_tmp_float;
double g_tmp_double;
long double g_tmp_longdouble;
qbs *g_swap_str;
qbs *pass_str;
unsigned long data_offset=0;

//inline functions

//CSNG
inline double func_csng_float(long double value){
if ((value<=3.402823466E38)&&(value>=-3.402823466E38)){
return value;
}
error(6); return 0;
}
inline double func_csng_double(double value){
if ((value<=3.402823466E38)&&(value>=-3.402823466E38)){
return value;
}
error(6); return 0;
}

//CDBL
inline double func_cdbl_float(long double value){
if ((value<=1.7976931348623157E308)&&(value>=-1.7976931348623157E308)){
return value;
}
error(6); return 0;
}

//CINT
//func_cint_single uses func_cint_double
inline short func_cint_double(double value){
if ((value<32767.5)&&(value>-32768.5)){
if (value<0) return(value-0.5); else return(value+0.5);
}
error(6); return 0;
}
inline short func_cint_float(long double value){
if ((value<32767.5)&&(value>-32768.5)){
if (value<0) return(value-0.5); else return(value+0.5);
}
error(6); return 0;
}
inline short func_cint_long(long value){
if ((value>=-32768)&&(value<=32767)) return value;
error(6); return 0;
}
inline short func_cint_ulong(unsigned long value){
if (value<=32767) return value;
error(6); return 0;
}
inline short func_cint_int64(int64 value){
if ((value>=-32768)&&(value<=32767)) return value;
error(6); return 0;
}
inline short func_cint_uint64(uint64 value){
if (value<=32767) return value;
error(6); return 0;
}

//CLNG
//func_clng_single uses func_clng_double
//–2147483648 to 2147483647
inline long func_clng_double(double value){
if ((value<2147483647.5)&&(value>-2147483648.5)){
if (value<0) return(value-0.5); else return(value+0.5);
}
error(6); return 0;
}
inline long func_clng_float(long double value){
if ((value<2147483647.5)&&(value>-2147483648.5)){
if (value<0) return(value-0.5); else return(value+0.5);
}
error(6); return 0;
}
inline long func_clng_ulong(unsigned long value){
if (value<=2147483647) return value;
error(6); return 0;
}
inline long func_clng_int64(int64 value){
if ((value>=-2147483648)&&(value<=2147483647)) return value;
error(6); return 0;
}
inline long func_clng_uint64(uint64 value){
if (value<=2147483647) return value;
error(6); return 0;
}

//_ROUND (note: round performs no error checking)
inline int64 func_round_double(double value){
if (value<0) return(value-0.5f); else return(value+0.5f);
}
inline int64 func_round_float(long double value){
if (value<0) return(value-0.5f); else return(value+0.5f);
}

//force abs to return floating point numbers correctly
inline double func_abs(double d){
return fabs(d);
}
inline long double func_abs(long double d){
return fabs(d);
}
inline float func_abs(float d){
return fabs(d);
}

inline unsigned char func_abs(unsigned char d){return d;}
inline unsigned short func_abs(unsigned short d){return d;}
inline unsigned long func_abs(unsigned long d){return d;}
inline uint64 func_abs(uint64 d){return d;}
inline char func_abs(char d){return abs(d);}
inline short func_abs(short d){return abs(d);}
inline long func_abs(long d){return abs(d);}
//inline int64 func_abs(int64 d){return abs(d);}
inline int64 func_abs(int64 d){return abs((long)d);}

inline long array_check(unsigned long index,unsigned long limit){
//nb. forces signed index into an unsigned variable for quicker comparison
if (index<limit) return index;
error(9); return 0;
}

inline long func_sgn(unsigned char v){
if (v) return 1; else return 0;
}
inline long func_sgn(char v){
if (v) if (v>0) return 1; else return -1;
return 0;
}
inline long func_sgn(unsigned short v){
if (v) return 1; else return 0;
}
inline long func_sgn(short v){
if (v) if (v>0) return 1; else return -1;
return 0;
}
inline long func_sgn(unsigned long v){
if (v) return 1; else return 0;
}
inline long func_sgn(long v){
if (v) if (v>0) return 1; else return -1;
return 0;
}
inline long func_sgn(uint64 v){
if (v) return 1; else return 0;
}
inline long func_sgn(int64 v){
if (v) if (v>0) return 1; else return -1;
return 0;
}
inline long func_sgn(float v){
if (v) if (v>0) return 1; else return -1;
return 0;
}
inline long func_sgn(double v){
if (v) if (v>0) return 1; else return -1;
return 0;
}
inline long func_sgn(long double v){
if (v) if (v>0) return 1; else return -1;
return 0;
}

//Working with 32bit colors:
inline unsigned long func__rgb32(long r,long g,long b){
if (r<0) r=0;
if (r>255) r=255;
if (g<0) g=0;
if (g>255) g=255;
if (b<0) b=0;
if (b>255) b=255;
return (r<<16)+(g<<8)+b|0xFF000000;
}
inline unsigned long func__rgba32(long r,long g,long b,long a){
if (r<0) r=0;
if (r>255) r=255;
if (g<0) g=0;
if (g>255) g=255;
if (b<0) b=0;
if (b>255) b=255;
if (a<0) a=0;
if (a>255) a=255;
return (a<<24)+(r<<16)+(g<<8)+b;
}
inline long func__alpha32(unsigned long col){
return col>>24;
}
inline long func__red32(unsigned long col){
return col>>16&0xFF;
}
inline long func__green32(unsigned long col){
return col>>8&0xFF;
}
inline long func__blue32(unsigned long col){
return col&0xFF;
}

const unsigned char QBMAIN_data[]={
#include "../temp/userdata.txt"
};
unsigned char *data=(unsigned char*)&QBMAIN_data[0];

#include "../temp/regsf.txt"
#include "../temp/global.txt"

int QBMAIN(void *unused)
{
long tmp_long;
long tmp_fileno;
qbs* tqbs;
unsigned long qbs_tmp_base=qbs_tmp_list_nexti;
#include "../temp/maindata.txt"
#include "../temp/mainerr.txt"
#include "../temp/main.txt"
//} (closed by main.txt)
