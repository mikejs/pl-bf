A brainfuck interpreter as a PostgreSQL procedural language.

To install:

::

  make install

To use:

::

  CREATE FUNCTION pl_bf_call_handler() RETURNS language_handler AS 'pl_bf.so' LANGUAGE C;

  CREATE LANGUAGE brainfuck HANDLER pl_bf_call_handler;

  -- Add the low byte of 2 integers:
  CREATE FUNCTION bf_add(int, int) RETURNS int AS $$
  >>>>[-<<<<+>>>>]
  $$ LANGUAGE brainfuck;

  SELECT * FROM bf_add(3, 5);  -- 8!
