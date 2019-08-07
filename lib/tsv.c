#include "tsv.h"

static int getBufLength ( FILE* stream ) {
    rewind ( stream );
    int out = 0;
    while ( 1 ) {
        int c = fgetc ( stream );
        if ( c == EOF ) {
            break;
        }
        out++;
    }
    return out;
}

static int getColumnNameLength ( FILE* stream ) {
    rewind ( stream );
    int out = 0;
    int last_char = TSV_DELIMITER_COLUMN;
    while ( 1 ) {
        int c = fgetc ( stream );
        if ( c == EOF || c == TSV_DELIMITER_ROW) {
            break;
        }
        if ( c != TSV_DELIMITER_COLUMN && c != TSV_DELIMITER_ROW && last_char == TSV_DELIMITER_COLUMN) {
            out++;
        }
        last_char = c;
    }
    return out;
}

static int getDataLength ( FILE* stream ) {
    rewind ( stream );
    TSV_SKIP_LINE ( stream )
    int out = 0;
    int last_char = TSV_DELIMITER_COLUMN;
    while ( 1 ) {
        int c = fgetc ( stream );
        if ( c == EOF ) {
            break;
        }
        if ( c != TSV_DELIMITER_COLUMN && c != TSV_DELIMITER_ROW && (last_char == TSV_DELIMITER_COLUMN || last_char == TSV_DELIMITER_ROW)) {
            out++;
        }
        last_char = c;
    }
    return out;
}

static int getColumnIndex ( TSVresult *r, const char * column_name ) {
    for ( int i = 0; i < r->column_name_length; i++ ) {
        if ( strcmp ( column_name, r->column_name[i] ) == 0 ) {
            return i;
        }
    }
    return -1;
}

static void parse ( TSVresult *r, FILE* stream ) {
    rewind ( stream );
    int state = 0;
    int i = 0;
    int j = 0;
    int last_char = TSV_DELIMITER_COLUMN;
    while ( 1 ) {
        int c = fgetc ( stream );
        if ( c == EOF ) {
            break;
        }
        if ( i >= r->buf_length ) {
            return;
        }
        if ( c == TSV_DELIMITER_ROW || c == TSV_DELIMITER_COLUMN ) {
            r->buf[i] = '\0';
        } else {
            r->buf[i] = c;
        }

        switch ( state ) {
        case 0://header
            if ( c == TSV_DELIMITER_ROW ) {
                j = 0;
                state = 1;
                break;
            }
            if ( c != TSV_DELIMITER_COLUMN && c != TSV_DELIMITER_ROW && last_char == TSV_DELIMITER_COLUMN ) {
				if ( j < r->column_name_length ) {
					r->column_name[j] = &r->buf[i];
					j++;
				}
                break;
            }
            break;
        case 1://data row
            if ( c != TSV_DELIMITER_COLUMN && c != TSV_DELIMITER_ROW && (last_char == TSV_DELIMITER_COLUMN || last_char == TSV_DELIMITER_ROW) ) {
				if ( j < r->data_length ) {
					r->data[j] = &r->buf[i];
					j++;
				}
                break;
            }
            break;
        default:
#ifdef MODE_DEBUG
            fprintf ( stderr, "%s(): unknown state\n", F );
#endif
            break;
        }
		last_char = c;
        i++;
    }
}

static int TSVcheck ( TSVresult *r ) {
    int row_count = TSVntuples ( r );
    int column_count = r->column_name_length;
    for ( int i = 0; i < row_count; i++ ) {
        for ( int j = 0; j < column_count; j++ ) {
            char * v = TSVgetvalue ( r, i, j );
            if ( v == NULL ) {
#ifdef MODE_DEBUG
                fprintf ( stderr, "%s(): bad value found\n", F );
#endif
                return 0;
            }
        }
    }
    return 1;
}

