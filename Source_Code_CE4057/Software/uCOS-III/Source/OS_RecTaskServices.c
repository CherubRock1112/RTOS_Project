#include <OS_RecTaskServices.h>


SkipListHead head;
extern int isNew = 0;


/*
************************************************************************************************************************
*                                                RANDOM LEVEL
*
* Description: This function is called when an element is inserted into the Skip List. It gives a number between
*              0 and SKIP_LIST_MAX_LVL - 1. The probability to have +1 is 1/2.
*
* Arguments  : non
*
* Returns    : The number of level of the skip list on which the TCB will be inserted
*
************************************************************************************************************************
*/

int randomLevel(void)
{
    int temp = rand(), n = 0;
    while (temp % 2 == 0 && n < SKIP_LIST_MAX_LVL - 1)
    {
        n++;
        temp = rand();
    }

    return n;
}

/*
************************************************************************************************************************
*                                                ADD TASK TO TICK LIST
*
* Description: This function is called to place a task in the skip list. It is inserted with the Period as key.
*
* Arguments  : p_tcb          is a pointer to the OS_TCB of the task to add to the skip list
*              
* Returns    : None
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
*
*              2) This function is assumed to be called with interrupts disabled.
************************************************************************************************************************
*/

void OS_SkipListInsert(OS_TCB *p_tcb)
{
    int lvl = randomLevel(), i;

    #if OS_TRACE > 0u
        fprintf(stdout, "%s %s %d\n", "Adding of ", p_tcb->NamePtr, lvl);
    #endif

    OS_TCB *currentTCB = head.firstTCB;
    OS_TCB *lastTCB = currentTCB;
    CPU_SR_ALLOC();

    OS_CRITICAL_ENTER();
    p_tcb->TickCtrMatch = OSTickCtr + ((OS_TICK)p_tcb->Prio);

    if (head.firstTCB == NULL_TCB)
    { //First node
        //fprintf(stdout, "FIRST INSERTION\n");
        head.firstTCB = p_tcb;
    }
    else if (p_tcb->Prio < head.firstTCB->TickRemain)
    { //Node to be added is the smallest of the list
        //fprintf(stdout, "INSERTION AT HEAD (%d)\n", lvl);
        for (i = 0; i < SKIP_LIST_MAX_LVL; i++)
        {
            if (i > lvl)
            { //We chose that the precedent first node should be in the list on lvl levels
                p_tcb->NextTCB[i] = head.firstTCB->NextTCB[i];
            }
            else
            {
                p_tcb->NextTCB[i] = currentTCB;
            }
        }
        head.firstTCB = p_tcb;
    }
    else
    {
        for (i = SKIP_LIST_MAX_LVL - 1; i >= 0; i--)
        {
            while (currentTCB != NULL_TCB && p_tcb->Prio > currentTCB->TickRemain)
            {
                lastTCB = currentTCB;
                currentTCB = currentTCB->NextTCB[i];
            }

            if (i > lvl)
                currentTCB = lastTCB;
            else
            {
                if (currentTCB == NULL_TCB)
                { //Adding a node at the end of a level
                    
                    #if OS_TRACE > 1u
                        fprintf(stdout, "INSERTION AT END OF LEVEL %d\n", i);
                    #endif
                    lastTCB->NextTCB[i] = p_tcb;
                    currentTCB = lastTCB;
                }
                else
                { //Adding a node between 2 others
                    #if OS_TRACE > 1u
                        fprintf(stdout, "INSERTION BETWEEN 2 NODES AT LEVEL %d\n", i);
                    #endif
                    lastTCB->NextTCB[i] = p_tcb;
                    lastTCB->NextTCB[i]->NextTCB[i] = currentTCB;
                    currentTCB = lastTCB;
                }
            }
        }
    }
    OS_CRITICAL_EXIT();
    #if OS_TRACE > 2u
        printSkipList();
    #endif
}

/*
************************************************************************************************************************
*                                                UPDATE THE TICK LIST
*
* Description: This function is called when a tick occurs and determines if the timeout waiting for a kernel object has
*              expired or a delay has expired.
*
* Arguments  : non
*
* Returns    : none
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

void OS_SkipListUpdate(void)
{
    CPU_BOOLEAN done;
    OS_TCB *p_tcb;
    OS_TCB *p_tcb_next;
    CPU_SR_ALLOC();

    OS_CRITICAL_ENTER();
    OSTickCtr++; /* Keep track of the number of ticks           */
    p_tcb = head.firstTCB;
    done = DEF_FALSE;
    while (done == DEF_FALSE)
    {
        if (p_tcb != (OS_TCB *)0)
        {
            p_tcb_next = p_tcb->NextTCB[0];         /* Point to next TCB to update                 */
            p_tcb->TickRemain = p_tcb->TickCtrMatch /* Compute time remaining of current TCB  USELESS?     */
                                - OSTickCtr;
            if (OSTickCtr == p_tcb->TickCtrMatch)
            { /* Process each TCB that expires               */
                p_tcb->TaskState = OS_TASK_STATE_RDY;
                OS_PeriodicTaskRdy(p_tcb); /* Make task ready to run                      */
            }else if (OSTickCtr + 1 == p_tcb->TickCtrMatch){
                stackReset(p_tcb);
            }
            p_tcb = p_tcb_next;
        }
        else
        {
            done = DEF_TRUE;
        }
    }
    OS_CRITICAL_EXIT();
}

