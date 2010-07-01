A brainfuck interpreter as a PostgreSQL procedural language.

To install:

::

  make install

To use:

::

  CREATE FUNCTION pl_bf_call_handler() RETURNS language_handler AS '/path/to/pl.so' LANGUAGE C;

  CREATE LANGUAGE brainfuck HANDLER pl_bf_call_handler;

  CREATE FUNCTION do_stuff() RETURNS int AS $$
  ++[->+<]+
  $$ LANGUAGE brainfuck;

  SELECT * FROM do_stuff();  -- 513, obviously