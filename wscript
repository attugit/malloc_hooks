#! /usr/bin/env python
# encoding: utf-8

VERSION = '0.1.0'
APPNAME = 'malloc_hooks'

top = '.'
out = 'build'
flags = [
    '-std=c++1z',
    '-Wall',
    '-Wextra',
    '-Wshadow',
    '-Wconversion',
    '-Wnon-virtual-dtor',
    '-Wold-style-cast',
    '-Wcast-align',
    '-Wunused',
    '-Woverloaded-virtual',
    '-pedantic',
    '-pedantic-errors',
    '-Werror'
]


def init(ctx):
    ctx.load('build_logs')

import os


def options(opt):
    opt.load('compiler_cxx')


def configure(conf):
    conf.load('compiler_cxx')


def build(bld):
    if bld.variant == 'debug':
        bld.env.CXXFLAGS += ['-g', '-O0']
        bld.env.DEFINES += ['DEBUG']
    if bld.variant == 'release':
        bld.env.CXXFLAGS += ['-O3', '-march=native', '-fPIC', '-fno-rtti']
        bld.env.DEFINES += ['NDEBUG']
    bld.env.CXXFLAGS += flags
    bld.define('APPNAME', APPNAME)
    bld.define('VERSION', VERSION)
    bld.env.INCLUDES += ['inc']
    bld(source=bld.path.ant_glob(['src/lib/**/*.cc']),
        target='mhook',
        features='cxx cxxstlib',
        cxxflags='-Wno-deprecated-declarations'
    )
    bld(source=bld.path.ant_glob(['src/malloc_hooks/**/*.cc']),
        target=APPNAME,
        features='cxx cxxshlib',
        cxxflags='-Wno-deprecated-declarations',
        use='mhook'
    )
    for suite in os.listdir('./test'):
        bld(
            source=bld.path.ant_glob(['test/%s/**/*.cpp' % suite]),
            target='test_' + str(suite),
            features='cxx cxxprogram',
            cxxflags='-Wno-deprecated-declarations',
            use='mhook',
            install_path=None,
        )
    inc = bld.path.find_dir('inc')
    for f in bld.path.ant_glob(['inc/**/*.hpp']):
        bld.install_files(
            '${PREFIX}/include/' +
            f.path_from(inc).replace(
                f.name,
                ''),
            f)

from waflib.Build import BuildContext
from waflib.Build import CleanContext
from waflib.Build import InstallContext
from waflib.Build import UninstallContext

for ctx in (BuildContext, CleanContext, InstallContext, UninstallContext):
    name = ctx.__name__.replace('Context', '').lower()

    class debug(ctx):
        cmd = name + '_debug'
        variant = 'debug'

    class release(ctx):
        cmd = name
        variant = 'release'
