# fast-kmeans
Simple, to the point, fast k-means implementation making use of multi-core cpu's

## Compile

Compile using:

```
./configure.sh
make
```
This builds *build/release/fastkmeans* which is a simple command-line parser around the fastkmeans-algorithm.

To include fastkmeans in your own program, include *include/fastkmeans* and link *build/release/libfastkmeans.a*.
