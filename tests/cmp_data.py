#!/usr/bin/env python

import numpy as np
from scipy.cluster.vq import vq, kmeans, whiten
from matplotlib import pyplot as plt


def easy_kmeans(points, k):
    # whitened = whiten(points)
    whitened = points
    # book = np.array((whitened[0], whitened[2]))
    return kmeans(whitened, k)


def plot_clusters(points, km, fname):
    plt.clf()
    colors = ['red',
              'blue',
              'green',
              'yellow',
              'cyan',
              'orange',
              'brown',
              'grey',
    ]
    owner_of = np.zeros(points.shape[0])
    clusters = {}
    total_err = 0
    for r in xrange(points.shape[0]):
        p = points[r, :]
        # print 'p:', r, p
        min_err = 1e308
        for n in xrange(len(km)):
            m = km[n, :]
            err = sum(np.abs(p-m))
            # print 'err', err
            if err < min_err:
                owner_of[r] = n
                min_err = err
        # print 'new min_err', min_err
        total_err += min_err
        if owner_of[r] not in clusters:
            clusters[owner_of[r]] = []
        clusters[owner_of[r]].append(r)
    print 'Has {} clusters with err {}'.format(len(clusters), total_err)
    for m, l in clusters.iteritems():
        color = colors[int(m)]
        plt.scatter(points[l, 0], points[l, 1], c=color)

    plt.scatter(km[:, 0], km[:, 1], c='black',  s=100, marker='x')
    plt.savefig(fname)

def main(fname_points, fname_clusters):
    points = np.loadtxt(fname_points, skiprows=1)
    print "Loaded points with dim {}".format(points.shape)
    fkm_clusters = np.loadtxt(fname_clusters, skiprows=1)
    k = fkm_clusters.shape[0]
    print "Loaded clusters with dim {}".format(fkm_clusters.shape)
    km, _ = easy_kmeans(points, k)
    plot_clusters(points, km, 'scipy.pdf')
    plot_clusters(points, fkm_clusters, 'fkm.pdf')


def parse_args():
    import argparse
    parser = argparse.ArgumentParser(
        description='Reference-implementation for comparison')
    parser.add_argument('FNAME_POINTS',
                        help='Which points to cluster')
    parser.add_argument('FNAME_CLUSTERS',
                        help='Clusters to compare with.')
    args = parser.parse_args()
    return args


if __name__ == '__main__':
    args = parse_args()
    plt.set_cmap('autumn')
    main(args.FNAME_POINTS, args.FNAME_CLUSTERS)

