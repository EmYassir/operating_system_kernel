
/* Fichier d'adaptation de lwip à notre plateforme */

#ifndef __CC_H__
#define __CC_H__

#include "stdio.h"

/* Types de base de lwIP */
typedef unsigned char u8_t;
typedef signed char s8_t;
typedef unsigned short u16_t;
typedef signed short s16_t;
typedef unsigned long u32_t;
typedef signed long s32_t;

typedef unsigned long mem_ptr_t;

/* Endianness du système */
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

/* formats de données des printfs */
#define X8_F  "02x"
#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

/* Choix de l'algorithme de checksum */
#define LWIP_CHKSUM_ALGORITHM 3

// TODO : PACK_STRUCT
#define PACK_STRUCT_FIELD(x) x __attribute__((packed))
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define LWIP_PLATFORM_DIAG(x) printf(x);

#define LWIP_FAILED 0

#define LWIP_PLATFORM_ASSERT(x) do{ printf(x); assert(LWIP_FAILED); }while(0)

// TODO : mailboxes et lwip timeouts dans sys_arch.c/.h

#endif
