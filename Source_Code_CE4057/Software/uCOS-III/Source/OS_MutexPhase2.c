#include <OS_MutexPhase2.h>


OS_TCB *RBTRoot;
OS_MUTEX *StackTop;
OS_TCB *blockedTask;


//Both called in OSStart()
void OS_RBT_init(void)
{
    RBTRoot = NULL_TCB;
}

void OS_Stack_init(void)
{
    StackTop = NULL_MUTEX;
}

void OS_RBTinsertion(OS_TCB *newElem, OS_MUTEX *mutex)
{
    OS_TCB *stack[MAX_HEIGHT], *ptr, *newnode, *xPtr, *yPtr;
    int dir[MAX_HEIGHT], ht = 0, index;
#if OS_TRACE_MUTEX > 1u
    fprintf(stdout, "%s %s %s %s\n", "Addition of ", newElem->NamePtr, " because of ", mutex->NamePtr);
#endif

#if SHOW_MUTEX_OVERHEAD > 1u
    TSRBT = OS_TS_GET();
#endif
    newElem->WaitingForMutex = mutex;
    ptr = RBTRoot;
    if (!RBTRoot)
    {
        RBTRoot = newElem;
        return;
    }
    stack[ht] = RBTRoot;
    dir[ht++] = 0;
    /* find the place to insert the new node */
    while (ptr != NULL_TCB)
    {
        /*if (ptr->Prio == newElem->Prio)
        {
            printf("Duplicates Not Allowed!!\n");
            return;
        }*/
        index = (newElem->Period - ptr->Period) > 0 ? 1 : 0;
        stack[ht] = ptr;
        ptr = ptr->RBTChildren[index];
        dir[ht++] = index;
    }
    /* insert the new node */
    stack[ht - 1]->RBTChildren[index] = newnode = newElem;
    while ((ht >= 3) && (stack[ht - 1]->color == RED))
    {
        if (dir[ht - 2] == 0)
        {
            yPtr = stack[ht - 2]->RBTChildren[1];
            if (yPtr != NULL_TCB && yPtr->color == RED)
            {
                /*
                                 * Red node having red child. B- black, R-red
                                 *     B                R
                                 *    / \             /   \
                                 *   R   R  =>     B     B
                                 *  /               /   
                                 * R               R
                                 */
                stack[ht - 2]->color = RED;
                stack[ht - 1]->color = yPtr->color = BLACK;
                ht = ht - 2;
            }
            else
            {
                if (dir[ht - 1] == 0)
                {
                    yPtr = stack[ht - 1];
                }
                else
                {
                    /*
                                         * XR - node X with red color
                                         * YR - node Y with red color
                                         * Red node having red child
                                         *(do single rotation left b/w X and Y)
                                         *         B             B
                                         *        /             /
                                         *      XR     =>      YR
                                         *        \           /
                                         *         YR        XR
                                         * one more additional processing will be
                                         * performed after this else part.  Since
                                         * we have red node (YR) with red child(XR)
                                         */
                    xPtr = stack[ht - 1];
                    yPtr = xPtr->RBTChildren[1];
                    xPtr->RBTChildren[1] = yPtr->RBTChildren[0];
                    yPtr->RBTChildren[0] = xPtr;
                    stack[ht - 2]->RBTChildren[0] = yPtr;
                }
                /*
                                 *  Red node(YR) with red child (XR) - single
                                 *  rotation b/w YR and XR for height balance. Still,
                                 *  red node (YR) is having red child.  So, change the
                                 *  color of Y to black and Black child B to Red R
                                 *          B           YR          YB
                                 *         /           /  \        /  \
                                 *        YR  =>   XR   B  =>  XR  R
                                 *       /
                                 *      XR
                                 */
                xPtr = stack[ht - 2];
                xPtr->color = RED;
                yPtr->color = BLACK;
                xPtr->RBTChildren[0] = yPtr->RBTChildren[1];
                yPtr->RBTChildren[1] = xPtr;
                if (xPtr == RBTRoot)
                {
                    RBTRoot = yPtr;
                }
                else
                {
                    stack[ht - 3]->RBTChildren[dir[ht - 3]] = yPtr;
                }
                break;
            }
        }
        else
        {
            yPtr = stack[ht - 2]->RBTChildren[0];
            if ((yPtr != NULL_TCB) && (yPtr->color == RED))
            {
                /*
                                 * Red node with red child
                                 *        B             R
                                 *      /   \         /   \
                                 *     R     R =>  B     B
                                 *             \              \
                                 *              R              R
                                 *
                                 */
                stack[ht - 2]->color = RED;
                stack[ht - 1]->color = yPtr->color = BLACK;
                ht = ht - 2;
            }
            else
            {
                if (dir[ht - 1] == 1)
                {
                    yPtr = stack[ht - 1];
                }
                else
                {
                    /*
                                         * Red node(XR) with red child(YR) 
                                         *   B          B
                                         *    \          \
                                         *     XR  => YR
                                         *    /            \
                                         *   YR             XR
                                         * Single rotation b/w XR(node x with red color) & YR
                                         */
                    xPtr = stack[ht - 1];
                    yPtr = xPtr->RBTChildren[0];
                    xPtr->RBTChildren[0] = yPtr->RBTChildren[1];
                    yPtr->RBTChildren[1] = xPtr;
                    stack[ht - 2]->RBTChildren[1] = yPtr;
                }
                /*
                                 *   B              YR          YB
                                 *    \             /  \        /  \
                                 *     YR  =>   B   XR => R    XR
                                 *      \
                                 *       XR
                                 * Single rotation b/w YR and XR and change the color to
                                 * satisfy rebalance property.
                                 */
                xPtr = stack[ht - 2];
                yPtr->color = BLACK;
                xPtr->color = RED;
                xPtr->RBTChildren[1] = yPtr->RBTChildren[0];
                yPtr->RBTChildren[0] = xPtr;
                if (xPtr == RBTRoot)
                {
                    RBTRoot = yPtr;
                }
                else
                {
                    stack[ht - 3]->RBTChildren[dir[ht - 3]] = yPtr;
                }
                break;
            }
        }
    }
    RBTRoot->color = BLACK;

#if SHOW_MUTEX_OVERHEAD > 1u
    fprintf(stdout, "%s %s %s  %d\n", "ADDING ", newElem->NamePtr, " TOOK ", OS_TS_GET() - TSRBT);
#endif

#if OS_TRACE_MUTEX > 1u
    fprintf(stdout, "%s\n\n", "Addition performed");
#endif
}

