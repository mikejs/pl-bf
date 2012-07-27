A brainfuck interpreter as a PostgreSQL procedural language.

To install:

::

  make install

To use in PostgreSQL 9.1 and later:

::

  CREATE EXTENSION plbf;

  -- Add the low byte of 2 integers:
  CREATE FUNCTION bf_add(int, int) RETURNS int AS $$
  >>>>[-<<<<+>>>>]
  $$ LANGUAGE plbf;

  SELECT * FROM bf_add(3, 5);  -- 8!

Older versions:

::

  CREATE FUNCTION plbf_call_handler() RETURNS language_handler AS 'plbf.so' LANGUAGE C;

  CREATE LANGUAGE plbf HANDLER plbf_call_handler;

  -- Add the low byte of 2 integers:
  CREATE FUNCTION bf_add(int, int) RETURNS int AS $$
  >>>>[-<<<<+>>>>]
  $$ LANGUAGE plbf;

  SELECT * FROM bf_add(3, 5);  -- 8!
