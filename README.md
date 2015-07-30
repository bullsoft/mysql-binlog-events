The MySQL Binlog Events Library is a C++ library for reading MySQL
replication events, either by connecting to a server or by reading
from a file. To handle reading from a server, it includes a very
simple client.


Dependencies
------------

You need to have CMake version 2.8 or later and MySQL-5.7.x .

Now to get the libmysqlclient and the necessary include files you will have to
do this:

1. Download the source code of mysql-5.7.x (http://dev.mysql.com/downloads/mysql/)
   from the tab Development Releases.

2. Download the binaries of the same version. There are few header files which
   are needed for mysql-binlog compilation and they only come as part of the
   source file. To make these files available during compilation please set the
   environment variable
   MYSQL_SOURCE_INCLUDE_DIR=<mysql-5.7.x source code>/include.

To be able to run the unit tests, you have to have Google Test
installed. Google Test will be automatically installed if cmake is
called as:

   cmake . -DENABLE_DOWNLOADS=1


Directory structure
-------------------

    .
    |-- bindings            Files for transport
    |   |-- include         Include files
    |   |-- src             Source files for library
    |-- examples            Examples
    |   |-- binlog-browser  Example application to browse the binary log
    |-- libbinlogevents     Files to decode binlog events
    |   |-- include         Include files
    |   |-- src             Source files for library
    |-- tests               Unit test files and directories


Building
--------

To build the entire package, it is first necessary to run CMake to build
all the makefiles. Before running CMake set this environment variable MYSQL_DIR
to point to the MySQL binaries and then you can figure out where to find the
right files in the cmake code.

We are using the statically linked version of libmysqlclient, so we need to
pass -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE along with the cmake command

        export MYSQL_DIR=<path of mysql directory or libmysql>
        cmake . -DMYSQLCLIENT_STATIC_LINKING:BOOL=TRUE
        make -j4

Some of the examples are using third-party software, which can require
extra parameters to be given to CMake.

If you want to perform an out-of-source build, you can just create a
build directory and execute CMake there.

      mkdir build
      cd build
      cmake <source directory>
      make -j4

