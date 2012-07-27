EXTENSION = plbf
MODULES   = plbf
DATA      = plbf--0.1.sql

PGXS := $(shell pg_config --pgxs)
include $(PGXS)
