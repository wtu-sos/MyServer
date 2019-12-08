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
        cmd = "protoc -I=proto/ --cpp_out=src/proto " + str(rp)
        print("exec command: " + cmd)
        #output = os.popen(cmd)
        ctx.exec_command(cmd)
        #ctx.exec_command('ls -al src/proto')

def post_build(ctx):
    print("after build .....")


includes_dir = ['.', 'libs/', 'libs/include', 'libs/glog/glog/include']
cxx_build_flags = ['-std=c++17', '-g', '-O0']

# 指定静态库以及对应的路径，不然链接时报错
ld_flags = ['-lpthread', '-lglog', '-L/home/yu/Documents/MyServer/libs/glog/glog/lib']
r_path = ['/home/yu/Documents/MyServer/libs/glog/glog/lib']

link_flags = []

#dynamic library
share_libs = ['protobuf']

def share_lib_path(conf):
    abs_path = conf.path.abspath()
    share_lib_path = ['/usr/local/lib']
    relate_share_lib_path = ['libs/']

    for rp in relate_share_lib_path:
        p = abs_path + '/' + rp
        share_lib_path.append(p)
        #cmd = "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"+p
        #print("exec cmd:", cmd)
        #conf.exec_command(cmd)

    print("share lib path: ", share_lib_path)
    return share_lib_path

#static library
static_lib = ['glog']

def static_lib_path(conf):
    abs_path = conf.path.abspath()
    static_lib_path = ['/usr/local/lib']
    relate_static_lib_path = ['libs/glog/glog/lib']

    for sp in relate_static_lib_path:
        static_lib_path.append(abs_path + '/' + sp)

    print("static lib path: ", static_lib_path)
    return static_lib_path

def build(bld):
    bld.add_pre_fun(pre_build)
    bld.add_post_fun(post_build)

    bld.program(
            source=bld.path.ant_glob(['src/**/*.cpp', 'src/**/*.cc'], excl=['src/server.cpp']), 
            includes = includes_dir,
            cxxflags = cxx_build_flags,
            lib = share_libs,
            libpath = share_lib_path(bld),
            stlib = static_lib,
            stlibpath = static_lib_path(bld),
            rpath = r_path,
            linkflags = link_flags,
            ldflags = ld_flags,
            target='client',
            use='asio'
            )

    bld.program(
            source=bld.path.ant_glob(['src/**/*.cpp', 'src/**/*.cc'], excl=['src/client.cpp']), 
            includes = includes_dir,
            cxxflags = cxx_build_flags,
            lib = share_libs,
            libpath = share_lib_path(bld),
            rpath = r_path,
            linkflags = link_flags,
            ldflags = ld_flags,
            target='server',
            use='asio'
            )

    #if bld.cmd != 'clean':
    #    from waflib import Logs
    #    bld.logger = Logs.make_logger('test.log', 'build') # just to get a clean output

    #bld.logger = None
