</br>

# 2-Phase-Commit-Protocol-Pattern

Two-phase commit protocol (2PC) is a type of atomic commitment protocol (ACP). It coordinates all the processes that participate in a distributed atomic transaction on whether to commit or abort (rollback) the transaction. It is a specialized type of consensus protocol and it is implemented in two phases - <i>Prepare</i> & <i>Commit</i> achieved through 2 types of server nodes - <i>Coordinator</i> & <i>Participants</i>. This protocol also follows the ACID (Atomicity, Consistency, Isolation & Durability) rules which is used in distributed transactions.

</br>

We have implemented the Two-Phase Commit Transactions using on multiple databases which are File Class Objects (2D String Array) as Participants at a thread level for the use-case - Amazon’s Payment & Shipping. The language of implementation is C++20.

It contains 3 Databases:
* Customer/User
* Payment
* Shipping

5 high-level operations can be performed:
* Register User
* Get User Details
* Get Transaction Details
* Update User Details
* Make Payment

The above 5 operations use a combination of File Operations in the Two-Phase Commit Protocol manner at a threaded level. Since we are using File Class Objects as Databases, the following File Operations can be formed:
* Add Record
* Delete Record
* Update Record/Cell
* Read Record/Cell

</br>

## <b> Steps of Excecution </b>

</br>

To execute Amazon client:
```
make Amazon_client
./client.o
```

</br>

To execute a simple Amazon testcase:
```
make Amazon_testcase
./check_testcase.o
```

</br>

To clean .o files:
```
make clean
```

</br>

## <b> Two-Phase Commit Protocol </b>

Two-Phase Commit Protocol consists of 2 types of Server Nodes:
* <i>Coordinator</i>: Single Leader Node that decides whether to Commit or Rollback Transactions
* <i>Participants/Worker</i>: Multiple Distributed Database Nodes

Two-Phase Commit Protocol is implemented in 2 Phases:

* <i>Prepare/Voting Phase:</i>
    * Coordinator sends the request to Prepare message to all Participants
    * All Participants Prepare up to the Transaction and sends back reply message (either Agreement(yes) or Abort(no) votes) to the Coordinator

* <i>Commit/Completion Phase:</i>
    * Coordinator can receive either Agreement or Abort from each Participant.
    * If the Coordinator received an agreement message from all the Participants, it is Ready:
        * Coordinator sends the Commit message to all the Participants
        * All Participants commits transactions & sends an acknowledgement to the coordinator
    * If the coordinator received an abort message from anyone participant, it has Failed:
        * Coordinator sends a Rollback message to all the Participants
        * All Participants undo the transaction & sends an acknowledgement to the Coordinator

<img src="assets/phase_commit.PNG" height="50%" width="50%">

</br>

## <b> Class Diagram </b>

<img src="assets/class_diagram.PNG" height="50%" width="50%">

</br>
</br>

## <b> GOF Patterns Used </b>

### <b>Mediator pattern</b>

<i>General Intent: To reduce the complexity and dependencies between tightly coupled objects communicating directly with one another.</i>

‘Coordinator node’ and ‘Worker node’ are the communicating objects. The interaction between the Coordinator and Worker node is a very crucial part of the 2 phase protocol. The Coordinator node communicates with the Worker nodes regarding the actions that the Worker nodes have to perform on their respective Files. The Worker nodes have to inform the Coordinator nodes regarding the failure or success of its operations on its Files. However tight coupling between the interaction of these nodes will require constant modification of one file in case of modification of the interface of the other file. Hence, the mediator pattern helps to reduce coupling between these objects.

### <b>Memento pattern</b>

<i>General Intent: To save and restore the previous state of an object without revealing the details of its implementation.</i>

‘Worker node’ performs file operations on the File objects using interfaces provided by DBFile. In cases of failure of ‘Worker nodes’, the File objects have to be rolled back to their previous state. Therefore, the ‘Worker node’ stores the current state of ‘File objects’ in the form of logs before performing any update operations on them. However, if the implementation of ‘Worker nodes’ depends on the structure of the state of ‘File objects’, constant modification has to be made in the way logs are stored and processed whenever the structure of ‘File objects’ are changed. Therefore, the ‘File object’ state remains the same and hidden (‘ void\* ’ in our case). Any rollbacks to the operations will require the ‘Worker node’ to pass the previous state (void\*) to the DBFile object which will take care of the rollback operation.

##

</br>

### Advantages of Two-Phase Commit Protocol
* Ensures strong consistency
* Guarantees atomic transactions
* Allows read-write isolation

### Disadvantages of Two-Phase Commit Protocol
* 2PC is a Blocking protocol hence it ends up delaying progress in the long run & does not utilize resources properly during this time (This can be overcome by implementing timeouts).

</br>



