/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                             TRACE SUPPORT
*
* Filename : os_trace.h
* Version  : V2.93.00
*********************************************************************************************************
* Note(s) : (1) The header file os_trace_events.h is the interface between uC/OS-II and your trace
*               recorder of choice.
*               To support trace recording, include one of the sub-folders at uCOS-II/Trace/ into
*               your project.
*********************************************************************************************************
*/

#ifndef   OS_TRACE_H
#define   OS_TRACE_H

#include  <os_cfg.h>

#if (defined(OS_TRACE_EN) && (OS_TRACE_EN > 0u))
#include  <os_trace_events.h>                                   /* See Note #1.                                         */
#endif


/*
**************************************************************************************************************************
*                                        uC/OS-II Trace Default Macros (Empty)
**************************************************************************************************************************
*/

#ifndef  OS_TRACE_INIT
#define  OS_TRACE_INIT()
#endif
#ifndef  OS_TRACE_START
#define  OS_TRACE_START()
#endif
#ifndef  OS_TRACE_STOP
#define  OS_TRACE_STOP()
#endif
#ifndef  OS_TRACE_CLEAR
#define  OS_TRACE_CLEAR()
#endif

#ifndef  OS_TRACE_ISR_ENTER
#define  OS_TRACE_ISR_ENTER()
#endif
#ifndef  OS_TRACE_ISR_EXIT
#define  OS_TRACE_ISR_EXIT()
#endif
#ifndef  OS_TRACE_ISR_EXIT_TO_SCHEDULER
#define  OS_TRACE_ISR_EXIT_TO_SCHEDULER()
#endif

#ifndef  OS_TRACE_TICK_INCREMENT
#define  OS_TRACE_TICK_INCREMENT(OSTickCtr)
#endif

#ifndef  OS_TRACE_TASK_CREATE
#define  OS_TRACE_TASK_CREATE(p_tcb)
#endif

#ifndef  OS_TRACE_TASK_CREATE_FAILED
#define  OS_TRACE_TASK_CREATE_FAILED(p_tcb)
#endif

#ifndef  OS_TRACE_TASK_DEL
#define  OS_TRACE_TASK_DEL(p_tcb)
#endif

#ifndef  OS_TRACE_TASK_READY
#define  OS_TRACE_TASK_READY(p_tcb)
#endif

#ifndef  OS_TRACE_TASK_SWITCHED_IN
#define  OS_TRACE_TASK_SWITCHED_IN(p_tcb)
#endif

#ifndef  OS_TRACE_TASK_DLY
#define  OS_TRACE_TASK_DLY(dly_ticks)
#endif

#ifndef  OS_TRACE_TASK_SUSPEND
#define  OS_TRACE_TASK_SUSPEND(p_tcb)
#endif

#ifndef  OS_TRACE_TASK_SUSPENDED
#define  OS_TRACE_TASK_SUSPENDED(p_tcb)
#endif

#ifndef  OS_TRACE_TASK_RESUME
#define  OS_TRACE_TASK_RESUME(p_tcb)
#endif

#ifndef  OS_TRACE_TASK_PRIO_CHANGE
#define  OS_TRACE_TASK_PRIO_CHANGE(p_tcb, prio)
#endif

#ifndef  OS_TRACE_TASK_NAME_SET
#define  OS_TRACE_TASK_NAME_SET(p_tcb)
#endif

#ifndef  OS_TRACE_EVENT_NAME_SET
#define  OS_TRACE_EVENT_NAME_SET(p_event, p_name)
#endif

#ifndef  OS_TRACE_ISR_REGISTER
#define  OS_TRACE_ISR_REGISTER(isr_id, isr_name, isr_prio)
#endif

#ifndef  OS_TRACE_ISR_BEGIN
#define  OS_TRACE_ISR_BEGIN(isr_id)
#endif

#ifndef  OS_TRACE_ISR_END
#define  OS_TRACE_ISR_END()
#endif

#ifndef  OS_TRACE_MBOX_CREATE
#define  OS_TRACE_MBOX_CREATE(p_mbox, p_name)
#endif

#ifndef  OS_TRACE_MUTEX_CREATE
#define  OS_TRACE_MUTEX_CREATE(p_mutex, p_name)
#endif

#ifndef  OS_TRACE_MUTEX_DEL
#define  OS_TRACE_MUTEX_DEL(p_mutex)
#endif

#ifndef  OS_TRACE_MUTEX_POST
#define  OS_TRACE_MUTEX_POST(p_mutex)
#endif

#ifndef  OS_TRACE_MUTEX_POST_FAILED
#define  OS_TRACE_MUTEX_POST_FAILED(p_mutex)
#endif

#ifndef  OS_TRACE_MUTEX_PEND
#define  OS_TRACE_MUTEX_PEND(p_mutex)
#endif

#ifndef  OS_TRACE_MUTEX_PEND_FAILED
#define  OS_TRACE_MUTEX_PEND_FAILED(p_mutex)
#endif

#ifndef  OS_TRACE_MUTEX_PEND_BLOCK
#define  OS_TRACE_MUTEX_PEND_BLOCK(p_mutex)
#endif

