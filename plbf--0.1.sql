CREATE FUNCTION plbf_call_handler() RETURNS language_handler AS '$libdir/plbf.so' LANGUAGE C;

CREATE LANGUAGE plbf HANDLER plbf_call_handler;
