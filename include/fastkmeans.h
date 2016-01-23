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

int fkm_kmeans(gsl_matrix* points, gsl_matrix* clusters,
               size_t max_iter);

gsl_matrix* fkm_matrix_load(FILE* fout);
int fkm_matrix_save(FILE* fout, gsl_matrix* mat);

#define WHERESTR  "[file %s, line %d]: "
#define WHEREARG  __FILE__, __LINE__
#define DEBUGPRINT2(...)       fprintf(stderr, __VA_ARGS__)
#define LOGFMT(_fmt, ...)  DEBUGPRINT2(WHERESTR _fmt "\n", WHEREARG, __VA_ARGS__)
#define LOG(_s) DEBUGPRINT2(WHERESTR "%s\n", WHEREARG, _s)
#define LOGTYPE(_tp, _s)  DEBUGPRINT2(WHERESTR "%s: %s" "\n", WHEREARG, _tp, _s)
#define LOGTYPEFMT(_tp, _fmt, ...)  DEBUGPRINT2(WHERESTR "%s: " _fmt "\n", WHEREARG, _tp, __VA_ARGS__)

#define DEBUGFMT(_fmt, ...)  LOGTYPEFMT("DEBUG", _fmt, __VA_ARGS__)
#define INFOFMT(_fmt, ...)  LOGTYPEFMT("INFO", _fmt, __VA_ARGS__)
#define WARNFMT(_fmt, ...)  LOGTYPEFMT("WARN", _fmt, __VA_ARGS__)
#define ERRORFMT(_fmt, ...)  LOGTYPEFMT("ERROR", _fmt, __VA_ARGS__)

#define DEBUG(...) LOGTYPE("DEBUG", _s)
#define INFO(...)  LOGTYPE("INFO", _s)
#define WARN(...)  LOGTYPE("WARN", _s)
#define ERROR(_s)  LOGTYPE("ERROR", _s)


#endif
