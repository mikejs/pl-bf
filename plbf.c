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

PG_FUNCTION_INFO_V1(plbf_call_handler);

int interpret(char*, int, char*);

size_t append_datum(char* buf, Datum val, bool isnull, Oid typeoid)
{
    HeapTuple typeTup;
    Form_pg_type typeStruct;
    FmgrInfo tmp_flinfo;
    char *str;
    size_t len;

    typeTup = SearchSysCache(TYPEOID, ObjectIdGetDatum(typeoid), 0, 0, 0);
    if (!HeapTupleIsValid(typeTup)) {
        elog(ERROR, "Cache lookup failed for %u", typeoid);
    }
    typeStruct = (Form_pg_type)GETSTRUCT(typeTup);

    if (typeStruct->typtype != 'b') {
        // Non-basic type
        elog(ERROR, "Don't support non-basic types (%s)",
             format_type_be(typeoid));
    }

    fmgr_info_cxt(typeStruct->typoutput, &tmp_flinfo, CurTransactionContext);

    ReleaseSysCache(typeTup);
    
    if (!isnull) {
        if (typeoid == INT4OID) {
            *((int*)buf) = DatumGetInt32(val);
            return 4;
        }
        
        SPI_push();
        str = OutputFunctionCall(&tmp_flinfo, val);
        SPI_pop();
        len = strlen(str);
        strncpy(buf, str, len);
        return len;
    }

    return 0;
}

Datum plbf_call_handler(PG_FUNCTION_ARGS)
{
    FmgrInfo flinfo;
    Datum retval, proc_source_datum;
    Form_pg_proc procStruct;
    Form_pg_type typeStruct;
    HeapTuple procTup;
    HeapTuple typeTup;
    bool isnull;
    Oid resultTypeIOParam, returnTypeOID;
    char *proc_source;
    char *a, *p;
    int i;

    // Get the function tuple
    procTup = SearchSysCache(PROCOID, ObjectIdGetDatum(fcinfo->flinfo->fn_oid),
                             0, 0, 0);
    if (!HeapTupleIsValid(procTup)) {
        elog(ERROR, "Cache lookup failed for procedure %u",
             fcinfo->flinfo->fn_oid);
    }
    procStruct = (Form_pg_proc)GETSTRUCT(procTup);
    
    // Get the function source
    proc_source_datum = SysCacheGetAttr(PROCOID, procTup,
                                        Anum_pg_proc_prosrc, &isnull);
    if (isnull) {
        elog(ERROR, "Function source is null");
    }
    proc_source = DatumGetCString(DirectFunctionCall1(textout,
                                                      proc_source_datum));

    // Get the return type tuple
    typeTup = SearchSysCache(TYPEOID, ObjectIdGetDatum(procStruct->prorettype),
                             0, 0, 0);
    if (!HeapTupleIsValid(typeTup)) {
        elog(ERROR, "Cache lookup failed for type %u", procStruct->prorettype);
    }
    typeStruct = (Form_pg_type)GETSTRUCT(typeTup);
    
    resultTypeIOParam = getTypeIOParam(typeTup);
    returnTypeOID = procStruct -> prorettype;

    a = calloc(5000, 1);
    if (!a) {
        elog(ERROR, "BAD A!");
    }

    p = a;
    for (i = 0; i < procStruct->pronargs; i++) {
        p += append_datum(p, fcinfo->arg[i], fcinfo->argnull[i],
                          procStruct->proargtypes.values[i]);
    }
    
    interpret(a, 0, proc_source);

    fmgr_info_cxt(typeStruct->typinput, &flinfo, TopMemoryContext);

    if (returnTypeOID != VOIDOID) {
        if (returnTypeOID == INT4OID) {
            retval = Int32GetDatum(*((int*)a));
        } else {
            SPI_push();
            if (returnTypeOID == BOOLOID)
                retval = InputFunctionCall(&flinfo, a[0] ? "TRUE" : "FALSE",
                                           resultTypeIOParam, -1);
            else {
                retval = InputFunctionCall(&flinfo, pstrdup(a),
                                           resultTypeIOParam, -1);
            }
            SPI_pop();
        }
    }

    ReleaseSysCache(procTup);
    ReleaseSysCache(typeTup);
    free(a);

    SPI_finish();
    
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
