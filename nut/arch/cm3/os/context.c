/*
 * Copyright (C) 2001-2005 by egnite Software GmbH. All rights reserved.
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
 *
 */

/*
 * $Log$
 * Revision 1.00  2010/08/10 18:55:12  ulrichprinz
 * Inital version
 *
 */

#include <cfg/os.h>

#include <string.h>

#include <sys/osdebug.h>
#include <sys/nutdebug.h>

#include <sys/atom.h>
#include <sys/heap.h>
#include <sys/thread.h>

/*!
 * \addtogroup xgNutArchCm3OsContext
 */
/*@{*/


/*!
 * \brief ARM7TDMI GCC context switch frame layout.
 *
 * This is the layout of the stack after a thread's context has been
 * switched-out. The stack pointer is stored in the thread info and
 * points to this structure.
 */
typedef struct {
    uint32_t csf_cpsr;
    uint32_t csf_r4;
    uint32_t csf_r5;
    uint32_t csf_r6;
    uint32_t csf_r7;
    uint32_t csf_r8;
    uint32_t csf_r9;
    uint32_t csf_r10;
    uint32_t csf_r11;             /* AKA fp */
    uint32_t csf_lr;
} SWITCHFRAME;

/*!
 * \brief Thread entry frame layout.
 *
 * This is the stack layout being build to enter a new thread.
 */
typedef struct {
    uint32_t cef_r0;
    uint32_t cef_pc;
} ENTERFRAME;

/*!
 * \brief Enter a new thread.
 */
static void NutThreadEntry(void) __attribute__ ((naked));
void NutThreadEntry(void)
{
    /* Load argument in r0 and jump to thread entry. */
    asm volatile ("ldmfd   sp!, {r0, lr}\n\tbx lr":::"r0", "lr");
}

/*!
 * \brief Switch to another thread.
 *
 * Stop the current thread, saving its context. Then start the
 * one with the highest priority, which is ready to run.
 *
 * Application programs typically do not call this function.
 *
 * \note CPU interrupts must be disabled before calling this function.
 *
 */
void NutThreadSwitch(void) __attribute__ ((naked));
void NutThreadSwitch(void)
{
    /* Save CPU context. */
    asm volatile (                      /* */
        "@ Save context\n\t"            /* */
        "stmfd   sp!, {r4-r11, lr}\n\t" /* Save registers. */
        "mrs     r4,  psr\n\t"          /* Save status. */
        "stmfd   sp!, {r4}\n\t"         /* */
        "str     sp, %0"                /* Save stack pointer. */
        ::"m" (runningThread->td_sp)    /* */
    );

    /* Select thread on top of the run queue. */
    runningThread = runQueue;
    runningThread->td_state = TDS_RUNNING;

    /* Restore context. */
     __asm__ __volatile__(              /* */
        "@ Load context\n\t"            /* */
        "ldr     sp, %0\n\t"            /* Restore stack pointer. */
        "ldmfd   sp!, {r4}\n\t"         /* Get saved status... */
        "msr     psr, r4\n\t"           /* ...and save in psr. */
        "cpsie   i\n\t"                 /* ...enable interrupts */
        "ldmfd   sp!, {r4-r11, lr}\n\t" /* Restore registers. */
        "bx      lr\n\t"                /* */
        ::"m"(runningThread->td_sp)     /* */
    );

#if defined(NUT_CRITICAL_NESTING) && !defined(NUT_CRITICAL_NESTING_STACK)
        critical_nesting_level = 0;
#endif
}

/*!
 * \brief Create a new thread.
 *
 * If the current thread's priority is lower or equal than the default
 * priority (64), then the current thread is stopped and the new one
 * is started.
 *
 * \param name      String containing the symbolic name of the new thread,
 *                  up to 8 characters long.
 * \param fn        The thread's entry point, typically created by the
 *                  THREAD macro.
 * \param arg       Argument pointer passed to the new thread.
 * \param stackSize Number of bytes of the stack space allocated for
 *                  the new thread.
 *
 * \note The thread must run in ARM mode. Thumb mode is not supported.
 *
 * \return Pointer to the NUTTHREADINFO structure or 0 to indicate an
 *         error.
 */
