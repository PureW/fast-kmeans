#! /usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2006-2010 (ita)

# the following two variables are used by the target "waf dist"
VERSION = '0.0.1'
APPNAME = 'fastkmeans'

INCLUDES = [
    'include',
]
SOURCES = [
    'src/main.c',
    'src/fastkmeans.c',
]

SHLIBS = [
    'pthread',
    'gsl',
    'm',
    'openblas',
]
STLIBS = []
USES = [
]

CFLAGS = [
    '-g',
    '--std=c11',
    '--pedantic',
    '-Werror',
    '-Wall',
    '-Wextra',
    '-Wfatal-errors',
    '-Wno-unused',
]

DEFINES = [
    'G_LOG_DOMAIN=\"{}\"'.format(APPNAME),
]

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.load('compiler_c')
    conf.check(header_name='stdio.h', features='c cprogram', mandatory=True)
    conf.check(header_name='pthread.h', features='c cprogram', mandatory=True)

def build(bld):
    bld.program(source=SOURCES,
                includes=INCLUDES,
                target=APPNAME,
                use=USES,
                lib=SHLIBS,
                defines=DEFINES,
                # libpath=,
                stlib=STLIBS,
                # stlibpath=,
                cflags=CFLAGS,
    )

