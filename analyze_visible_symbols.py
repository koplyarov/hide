#!/usr/bin/python

import argparse
import os
import re
import sys
import copy
import subprocess
from collections import defaultdict

unknown_includes = []

include_regex = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"].*')

include_dirs = [ '.', 'stingray/stingraykit', '/usr/include/c++/5', '/usr/include/x86_64-linux-gnu/c++/5', '/usr/include/c++/5/backward', '/usr/lib/gcc/x86_64-linux-gnu/5/include', '/usr/local/include', '/usr/lib/gcc/x86_64-linux-gnu/5/include-fixed', '/usr/include/x86_64-linux-gnu', '/usr/include' ]
#include_dirs = [ '.', 'stingray/stingraykit' ]
def get_real_file_name(filename, loc):
    if os.path.isfile("{}/{}".format(os.path.dirname(loc), filename)):
        return os.path.normpath("{}/{}".format(os.path.dirname(loc), filename))
    for id in include_dirs:
        if os.path.isfile("{}/{}".format(id, filename)):
            return os.path.normpath("{}/{}".format(id, filename))
    return filename

def get_includes(fn):
    res = []
    if os.path.isfile(fn):
        with open(fn) as f:
            for l in f.readlines():
                m = include_regex.match(l)
                if m:
                    yield os.path.normpath(m.group(1))

visited_includes_inc = {}
def get_includes_recursive(fn, include_stack = set()):
    path = os.path.normpath(fn)
    try:
        return visited_includes_inc[path]
    except KeyError:
        inc = list(get_includes(path))
        if len(inc) == 0:
            includes = set()
        else:
            includes = set()
            for i in inc:
                real_i = get_real_file_name(i, path)
                if real_i in include_stack:
                    continue
                includes.add(real_i)
                new_include_stack = copy.copy(include_stack)
                new_include_stack.add(path)
                includes.update(set(get_includes_recursive(real_i, new_include_stack)))

        visited_includes_inc[path] = includes
        return includes

file_to_syms = {}

#public_regex = re.compile(r'^([^\t]+)\t.*access:public.*')
public_regex = re.compile(r'^([^\t]+)\t.*')
stupid_macro_regex = re.compile(r'^[A-Z0-9_]+_\d+(_[A-Z])?$')
intel_simd_regex = re.compile(r'^_mm\d*_.*$')
detail_shit_regex = re.compile(r'^__.*$')
def get_file_syms(filename):
    try:
        return file_to_syms[filename]
    except KeyError:
        raw_syms = subprocess.check_output(['ctags', '-f-', '--fields=+a', '--c-kinds=+p', filename]).split('\n')
        syms = set()
        for s in raw_syms:
            m = public_regex.match(s)
            if m:
                sym_name = m.group(1)
                if 'access:private' in s:
                    #print "Skipping private {}".format(m.group(1))
                    continue
                if stupid_macro_regex.match(sym_name):
                    #print "Nein! {}".format(sym_name)
                    continue
                if intel_simd_regex.match(sym_name):
                    #print "Nein! {}".format(sym_name)
                    continue
                if detail_shit_regex.match(sym_name):
                    #print "Nein! {}".format(sym_name)
                    continue
                syms.add(sym_name)
        file_to_syms[filename] = syms
        return syms

visited_includes_syms = {}
def get_visible_syms(filename):
    path = os.path.normpath(filename)
    res = copy.copy(get_file_syms(path))
    for i in get_includes_recursive(path):
        try:
            inc_syms = visited_includes_syms[i]
        except KeyError:
            inc_syms = visited_includes_syms[i] = get_file_syms(i)
        res.update(inc_syms)
    return res

parser = argparse.ArgumentParser(description='Symbols visibility analyzer')
parser.add_argument('--show-visible', dest='show_visible', help='AZAZA')

args = parser.parse_args()
if args.show_visible:
    for s in get_visible_syms(args.show_visible):
        print s
else:
    results = defaultdict(lambda: 0)

    for root, dirs, files in os.walk('.'):
        for f in files:
            if f.endswith('.cpp') or f.endswith('.c') or f.endswith('.hpp') or f.endswith('.h'):
                path = os.path.normpath("{}/{}".format(root, f))
                results[path] = len(get_visible_syms(path))

    results_list = sorted(results.items(), key=lambda x: x[1], reverse=True)
    for r in results_list:
        print "{: 5d} {}".format(r[1], r[0])