void OS_RBTdeletion(OS_TCB *oldElem)
{
    OS_TCB *stack[MAX_HEIGHT], *ptr, *xPtr, *yPtr;
    OS_TCB *pPtr, *qPtr, *rPtr;
    int dir[MAX_HEIGHT], ht = 0, diff, i;
    enum nodeColor color;
#if OS_TRACE_MUTEX > 1u
    fprintf(stdout, "%s %s\n", "Deletion of ", oldElem->NamePtr);
#endif

#if SHOW_MUTEX_OVERHEAD > 1u
    TSRBT = OS_TS_GET();
#endif

    oldElem->WaitingForMutex = NULL_MUTEX;

    if (RBTRoot == NULL_TCB)
    {
        printf("Tree not available\n");
        return;
    }

    ptr = RBTRoot;
    /* search the node to delete */
    while (ptr != NULL_TCB)
    {
        if (oldElem->NamePtr == ptr->NamePtr)
            break;
        diff = (oldElem->Period - ptr->Period) > 0 ? 1 : 0;
        stack[ht] = ptr;
        dir[ht++] = diff;
        ptr = ptr->RBTChildren[diff];
    }

    if (ptr->RBTChildren[1] == NULL_TCB)
    {
        /* node with no children */
        if ((ptr == RBTRoot) && (ptr->RBTChildren[0] == NULL_TCB))
        {
            RBTRoot = NULL_TCB;
        }
        else if (ptr == RBTRoot)
        {
            /* deleting RBTRoot - RBTRoot with one child */
            RBTRoot = ptr->RBTChildren[0];
        }
        else
        {
            /* node with one child */
            stack[ht - 1]->RBTChildren[dir[ht - 1]] = ptr->RBTChildren[0];
        }
    }
    else
    {
        xPtr = ptr->RBTChildren[1];
        if (xPtr->RBTChildren[0] == NULL_TCB)
        {
            /*
                         * node with 2 children - deleting node 
                         * whose right child has no left child
                         */
            xPtr->RBTChildren[0] = ptr->RBTChildren[0];
            color = xPtr->color;
            xPtr->color = ptr->color;
            ptr->color = color;

            if (ptr == RBTRoot)
            {
                RBTRoot = xPtr;
            }
            else
            {
                stack[ht - 1]->RBTChildren[dir[ht - 1]] = xPtr;
            }

            dir[ht] = 1;
            stack[ht++] = xPtr;
        }
        else
        {
            /* deleting node with 2 children */
            i = ht++;
            while (1)
            {
                dir[ht] = 0;
                stack[ht++] = xPtr;
                yPtr = xPtr->RBTChildren[0];
                if (!yPtr->RBTChildren[0])
                    break;
                xPtr = yPtr;
            }

            dir[i] = 1;
            stack[i] = yPtr;
            if (i > 0)
                stack[i - 1]->RBTChildren[dir[i - 1]] = yPtr;

            yPtr->RBTChildren[0] = ptr->RBTChildren[0];
            xPtr->RBTChildren[0] = yPtr->RBTChildren[1];
            yPtr->RBTChildren[1] = ptr->RBTChildren[1];

            if (ptr == RBTRoot)
            {
                RBTRoot = yPtr;
            }

            color = yPtr->color;
            yPtr->color = ptr->color;
            ptr->color = color;
        }
    }
    if (ht < 1)
        return;
    if (ptr->color == BLACK)
    {
        while (1)
        {
            pPtr = stack[ht - 1]->RBTChildren[dir[ht - 1]];
            if (pPtr && pPtr->color == RED)
            {
                pPtr->color = BLACK;
                break;
            }

            if (ht < 2)
                break;

            if (dir[ht - 2] == 0)
            {
                rPtr = stack[ht - 1]->RBTChildren[1];

                if (!rPtr)
                    break;

                if (rPtr->color == RED)
                {
                    /*
                                         * incase if rPtr is red, we need
                                         * change it to black..
                                         *    aB                 rPtr (red)  rPtr(black)
                                         *   /  \      =>      /    \  =>    /   \
                                         *  ST  rPtr(red)  aB    cB      aR   cB
                                         *       /  \       /  \           /  \
                                         *     bB  cB   ST  bB       ST  bB
                                         *  ST - subtree
                                         *  xB - node x with Black color
                                         *  xR - node x with Red color
                                         * the above operation will simply rebalace
                                         * operation in RB tree
                                         */
                    stack[ht - 1]->color = RED;
                    rPtr->color = BLACK;
                    stack[ht - 1]->RBTChildren[1] = rPtr->RBTChildren[0];
                    rPtr->RBTChildren[0] = stack[ht - 1];

                    if (stack[ht - 1] == RBTRoot)
                    {
                        RBTRoot = rPtr;
                    }
                    else
                    {
                        stack[ht - 2]->RBTChildren[dir[ht - 2]] = rPtr;
                    }
                    dir[ht] = 0;
                    stack[ht] = stack[ht - 1];
                    stack[ht - 1] = rPtr;
                    ht++;

                    rPtr = stack[ht - 1]->RBTChildren[1];
                }

                if ((!rPtr->RBTChildren[0] || rPtr->RBTChildren[0]->color == BLACK) &&
                    (!rPtr->RBTChildren[1] || rPtr->RBTChildren[1]->color == BLACK))
                {
                    /*
                                         *      rPtr(black)         rPtr(Red)
                                         *     /    \          =>  /    \
                                         *    B      B            R      R
                                         *
                                         */
                    rPtr->color = RED;
                }
                else
                {
                    if (!rPtr->RBTChildren[1] || rPtr->RBTChildren[1]->color == BLACK)
                    {
                        /*
                                                 * Below is a subtree. rPtr with red left child
                                                 * single rotation right b/w yR and rPtr  &
                                                 * change the color as needed
                                                 *        wR                        wR
                                                 *       /  \                      /  \
                                                 *      xB   rPtr(Black) =>   xB  yB
                                                 *     / \   /  \               /  \  /  \
                                                 *    a   b yR   e           a   b c   rPtr(Red)
                                                 *          /  \                          /  \
                                                 *         c    d                        d    e
                                                 */
                        qPtr = rPtr->RBTChildren[0];
                        rPtr->color = RED;
                        qPtr->color = BLACK;
                        rPtr->RBTChildren[0] = qPtr->RBTChildren[1];
                        qPtr->RBTChildren[1] = rPtr;
                        rPtr = stack[ht - 1]->RBTChildren[1] = qPtr;
                    }
                    /*
                                         * Below is a subtree. rPtr with Right red child
                                         * single rotation b/w rPtr & wR and change colors
                                         *       wR (stack[ht-1])      rPtr(Red)
                                         *     /   \                        /    \
                                         *    xB    rPtr(black)     wB     yB
                                         *   / \   /  \        =>    /   \   /  \
                                         *  a   b c    yR         xB    c d    e
                                         *              /  \       /  \
                                         *             d    e    a    b
                                         */
                    rPtr->color = stack[ht - 1]->color;
                    stack[ht - 1]->color = BLACK;
                    rPtr->RBTChildren[1]->color = BLACK;
                    stack[ht - 1]->RBTChildren[1] = rPtr->RBTChildren[0];
                    rPtr->RBTChildren[0] = stack[ht - 1];
                    if (stack[ht - 1] == RBTRoot)
                    {
                        RBTRoot = rPtr;
                    }
                    else
                    {
                        stack[ht - 2]->RBTChildren[dir[ht - 2]] = rPtr;
                    }
                    break;
                }
            }
            else
            {
                rPtr = stack[ht - 1]->RBTChildren[0];
                if (!rPtr)
                    break;

                if (rPtr->color == RED)
                {
                    stack[ht - 1]->color = RED;
                    rPtr->color = BLACK;
                    stack[ht - 1]->RBTChildren[0] = rPtr->RBTChildren[1];
                    rPtr->RBTChildren[1] = stack[ht - 1];

                    if (stack[ht - 1] == RBTRoot)
                    {
                        RBTRoot = rPtr;
                    }
                    else
                    {
                        stack[ht - 2]->RBTChildren[dir[ht - 2]] = rPtr;
                    }
                    dir[ht] = 1;
                    stack[ht] = stack[ht - 1];
                    stack[ht - 1] = rPtr;
                    ht++;

                    rPtr = stack[ht - 1]->RBTChildren[0];
                }
                if ((!rPtr->RBTChildren[0] || rPtr->RBTChildren[0]->color == BLACK) &&
                    (!rPtr->RBTChildren[1] || rPtr->RBTChildren[1]->color == BLACK))
                {
                    rPtr->color = RED;
                }
                else
                {
                    if (!rPtr->RBTChildren[0] || rPtr->RBTChildren[0]->color == BLACK)
                    {
                        qPtr = rPtr->RBTChildren[1];
                        rPtr->color = RED;
                        qPtr->color = BLACK;
                        rPtr->RBTChildren[1] = qPtr->RBTChildren[0];
                        qPtr->RBTChildren[0] = rPtr;
                        rPtr = stack[ht - 1]->RBTChildren[0] = qPtr;
                    }
                    rPtr->color = stack[ht - 1]->color;
                    stack[ht - 1]->color = BLACK;
                    rPtr->RBTChildren[0]->color = BLACK;
                    stack[ht - 1]->RBTChildren[0] = rPtr->RBTChildren[1];
                    rPtr->RBTChildren[1] = stack[ht - 1];
                    if (stack[ht - 1] == RBTRoot)
                    {
                        RBTRoot = rPtr;
                    }
                    else
                    {
                        stack[ht - 2]->RBTChildren[dir[ht - 2]] = rPtr;
                    }
                    break;
                }
            }
            ht--;
        }
    }

#if SHOW_MUTEX_OVERHEAD > 1u
    fprintf(stdout, "%s %s %s  %d\n", "DELETING ", oldElem->NamePtr, " TOOK ", OS_TS_GET() - TSRBT);
#endif

#if OS_TRACE_MUTEX > 1u
    fprintf(stdout, "%s\n\n", "Deletion performed");
#endif
}

