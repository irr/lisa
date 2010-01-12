====
LISA
====

About
-----
LISA is a minimal service provider built over asynchronous IO (Boost.Asio) and HTTP protocol which uses as primary service a priority-queue implementation using MySQL as persistence backend.

Dependencies
------------
- Boost (C++ Libraries)
- SOCI (The C++ Database Access Library)
- Log4cpp (Log for C++)
- MySQL (MySQL database client development files)
- CMake (Cross-platform open-source build system)

Author
------
Ivan Ribeiro Rocha <ivan.ribeiro@gmail.com> 
(LISA code and HTTP-Server enhancements and patches)

Credits
-------
Thanks to Christopher M. Kohlhoff for HTTP-Server sample code (Boost.Asio)

Licenses
--------
Boost Software License Version 1.0 (see accompanying file LICENSE_1.0.txt)

=====
Setup
=====

C++ Environment

::

 sudo apt-get install cmake libboost1.40-all-dev libsoci-core-gcc-dev libsoci-mysql-gcc 
                            libmysqlclient15-dev liblog4cpp5 liblog4cpp5-dev
 git clone git://github.com/irr/lisa.git
 cd lisa
 cmake .
 make

MySQL

::

  CREATE TABLE q(k SERIAL, d VARCHAR(<N>) NOT NULL, p INT NOT NULL, PRIMARY KEY(k));
  CREATE INDEX ip ON q(p DESC);

======
Syntax
======

Queue item

::

  curl http://<server:port>/<priority=0(default)> -d "d=<data>"
  
Dequeue/check item

::

  curl http://<server:port>/[spy]
  
Query size/count

::

  curl http://<server:port>/<size|count>
  
==========
Running
==========

Start server

::

  ./lisa -d "db=lisa user=root password=test" -a localhost
  
Queue items

::

  curl -v http://localhost:1972/10 -d "d=lara"
  (queue "lara" with priority 10)
  
::

  curl -v http://localhost:1972/10 -d "d=lara"
  > POST /10 HTTP/1.1
  > Host: localhost:1972
  > Content-Length: 6
  > Content-Type: application/x-www-form-urlencoded
  > 

  < HTTP/1.0 200 OK
  < Server: Lisa 1.0
  < Content-Length: 0
  < Content-Type: text/plain
  
::

  curl -v http://localhost:1972/99 -d "d=luma"
  (queue "luma" with priority 99)
  
::

  curl -v http://localhost:1972/99 -d "d=luma"
  > POST /10 HTTP/1.1
  > Host: localhost:1972
  > Content-Length: 6
  > Content-Type: application/x-www-form-urlencoded
  > 

  < HTTP/1.0 200 OK
  < Server: Lisa 1.0
  < Content-Length: 0
  < Content-Type: text/plain
  < 
  
Query item

::

  curl -v http://localhost:1972/spy
  (query next item on queue)

::

  curl -v http://localhost:1972/spy
  > GET /spy HTTP/1.1
  > Host: localhost:1972

  < HTTP/1.0 200 OK
  < Server: Lisa 1.0
  < Content-Length: 4
  < Content-Type: text/plain
  luma
  
Query size/count
 
::

  curl -v http://localhost:1972/size
  (check queue size)
  
::

  curl -v http://localhost:1972/size
  > GET /size HTTP/1.1
  > Host: localhost:1972

  < HTTP/1.0 200 OK
  < Server: Lisa 1.0
  < Content-Length: 1
  < Content-Type: text/plain
  2
  
Dequeue item

::

  curl -v http://localhost:1972
  (deque item)
  
::

  curl -v http://localhost:1972/
  > GET / HTTP/1.1
  > Host: localhost:1972
   
  < HTTP/1.0 200 OK
  < Server: Lisa 1.0
  < Content-Length: 4
  < Content-Type: text/plain
  luma
  
