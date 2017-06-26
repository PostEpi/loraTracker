/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>


extern int DebugFlag;
#define DEBUGZONE(b) (DebugFlag&(0x00000001<<b))

#define ZONE_ERROR      DEBUGZONE(0)
#define ZONE_WARN       DEBUGZONE(1)
#define ZONE_TRACE      ZONE_XMIT
#define ZONE_FUNCTION   DEBUGZONE(2)
#define ZONE_INIT       DEBUGZONE(3)
#define ZONE_INTR       DEBUGZONE(4)
#define ZONE_RCV        DEBUGZONE(5)
#define ZONE_XMIT       DEBUGZONE(6)
#define ZONE_SEND       ZONE_XMIT
#define ZONE_BUFFER		DEBUGZONE(7)
#define ZONE_HALT		DEBUGZONE(8)

/** print debug message only if debug message type is enabled...
 *  AND is of correct type AND is at least LWIP_DBG_LEVEL
 */
#define DEBUG(cond, msg) \
		((cond)? (printf msg), 1: 0);

#endif /* __DEBUG_H__ */

