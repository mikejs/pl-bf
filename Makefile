MODULES = pl_bf

PGXS := $(shell pg_config --pgxs)
include $(PGXS)