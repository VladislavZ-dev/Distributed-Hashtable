# Distributed-Hashtable

This is a project focused on creating a hashtable which stores its values in a server, which can then be accessed by multiple clients simultaneously. 

## Topics explored
- Distributed systems
- Communication through TCP
- Serialization
- Concurrency

# Compiling and Executing the project

In order to compile the project, insert the following commands:
- `cd Distributed-Hashtable/`
- `make`

In order to execute the project, run the following commands:
- `cd Distributed-Hashtable/`
- `./binary/server_hashtable <port> <number_lists>`


In another terminal,
- `./binary/client_hashtable <server_IP>:<port>`

