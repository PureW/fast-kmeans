# fast-kmeans
Simple and fast k-means implementation making use of multi-core cpu's.


## Dependencies

 * C11
 * pthreads
 * GSL: Gnu Scientific Library https://www.gnu.org/software/gsl/
 * TODO

## Compile

Compile using:

```
./configure.sh
make
```
This builds *build/release/libfastkmeans.a* which is the static library and  *build/release/fastkmeans* which is a simple command-line parser around the fastkmeans-library.

To include fastkmeans in your own program, include *include/fastkmeans* and link *build/release/libfastkmeans.a*.

## Usage

```c
// gsl_matrix* observations is setup to contain data to cluster
...
// Allocate matrix for clusters
gsl_matrix* clusters = gsl_matrix_alloc(rows, cols);

// Calculate clusters
fkm_kmeans(points, clusters, args.max_iter, args.num_threads);

// See clusters for detected clusters
```

## License

The MIT-license, see LICENSE.