int OS_RBTsearchTCB(OS_TCB *p_tcb)
{
    OS_TCB *temp = RBTRoot;
    int diff;

    while (temp != NULL_TCB)
    {
        diff = p_tcb->Period - temp->Period;
        if (diff > 0)
        {
            temp = temp->RBTChildren[1];
        }
        else if (diff < 0)
        {
            temp = temp->RBTChildren[0];
        }
        else if (p_tcb->NamePtr == temp->NamePtr)
        {
            return 1;
        }
    }
    return 0;
}

OS_TCB *OS_FindBlockedTask(OS_MUTEX *p_mutex)
{ //Supposed to find the highest priority Task blocked by a specific mutex
    OS_TCB *p_tcb = RBTRoot;
    blockedTask = NULL_TCB;
    FindBlockedTask(p_tcb, p_mutex);
    return blockedTask;
}

void FindBlockedTask(OS_TCB *root, OS_MUTEX *p_mutex)
{
    if (root == NULL_TCB || blockedTask != NULL_TCB)
        return;

    FindBlockedTask(root->RBTChildren[1], p_mutex);

    if (root->WaitingForMutex == p_mutex && blockedTask == NULL_TCB)
        blockedTask = root;

    FindBlockedTask(root->RBTChildren[0], p_mutex);
}

