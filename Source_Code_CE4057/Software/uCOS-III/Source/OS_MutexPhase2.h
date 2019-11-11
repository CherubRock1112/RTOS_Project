#include <os.h>
#include <stdio.h>

#define COUNT 10
#define MAX_HEIGHT 10


void OS_RBTinsertion(OS_TCB *newElem, OS_MUTEX *mutex);
void OS_RBTdeletion(OS_TCB *oldElem);
void OS_RBT_init (void);
int OS_RBTsearchTCB(OS_TCB *p_tcb);
void OS_RBTprint2D(void);
void print2DUtil(OS_TCB *root, int space);

void FindBlockedTask(OS_TCB *root, OS_MUTEX *p_mutex);
OS_TCB *OS_FindBlockedTask(OS_MUTEX *p_mutex);

void OS_Stack_init(void);
void OS_StackPush(OS_MUTEX *p_mutex);
void OS_StackPop();

void  OSPCPMutexCreate (OS_MUTEX    *p_mutex,
                     CPU_CHAR    *p_name,
                     OS_ERR      *p_err,
                     OS_TCB      *p_tcb);
void OSPCPMutexPend(OS_MUTEX *p_mutex,
                 OS_TICK timeout,
                 OS_OPT opt,
                 CPU_TS *p_ts,
                 OS_ERR *p_err);
void OSPCPMutexPost(OS_MUTEX *p_mutex,
                 OS_OPT opt,
                 OS_ERR *p_err);

OS_TCB * OS_ChangeOwner (OS_MUTEX *p_mutex);

void OS_PCPPost(OS_TCB *p_tcb, CPU_TS ts);

void OS_PCPPend(OS_TCB *p_tcb, OS_MUTEX *p_mutex, OS_TICK timeout);

