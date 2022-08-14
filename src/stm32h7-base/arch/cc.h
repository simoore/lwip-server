#ifndef CC_H
#define CC_H

#include <stdlib.h>
#include <stdio.h>

// ARM processors are little endian.
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif 

typedef int sys_prot_t;

#define LWIP_PROVIDE_ERRNO

#if defined (__GNUC__) & !defined (__CC_ARM)
#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/time.h>
#endif

// These definitions apply to the GCC compiler. These are used to assist with packing structs.
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#define LWIP_PLATFORM_ASSERT(x) { printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__); }
#define LWIP_RAND() ((u32_t)rand())

#endif // CC_H
