#!python
import os, subprocess

# This folder is where final products go
compiled_path = "./compiled/" 

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

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

# Main function of this script
def compile():
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
        env.Append(CCFLAGS = ['-fopenmp', '-std=c++2a'])
        env.Append(LINKFLAGS = ['-fopenmp', '-std=c++2a'])
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

    sources = []
    # Change this to choose which variant
    sources.extend(Glob('src/rules/chess/*.cpp'))
    # Core source code
    sources.extend(Glob('src/core/*.cpp'))
    sources.extend(Glob('src/utils/*.cpp'))
    # Main file
    sources.extend(['src/test.cpp'])

    env.Program(compiled_path + 'scrappytbgen', sources)

if env['platform'] == '':
    print("\nNo valid target platform selected. Try `scons platform=[platform]` or add a scons.config file.\nType `scons --help` for more parameters.\n")
else:
    compile()