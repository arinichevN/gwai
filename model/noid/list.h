#ifndef MODEL_NOID_LIST_H
#define MODEL_NOID_LIST_H

#include "../../lib/debug.h"
#include "../../lib/dstructure.h"
#include "main.h"

DEC_LIST(Noid)

extern void noidList_terminate (NoidList *list);

extern void noidList_free (NoidList *list);

#endif 
