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
    size_t num_points;
    size_t dim;
    size_t k;
} meta_data;

int fkm_clusters_assignment(const gsl_matrix* points,
                            gsl_matrix* clusters,
                            unsigned* owner_of,
                            const meta_data* meta);
double fkm_clusters_update(const gsl_matrix* points,
                           gsl_matrix* clusters,
                           unsigned* owner_of,
                           const meta_data* meta);

double fkm_matrix_diff(gsl_matrix* a, gsl_matrix* b);

int fkm_kmeans(const gsl_matrix* points, gsl_matrix* clusters,
               size_t max_iter) {

    const meta_data meta = {
        .num_points=points->size1,
        .dim = points->size2,
        .k = clusters->size1,
    };
    unsigned owner_of[meta.num_points];
    gsl_rng* rng = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(rng, time(NULL));

    // Randomize which center each point belongs to
    for (size_t p=0; p < meta.num_points; ++p) {
        // TODO: Make sure every cluster gets assigned
        owner_of[p] = gsl_rng_uniform_int(rng, meta.k);
        /*DEBUGFMT("Made %u owner of %lu", owner_of[p], p);*/
    }

    // Initiate clusters to random point in input
    for (size_t m=0; m<meta.k; m++) {
        size_t rand_p = gsl_rng_uniform_int(rng, meta.num_points);
        for (size_t j=0; j<meta.dim; ++j) {
            double val = gsl_matrix_get(points,
                                        rand_p,
                                        j);
            gsl_matrix_set(clusters, m, j, val);
        }
    }

    for (size_t it=0; it<max_iter; ++it) {
        DEBUGFMT("iter %lu num_points %lu dim %lu, k %lu", 
                it, meta.num_points, meta.dim, meta.k);
        /*for (size_t p=0; p<num_points;++p) {*/
            /*printf("%lu:%u ", p, owner_of[p]);*/
        /*}*/
        /*printf("\n");*/
        fkm_clusters_assignment(points, clusters, owner_of, &meta);
        double diff = fkm_clusters_update(points, clusters, owner_of, &meta);
        DEBUGFMT("Diff: %f", diff);
        if (fabs(diff) < 1e-10) {
            DEBUG("Break since no progress");
            break;
        }
    }
    return 0;
}

typedef struct {
    const meta_data* meta;
    const gsl_matrix* points;
    gsl_matrix* clusters;
    unsigned* owner_of;
    size_t thread_id;
    size_t num_threads;
} assignment_thread_args;

void* fkm_clusters_assignment_single(void* data) {
    assignment_thread_args* args = data;
    const meta_data* meta = args->meta;

    size_t offset = args->thread_id;
    size_t stride = args->num_threads;
    for (size_t p=offset; p<meta->num_points; p+=stride) {
        double min_err = 1e308;
        /*DEBUGFMT("p:%lu", p);*/
        for (size_t m=0; m<meta->k; ++m) {
            double err = 0;
            for (size_t j=0; j<meta->dim; ++j) {
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

int fkm_clusters_assignment(const gsl_matrix* points,
                            gsl_matrix* clusters,
                            unsigned* owner_of,
                            const meta_data* meta) {
    size_t num_threads = 4;
    pthread_t threads[num_threads];
    for (size_t thr=0; thr<num_threads;++thr) {
        assignment_thread_args args = {
            .meta = meta,
            .points = points,
            .clusters = clusters,
            .owner_of=owner_of,
            .thread_id=thr,
            .num_threads=num_threads};

        pthread_create(&threads[thr], 
                       NULL, 
                       &fkm_clusters_assignment_single,
                       (void*)&args);
    }
    for (size_t thr=0; thr<num_threads;++thr) {
        pthread_join(threads[thr], NULL);
    }
    return 0;
}
/**
 * Update clusters.
 *
 *
 */
double fkm_clusters_update(const gsl_matrix* points,
                           gsl_matrix* clusters,
                           unsigned* owner_of,
                           const meta_data* meta) {
    /*printf("DEBUG: num_points %lu dim %lu, k %lu\n", num_points, dim, k);*/
    unsigned cluster_size[meta->k];
    for (size_t m=0; m<meta->k; ++m) {
        cluster_size[m] = 0;
    }
    gsl_matrix* mean_sums = gsl_matrix_calloc(meta->k, meta->dim);
    for (size_t p=0; p<meta->num_points; ++p) {
        unsigned m = owner_of[p];
        assert(m < meta->k);
        cluster_size[m] += 1;
        /*LOGFMT("p:%lu belongs to %u. Children %u", p, m, cluster_size[m]);*/
        for (size_t j=0; j<meta->dim; ++j) {
            double val = gsl_matrix_get(mean_sums, m, j)
                + gsl_matrix_get(points, p, j);
            gsl_matrix_set(mean_sums, m, j, val);
        }
    }
    double diff = 0;
    for (size_t m=0; m<meta->k; ++m) {
        /*printf("Updating cluster %lu to", m);*/
        for (size_t j=0; j<meta->dim; ++j) {
            double val = gsl_matrix_get(mean_sums, m, j);
            val /= cluster_size[m];
            diff += val - gsl_matrix_get(clusters, m, j);
            gsl_matrix_set(clusters, m, j, val);
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