/*
************************************************************************************************************************
*                                         REMOVE A TASK FROM THE TICK LIST
*
* Description: This function is called to remove a task from the tick list
*
* Arguments  : p_tcb          Is a pointer to the OS_TCB to remove.
*              -----
*
* Returns    : none
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
*
*              2) This function is assumed to be called with interrupts disabled.
************************************************************************************************************************
*/

void OS_SkipListRemove(OS_TCB *p_tcb)
{
    #if OS_TRACE > 0u
        fprintf(stdout, "%s %s\n", "Removal of ", p_tcb->NamePtr);
    #endif
    int i;
    OS_TCB *currentTCB = head.firstTCB;
    OS_TCB *lastTCB = head.firstTCB;
    for (i = SKIP_LIST_MAX_LVL - 1; i >= 0; i--)
    {
        while (currentTCB != NULL_TCB && p_tcb->NamePtr != currentTCB->NamePtr)
        {
            lastTCB = currentTCB;
            currentTCB = currentTCB->NextTCB[i];
        }

        if (currentTCB == head.firstTCB)
        {
            head.firstTCB = currentTCB->NextTCB[0];
        }

        if (currentTCB == NULL_TCB)
            currentTCB = lastTCB;
        else
        {
            #if OS_TRACE > 1u
                fprintf(stdout, "%s %s %s %d\n", "DELETION OF ", currentTCB->NamePtr, " at level ", i);
            #endif
            lastTCB->NextTCB[i] = currentTCB->NextTCB[i]; //Update the pointers of the previous node
            currentTCB->NextTCB[i] = NULL_TCB;
            currentTCB = lastTCB;
        }
    }
    #if OS_TRACE > 2u
        printSkipList();
    #endif
}

/*
************************************************************************************************************************
*                                               INITIALIZE THE TICK LIST
*
* Description: This function initializes the skip list
*
* Arguments  : none
*
* Returns    : None
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

void OS_SkipListInit(void)
{
    head.firstTCB = NULL_TCB;
}

/*
************************************************************************************************************************
*                                               RESET THE STACK OF A TASK
*
* Description: Clears the stack of a task, and initialize it.
*
* Arguments  : p_tcb          Is a pointer to the OS_TCB of the task.
*
* Returns    : None
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

void stackReset(OS_TCB *p_tcb)
{
    #if OS_TRACE > 1u
        fprintf(stdout, "%s %s\n", "RESETING THE STACK OF ", p_tcb->NamePtr);
    #endif
    CPU_STK *p_sp = p_tcb->StkBasePtr;
    p_sp = p_tcb->StkBasePtr;
    for (int i = 0u; i < p_tcb->StkSize; i++) {               // Stack grows from HIGH to LOW memory                    
        *p_sp = (CPU_STK)0;                         // Clear from bottom of stack and up!                     
        p_sp++;
    }

    p_sp = OSTaskStkInit(p_tcb->TaskEntryAddr,
                         p_tcb->TaskEntryArg,
                         p_tcb->StkBasePtr,
                         p_tcb->StkLimitPtr,
                         p_tcb->StkSize,
                         p_tcb->Opt);

    p_tcb->StkPtr        = p_sp;
}


/*
************************************************************************************************************************
*                                               MAKE A TASK READY TO RUN
*
* Description: This function is called when the tick counter of a task is matched. Removes the task from the skip list,
*              clear its stack before adding it to the ready list and adds it again in the skip list.
*
* Arguments  : p_tcb          Is a pointer to the OS_TCB to the specified task.
*
* Returns    : None
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

void OS_PeriodicTaskRdy(OS_TCB *p_tcb)
{
    CPU_CHAR *name = p_tcb->NamePtr;
    #if OS_TRACE > 0u
        fprintf(stdout, "%s %s %d\n", "RELEASE OF", p_tcb->NamePtr, OS_TS_GET() - LastTickISR);
    #endif
    OS_RdyListInsert(p_tcb);
    p_tcb->NamePtr = name;
    OS_SkipListRemove(p_tcb);
    isNew = 1;
    OS_SkipListInsert(p_tcb);
}


/*
************************************************************************************************************************
*                                               MAKE A TASK PERIODIC
*
* Description: This function is called to make a task periodic. Because we implement Rate Monotonic scheduling, we chose
*              to have Priority = Period in ticks. Assigns the priority and inserts the task in both the Ready List and Skip List.
*
* Arguments  : p_tcb          Is a pointer to the OS_TCB to the specified task.
*              period         Period wanted for the task, in OS Ticks (10Hz).
*
* Returns    : None
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

void addPeriodicTask(OS_TCB *p_tcb, OS_TICK period)
{
    p_tcb->Prio = (OS_PRIO)period;
    CPU_CHAR *name = p_tcb->NamePtr;
    isNew = 1;
    OS_RdyListInsert(p_tcb);
    OS_SkipListInsert(p_tcb);
    p_tcb->NamePtr = name;
    
    #if OS_TRACE > 0u
        fprintf(stdout, "%s %s\n", p_tcb->NamePtr, " is now periodic.");
    #endif

}

/*
************************************************************************************************************************
*                                               REMOVE A PERIODIC TASK
*
* Description: This function is called to remove a periodic task from the system. Simply removes the task from the Skip list
*              and clear its stack.
*
* Arguments  : p_tcb          Is a pointer to the OS_TCB of the task to remove.
*
* Returns    : None
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

void removePeriodicTask(OS_TCB *p_tcb)
{
    OS_SkipListRemove(p_tcb);
    stackReset(p_tcb);

    
    
    #if OS_TRACE > 0u
        fprintf(stdout, "%s %s\n", p_tcb->NamePtr, " is no longer periodic.");
    #endif
}

/*
************************************************************************************************************************
*                                               END OF A TASK CODE
*
* Description: This function must be called at the end of the code of a periodic function. Remove the task from the ready list
*              and call the scheduler to wfind which task to run next.
*
* Arguments  : p_tcb          Is a pointer to the OS_TCB to the specified task.
*
* Returns    : None
*
************************************************************************************************************************
*/

