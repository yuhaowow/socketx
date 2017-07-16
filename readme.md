

`TODO: Refactor the library`

# socketx

socketx is a library for wrapping linux APIs for socket programming. It is writen by C/C++ and aims to provide C++ interfaces for users. This library is originally used for some simple projects of myself when I was learning networking programming. You can find more details in some of [Examples](##Examples).

## Features
- c++ APIs for socket programming in Linux
- Provide thread pool
- Thread safe data structures
- Support robust writing and reading

## Examples
- [echo](./examples/echo/)
- [multi-echo](./examples/multi-echo/)
- [Minerx](https://github.com/fancyqlx/Minerx)

## Quickstart

### Create sockets for clients or servers
The sockets for clients and servers are regarded as object in socketx. It encapsulates connect, bind, listen and accept, all of these processes are transparent for users.
- Create a client
```C++
socketx::client_socket client
```
- Connect to a host
```C++
client.connect_to(host,port)
```
- Create a server
```C++
socketx::server_socket server
```
- Listen to a port
```C++
server.listen_to(port)
```
- Accept a connection from port
```C++
server.accept_from()
```

### Communication between hosts
socketx provides robust writing and reading between hosts. The communication process was managed by an object, which encapsulates all the function used for communication.
- Create a communication object
```C++
socketx::communication comm
```
- Create connection with a file descriptor
```C++
comm.communication_init(fd)
```
- Communication
```C++
comm.send(fd,buffer,size)
comm.recv(buffer,size)
comm.readline(buffer,size)
comm.sendmsg(fd,&msg)
comm.recv()
```


### Thread pool
Thread pool is an important feature for socketx. In order for the completeness, socketx also provides some thread safe structures. 
- Create a thread pool
```C++
socketx::thread_pool pool;
socketx::thread_pool pool(4);
```
- Submit a task
```C++
pool.submit(std::bind(task,args));
```