//bit-array access inline functions
static long boff;
static __int64 bmask;
static unsigned __int64 *bptr64;
static __int64 bval64;
inline unsigned __int64 getubits(unsigned long bsize,unsigned char *base,unsigned long i){
bmask=~(-(1<<bsize));
i*=bsize; boff=i&bmask;
return ((*(unsigned __int64*)(base+(i>>3)))&(bmask<<boff))>>boff;
}
inline __int64 getbits(unsigned long bsize,unsigned char *base,unsigned long i){
bmask=~(-(1<<bsize));
i*=bsize; boff=i&bmask;
bval64=((*(unsigned __int64*)(base+(i>>3)))&(bmask<<boff))>>boff;
if (bval64&(1<<(bsize-1))) return (bval64|(~bmask));
return bval64;
}
inline void setbits(unsigned long bsize,unsigned char *base,unsigned long i,long val){
bmask=~(-(1<<bsize));
i*=bsize; boff=i&bmask; val=(val&bmask)<<boff; bptr64=(unsigned __int64*)(base+(i>>3));
*bptr64=(*bptr64&(~(bmask<<boff)))+val;
}

/*
#define bmask 7
#define bsize 3
inline unsigned long getubits_3(unsigned char *base,unsigned long i){
i*=bsize; boff=i&bmask;
return ((*(unsigned long*)(base+(i>>3)))&(bmask<<boff))>>boff;
}
inline long getbits_3(unsigned char *base,unsigned long i){
i*=bsize; boff=i&bmask;
bval=((*(unsigned long*)(base+(i>>3)))&(bmask<<boff))>>boff;
if (bval&4) return (bval|(~bmask));
return bval;
}
inline void setbits_3(unsigned char *base,unsigned long i,long val){
i*=bsize; boff=i&bmask; val=(val&bmask)<<boff; bptr=(unsigned long*)(base+(i>>3));
*bptr=(*bptr&(~(bmask<<boff)))+val;
}
#undef bmask
#undef bsize
*/
