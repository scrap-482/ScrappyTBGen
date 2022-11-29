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
opts.Add(EnumVariable('target', "Compile targets in debug or release mode", 'debug', ['debug', 'release']))
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'linux', 'osx']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", 'no'))

# Updates the environment with the option variables.
opts.Update(env)

# Generates help for the -h scons option.
Help("\nScrappyTBGen Compiler Options:\n---------------------------")
Help(opts.GenerateHelpText(env))

# Process some arguments
if env['use_llvm']:
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

# loads the json from the given configuration file
def read_json_cc_args(fname):
    if fname is None:
        print("ERROR: Please enter the path to your config directory.")
        quit()

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
    userspecargs = parse_json_cc_args(userspecs)
# ------- First, do the things that are common to all compiled targets ------- #
    print("Platform =",  env['platform'], ("(llvm)" if  env['use_llvm'] else ''))
    # Check our platform specifics
    if env['platform'] == "osx":
        if env['target'] == 'debug':
            env.Append(CCFLAGS = ['-g','-O2', '-arch', 'x86_64', '-std=c++17'])
            env.Append(LINKFLAGS = ['-arch', 'x86_64'])
        else:
            env.Append(CCFLAGS = ['-g','-O3', '-arch', 'x86_64', '-std=c++17'])
            env.Append(LINKFLAGS = ['-arch', 'x86_64'])

    elif env['platform'] == "linux":
        clargs = ['-fopenmp', '-std=c++20', '-O3']
        clargs.extend(userspecargs)
        env.Append(CCFLAGS = clargs)
        env.Append(LINKFLAGS = clargs)
        # if env['target'] == 'debug':
        #     env.Append(CCFLAGS = ['-fPIC', '-g3','-Og', '-std=c++17'])
        # else:
        #     env.Append(CCFLAGS = ['-fPIC', '-g','-O3', '-std=c++17'])

    elif env['platform'] == "windows":
        # This makes sure to keep the session environment variables on windows,
        # that way you can run scons in a vs 2017 prompt and it will find all the required tools
        env.Append(ENV = os.environ)

        env.Append(CCFLAGS = ['-DWIN32', '-D_WIN32', '-D_WINDOWS', '-W3', '-GR', '-D_CRT_SECURE_NO_WARNINGS'])
        if env['target'] == 'debug':
            env.Append(CCFLAGS = ['-EHsc', '-D_DEBUG', '-MDd'])
        else:
            env.Append(CCFLAGS = ['-O2', '-EHsc', '-DNDEBUG', '-MD'])

    # if env['target'] == 'debug':
    #     env.Append(CPPDEFINES=['DEBUG'])

    # Change this to choose which variant
    sources = []
    # Core source code
    srces = userspecs['SRC_DIRS']
    for src in srces:
        sources.extend(Glob(src + '/*.cpp')) 
    # Main file
    sources.extend(['src/retrograde_analysis/main.cpp'])

    env.Program(compiled_path + 'scrappytbgen', sources)

if env['platform'] == '':
    print("\nNo valid target platform selected. Try `scons platform=[platform]` or add a scons.config file.\nType `scons --help` for more parameters.\n")
else:
    compile()
