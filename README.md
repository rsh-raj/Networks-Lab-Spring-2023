# Networks-Lab-Spring-2023
This contains the assignment of Networks Lab IIT KGP
# Assignment 6: PingNetInfo

This program takes a site address (either a name like cse.iitkgp.ac.in or an IP address like 10.3.45.6), the number of times a probe will be sent per link (n), and the time difference between any two probes (T) as input and finds the route and estimates the latency and bandwidth of each link in the path.

## Prerequisites

- A Unix-based system
- GCC compiler
- Root access

## Usage

Compile the program using the following command:
```bash
gcc pingNetInfo.c -o pingnetinfo
```
Run the program with the following command:
```bash
sudo ./pingnetinfo <site_address> <n> <T>
```

Example usage:

```bash
sudo ./pingnetinfo cse.iitkgp.ac.in 5 1
```


## Output
The program estimates the route taken to reach the target site and calculates the latency and bandwidth of each intermediate link. The output displays the route and the corresponding latency and bandwidth for each hop.It also prints out the header fields of every ICMP packet sent and received in a nice format.


# C Backend Framework (CBack)
CBack is a C library for building basic concurrent RESTful web servers with limited functionalities limited to classroom level applications.
Not recommended for high performance applications. Solely written for the purpose of attempting to 'easily' write a backend in C without sweating too much!

**Features:**
- HTTP 1.1 compatible request parser
- RESTful oriented interface
- Handler creation for endpoints
- Template rendering for endpoints
- Custom dictionary module for data handling
- Equiped with form data, query parameters, JSON parsers
- Implementation is HTTP 1.1 compliant
- Multiple threading models
- Support for cookie based authentication
- Support for CORS and CSRF

## Table of Contents
* [Introduction](#introduction)
* [Requirements](#requirements)
* [How to run](#building)
* [Getting Started](#getting-started)
* [Structures definition](#structures-and-classes-type-definition)
* [Create and work with a webserver](#create-and-work-with-a-webserver)
* [Parsing requests](#parsing-requests)
* [Building responses to requests](#building-responses-to-requests)
* [Authentication](#authentication)
* [Other Examples](#other-examples)


## Introduction
CBack is meant to constitute an easy system to build university level HTTP servers with REST fashion in C.
The mission of this library is to support all possible HTTP features directly and with a simple semantic allowing then the user to concentrate only on his application and not on HTTP request handling details. With this framework, you can enter the club of developers who wrote RESTful HTTP server in C, but by sweating a lot lesser!

CBack is able to decode certain body formats and automatically format them in custom dictionary fashion. This is true for query arguments and for *POST* and *PUT* requests bodies if *application/x-www-form-urlencoded* header are passed.

[Back to TOC](#table-of-contents)

## Requirements
To be declared

## How to run
Make a file `main.c` and import the file `#include "CBack.h"`
Create the endpoints as per the instructions. Then -
1. First run `make` to generate the executable file `server`
2. Run the executable by specifying the port number in the command line argument `./server 8080`

## Getting Started
The most basic example of creating a server and handling a requests for the path `/hello`:
```cpp
    #include <stdio.h>
    #include "CBack.h"

    char* hello(Request *req, int new_socket)
    {
        return "Hello, This is CBack!";
    }

    int main(int argc, char *argv[])
    {
        char *methods[] = {"GET", "POST"};
        int num = 2;
        add_route("/hello", &hello, methods, num);
        create_app(port);

        return 0;
    }
```
To test the above example, you could run the following command from a terminal:
    
    curl -XGET -v http://localhost:8080/hello

Here `add_route` registers the route with the methods allowed. `add_route` takes in the route path, function pointer of the handler function, an array of string of the methods allowed and the length of the array as the argument. Every handler will have `Request` struct and the client socket descriptor as the argument. In this case, the handler returns a string literal, which can be seen on the client.

[Back to TOC](#table-of-contents)


