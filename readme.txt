PROJET:

* Skiplist implementation based on https://www.youtube.com/watch?v=Dx7Hk8-8Kdw
* For the periodic tasks and because we are implementing RM, we chose to have period value = priority value for our TCB in task creation.
* Every time a task is added in the ready list, it is also reinsterted into the skip list with TickRemain = Period = Priority PRIO ONLY GO TO 255
-> PRIO RAISED TO CPU_INT16U, PRIO IS PERIOD, WITH 10HZ TICKS. EX : 27s period : PRIO  = 270.
* Problematic if one TCB is in both Tick Wheel and Ready List?
* Added NextTCB element in the OS_TCB structure, initialiased to (OS_TCB*)0 in OS_TaskInitTCB (called in OSTaskCreate). This gives us the next neighbour on every skip list level
* Periodic API : The TCB passed in parameter of the functions needs to be allocated with OSTaskCreate prior.
* Deletion of an element : Done searching for the task in the skip list, through it's name.
* Tick Rate to 10Hz (OS_CFG_APP)
* Functions defined in os.h line 2060
NEED TO MODIFY TICK TASK RATE


APPTASKSTART
-> OSTASKCREATE THEN ADD¨PERIODICTASK
-> INSERTELEMENT, TASK ADDED TO THE SKIP LIST WITH AEQUAT TICKCTRMATCH

EVERY TICK
-> TICK UPDATE, UPDATE NUMBER OF TICKREMAIN
WHEN TICKCTRMATCH IS MATCHED
-> OS_TASKRDY
-> DELETEELEMENT, ADD ELEM TO READY LIST, ELEM INSTERTED BACK IN THE SKIP LIST WITH TICKCTRMATCH = OSTICKCTR + PRIO

WHEN TASK IS EXECUTED:
EXEC
WHEN END OF EXEC CODE, ADD CODE TO RESET THE STACK AND SET TASK STATE TO OS_TASK_STATE_DLY.


WATCH WHAT IS DONE IN OSTASKCREATE, ESP READY LIST INSERTION


IDEE : IN THE MEANTIME, REMPLACER READY LIST PAR LIST SIMPLE, QUAND ON SELECTIONNE LA TACHE, SIGNAL JUSTE LA TACHE