int TSVinit (TSVresult **item, const char *path ) {
    TSVresult *r = malloc ( sizeof (TSVresult) );
    if ( r == NULL ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: failed to allocate memory for tsv: ", F, path );
        perror ( "" );
#endif
        return 0;
    }
    r->buf = NULL, r->column_name = NULL, r->data = NULL, r->buf_length = 0, r->column_name_length = 0, r->data_length = 0, r->null_returned=0;
    FILE* stream = fopen ( path, "r" );
    if ( stream == NULL ) {
#ifdef MODE_DEBUG
        printde ( "%s(): on file: %s - ", F, path );
        perror ( "" );
#endif
        return 0;
    }
    int l = getBufLength ( stream );
    if ( l <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: not enough data for buffer\n", F,path );
#endif
        fclose ( stream );
        return 0;
    }
    size_t sz = l * sizeof * ( r->buf );
    r->buf = malloc ( sz );
    if ( r->buf == NULL ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: failed to allocate memory for buffer: ", F, path );
        perror ( "" );
#endif
        fclose ( stream );
        return 0;
    }
    memset ( r->buf, 0, sz );
    r->buf_length = l;
    l = getColumnNameLength ( stream );
    if ( l <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: not enough data for column\n", F, path );
#endif
        fclose ( stream );
        return 0;
    }
    sz = l * sizeof * ( r->column_name );
    r->column_name = malloc ( sz );
    if ( r->column_name == NULL ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: failed to allocate memory for column_name", F, path );
        perror ( "" );
#endif
        fclose ( stream );
        return 0;
    }
    memset ( r->column_name, 0, sz );
    r->column_name_length = l;
    l = getDataLength ( stream );
    if ( l <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: not enough data for data\n", F, path );
#endif
        fclose ( stream );
        return 0;
    }
    sz = l * sizeof * ( r->data );
    r->data = malloc ( sz );
    if ( r->data == NULL ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: failed to allocate memory for data", F, path );
        perror ( "" );
#endif
        fclose ( stream );
        return 0;
    }
    memset ( r->data, 0, sz );
    r->data_length = l;
    parse ( r, stream );
    fclose ( stream );
    if ( !TSVcheck ( r ) ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: checking result is bad\n", F, path );
#endif
        return 0;
    }
    *item = r;
    return 1;
}

int TSVntuples ( TSVresult *r ) {
    if ( r->column_name_length == 0 ) {
        return 0;
    }
    return r->data_length / r->column_name_length;
}

char * TSVgetvalue ( TSVresult *r, int row_number, int column_number ) {
    int ind = r->column_name_length * row_number + column_number;
    if ( ind >= r->data_length ) {
        r->null_returned=1;
        return NULL;
    }
    return r->data[ind];
}

char * TSVgetvalues ( TSVresult *r, int row_number, const char * column_name ) {
    for ( int i = 0; i < r->column_name_length; i++ ) {
        if ( strcmp ( column_name, r->column_name[i] ) == 0 ) {
            int ind = r->column_name_length * row_number + i;
            if ( ind >= r->data_length ) {
                printde ( "index out of valid bound for column '%s' and row %d", column_name, row_number );
                r->null_returned=1;
                return NULL;
            }
            return r->data[ind];
        }
    }
    printde ( "column '%s' not found\n", column_name );
    r->null_returned=1;
    return NULL;
}

int TSVgetis ( TSVresult *r, int row_number, const char * column_name ) {
    char *s = TSVgetvalues ( r, row_number, column_name );
    if ( s == NULL ) {
        return 0;
    }
    return atoi ( s );
}

double TSVgetfs ( TSVresult *r, int row_number, const char * column_name ) {
    char *s = TSVgetvalues ( r, row_number, column_name );
    if ( s == NULL ) {
        return 0.0;
    }
    return atof ( s );
}

int TSVgeti ( TSVresult *r, int row_number, int column_number ) {
    char *s = TSVgetvalue ( r, row_number, column_number );
    if ( s == NULL ) {
        return 0;
    }
    return atoi ( s );
}

double TSVgetf ( TSVresult *r, int row_number, int column_number ) {
    char *s = TSVgetvalue ( r, row_number, column_number );
    if ( s == NULL ) {
        return 0.0;
    }
    return atof ( s );
}
//(idc, valuec, max_retry)
char *TSVgetvalueById ( TSVresult *r, const char *id_column_name, const char *value_column_name, const char *id ) {
	int id_column_ind = getColumnIndex(r, id_column_name);
	if(id_column_ind < 0){
		r->null_returned=1;
		return NULL;
	}
	int value_column_ind = getColumnIndex(r, value_column_name);
	if(value_column_ind < 0){
		r->null_returned=1;
		return NULL;
	}
	for(int i = id_column_ind, c = 0; i < r->data_length; i+= r->column_name_length, c++){
	    if ( strcmp ( id, r->data[i] ) == 0 ) {
			int ind = value_column_ind + r->column_name_length * c;
			if(ind >= r->data_length){
				r->null_returned=1;
				return NULL;
			}
			return r->data[ind];
		}
	}
	r->null_returned=1;
	return NULL;
}

int TSVgetiById ( TSVresult *r, const char *id_column_name, const char *value_column_name, const char *id ) {
    char *s = TSVgetvalueById ( r, id_column_name, value_column_name, id );
    if ( s == NULL ) {
        return 0;
    }
    return atoi ( s );
}

double TSVgetfById ( TSVresult *r, const char *id_column_name, const char *value_column_name, const char *id ) {
    char *s = TSVgetvalueById ( r, id_column_name, value_column_name, id );
    if ( s == NULL ) {
        return 0;
    }
    return atof ( s );
}

int TSVnullreturned ( TSVresult *r ) {
    return r->null_returned;
}
void TSVclear ( TSVresult *r ) {
	if(r==NULL) return;
    free ( r->buf );
    free ( r->column_name );
    free ( r->data );
    free ( r );
}
