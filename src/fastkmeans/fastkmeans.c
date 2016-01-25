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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <gsl/gsl_rng.h>
#include "fastkmeans/fastkmeans.h"

typedef enum {
    SUCCESS=0,
    ALLOCATION_ERROR,
} fkm_error;

typedef struct {
    const gsl_matrix* points;
    gsl_matrix* clusters;
    const size_t num_points;
    const size_t dim;
    const size_t k;
    unsigned* owner_of;
} targs;

typedef struct {
    targs* args;
    size_t thread_id;
    pthread_t pthread_id;
    size_t num_threads;
} thread_meta;

typedef struct {
    thread_meta* threads;
    const size_t num_threads;
    int is_inited;
    pthread_barrier_t* barrier;
} thread_pool;

int fkm_clusters_assignment(const thread_pool* tpool);

double fkm_clusters_update(const targs* args);

double fkm_matrix_diff(gsl_matrix* a, gsl_matrix* b);

int fkm_kmeans(const gsl_matrix* points, gsl_matrix* clusters,
               size_t max_iter, int num_threads) {

    if (num_threads<0) {
        printf("Invalid number of threads\n");
        return 1;
    }

    size_t num_points = points->size1;
    unsigned owner_of[num_points];
    targs args = {
        .points=points,
        .clusters=clusters,
        .num_points=num_points,
        .dim=points->size2,
        .k=clusters->size1,
        .owner_of=owner_of,
    };

    thread_meta tmeta[num_threads];
    thread_pool tpool = {
        .num_threads=num_threads,
        .threads=tmeta,
        .is_inited=0,
        .barrier=PTHREAD_BARRIER_INITIALIZER(num_threads)
    };
    for (int i=0; i<num_threads; ++i) {
        tmeta[i].args = &args;
        tmeta[i].pthread_id = 0;
        tmeta[i].thread_id = i;
        tmeta[i].num_threads = num_threads;
    }
    gsl_rng* rng = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(rng, time(NULL));

    // Randomize which center each point belongs to
    for (size_t p=0; p < args.num_points; ++p) {
        // TODO: Make sure every cluster gets assigned
        owner_of[p] = gsl_rng_uniform_int(rng, args.k);
        /*DEBUGFMT("Made %u owner of %lu", owner_of[p], p);*/
    }

    // Initiate clusters to random point in input
    for (size_t m=0; m<args.k; m++) {
        size_t rand_p = gsl_rng_uniform_int(rng, args.num_points);
        for (size_t j=0; j<args.dim; ++j) {
            double val = gsl_matrix_get(points,
                                        rand_p,
                                        j);
            gsl_matrix_set(clusters, m, j, val);
        }
    }

    for (size_t it=0; it<max_iter; ++it) {
        DEBUGFMT("iter %lu num_points %lu dim %lu, k %lu", 
                it, args.num_points, args.dim, args.k);
        /*for (size_t p=0; p<num_points;++p) {*/
            /*printf("%lu:%u ", p, owner_of[p]);*/
        /*}*/
        /*printf("\n");*/
        fkm_clusters_assignment(&tpool);
        double diff = fkm_clusters_update(&args);
        DEBUGFMT("Diff: %f", diff);
        if (fabs(diff) < 1e-10) {
            DEBUG("Break since no progress");
            break;
        }
    }
    return 0;
}


void* fkm_clusters_assignment_single(void* _data) {
    thread_meta* tmeta = _data;
    targs* args = tmeta->args;

    size_t offset = tmeta->thread_id;
    size_t stride = tmeta->num_threads;
    for (size_t p=offset; p<args->num_points; p+=stride) {
        double min_err = 1e308;
        /*DEBUGFMT("p:%lu", p);*/
        for (size_t m=0; m<args->k; ++m) {
            double err = 0;
            for (size_t j=0; j<args->dim; ++j) {
                double a = fabs(gsl_matrix_get(args->clusters, m, j)
                                - gsl_matrix_get(args->points, p, j));
                err += a*a;
            }
            /*DEBUGFMT("err: %f", err);*/
            if (err < min_err) {
                min_err = err;
                /*DEBUGFMT("NEW min_err m:%lu, err:%f", m, err);*/
                /*if (m != owner_of[p]) {*/
                    /*DEBUGFMT("p:%lu changed center to %lu from %u",*/
                             /*p, m, owner_of[p]);*/
                /*}*/
                args->owner_of[p] = m;
            }
        }
    }
    return 0;
}

