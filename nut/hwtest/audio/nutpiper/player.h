#ifndef _PLAYER_H_
#define _PLAYER_H_

/*
 * Copyright (C) 2003 by egnite Software GmbH
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * For additional information see http://www.ethernut.de/
 */

/*!
 * $Id: player.h 4246 2012-06-12 14:54:04Z haraldkipp $
 */

#include <sys/socket.h>

#define PSI_IDLE    0
#define PSI_START   1
#define PSI_RUNNING 2
#define PSI_STOP    3

typedef struct {
    uint8_t psi_status;
    HANDLE psi_cmdevt;
    HANDLE psi_stsevt;
    /*! \brief Metadata interval. */
    uint32_t psi_metaint;
    /*! \brief MP3 data left until next metadata. */
    uint32_t psi_mp3left;
    uint32_t psi_start;
    uint8_t psi_metaupdate;
    char  *psi_metatitle;
    char  *psi_metaurl;
    TCPSOCKET *psi_sock;
} PLAYERINFO;

extern PLAYERINFO player;

extern int PlayerInit(void);
extern int PlayerStop(uint32_t tmo);
extern int PlayerStart(TCPSOCKET * sock, uint32_t metaint, uint32_t tmo);

#endif