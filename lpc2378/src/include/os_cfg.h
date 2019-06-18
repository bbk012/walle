/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                        (c) Copyright 1992-1998, Jean J. Labrosse, Plantation, FL
*                                           All Rights Reserved
*
*                                   Configuration for Intel 80x86 (Large)
*
* File : OS_CFG.H
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         uC/OS-II CONFIGURATION
*********************************************************************************************************
*/

#ifndef __OS_CFG_H
#define __OS_CFG_H

#ifdef __cplusplus
   extern "C" {
#endif

#define OS_MAX_EVENTS            36    /* Max. number of event control blocks in your application ...  */
                                       /* ... MUST be >= 2                                             */
#define OS_MAX_MEM_PART          10    /* Max. number of memory partitions ...                         */
                                       /* ... MUST be >= 2                                             */
#define OS_MAX_QS                32    /* Max. number of queue control blocks in your application ...  */
                                       /* ... MUST be >= 2                                             */
#define OS_MAX_TASKS             16    /* Max. number of tasks in your application ...                 */
                                       /* ... MUST be >= 2                                             */

#define OS_LOWEST_PRIO           63    /* Defines the lowest priority that can be assigned ...         */
                                       /* ... MUST NEVER be higher than 63!                            */

#define OS_TASK_IDLE_STK_SIZE    64    /* Idle task stack size (# of OS_STK wide entries)              */

#define OS_TASK_STAT_EN           0    /* Enable (1) or Disable(0) the statistics task                 */
#define OS_TASK_STAT_STK_SIZE   512    /* Statistics task stack size (# of OS_STK wide entries)        */

#define OS_CPU_HOOKS_EN           0    /* B.K. this port is not ready to use hooks                     */
#define OS_APP_HOOKS_EN           0    /* Application hooks are not used */
#define OS_TASK_SW_HOOK_EN        0    /* Task hooks are not used as well */
#define OS_TIME_TICK_HOOK_EN      0    /* Time tick hooks are not used */
#define OS_MBOX_EN                1    /* Include code for MAILBOXES                                   */
#define OS_MEM_EN                 0    /* Include code for MEMORY MANAGER (fixed sized memory blocks)  */
#define OS_Q_EN                   1    /* Include code for QUEUES                                      */
#define OS_SEM_EN                 1    /* Include code for SEMAPHORES                                  */
#define OS_TASK_CHANGE_PRIO_EN    1    /* Include code for OSTaskChangePrio()                          */
#define OS_TASK_CREATE_EN         1    /* Include code for OSTaskCreate()                              */
#define OS_TASK_CREATE_EXT_EN     1    /* Include code for OSTaskCreateExt()                           */
#define OS_TASK_DEL_EN            1    /* Include code for OSTaskDel()                                 */
#define OS_TASK_SUSPEND_EN        1    /* Include code for OSTaskSuspend() and OSTaskResume()          */

#define OS_TICKS_PER_SEC        100    /* Set the number of ticks in one second                        */

#define OS_TASK_STACK_SIZE		128    /* B.K. this is size of stack used by tasks                     */

#ifdef __cplusplus
}
#endif //to close extern "C" if used
#endif /* __OS_CFG_H */