int fkm_clusters_assignment(const thread_pool* tpool) {

    if (!tpool->is_inited) {
    for (size_t thr=0; thr<tpool->num_threads;++thr) {
        thread_meta* tdata = &tpool->threads[thr];
        int err = pthread_create(&tdata->pthread_id,
                                 NULL, 
                                 &fkm_clusters_assignment_single,
                                 tdata);
        if (err) {
            ERRORFMT("pthread_create-error %i", err);
        }
    }
    for (size_t thr=0; thr<tpool->num_threads;++thr) {
        thread_meta* tdata = &tpool->threads[thr];
        int err = pthread_join(tdata->pthread_id, NULL);
        if (err) {
            ERRORFMT("pthread_join-error %i", err);
        }
    }
    return 0;
}
/**
 * Update clusters.
 *
 *
 */
double fkm_clusters_update(const targs* args) {
    /*printf("DEBUG: num_points %lu dim %lu, k %lu\n", num_points, dim, k);*/
    unsigned cluster_size[args->k];
    for (size_t m=0; m<args->k; ++m) {
        cluster_size[m] = 0;
    }
    gsl_matrix* mean_sums = gsl_matrix_calloc(args->k, args->dim);
    for (size_t p=0; p<args->num_points; ++p) {
        unsigned m = args->owner_of[p];
        assert(m < args->k);
        cluster_size[m] += 1;
        /*LOGFMT("p:%lu belongs to %u. Children %u", p, m, cluster_size[m]);*/
        for (size_t j=0; j<args->dim; ++j) {
            double val = gsl_matrix_get(mean_sums, m, j)
                + gsl_matrix_get(args->points, p, j);
            gsl_matrix_set(mean_sums, m, j, val);
        }
    }
    double diff = 0;
    for (size_t m=0; m<args->k; ++m) {
        /*printf("Updating cluster %lu to", m);*/
        for (size_t j=0; j<args->dim; ++j) {
            double val = gsl_matrix_get(mean_sums, m, j);
            val /= cluster_size[m];
            diff += val - gsl_matrix_get(args->clusters, m, j);
            gsl_matrix_set(args->clusters, m, j, val);
            /*printf(" %u %f", cluster_size[m], val);*/
        }
        /*printf("\n");*/
    }
    gsl_matrix_free(mean_sums);
    return diff;
}

typedef double (*elem_func)(double, double);

double fkm_matrix_elem_func(gsl_matrix* a, gsl_matrix* b, elem_func f) {
    double v = 0;
    for (size_t i=0; i<a->block->size; ++i) {
        v += f(a->block->data[i], b->block->data[i]);
    }
    return v;
}

double diff(double a, double b) {
    return fabs(a - b);
}

double fkm_matrix_diff(gsl_matrix* a, gsl_matrix* b) {
    return fkm_matrix_elem_func(a, b, diff);
}

void print_matrix(gsl_matrix* a) {
    for (size_t i=0; i<5 && i<a->block->size; ++i) {
        printf("%lu: %f\n", i, a->block->data[i]);
    }
}

gsl_matrix* fkm_matrix_load(FILE* fin) {
    size_t rows, cols;
    int err = fscanf(fin, "%lu %lu", &rows, &cols);
    if (err == EOF) {
        printf("ERROR: Could not parse matrix dimensions\n");
        return 0;
    }
    gsl_matrix* mat = gsl_matrix_alloc(rows, cols);
    err = gsl_matrix_fscanf(fin, mat);
    if (err) {
        printf("ERROR: Could not parse matrix data\n");
        return 0;
    }
    printf("Loaded %lu rows, %lu columns matrix\n", rows, cols);
    return mat;
}

int fkm_matrix_save(FILE* fout, gsl_matrix* mat) {
    int err = 0;
    fprintf(fout, "%zu %zu\n", mat->size1, mat->size2);
    for (size_t i=0; i<mat->block->size; ++i) {
        if (i && i % mat->size2 == 0) {
            err = fprintf(fout, "\n");
            if (err<0) {
                ERRORFMT("fprintf returned %i", err);
            }
        }
        int err = fprintf(fout, "%f ", mat->block->data[i]);
        if (err<0) {
            ERRORFMT("fprintf returned %i", err);
        }
    }
    return err;
}

