MODULES = plbf

PGXS := $(shell pg_config --pgxs)
include $(PGXS)