#ifndef  OS_TRACE_MUTEX_TASK_PRIO_INHERIT
#define  OS_TRACE_MUTEX_TASK_PRIO_INHERIT(p_tcb, prio)
#endif

#ifndef  OS_TRACE_MUTEX_TASK_PRIO_DISINHERIT
#define  OS_TRACE_MUTEX_TASK_PRIO_DISINHERIT(p_tcb, prio)
#endif

#ifndef  OS_TRACE_SEM_CREATE
#define  OS_TRACE_SEM_CREATE(p_sem, p_name)
#endif

#ifndef  OS_TRACE_SEM_DEL
#define  OS_TRACE_SEM_DEL(p_sem)
#endif

#ifndef  OS_TRACE_SEM_POST
#define  OS_TRACE_SEM_POST(p_sem)
#endif

#ifndef  OS_TRACE_SEM_POST_FAILED
#define  OS_TRACE_SEM_POST_FAILED(p_sem)
#endif

#ifndef  OS_TRACE_SEM_PEND
#define  OS_TRACE_SEM_PEND(p_sem)
#endif

#ifndef  OS_TRACE_SEM_PEND_FAILED
#define  OS_TRACE_SEM_PEND_FAILED(p_sem)
#endif

#ifndef  OS_TRACE_SEM_PEND_BLOCK
#define  OS_TRACE_SEM_PEND_BLOCK(p_sem)
#endif

#ifndef  OS_TRACE_Q_CREATE
#define  OS_TRACE_Q_CREATE(p_q, p_name)
#endif

#ifndef  OS_TRACE_Q_DEL
#define  OS_TRACE_Q_DEL(p_q)
#endif

#ifndef  OS_TRACE_Q_POST
#define  OS_TRACE_Q_POST(p_q)
#endif

#ifndef  OS_TRACE_Q_POST_FAILED
#define  OS_TRACE_Q_POST_FAILED(p_q)
#endif

#ifndef  OS_TRACE_Q_PEND
#define  OS_TRACE_Q_PEND(p_q)
#endif

#ifndef  OS_TRACE_Q_PEND_FAILED
#define  OS_TRACE_Q_PEND_FAILED(p_q)
#endif

#ifndef  OS_TRACE_Q_PEND_BLOCK
#define  OS_TRACE_Q_PEND_BLOCK(p_q)
#endif

#ifndef  OS_TRACE_FLAG_CREATE
#define  OS_TRACE_FLAG_CREATE(p_grp, p_name)
#endif

#ifndef  OS_TRACE_FLAG_DEL
#define  OS_TRACE_FLAG_DEL(p_grp)
#endif

#ifndef  OS_TRACE_FLAG_POST
#define  OS_TRACE_FLAG_POST(p_grp)
#endif

#ifndef  OS_TRACE_FLAG_POST_FAILED
#define  OS_TRACE_FLAG_POST_FAILED(p_grp)
#endif

#ifndef  OS_TRACE_FLAG_PEND
#define  OS_TRACE_FLAG_PEND(p_grp)
#endif

#ifndef  OS_TRACE_FLAG_PEND_FAILED
#define  OS_TRACE_FLAG_PEND_FAILED(p_grp)
#endif

#ifndef  OS_TRACE_FLAG_PEND_BLOCK
#define  OS_TRACE_FLAG_PEND_BLOCK(p_grp)
#endif

#ifndef  OS_TRACE_MEM_CREATE
#define  OS_TRACE_MEM_CREATE(p_mem)
#endif

#ifndef  OS_TRACE_MEM_PUT
#define  OS_TRACE_MEM_PUT(p_mem)
#endif

#ifndef  OS_TRACE_MEM_PUT_FAILED
#define  OS_TRACE_MEM_PUT_FAILED(p_mem)
#endif

#ifndef  OS_TRACE_MEM_GET
#define  OS_TRACE_MEM_GET(p_mem)
#endif

#ifndef  OS_TRACE_MEM_GET_FAILED
#define  OS_TRACE_MEM_GET_FAILED(p_mem)
#endif

#ifndef  OS_TRACE_TMR_CREATE
#define  OS_TRACE_TMR_CREATE(p_tmr, p_name)
#endif



#ifndef  OS_TRACE_MBOX_DEL_ENTER
#define  OS_TRACE_MBOX_DEL_ENTER(p_mbox, opt)
#endif

#ifndef  OS_TRACE_MBOX_POST_ENTER
#define  OS_TRACE_MBOX_POST_ENTER(p_mbox)
#endif

#ifndef  OS_TRACE_MBOX_POST_OPT_ENTER
#define  OS_TRACE_MBOX_POST_OPT_ENTER(p_mbox, opt)
#endif

#ifndef  OS_TRACE_MBOX_PEND_ENTER
#define  OS_TRACE_MBOX_PEND_ENTER(p_mbox, timeout)
#endif

#ifndef  OS_TRACE_MUTEX_DEL_ENTER
#define  OS_TRACE_MUTEX_DEL_ENTER(p_mutex, opt)
#endif