void OS_RBTprint2D(void)
{
    // Pass initial space count as 0
    print2DUtil(RBTRoot, 0);
}

void print2DUtil(OS_TCB *root, int space)
{
    // Base case
    if (RBTRoot == NULL_TCB)
        return;

    // Increase distance between levels
    space += COUNT;

    // Process right child first
    print2DUtil(root->RBTChildren[1], space);

    // Print current node after space
    // count
    printf("\n");
    for (int i = COUNT; i < space; i++)
        printf(" ");
    printf("%d (%c)\n", root->Prio, (root->color ? 'B' : 'R'));

    // Process left child
    print2DUtil(root->RBTChildren[0], space);
}

void OS_StackPush(OS_MUTEX *p_mutex)
{
    p_mutex->stackNext = StackTop;
    StackTop = p_mutex;
#if OS_TRACE_MUTEX > 0u
    fprintf(stdout, "%d %s\n", StackTop->Ceiling, " has been added to the Stack");
    fprintf(stdout, "%s %d\n", "SysCeiling is now ", StackTop->Ceiling);
#endif
}

void OS_StackPop()
{
    OS_MUTEX *temp = StackTop;
    if (StackTop == NULL_MUTEX)
        return;
    StackTop = StackTop->stackNext;
    temp->stackNext = NULL_MUTEX;
#if OS_TRACE_MUTEX > 0u
    fprintf(stdout, "%d %s\n", temp->Ceiling, " has been removed from the Stack");
    fprintf(stdout, "%s %d\n", "SysCeiling is now ", StackTop->Ceiling);
#endif
}