void endTask(OS_TCB *p_tcb)
{
    OS_RdyListRemove(p_tcb);                          /* Remove current task from ready list                    */
    OSSched();
}

/*
************************************************************************************************************************
*                                               PRINT THE SKIP LIST
*
* Description: Print the skip list, showing the task name with the level of the list in which it is present.
*
* Arguments  : None
*
* Returns    : None
*
************************************************************************************************************************
*/

void printSkipList(void)
{
    int i;
    OS_TCB *originalNode, *currentTCB;
    originalNode = head.firstTCB;
    for (i = SKIP_LIST_MAX_LVL - 1; i >= 0; i--)
    {
        currentTCB = originalNode;
        fprintf(stdout, "%s (%d) %d %s\n", currentTCB->NamePtr, i, currentTCB->TickRemain, " remaining");
        while (currentTCB->NextTCB[i] != NULL_TCB)
        {
            currentTCB = currentTCB->NextTCB[i];
            fprintf(stdout, "%s (%d) %d %s\n", currentTCB->NamePtr, i, currentTCB->TickRemain, " remaining");
        }
    }
}

/*
************************************************************************************************************************
*                                               CREATE A TASK DESTINED TO BE PERIODIC
*
* Description: Identical to the normal OSTaskCreate function, simply doesn't add the task to the Ready List
*
* Arguments  : None
*
* Returns    : None
*
************************************************************************************************************************
*/

