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
 git clone git@github.com:irr/lisa.git
 cd lisa
 cmake .
 make

MySQL

::

  CREATE TABLE q(k BIGINT UNSIGNED NOT NULL AUTO_INCREMENT, 
                 d VARCHAR(<N>) NOT NULL, 
                 p INT NOT NULL, 
                 PRIMARY KEY(k)) ENGINE=INNODB;
  CREATE INDEX ip ON q(p DESC);
  
::
  
  DELIMITER //
  DROP FUNCTION IF EXISTS p//
  CREATE FUNCTION p(remove INT) 
  RETURNS VARCHAR(<N>)
  NOT DETERMINISTIC
  MODIFIES SQL DATA
  BEGIN 
    DECLARE data VARCHAR(<N>);
    DECLARE rowid BIGINT;
    SELECT k, d INTO rowid, data FROM q ORDER BY p DESC,k LIMIT 1 FOR UPDATE; 
    IF rowid > 0 THEN
      IF remove > 0 THEN 
        DELETE FROM q WHERE k = rowid; 
      END IF; 
    END IF;
    RETURN data;
  END//
  DELIMITER ;

::

  [mysqld]
  init-connect          = 'SET AUTOCOMMIT=0'
  transaction-isolation = READ-COMMITTED

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

  ./lisa --help

::

  LISA 1.0 beta (http://github.com/irr/lisa)
  This is free software, and you are welcome to redistribute it and/or modify
  it under the terms of the Boost Software License - Version 1.0.
  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
  LISA comes with ABSOLUTELY NO WARRANTY.

  Allowed Options:
    -h [ --help ]                                               help message
    -d [ --database ] arg (=db=<db> user=<user> password=<pwd>) dsn 
                                                                (mandatory)
    -a [ --address ] arg (=0.0.0.0)                             interface 
                                                                (optional)
    -p [ --port ] arg (=1972)                                   port [1,65535] 
                                                                (optional)
    -t [ --threads ] arg (=42)                                  threads [1,100] 
                                                                (optional)

  samples: ./lisa -d "db=lisa user=root password=irr" or 
           ./lisa -d "db=lisa user=root password=irr" -a localhost
           ./lisa -d "db=lisa user=root password=irr" -a 127.0.0.1 -p 1972 -t 10

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

  < HTTP/1.0 200 OK
  < Server: Lisa 1.0
  < Content-Length: 0
  < Content-Type: text/plain
  
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
  
=====
Tests
=====

jmeter [10.000 concurrent (en/de)queues using 100 threads]

::

  curl http://localhost:1972/size (*)
  jmeter -n -t lisa.jmx -p lisa.properties
  (data.csv will be generated) (**)

:: 
  
  Creating summariser <summary>
  Created the tree successfully using lisa.jmx
  Starting the test @ Fri Jan 22 22:35:13 BRST 2010 (1264206913338)
  Waiting for possible shutdown message on port 4445
  summary + 16975 in  46.2s =  367.0/s Avg:   267 Min:    50 Max:  1238 Err:     0 (0.00%)
  summary +  3025 in   7.4s =  410.2/s Avg:   220 Min:    68 Max:   355 Err:     0 (0.00%)
  summary = 20000 in  53.3s =  375.2/s Avg:   260 Min:    50 Max:  1238 Err:     0 (0.00%)
  Tidying up ...    @ Fri Jan 22 22:36:07 BRST 2010 (1264206967076)
  ... end of run

::

  curl http://localhost:1972/size (must match with (*))

========
Analysis
========

Awk

::

  awk -F "," '{print $2}' data.csv > data.dat (**)
  
R

::

  > d <- read.table("data.dat")
  > v <- as.vector(d$V1)
  > summary(v)
     Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
      0.0    54.0   209.0   191.3   255.0  1238.0 
  > sd(v)
  [1] 138.7683
