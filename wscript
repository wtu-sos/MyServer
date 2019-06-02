#! /usr/bin/env python
# encoding: utf-8
import os

# the following two variables are used by the target "waf dist"
VERSION='0.0.1'
APPNAME='io'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(opt):
    opt.load('g++')

def configure(conf):
    conf.load('g++')
    #if not conf.env['LIB_PTHREAD']:
        # If we have not looked for pthread yet
        #conf.check_cxx(lib='pthread')
    #conf.check(header_name='stdio.h', features='cxx cxxprogram', mandatory=False)

def pre_build(ctx):
    print("before building ... ")
    protos = ctx.path.ant_glob(['proto/*.proto'])
    print("protos: ", protos)
    cur_dir = ctx.path
    for p in protos:
        rp = p.path_from(cur_dir)
        print("item: ", rp)
        cmd = "protoc -I=proto/ --cpp_out=src/proto " + str(rp)
        print("exec command: " + cmd)
        #output = os.popen(cmd)
        ctx.exec_command(cmd)
        ctx.exec_command('ls -al src/proto')

def post_build(ctx):
    print("after build .....")


includes_dir = ['.', 'libs/', 'libs/include']
cxx_build_flags = ['-std=c++14']
ld_flags = ['-lpthread']

#dynamic library
share_libs = ['protobuf']
share_lib_path = ['/usr/local/lib']

link_flags = []

def build(bld):
    bld.add_pre_fun(pre_build)
    bld.add_post_fun(post_build)

    bld.program(
            source=bld.path.ant_glob(['src/**/*.cpp', 'src/**/*.cc'], excl=['src/server.cpp']), 
            includes = includes_dir,
            cxxflags = cxx_build_flags,
            linkflags = link_flags,
            ldflags = ld_flags,
            lib = share_libs,
            libpath = share_lib_path,
            target='client',
            use='asio'
            )
    bld.program(
            source=bld.path.ant_glob(['src/**/*.cpp', 'src/**/*.cc'], excl=['src/client.cpp']), 
            includes = includes_dir,
            cxxflags = cxx_build_flags,
            linkflags = link_flags,
            ldflags = ld_flags,
            lib = share_libs,
            libpath = share_lib_path,
            target='server',
            use='asio'
            )

    if bld.cmd != 'clean':
        from waflib import Logs
        bld.logger = Logs.make_logger('test.log', 'build') # just to get a clean output

    bld.logger = None