void  OSTaskPeriodicCreate (OS_TCB        *p_tcb,
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
                    OS_ERR        *p_err)
{
    CPU_STK_SIZE   i;
#if OS_CFG_TASK_REG_TBL_SIZE > 0u
    OS_OBJ_QTY     reg_nbr;
#endif
    CPU_STK       *p_sp;
    CPU_STK       *p_stk_limit;
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == DEF_TRUE) {
       *p_err = OS_ERR_ILLEGAL_CREATE_RUN_TIME;
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* ---------- CANNOT CREATE A TASK FROM AN ISR ---------- */
        *p_err = OS_ERR_TASK_CREATE_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u                                  /* ---------------- VALIDATE ARGUMENTS ------------------ */
    if (p_tcb == (OS_TCB *)0) {                             /* User must supply a valid OS_TCB                        */
        *p_err = OS_ERR_TCB_INVALID;
        return;
    }
    if (p_task == (OS_TASK_PTR)0) {                         /* User must supply a valid task                          */
        *p_err = OS_ERR_TASK_INVALID;
        return;
    }
    if (p_stk_base == (CPU_STK *)0) {                       /* User must supply a valid stack base address            */
        *p_err = OS_ERR_STK_INVALID;
        return;
    }
    if (stk_size < OSCfg_StkSizeMin) {                      /* User must supply a valid minimum stack size            */
        *p_err = OS_ERR_STK_SIZE_INVALID;
        return;
    }
    if (stk_limit >= stk_size) {                            /* User must supply a valid stack limit                   */
        *p_err = OS_ERR_STK_LIMIT_INVALID;
        return;
    }
    if (prio >= OS_CFG_PRIO_MAX) {                          /* Priority must be within 0 and OS_CFG_PRIO_MAX-1        */
        *p_err = OS_ERR_PRIO_INVALID;
        return;
    }
#endif

#if OS_CFG_ISR_POST_DEFERRED_EN > 0u
    if (prio == (OS_PRIO)0) {
        if (p_tcb != &OSIntQTaskTCB) {
            *p_err = OS_ERR_PRIO_INVALID;                   /* Not allowed to use priority 0                          */
            return;
        }
    }
#endif

    if (prio == (OS_CFG_PRIO_MAX - 1u)) {
        if (p_tcb != &OSIdleTaskTCB) {
            *p_err = OS_ERR_PRIO_INVALID;                   /* Not allowed to use same priority as idle task          */
            return;
        }
    }

    OS_TaskInitTCB(p_tcb);                                  /* Initialize the TCB to default values                   */

    *p_err = OS_ERR_NONE;
                                                            /* --------------- CLEAR THE TASK'S STACK --------------- */
    if ((opt & OS_OPT_TASK_STK_CHK) != (OS_OPT)0) {         /* See if stack checking has been enabled                 */
        if ((opt & OS_OPT_TASK_STK_CLR) != (OS_OPT)0) {     /* See if stack needs to be cleared                       */
            p_sp = p_stk_base;
            for (i = 0u; i < stk_size; i++) {               /* Stack grows from HIGH to LOW memory                    */
                *p_sp = (CPU_STK)0;                         /* Clear from bottom of stack and up!                     */
                p_sp++;
            }
        }
    }
                                                            /* ------- INITIALIZE THE STACK FRAME OF THE TASK ------- */
#if (CPU_CFG_STK_GROWTH == CPU_STK_GROWTH_HI_TO_LO)
    p_stk_limit = p_stk_base + stk_limit;
#else
    p_stk_limit = p_stk_base + (stk_size - 1u) - stk_limit;
#endif

    p_sp = OSTaskStkInit(p_task,
                         p_arg,
                         p_stk_base,
                         p_stk_limit,
                         stk_size,
                         opt);

                                                            /* -------------- INITIALIZE THE TCB FIELDS ------------- */
    p_tcb->TaskEntryAddr = p_task;                          /* Save task entry point address                          */
    p_tcb->TaskEntryArg  = p_arg;                           /* Save task entry argument                               */

    p_tcb->NamePtr       = p_name;                          /* Save task name                                         */

    p_tcb->Prio          = prio;                            /* Save the task's priority                               */

    p_tcb->StkPtr        = p_sp;                            /* Save the new top-of-stack pointer                      */
    p_tcb->StkLimitPtr   = p_stk_limit;                     /* Save the stack limit pointer                           */

    p_tcb->TimeQuanta    = time_quanta;                     /* Save the #ticks for time slice (0 means not sliced)    */
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
    if (time_quanta == (OS_TICK)0) {
        p_tcb->TimeQuantaCtr = OSSchedRoundRobinDfltTimeQuanta;
    } else {
        p_tcb->TimeQuantaCtr = time_quanta;
    }
#endif
    p_tcb->ExtPtr        = p_ext;                           /* Save pointer to TCB extension                          */
    p_tcb->StkBasePtr    = p_stk_base;                      /* Save pointer to the base address of the stack          */
    p_tcb->StkSize       = stk_size;                        /* Save the stack size (in number of CPU_STK elements)    */
    p_tcb->Opt           = opt;                             /* Save task options                                      */

#if OS_CFG_TASK_REG_TBL_SIZE > 0u
    for (reg_nbr = 0u; reg_nbr < OS_CFG_TASK_REG_TBL_SIZE; reg_nbr++) {
        p_tcb->RegTbl[reg_nbr] = (OS_REG)0;
    }
#endif

#if OS_CFG_TASK_Q_EN > 0u
    OS_MsgQInit(&p_tcb->MsgQ,                               /* Initialize the task's message queue                    */
                q_size);
#endif

    OSTaskCreateHook(p_tcb);                                /* Call user defined hook                                 */

                                                            /* --------------- ADD TASK TO READY LIST --------------- */
    OS_CRITICAL_ENTER();

#if OS_CFG_DBG_EN > 0u
    OS_TaskDbgListAdd(p_tcb);
#endif

    OSTaskQty++;                                            /* Increment the #tasks counter                           */

    if (OSRunning != OS_STATE_OS_RUNNING) {                 /* Return if multitasking has not started                 */
        OS_CRITICAL_EXIT();
        return;
    }

    OS_CRITICAL_EXIT_NO_SCHED();

    OSSched();
}