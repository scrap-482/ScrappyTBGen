#!python
import os, subprocess
import json

# This folder is where final products go
compiled_path = "./compiled/" 

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
AddOption('--config_dir', dest='pathname', type='string', nargs=1, action='store', 
metavar='PATH', help='location of user-defined callbacks')
env = Environment(PATHNAME = GetOption('pathname'))
filename = env['PATHNAME']

cluster = False
AddOption('--enable_cluster', dest='cluster', type='string', nargs=0, action='store', 
metavar='CLUSTER', help='whether or not cluster implementation is used')
env = Environment(CLUSTER = GetOption('cluster'))
if(env['CLUSTER'] != None):
    cluster = True


# Define our options
opts.Add(BoolVariable('use2a', "Use C++2a instead of C++20", 'no'))

# Updates the environment with the option variables.
opts.Update(env)

# Generates help for the -h scons option.
Help("\nScrappyTBGen Compiler Options:\n---------------------------")
Help(opts.GenerateHelpText(env))

# loads the json from the given configuration file. Returns false if it failed.
def read_json_cc_args(fname):
    if fname is None:
        print("ERROR: Please enter the path to your config directory.")
        return False

    with open(fname) as f:
        load_dict = json.load(f)
    return load_dict

# this is not complete. some thing still need to be fully worked out but is a
# start to show how it should be implemented
def parse_json_cc_args(config_dict):
    cl_args = []
    for key, val in config_dict.items():
        if key == 'INCLUDE_HEADERS':
            for header in val:
                cl_args.append(str('-include' + header))
        elif key == 'NO_ROYALTY_PIECESET':
            nonroyalty_pieces = "{"
            for i, element in enumerate(val):
                if i != len(val) - 1:
                    nonroyalty_pieces += "'" + element + "', "
                else:
                    nonroyalty_pieces += "'" + element + "'};"
            cl_args.append('-D' + key + '=' + str(nonroyalty_pieces))
        elif key == 'ROYALTY_PIECESET':
            royalty_pieces = "{"
            for i, element in enumerate(val):
                if i != len(val) - 1:
                    royalty_pieces += "'" + element + "', "
                else:
                    royalty_pieces += "'" + element + "'};"
            cl_args.append('-D' + key + '=' + str(royalty_pieces))
        elif not (val is None) and key != 'SRC_DIRS':
            cl_args.append('-D' + key + '=' + str(val)) 
    return cl_args


# Main function of this script
def compile():
    userspecs = read_json_cc_args(filename)
    if not userspecs:
        print("\nNo config specified. Try `scons --config_dir=[path/to/config.json]`\nType `scons --help` for more parameters.\n")
        return False

    userspecargs = parse_json_cc_args(userspecs)
# ------- First, do the things that are common to all compiled targets ------- #

    clargs = ['-fopenmp', '-O3']
    if env['use2a']:
        clargs.extend(['-std=c++2a', '-fconcepts'])
    else:
        clargs.extend(['-std=c++20'])
    if cluster:
        clargs.extend(['-DMULTI_NODE'])
        env['CXX'] = 'mpic++'
        env['CC'] = 'mpicc'

    clargs.extend(userspecargs)
    env.Append(CCFLAGS = clargs)
    env.Append(LINKFLAGS = clargs)

    # Change this to choose which variant
    sources = []
    # Core source code
    srces = userspecs['SRC_DIRS']
    for src in srces:
        sources.extend(Glob(src + '/*.cpp')) 
    # Main file
    sources.extend(['src/retrograde_analysis/main.cpp'])
    # sources.extend(['src/test.cpp'])

    env.Program(compiled_path + 'scrappytbgen', sources)

# if env['platform'] == '':
# else:

compile()
