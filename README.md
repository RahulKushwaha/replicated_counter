# replicated_counter
[![CMake](https://github.com/RahulKushwaha/replicated_counter/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/RahulKushwaha/replicated_counter/actions/workflows/cmake.yml)


Directory Structure: 

/applications

Applications made on top of the replicated log. 

* [/applications/counter](https://github.com/RahulKushwaha/replicated_counter/tree/main/applications/counter)
  * It is a simple in-memory application which stores counter values for a given key. We can increment/decrement a bunch of keys atomically. 
* [/applications/mydb](https://github.com/RahulKushwaha/replicated_counter/tree/main/applications/mydb)
  * Database 
* [/common](https://github.com/RahulKushwaha/replicated_counter/tree/main/common/)
* [/configs](https://github.com/RahulKushwaha/replicated_counter/tree/main/configs)
* [/docker](https://github.com/RahulKushwaha/replicated_counter/tree/main/docker)
* [/log](https://github.com/RahulKushwaha/replicated_counter/tree/main/log)
  * Log contains an implementation of multi-paxos with dynamic reconfiguration.
* [/persistence](https://github.com/RahulKushwaha/replicated_counter/tree/main/persistence)
  * wrapper around rocksdb
* [/statemachine](https://github.com/RahulKushwaha/replicated_counter/tree/main/statemachine)
  * Abstractions for State Machine Replication. These abstractions are used by app and log. 
* [/wor](https://github.com/RahulKushwaha/replicated_counter/tree/main/wor)
  * Write Once Register
  * Paxos configuration is stored itself in a replicated LinkedList of Write Once Registers. Each register contains are pointer to the next configuration. Configuration is replicated using Single Decree Paxos.