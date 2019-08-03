#ifndef LIBPAS_TSV_H
#define LIBPAS_TSV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define TSV_SKIP_LINE(S) while (1) {int x = fgetc(S);if (x == EOF || x == '\n') { break; }    }

typedef struct {
    char *buf;
    int buf_length;
    char **column_name;
    int column_name_length;
    char **data;
    int data_length;
    int null_returned;
} TSVresult;

extern int TSVinit(TSVresult **r, const char *path);

extern int TSVntuples(TSVresult *r) ;

extern char * TSVgetvalue(TSVresult *r, int row_number, int column_number);

extern char * TSVgetvalues(TSVresult *r, int row_number, const char * column_name) ;

extern int TSVgetis(TSVresult *r, int row_number, const char * column_name) ;

extern double TSVgetfs(TSVresult *r, int row_number, const char * column_name) ;

extern int TSVgeti(TSVresult *r, int row_number, int column_number) ;

extern double TSVgetf(TSVresult *r, int row_number, int column_number) ;

extern char *TSVgetvalueById ( TSVresult *r, const char *id_column_name, const char *value_column_name, const char *id ) ;

extern int TSVgetiById ( TSVresult *r, const char *id_column_name, const char *value_column_name, const char *id );

extern double TSVgetfById ( TSVresult *r, const char *id_column_name, const char *value_column_name, const char *id ) ;

extern int TSVnullreturned(TSVresult *r);

extern void TSVclear(TSVresult *r) ;
    


#endif 