/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/

void  OSPCPMutexCreate (OS_MUTEX    *p_mutex,
                     CPU_CHAR    *p_name,
                     OS_ERR      *p_err,
                     OS_TCB      *p_tcb)
{
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
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* Not allowed to be called from an ISR                   */
        *p_err = OS_ERR_CREATE_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u
    if (p_mutex == (OS_MUTEX *)0) {                         /* Validate 'p_mutex'                                     */
        *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
#endif

    CPU_CRITICAL_ENTER();
    p_mutex->Type              =  OS_OBJ_TYPE_MUTEX;        /* Mark the data structure as a mutex                     */
    p_mutex->NamePtr           =  p_name;
    p_mutex->OwnerTCBPtr       = (OS_TCB       *)0;
    p_mutex->OwnerNestingCtr   = (OS_NESTING_CTR)0;         /* Mutex is available                                     */
    p_mutex->TS                = (CPU_TS        )0;
    p_mutex->OwnerOriginalPrio =  OS_CFG_PRIO_MAX;
    p_mutex->TCBWaiting = 0;
    p_mutex->Ceiling = p_tcb->Prio;
    p_mutex->stackNext = NULL_MUTEX;
    OS_PendListInit(&p_mutex->PendList);                    /* Initialize the waiting list                            */

#if OS_CFG_DBG_EN > 0u
    OS_MutexDbgListAdd(p_mutex);
#endif
    OSMutexQty++;

    CPU_CRITICAL_EXIT();
    *p_err = OS_ERR_NONE;
}

/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/

void OSPCPMutexPend(OS_MUTEX *p_mutex,
                 OS_TICK timeout,
                 OS_OPT opt,
                 CPU_TS *p_ts,
                 OS_ERR *p_err)
{
    OS_PEND_DATA pend_data;
    OS_TCB *p_tcb;
    CPU_SR_ALLOC();

#if SHOW_MUTEX_OVERHEAD > 0u
    TSMutex = OS_TS_GET();
#endif

#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0)
    {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0)
    { /* Not allowed to call from an ISR                        */
        *p_err = OS_ERR_PEND_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u
    if (p_mutex == (OS_MUTEX *)0)
    { /* Validate arguments                                     */
        *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
    switch (opt)
    {
    case OS_OPT_PEND_BLOCKING:
    case OS_OPT_PEND_NON_BLOCKING:
        break;

    default:
        *p_err = OS_ERR_OPT_INVALID;
        return;
    }
#endif

#if OS_CFG_OBJ_TYPE_CHK_EN > 0u
    if (p_mutex->Type != OS_OBJ_TYPE_MUTEX)
    { /* Make sure mutex was created                            */
        *p_err = OS_ERR_OBJ_TYPE;
        return;
    }
#endif

    if (p_ts != (CPU_TS *)0)
    {
        *p_ts = (CPU_TS)0; /* Initialize the returned timestamp                      */
    }

    CPU_CRITICAL_ENTER();
    if (p_mutex->OwnerTCBPtr == NULL_TCB && 
    (OSTCBCurPtr->hasMutex > 0 || StackTop == NULL_MUTEX || OSTCBCurPtr->Prio < StackTop->Ceiling))
    {                                       /* Resource available or task already owns mutexes or Prio of the task higher that system ceiling? */
        p_mutex->OwnerTCBPtr = OSTCBCurPtr; /* Yes, caller may proceed                                */
        p_mutex->OwnerOriginalPrio = OSTCBCurPtr->Prio;
        p_mutex->OwnerNestingCtr = (OS_NESTING_CTR)1;
        OSTCBCurPtr->hasMutex++;
        if (p_ts != (CPU_TS *)0)
        {
            *p_ts = p_mutex->TS;
        }
#if OS_TRACE_MUTEX > 0u
        fprintf(stdout, "%s %s %s\n", p_mutex->OwnerTCBPtr->NamePtr, " has acquired ", p_mutex->NamePtr);
#endif
        if (StackTop == NULL_MUTEX || StackTop->Ceiling >= p_mutex->Ceiling)
            OS_StackPush(p_mutex);

        CPU_CRITICAL_EXIT();
        *p_err = OS_ERR_NONE;

        
#if SHOW_MUTEX_OVERHEAD > 0u
    fprintf(stdout, "%s %s %s %s %s %d\n", "ACQUIRING ", p_mutex->NamePtr, " WITH ", p_mutex->OwnerTCBPtr->NamePtr, " TOOK ", OS_TS_GET() - TSMutex);
#endif
        fprintf(stdout, "\n");

        return;
    }


    if (OSTCBCurPtr == p_mutex->OwnerTCBPtr)
    { /* See if current task is already the owner of the mutex  */
        p_mutex->OwnerNestingCtr++;
        if (p_ts != (CPU_TS *)0)
        {
            *p_ts = p_mutex->TS;
        }
        CPU_CRITICAL_EXIT();
        *p_err = OS_ERR_MUTEX_OWNER; /* Indicate that current task already owns the mutex      */
        return;
    }

    if ((opt & OS_OPT_PEND_NON_BLOCKING) != (OS_OPT)0)
    { /* Caller wants to block if not available?                */
        CPU_CRITICAL_EXIT();
        *p_err = OS_ERR_PEND_WOULD_BLOCK; /* No                                                     */
        return;
    }
    else
    {
        if (OSSchedLockNestingCtr > (OS_NESTING_CTR)0)
        { /* Can't pend when the scheduler is locked                */
            CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_SCHED_LOCKED;
            return;
        }
    }

    OS_CRITICAL_ENTER_CPU_CRITICAL_EXIT(); /* Lock the scheduler/re-enable interrupts                */
    p_tcb = p_mutex->OwnerTCBPtr;          /* Point to the TCB of the Mutex owner                    */
    //IMPLEMENT PCP
    if (p_tcb->Prio < OSTCBCurPtr->Prio)
    { /* See if mutex owner has a higher priority than current   */
        switch (p_tcb->TaskState)
        {
        case OS_TASK_STATE_RDY:
            OS_RdyListRemove(p_tcb);         /* Remove from ready list at current priority             */
            p_tcb->Prio = OSTCBCurPtr->Prio; /* Raise owner's priority                                 */
            OS_PrioInsert(p_tcb->Prio);
            OS_RdyListInsertHead(p_tcb); /* Insert in ready list at new priority                   */
            break;

        case OS_TASK_STATE_DLY:
        case OS_TASK_STATE_DLY_SUSPENDED:
        case OS_TASK_STATE_SUSPENDED:
            p_tcb->Prio = OSTCBCurPtr->Prio; /* Only need to raise the owner's priority                */
            break;
        default:
            OS_CRITICAL_EXIT();
            *p_err = OS_ERR_STATE_INVALID;
            return;
        }
    }

    OS_PCPPend(OSTCBCurPtr, p_mutex, timeout);

    OS_CRITICAL_EXIT_NO_SCHED();

    OSSched(); /* Find the next highest priority task ready to run       */

    CPU_CRITICAL_ENTER();
    switch (OSTCBCurPtr->PendStatus)
    {
    case OS_STATUS_PEND_OK: /* We got the mutex                                       */
        if (p_ts != (CPU_TS *)0)
        {
            *p_ts = OSTCBCurPtr->TS;
        }
        *p_err = OS_ERR_NONE;
        break;

    case OS_STATUS_PEND_ABORT: /* Indicate that we aborted                               */
        if (p_ts != (CPU_TS *)0)
        {
            *p_ts = OSTCBCurPtr->TS;
        }
        *p_err = OS_ERR_PEND_ABORT;
        break;

    case OS_STATUS_PEND_TIMEOUT: /* Indicate that we didn't get mutex within timeout       */
        if (p_ts != (CPU_TS *)0)
        {
            *p_ts = (CPU_TS)0;
        }
        *p_err = OS_ERR_TIMEOUT;
        break;

    case OS_STATUS_PEND_DEL: /* Indicate that object pended on has been deleted        */
        if (p_ts != (CPU_TS *)0)
        {
            *p_ts = OSTCBCurPtr->TS;
        }
        *p_err = OS_ERR_OBJ_DEL;
        break;

    default:
        *p_err = OS_ERR_STATUS_INVALID;
        break;
    }
    fprintf(stdout, "\n");
    CPU_CRITICAL_EXIT();
}


/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/
/*********************************/

void OSPCPMutexPost(OS_MUTEX *p_mutex,
                 OS_OPT opt,
                 OS_ERR *p_err)
{
    OS_TCB *p_tcb;
    CPU_TS ts;
    CPU_SR_ALLOC();
#if SHOW_MUTEX_OVERHEAD > 0u
    TSMutex = OS_TS_GET();    
#endif

#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0)
    {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0)
    { /* Not allowed to call from an ISR                        */
        *p_err = OS_ERR_POST_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u
    if (p_mutex == (OS_MUTEX *)0)
    { /* Validate 'p_mutex'                                     */
        *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
#endif

#if OS_CFG_OBJ_TYPE_CHK_EN > 0u
    if (p_mutex->Type != OS_OBJ_TYPE_MUTEX)
    { /* Make sure mutex was created                            */
        *p_err = OS_ERR_OBJ_TYPE;
        return;
    }
#endif

    CPU_CRITICAL_ENTER();
    if (OSTCBCurPtr != p_mutex->OwnerTCBPtr)
    { /* Make sure the mutex owner is releasing the mutex       */
        CPU_CRITICAL_EXIT();
        *p_err = OS_ERR_MUTEX_NOT_OWNER;
        return;
    }

    OS_CRITICAL_ENTER_CPU_CRITICAL_EXIT();
    ts = OS_TS_GET(); /* Get timestamp                                          */
    p_mutex->TS = ts;
    p_mutex->OwnerNestingCtr--; /* Decrement owner's nesting counter                      */
    if (p_mutex->OwnerNestingCtr > (OS_NESTING_CTR)0)
    {                       /* Are we done with all nestings?                         */
        OS_CRITICAL_EXIT(); /* No                                                     */
        *p_err = OS_ERR_MUTEX_NESTING;
        return;
    }
    
    OSTCBCurPtr->hasMutex--;

    if (p_mutex->TCBWaiting == 0)
    {                                       /* Any task waiting on mutex?                             */
#if OS_TRACE_MUTEX > 0u
        fprintf(stdout, "%s %s %s\n", p_mutex->NamePtr, " has been released (1) by ", p_mutex->OwnerTCBPtr->NamePtr);
#endif
        p_mutex->OwnerTCBPtr = (OS_TCB *)0; /* No                                                     */
        p_mutex->OwnerNestingCtr = (OS_NESTING_CTR)0;
        OS_CRITICAL_EXIT();
        *p_err = OS_ERR_NONE;
        if(p_mutex->Ceiling == StackTop->Ceiling)
            OS_StackPop();
#if SHOW_MUTEX_OVERHEAD > 0u
        fprintf(stdout, "%s %s %s %d\n", "RELEASING ", p_mutex->NamePtr, " TOOK ", OS_TS_GET() - TSMutex);
#endif
        fprintf(stdout, "\n");
        return;
    }
    /* Yes                                                    */
    if (OSTCBCurPtr->Prio != p_mutex->OwnerOriginalPrio)
    {
        OS_RdyListRemove(OSTCBCurPtr);
        OSTCBCurPtr->Prio = p_mutex->OwnerOriginalPrio; /* Lower owner's priority back to its original one        */
        OS_PrioInsert(OSTCBCurPtr->Prio);
        OS_RdyListInsertTail(OSTCBCurPtr); /* Insert owner in ready list at new priority             */
        OSPrioCur = OSTCBCurPtr->Prio;
    }

    p_tcb = OS_ChangeOwner(p_mutex);
    OS_PCPPost(p_tcb, ts);

    /********************************************/

    OS_CRITICAL_EXIT_NO_SCHED();

    if ((opt & OS_OPT_POST_NO_SCHED) == (OS_OPT)0)
    {
        OSSched(); /* Run the scheduler                                      */
    }

    *p_err = OS_ERR_NONE;
#if SHOW_MUTEX_OVERHEAD > 0u
    fprintf(stdout, "%s %s %s %s %s %d\n", "RELEASING", p_mutex->NamePtr, " TO ", p_mutex->OwnerTCBPtr->NamePtr, " TOOK ", OS_TS_GET() - TSMutex);
#endif
    fprintf(stdout, "\n");
}


OS_TCB * OS_ChangeOwner (OS_MUTEX *p_mutex)
{
    OS_TCB *p_tcb = OS_FindBlockedTask(p_mutex);
#if OS_TRACE_MUTEX > 0u
    fprintf(stdout, "%s %s %s\n", p_mutex->NamePtr, " has been released (2) by ", p_mutex->OwnerTCBPtr->NamePtr);
    fprintf(stdout, "%s %s\n", p_tcb->NamePtr, " is the new TCB");
#endif
    if(p_mutex->Ceiling == StackTop->Ceiling)
        OS_StackPop();
    p_mutex->OwnerTCBPtr = p_tcb; /* Give mutex to new owner                                */
    p_mutex->OwnerOriginalPrio = p_tcb->Prio;
    p_mutex->OwnerNestingCtr = (OS_NESTING_CTR)1;
    p_mutex->TCBWaiting--;
    p_tcb->hasMutex++;
    if (StackTop == NULL_MUTEX || StackTop->Ceiling >= p_mutex->Ceiling) //If no ceiling or ressource ceiling lower than the system ceiling
            OS_StackPush(p_mutex);

#if OS_TRACE_MUTEX > 0u
    fprintf(stdout, "%s %s %s\n", p_mutex->OwnerTCBPtr->NamePtr, " has acquired ", p_mutex->NamePtr);
#endif
    return p_tcb;
}


void OS_PCPPend(OS_TCB *p_tcb, OS_MUTEX *p_mutex, OS_TICK timeout)
{
    p_mutex->TCBWaiting++;
#if OS_TRACE_MUTEX > 0u
    fprintf(stdout, "%s %s %s\n", p_tcb->NamePtr, " has been blocked by ", p_mutex->NamePtr);
#endif
    OS_RBTinsertion(p_tcb, p_mutex);
    p_tcb->PendOn = OS_TASK_PEND_ON_MUTEX; /* Resource not available, wait until it is              */
    p_tcb->PendStatus = OS_STATUS_PEND_OK;
    OS_TaskBlock(p_tcb, /* Block the task and add it to the tick list if needed  */
                 timeout);

#if SHOW_MUTEX_OVERHEAD > 0u
    fprintf(stdout, "%s %s %s %d\n", "BLOCKING ", p_tcb->NamePtr, " TOOK ", OS_TS_GET() - TSMutex);
#endif
}


void OS_PCPPost(OS_TCB *p_tcb, CPU_TS ts)
{
    #if (OS_MSG_EN > 0u)
    p_tcb->MsgPtr = (void *)0;              /* Deposit message in OS_TCB of task waiting         */
    p_tcb->MsgSize = (OS_MSG_SIZE)0;        /* ... assuming posting a message                    */
#endif
    p_tcb->TS = ts;

    OS_RBTdeletion(p_tcb);
    OS_TaskRdy(p_tcb);                       /* Make task ready to run                            */
    p_tcb->TaskState = OS_TASK_STATE_RDY;
    p_tcb->PendStatus = OS_STATUS_PEND_OK;   /* Clear pend status                                 */
    p_tcb->PendOn = OS_TASK_PEND_ON_NOTHING; /* Indicate no longer pending                        */
}