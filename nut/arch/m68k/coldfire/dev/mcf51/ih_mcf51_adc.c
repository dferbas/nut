/*
 * Copyright 2012-2016 by Embedded Technologies s.r.o. All rights reserved.
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

#include <arch/m68k.h>
#include <dev/irqreg.h>

/*!
 * \addtogroup xgMcf51
 */
/*@{*/

static int IrqCtl(int cmd, void *param);

IRQ_HANDLER sig_ADC =
{
#ifdef NUT_PERFMON
		0,
#endif
		NULL,
		NULL, IrqCtl
};

/*! \brief ADC interrupt control.
 *
 * \param cmd 		Control command.
 * \param *param 	Pointer to optional parameter.
 *
 * \return IrqCtlCommon 0 on success, -1 otherwise.
 */
static int IrqCtl(int cmd, void *param)
{
	return IrqCtlCommon(&sig_ADC, cmd, param, &MCF_ADC_SC1, MCF_ADC_SC1_AIEN, 1);
}

SIGNAL(IH_ADC)
{
	/*
	 * Interrupt flag (MCF_ADC_SC1_COCO bit) is cleared when converted value (MCF_ADC_R) is read.
	 * This must be ensured by assigned signal handler.
	 */
	CallHandler(&sig_ADC);
}

