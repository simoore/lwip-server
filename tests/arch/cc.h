/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#include <errno.h>

#ifdef __GNUC__
#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/time.h>
#endif

/* Define platform endianness (might already be defined) */
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif /* BYTE_ORDER */

typedef int sys_prot_t;

/* Compiler hints for packing structures */
#define PACK_STRUCT_USE_INCLUDES

#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
  LWIP_PLATFORM_DIAG(("Assertion \"%s\" failed at line %d in %s\n", message, __LINE__, __FILE__)); \
  handler;} } while(0)

/* Define an example for LWIP_PLATFORM_DIAG: since this uses varargs and the old
* C standard lwIP targets does not support this in macros, we have extra brackets
* around the arguments, which are left out in the following macro definition:
*/
#if !defined(LWIP_TESTMODE) || !LWIP_TESTMODE
void lwip_win32_platform_diag(const char *format, ...);
#define LWIP_PLATFORM_DIAG(x) lwip_win32_platform_diag x
#endif

extern unsigned int lwip_port_rand(void);
#define LWIP_RAND() ((uint32_t)lwip_port_rand())

#define PPP_INCLUDE_SETTINGS_HEADER

#endif /* LWIP_ARCH_CC_H */
