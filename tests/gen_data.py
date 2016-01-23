#!/usr/bin/env python


import random

def gen_data(fname, cols=10, rows=100):
    random.seed()
    with open(fname, 'w') as f:
        f.write('{} {}\n'.format(rows, cols))
        for _ in xrange(0, rows):
            for _ in xrange(cols):
                f.write('{} '.format(random.random()))
            f.write('\n')


def parse_args():
    import argparse
    parser = argparse.ArgumentParser(description='Update routes for stations')
    parser.add_argument('OUTFILE',
                        help='Where to store testdata')
    parser.add_argument('--cols',
                        type=int,
                        default=10,
                        help='Number of cols in testdata')
    parser.add_argument('--rows',
                        type=int,
                        default=100,
                        help='Number of rows in testdata')
    args = parser.parse_args()
    return args


if __name__ == '__main__':
    args = parse_args()
    gen_data(args.OUTFILE, args.cols, args.rows)


