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
#include "fastkmeans.h"

typedef enum {
    SUCCESS=0,
    ALLOCATION_ERROR,
} fkm_error;

double fkm_clusters_update(const gsl_matrix* points,
                           gsl_matrix* clusters,
                           unsigned* owner_of);

double fkm_matrix_diff(gsl_matrix* a, gsl_matrix* b);

int fkm_kmeans(gsl_matrix* points, gsl_matrix* clusters,
               size_t max_iter) {

    const size_t num_points = points->size1;
    const size_t dim = points->size2;
    const size_t k = clusters->size1;
    unsigned owner_of[num_points];
    gsl_rng* rng = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(rng, time(NULL));

    // Randomize which center each point belongs to
    for (size_t p; p < num_points; ++p) {
        owner_of[p] = gsl_rng_uniform_int(rng, k);
    }

    // Initiate clusters to random point in input
    for (size_t i=0; i<k; i++) {
        for (size_t j=0; j<dim; ++j) {
            double val = gsl_matrix_get(points,
                                        owner_of[i],
                                        j);
            gsl_matrix_set(clusters, i, j, val);
        }
    }

    for (size_t it=0; it<max_iter; ++it) {
        LOGFMT("DEBUG: iter %lu num_points %lu dim %lu, k %lu", 
               it, num_points, dim, k);
        for (size_t p=0; p<num_points; p++) {
            double min_err = 1e308;
            size_t closest_m = 0;
            double err = 0;
            for (size_t m=0; m<k; ++m) {
                for (size_t j=0; j<dim; ++j) {
                    err += fabs(gsl_matrix_get(clusters, m, j) -
                                gsl_matrix_get(points, p, j));
                }
                if (err < min_err) {
                    min_err = err;
                    closest_m = m;
                }
            }
        }
        double diff = fkm_clusters_update(points, clusters, owner_of);
        DEBUGFMT("Diff: %f", diff);
        if (fabs(diff) < 1e-3) {
            break;
        }
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
                         unsigned* owner_of) {
    const size_t num_points = points->size1;
    const size_t dim = points->size2;
    const size_t k = clusters->size1;
    /*printf("DEBUG: num_points %lu dim %lu, k %lu\n", num_points, dim, k);*/
    int cluster_size[k];
    for (size_t m=0; m<k; ++m) {
        cluster_size[m] = 0;
    }
    gsl_matrix* mean_sums = gsl_matrix_calloc(k, dim);
    for (size_t p=0; p<num_points; ++p) {
        unsigned m = owner_of[p];
        assert(m < k);
        cluster_size[m] += 1;
        for (size_t j=0; j<dim; ++j) {
            double val = gsl_matrix_get(mean_sums, m, j) 
                + gsl_matrix_get(points, m, j);
            gsl_matrix_set(mean_sums, m, j, val);
        }
    }
    double diff = 0;
    for (size_t m=0; m<k; ++m) {
        for (size_t j=0; j<dim; ++j) {
            double val = gsl_matrix_get(mean_sums, m, j);
            val /= cluster_size[m];
            diff += val - gsl_matrix_get(clusters, m, j);
            gsl_matrix_set(clusters, m, j, val);
        }
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
    fprintf(fout, "%zu %zu\n", mat->size1, mat->size2);
    int err = gsl_matrix_fprintf(fout, mat, "%f");
    if (err) {
        ERROR("Problem saving matrix");
    }
    return err;
}

