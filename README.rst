LiteSQL - C++ Object-Relational Persistence Framework
-----------------------------------------------------

LiteSQL is a C++ library that integrates C++ objects tightly to relational
database and thus provides an object persistence layer. LiteSQL supports
SQLite3, PostgreSQL and MySQL as backends. LiteSQL creates tables, 
indexes and sequences to database and upgrades schema when needed. 

In addition to object persistence, LiteSQL provides object relations which
can be used to model any kind of C++ data structures. Objects can be selected,
filtered and ordered using template- and class-based API with type 
checking at compile time.

See HTML documentation for details docs/html/index.html.

Project home page: https://lab.louiz.org/louiz/litesql


Authors
-------

Project initiated by Tero Laitinen <tero.laitinen@iki.fi>

Fork maintained by Florent Le Coz <louiz@louiz.org>


Installation
------------

    mkdir build
    cd build
    cmake ..
    make


License
-------

LiteSQL is Free Software.
(learn more: http://www.gnu.org/philosophy/free-sw.html)

LiteSQL is released under the BSD 3-Clause license.
Please read the LICENSE file for details.
