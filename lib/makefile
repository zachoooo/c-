# Recursive portion
SUBDIRS = ourgetopt symbolTable yyerror emitcode

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

# End Recursive Portion

.PHONY: default
default: subdirs

.PHONY: debug
debug: subdirs

.PHONY: optimized
optimized: subdirs

.PHONY: all
all: subdirs