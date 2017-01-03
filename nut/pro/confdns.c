/*
 * Copyright (C) 2009 by egnite GmbH
 * Copyright (C) 2001-2003 by egnite Software GmbH
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

/*
 * $Id$
 */

#include <cfg/os.h>

#include <sys/device.h>
#include <sys/timer.h>
#include <sys/heap.h>

#include <arpa/inet.h>
#include <net/if_var.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <string.h>
#include <memdebug.h>

#include <netdb.h>

#ifdef NUTDEBUG
#include <stdio.h>
#endif

/*!
 * \addtogroup xgDNS
 */
/*@{*/

DNSCONFIG confdns;

/*!
 * \brief Set DNS configuration.
 *
 * \param hostname DNS name of the local host.
 * \param domain Name of the domain of the local host.
 * \param pdnsip IP address of the primary DNS server.
 * \param sdnsip IP address of the secondary DNS server.
 */
void NutDnsConfig2(const uint8_t * hostname, const uint8_t * domain, uint32_t pdnsip, uint32_t sdnsip)
{
    if (confdns.doc_hostname) {
        free(confdns.doc_hostname);
        confdns.doc_hostname = 0;
    }
    if (confdns.doc_domain) {
        free(confdns.doc_domain);
        confdns.doc_domain = 0;
    }
    if (hostname) {
        confdns.doc_hostname = (uint8_t *) strdup((char *) hostname);
    }
    if (domain) {
        confdns.doc_domain = (uint8_t *) strdup((char *) domain);
    }
    confdns.doc_ip1 = pdnsip;
    confdns.doc_ip2 = sdnsip;
}

/*!
 * \brief Sets DNS configuration.
 *
 * \deprecated New applications should use NutDnsConfig2().
 *
 * \param hostname DNS name of the local host.
 * \param domain Name of the domain of the local host.
 * \param dnsip IP address of the DNS server.
 */
void NutDnsConfig(const uint8_t * hostname, const uint8_t * domain, uint32_t dnsip)
{
    NutDnsConfig2(hostname, domain, dnsip, 0);
}

void NutDnsGetConfig2(char **hostname, char **domain, uint32_t * pdnsip, uint32_t * sdnsip)
{
    if (hostname) {
        *hostname = (char *) confdns.doc_hostname;
    }
    if (domain) {
        *domain = (char *) confdns.doc_domain;
    }
    if (pdnsip) {
        *pdnsip = confdns.doc_ip1;
    }
    if (sdnsip) {
        *sdnsip = confdns.doc_ip2;
    }
}

/*@}*/