HANDLE NutThreadCreate(char * name, void (*fn) (void *), void *arg, size_t stackSize)
{
    uint8_t *threadMem;
    SWITCHFRAME *sf;
    ENTERFRAME *ef;
    NUTTHREADINFO *td;

    /*
     * Allocate stack and thread info structure in one block.
     * We sill setup the following layout:
     *
     * Upper memory addresses.
     *
     *              +--------------------+
     *              I                    I
     *              I   NUTTHREADINFO    I
     *              I                    I
     * td ->        +-----+--------------+ <- Stack top
     *              I     I              I
     *              I  T  I   ENTERFRAME I
     *              I  H  I              I
     * ef ->        I  R  +--------------+
     *              I  E  I              I    ^
     *              I  A  I  SWITCHFRAME I    I
     *              I  D  I              I    I  pop moves up
     * sf ->        I     +--------------+ <- Initial stack pointer
     *              I  S  I              I    I  push moves down
     *              I  T  I Application  I    I
     *              I  A  I Stack        I    V
     *              I  C  I              I
     *              I  K  I              I
     * threadMem -> +-----+--------------+ <- Stack bottom
     *
     * Lower memory addresses.
     */
    if ((threadMem = NutHeapAlloc(stackSize + sizeof(NUTTHREADINFO))) == 0) {
        return 0;
    }
    td = (NUTTHREADINFO *) (threadMem + stackSize);
    ef = (ENTERFRAME *) ((uintptr_t) td - sizeof(ENTERFRAME));
    sf = (SWITCHFRAME *) ((uintptr_t) ef - sizeof(SWITCHFRAME));

    /*
     * Set predefined values at the stack bottom. May be used to detect
     * stack overflows.
     */
#if defined(NUTDEBUG_CHECK_STACKMIN) || defined(NUTDEBUG_CHECK_STACK)
    {
        uint32_t *fip = (uint32_t *)threadMem;
        while (fip < (uint32_t *)sf) {
            *fip++ = DEADBEEF;
        }
    }
#else
    *((uint32_t *) threadMem) = DEADBEEF;
    *((uint32_t *) (threadMem + 4)) = DEADBEEF;
    *((uint32_t *) (threadMem + 8)) = DEADBEEF;
    *((uint32_t *) (threadMem + 12)) = DEADBEEF;
#endif

    /*
     * Setup the entry frame to simulate C function entry.
     */
    ef->cef_pc = (uintptr_t) fn;
    ef->cef_r0 = (uintptr_t) arg;

    /*
     * Setup the switch frame.
     */
    sf->csf_lr = (uintptr_t) NutThreadEntry;
    sf->csf_cpsr = 0;

    /*
     * Initialize the thread info structure and insert it into the
     * thread list and the run queue.
     */
    memcpy(td->td_name, name, sizeof(td->td_name) - 1);
    td->td_name[sizeof(td->td_name) - 1] = 0;
    td->td_state = TDS_READY;
    td->td_sp = (uintptr_t) sf;
    td->td_priority = 64;
    td->td_memory = threadMem;
    td->td_timer = 0;
    td->td_queue = 0;

    NutEnterCritical();
    td->td_next = nutThreadList;
    nutThreadList = td;
    NutThreadAddPriQueue(td, (NUTTHREADINFO **) & runQueue);

    /*
     * If no thread is running, then this is the first thread ever
     * created. In Nut/OS, the idle thread is created first.
     */
    if (runningThread == 0) {
        /* This will never return. */
        runningThread = runQueue;
        runningThread->td_state = TDS_RUNNING;
        /* Restore context. */
        __asm__ __volatile__(               /* */
            "@ Load context\n\t"            /* */
            "ldr     sp, %0\n\t"            /* Restore stack pointer. */
            "ldmfd   sp!, {r4}\n\t"         /* Get saved status... */
            "msr     psr, r4\n\t"           /* ...and save in spsr. */
            "cpsie   i\n\t"                 /* ...enable interrupts */
            "ldmfd   sp!, {r4-r11, lr}\n\t" /* Restore registers. */
            "bx     lr"                     /* */
            ::"m"(runningThread->td_sp)     /* */
        );
    }

    /*
     * If current context is not in front of the run queue (highest
     * priority), then switch to the thread in front.
     */
    if (runningThread != runQueue) {
        runningThread->td_state = TDS_READY;
        NutThreadSwitch();
    }
    NutExitCritical();

    return td;
}

/*@}*/
