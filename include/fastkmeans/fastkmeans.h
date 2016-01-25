/** 
 * Simple and fast kmeans-implementation making use of multithreading.
 *
 * Author: Anders Bennehag
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Anders Bennehag
 *
 */
#ifndef FASTKMEANS_H_
#define FASTKMEANS_H_

#include <gsl/gsl_matrix.h>

int fkm_kmeans(const gsl_matrix* points, gsl_matrix* clusters,
               size_t max_iter, int num_threads);

gsl_matrix* fkm_matrix_load(FILE* fout);
int fkm_matrix_save(FILE* fout, gsl_matrix* mat);

#define WHERESTR  "[%s:%d]: "
#define WHEREARG  __FILE__, __LINE__
#define DEBUGPRINT2(...)       fprintf(stderr, __VA_ARGS__)

#define FKM_LOGFMT(_fmt, ...)  DEBUGPRINT2(WHERESTR _fmt "\n", WHEREARG, __VA_ARGS__)
#define FKM_LOG(_s) DEBUGPRINT2(WHERESTR "%s\n", WHEREARG, _s)
#define FKM_LOGTYPE(_tp, _s)  DEBUGPRINT2(WHERESTR "%s: %s" "\n", WHEREARG, _tp, _s)
#define FKM_LOGTYPEFMT(_tp, _fmt, ...)  DEBUGPRINT2(WHERESTR "%s: " _fmt "\n", WHEREARG, _tp, __VA_ARGS__)

#if (ISDEBUG + 0)
    #define FKM_DEBUGFMT(_fmt, ...)  FKM_LOGTYPEFMT("DEBUG", _fmt, __VA_ARGS__)
    #define FKM_DEBUG(_s)  FKM_LOGTYPE("DEBUG", _s)
#else
    #define FKM_DEBUGFMT(...)
    #define FKM_DEBUG(...)
#endif
#define FKM_INFOFMT(_fmt, ...)   FKM_LOGTYPEFMT("INFO", _fmt, __VA_ARGS__)
#define FKM_WARNFMT(_fmt, ...)   FKM_LOGTYPEFMT("WARN", _fmt, __VA_ARGS__)
#define FKM_ERRORFMT(_fmt, ...)  FKM_LOGTYPEFMT("ERROR", _fmt, __VA_ARGS__)

#define FKM_INFO(_s)   FKM_LOGTYPE("INFO", _s)
#define FKM_WARN(_s)   FKM_LOGTYPE("WARN", _s)
#define FKM_ERROR(_s)  FKM_LOGTYPE("ERROR", _s)


#endif
