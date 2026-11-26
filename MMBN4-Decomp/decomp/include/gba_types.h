#ifndef GBA_TYPES_H
#define GBA_TYPES_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;
typedef volatile signed char vs8;
typedef volatile signed short vs16;
typedef volatile signed int vs32;

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif /* GBA_TYPES_H */
