Time-Critical Computing Project:

Phase 1 :
* Create an API to make tasks periodic;
* Replace the data structure implemented in the OS for the Tick Wheel (Hash Table) by a Skip List.

--> OS_RecTaskServices files

Phase 2 :
* Implement PCP instead of PIP (which is the resource sharing protocol implemented in Micrium)
* Use a Red black Tree instead of Linked List for Blocked Tasks

--> OS_MutexPhase2 files