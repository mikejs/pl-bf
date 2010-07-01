A brainfuck interpreter as a PostgreSQL procedural language.

To build:

::

  gcc -c -I`pg_config --includedir-server` pl_bf.c
  gcc -bundle -flat_namespace -undefined suppress -o pl_bf.so pl_bf.o

To install:

::

  CREATE FUNCTION pl_bf_call_handler() RETURNS language_handler AS '/path/to/pl.so' LANGUAGE C;

  CREATE LANGUAGE brainfuck HANDLER pl_bf_call_handler;

To use:

::

  CREATE FUNCTION do_stuff() RETURNS int AS $$
  ++[->+<]+
  $$ LANGUAGE brainfuck;

  SELECT * FROM do_stuff();  -- 513, obviously