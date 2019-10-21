# Ticket

Test assignment for Lockwood Publishing. Demonstrats a small service with an API that can keep track of requests for ticket ids.


## Build

This project is written in C++ and was built and tested with MinGW-W64 GCC 8.1.0 and associated build tools (particularly GNUMake) under Windows.
Building on another platform may require changes to LIBS, paths and command line arguments in GNUmakefile.

Compile the service:
```
make
```

Compile and run the service (using ports 8080/8081 defined in the GNUmakefile):
```
make test
```

Clean up working directory:
```
make clean
```


## Usage

The service runs in a daemon like manner and provides two local TCP ports for interaction.
This was intended to show that multiple instances can be started with each receiving messages on the defined ports, representing message passing between other parts of a distributed system.

To start the service:
```
ticket <public-port> <internal-port>
```

#### public-port
Accepts messages from a user where the connection would be authenticated elsewhere.

#### internal-port
Accepts messages from an internal source and would not be authenticated.


In order to test the service, use one or more terminal clients (eg. `netcat` or `putty`) to connect to `localhost <desired-port>`.
Messages are sent as text followed by a newline. The following lists the available messages on each of the ports:

### Public connection messages
All of these messages require a `<user-name>` that is a stand in for authentication.

#### `new_ticket <user-name>`
Creates and returns a newly allocated ticket.

#### `is_valid <user-name> <ticket-id>`
Returns if the given <ticket-id> is currently allocated to the specified user.

### Internal connection messages
These messages are simpler since they are not authenticated.

#### `tickets_issued`
Returns the number of currently valid tickets that have been issued.

#### `invalidate_ticket <ticket-id>`
Invalidates the given <ticket-id>.

#### `stop`
Shuts the instance of the service handling this connection down.


## Source file breakdown

### main.cc
Main entry point to setup the service and contains the parsing of the messages.
Not intended to be part of the exercise, just the glue to run the service.

### database.h
Contains a stand-in emulating a real database that just holds currently issued ticket data.
Having a single database with blocking operations would be a main point of contention, harming the scalability of the service.

### task_queue.h
### tcp_connection.h
These are not part of the exercise and are representing a framework that can send/receive messages that will be processed concurrently in a distributed manner.
`task_queue` uses a pool of threads to deliver messages to the API and this, along with having multiple distributed instances of the service, represents a framework that should lead to scalability.
Message passing should be asynchronus where possible and the number of threads should be bounded (to a multiple of the CPU cores as a suggestion) or scaling will be impacted (the overhead would cost more to have unbounded threads of execution).

### message.h
The message data being passed to the API. In a real solution this would be serialised/deserialised.

### ticket.h
The API itself. Each public method is an entry point for a specific message and operates asynchronously to the other methods.


## Note

This example does not include extensive error checking. A real world service would.
