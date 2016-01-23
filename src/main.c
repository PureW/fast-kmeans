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
#include "fastkmeans.h"


typedef struct {
    char* infile;
    char* outfile;
    unsigned k;
    unsigned max_iter;
    int err;
} pargs;

pargs parse_args(int argc, char** argv);
FILE* open_stream(char* fname, char* mode, FILE* default_stream);
void display_help(int argc, char** argv);

int main(int argc, char** argv) {
    pargs args = parse_args(argc, argv);
    if (args.err) {
        return 99;
    } else {
        // Init data
        FILE* fin = open_stream(args.infile, "r", stdin);
        gsl_matrix* points = fkm_matrix_load(fin);
        if (fin != stdin) {
            fclose(fin);
        }
        gsl_matrix* clusters = gsl_matrix_alloc(args.k, points->size2);

        // Calculate
        fkm_kmeans(points, clusters, args.max_iter);

        FILE* fout = open_stream(args.outfile, "w", stdout);
        fkm_matrix_save(fout, clusters);
        // Cleanup
        if (fout != stdout) {
            fclose(fout);
        }
        gsl_matrix_free(points);
        gsl_matrix_free(clusters);
    }
    return 0;
}

FILE* open_stream(char* fname, char* mode, FILE* default_stream) {
    FILE *stream;
    if (strcmp("-", fname)) {
        stream = fopen(fname, mode);
        if (!stream) {
            printf("ERROR: Could not open %s\n", fname);
            return 0;
        }
    } else {
        stream = default_stream;
    }
    return stream;
}

pargs parse_args(int argc, char** argv) {
    pargs args = {0};
    if (argc > 1 && strcmp(argv[1], "-h") == 0) {
        display_help(argc, argv);
        args.err = 99;
        return args;
    }
    if (argc <= 3) {
        printf("ERROR: Not enough arguments\n");
        display_help(argc, argv);
        args.err = 99;
        return args;
    }
    args.k = atoi(argv[argc-3]);
    args.infile = argv[argc-2];
    args.outfile = argv[argc-1];
    if (argc == 5) {
        args.max_iter = atoi(argv[2]);
    } else {
        args.max_iter = 10;
    }
    return args;
}
void display_help(int argc, char** argv) {
    printf("Usage: %s [MAX_ITER=100] K INFILE OUTFILE\n", argv[0]);
    printf("Find K clusters in data from INFILE and write to OUTFILE.\n");
    printf("\n");
    printf("INFILE and OUTFILE can be '-' to use stdin and stdout resp.\n");
    printf("Do max MAX_ITER iterations of K-means algorithm.\n");
}
