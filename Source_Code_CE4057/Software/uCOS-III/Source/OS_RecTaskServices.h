#include <os.h>
#include <time.h>
#include <stdio.h>


/* ------------------------------------------------PROJECT FUNCTIONS------------------------------------------------ */



int           randomLevel               (void);

void          OS_SkipListInsert         (OS_TCB *p_tcb);

void          OS_SkipListRemove         (OS_TCB *p_tcb);

void          OS_SkipListInit           (void);

void          OS_PeriodicTaskRdy        (OS_TCB *p_tcb);

void          OS_SkipListUpdate         (void);

void          AppTaskSkipListHead       (void  *p_arg);

void          addPeriodicTask           (OS_TCB *p_tcb, 
                                         OS_TICK period);

void          removePeriodicTask        (OS_TCB *p_tcb);

void          endTask                   (OS_TCB *p_tcb);

void          printSkipList             (void);

void          OSTaskPeriodicCreate      (OS_TCB        *p_tcb,
                                        CPU_CHAR      *p_name,
                                        OS_TASK_PTR    p_task,
                                        void          *p_arg,
                                        OS_PRIO        prio,
                                        CPU_STK       *p_stk_base,
                                        CPU_STK_SIZE   stk_limit,
                                        CPU_STK_SIZE   stk_size,
                                        OS_MSG_QTY     q_size,
                                        OS_TICK        time_quanta,
                                        void          *p_ext,
                                        OS_OPT         opt,
                                        OS_ERR        *p_err);


void         stackReset                 (OS_TCB *p_tcb);     
typedef struct skipListHead
{
    OS_TCB *firstTCB;
}SkipListHead;