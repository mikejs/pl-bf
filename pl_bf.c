#include <stdio.h>
#include "postgres.h"
#include "executor/spi.h"
#include "commands/trigger.h"
#include "fmgr.h"
#include "access/heapam.h"
#include "utils/syscache.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_type.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pl_bf_call_handler);

int interpret(char*, int, char*);

Datum pl_bf_call_handler(PG_FUNCTION_ARGS)
{
    Datum retval, proc_source_datum;
    Form_pg_proc procStruct;
    HeapTuple procTup;
    bool isnull;
    char *proc_source;
    char *a;

    procTup = SearchSysCache(PROCOID, ObjectIdGetDatum(fcinfo->flinfo->fn_oid),
                             0, 0, 0);

    if (!HeapTupleIsValid(procTup)) {
        elog(ERROR, "Cache lookup failed for procedure %u",
             fcinfo->flinfo->fn_oid);
    }

    procStruct = (Form_pg_proc)GETSTRUCT(procTup);

    ReleaseSysCache(procTup);

    proc_source_datum = SysCacheGetAttr(PROCOID, procTup,
                                        Anum_pg_proc_prosrc, &isnull);

    if (isnull) {
        elog(ERROR, "Function source is null");
    }

    proc_source = DatumGetCString(DirectFunctionCall1(textout,
                                                      proc_source_datum));

    a = calloc(5000, 1);
    
    interpret(a, 0, proc_source);

    retval = Int32GetDatum(*((int*)a));

    free(a);
    
    return retval;
}

int interpret(char *a, int p, char *c)
{
    char *d;
    char b, o;

    while(*c) {
        switch(o=1,*c++) {
        case '<': p--;        break;
        case '>': p++;        break;
        case '+': a[p]++;     break;
        case '-': a[p]--;     break;
        case '.': putchar(a[p]); fflush(stdout); break;
        case ',': a[p]=getchar();fflush(stdout); break;
        case '[':
            for( b=1,d=c; b && *c; c++ )
                b+=*c=='[', b-=*c==']';
            if(!b) {
                c[-1]=0;
                while( a[p] )
                    p = interpret(a, p, d);
                c[-1]=']';
                break;
            }
        case ']':
            puts("UNBALANCED BRACKETS"), exit(0);
        default: o=0;
        }
        if( p<0 || p>100)
            puts("RANGE ERROR"), exit(0);
    }
    return p;
}