#ifndef  OS_TRACE_MUTEX_POST_ENTER
#define  OS_TRACE_MUTEX_POST_ENTER(p_mutex)
#endif

#ifndef  OS_TRACE_MUTEX_PEND_ENTER
#define  OS_TRACE_MUTEX_PEND_ENTER(p_mutex, timeout)
#endif

#ifndef  OS_TRACE_SEM_DEL_ENTER
#define  OS_TRACE_SEM_DEL_ENTER(p_sem, opt)
#endif

#ifndef  OS_TRACE_SEM_POST_ENTER
#define  OS_TRACE_SEM_POST_ENTER(p_sem)
#endif

#ifndef  OS_TRACE_SEM_PEND_ENTER
#define  OS_TRACE_SEM_PEND_ENTER(p_sem, timeout)
#endif

#ifndef  OS_TRACE_Q_DEL_ENTER
#define  OS_TRACE_Q_DEL_ENTER(p_q, opt)
#endif

#ifndef  OS_TRACE_Q_POST_ENTER
#define  OS_TRACE_Q_POST_ENTER(p_q)
#endif

#ifndef  OS_TRACE_Q_POST_FRONT_ENTER
#define  OS_TRACE_Q_POST_FRONT_ENTER(p_q)
#endif

#ifndef  OS_TRACE_Q_POST_OPT_ENTER
#define  OS_TRACE_Q_POST_OPT_ENTER(p_q, opt)
#endif

#ifndef  OS_TRACE_Q_PEND_ENTER
#define  OS_TRACE_Q_PEND_ENTER(p_q, timeout)
#endif

#ifndef  OS_TRACE_FLAG_DEL_ENTER
#define  OS_TRACE_FLAG_DEL_ENTER(p_grp, opt)
#endif

#ifndef  OS_TRACE_TMR_DEL_ENTER
#define  OS_TRACE_TMR_DEL_ENTER(p_tmr)
#endif

#ifndef  OS_TRACE_TMR_START_ENTER
#define  OS_TRACE_TMR_START_ENTER(p_tmr)
#endif

#ifndef  OS_TRACE_TMR_STOP_ENTER
#define  OS_TRACE_TMR_STOP_ENTER(p_tmr)
#endif

#ifndef  OS_TRACE_TMR_EXPIRED
#define  OS_TRACE_TMR_EXPIRED(p_tmr)
#endif

#ifndef  OS_TRACE_FLAG_POST_ENTER
#define  OS_TRACE_FLAG_POST_ENTER(p_grp, flags, opt)
#endif

#ifndef  OS_TRACE_FLAG_PEND_ENTER
#define  OS_TRACE_FLAG_PEND_ENTER(p_grp, flags, timeout, opt)
#endif

#ifndef  OS_TRACE_MEM_PUT_ENTER
#define  OS_TRACE_MEM_PUT_ENTER(p_mem, p_blk)
#endif

#ifndef  OS_TRACE_MEM_GET_ENTER
#define  OS_TRACE_MEM_GET_ENTER(p_mem)
#endif

#ifndef  OS_TRACE_MBOX_DEL_EXIT
#define  OS_TRACE_MBOX_DEL_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_MBOX_POST_EXIT
#define  OS_TRACE_MBOX_POST_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_MBOX_POST_OPT_EXIT
#define  OS_TRACE_MBOX_POST_OPT_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_MBOX_PEND_EXIT
#define  OS_TRACE_MBOX_PEND_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_MUTEX_DEL_EXIT
#define  OS_TRACE_MUTEX_DEL_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_MUTEX_POST_EXIT
#define  OS_TRACE_MUTEX_POST_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_MUTEX_PEND_EXIT
#define  OS_TRACE_MUTEX_PEND_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_SEM_DEL_EXIT
#define  OS_TRACE_SEM_DEL_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_SEM_POST_EXIT
#define  OS_TRACE_SEM_POST_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_SEM_PEND_EXIT
#define  OS_TRACE_SEM_PEND_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_Q_DEL_EXIT
#define  OS_TRACE_Q_DEL_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_Q_POST_EXIT
#define  OS_TRACE_Q_POST_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_Q_POST_FRONT_EXIT
#define  OS_TRACE_Q_POST_FRONT_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_Q_POST_OPT_EXIT
#define  OS_TRACE_Q_POST_OPT_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_Q_PEND_EXIT
#define  OS_TRACE_Q_PEND_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_FLAG_DEL_EXIT
#define  OS_TRACE_FLAG_DEL_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_FLAG_POST_EXIT
#define  OS_TRACE_FLAG_POST_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_FLAG_PEND_EXIT
#define  OS_TRACE_FLAG_PEND_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_MEM_PUT_EXIT
#define  OS_TRACE_MEM_PUT_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_MEM_GET_EXIT
#define  OS_TRACE_MEM_GET_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_TMR_DEL_EXIT
#define  OS_TRACE_TMR_DEL_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_TMR_START_EXIT
#define  OS_TRACE_TMR_START_EXIT(RetVal)
#endif

#ifndef  OS_TRACE_TMR_STOP_EXIT
#define  OS_TRACE_TMR_STOP_EXIT(RetVal)
#endif

#endif
