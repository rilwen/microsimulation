# SCons build script for Averisera microsimulation model.
# (C) Averisera Ltd 2014-2020.

import os
import SCons

print('Platform: ' + os.name)

# Get the mode flag from the command line.
mymode = ARGUMENTS.get('mode', 'debug')
Export('mymode')

arch = 'x64'
Export('arch')

# Only 'debug' or 'release' allowed.
if not (mymode in ['debug', 'release']):
   print("Error: expected 'debug' or 'release' for 'mymode' parameter, found: " + mymode)
   Exit(1)
   
print('**** Compiling in %s mode for architecture %s ****' % (mymode, arch))

# Extra compile flags for debug mode.
debugcflags = ['-g']
# Extra compile flags for release mode.
releasecflags = ['-O2', '-march=native', '-flto', '-DNDEBUG']

# Paths for Ubuntu LTS.
EIGEN_PATH = '/usr/include/eigen3/'
SACADO_PATH = '/usr/include/trilinos/'
MPI_PATH = '/usr/include/mpich/' # Or /usr/include/openmpi/.
system_include_paths = [('-isystem' + path) for path in [EIGEN_PATH, SACADO_PATH, MPI_PATH]] # no space after -isystem!
disabled_warnings = ['-Wno-unused-parameter']
boost_c_flags = ['-DBOOST_LOG_DYN_LINK']
compilation_options = ['-fno-strict-overflow', '-fdiagnostics-color']
enabled_warnings = ['-Wall', '-Werror', '-Wfatal-errors', '-Wpedantic', '-Wformat', '-Wextra', '-Wconversion']
c_flags = system_include_paths + compilation_options + enabled_warnings + disabled_warnings + boost_c_flags
linkflags = []
if arch == 'x64':
   arch_switch = '-m64'
else:
   arch_switch = '-m32'
c_flags.append(arch_switch)
linkflags.append(arch_switch)
flags = ["-std=c++14"] + c_flags
#home = os.path.expanduser('~')
if mymode == 'debug':
   BUILD_DIR = '%s/Debug' % arch
   flags += debugcflags
else:
   BUILD_DIR = '%s/Release' % arch
   flags += releasecflags
Export('BUILD_DIR')

cpp_path = ['#']#, os.path.join(SACADO_PATH, 'mpl')]
# needed by gcc-4.9.x
ar = 'gcc-ar'
ranlib = 'gcc-ranlib'
env = Environment(CPPPATH=cpp_path,
                  CXXFLAGS=flags,
                  CFLAGS=c_flags,
                  LINKFLAGS=linkflags,
                  AR=ar,
                  RANLIB=ranlib,
                  ENV={'PATH' : os.environ['PATH'],
                       'TERM' : os.environ['TERM'],
                       'HOME' : os.environ['HOME']}
)
Export('env')

# Linked libraries.
OTHER_LIBS = ['trilinos_teuchoscore', 'trilinos_kokkoscore', 'trilinos_teuchoscomm', 'boost_date_time', 'nlopt', 'f2c', 'boost_log', 'boost_system', 'pthread']
Export('OTHER_LIBS')

def call(subdir, name='SConscript'):
   return SConscript(os.path.join(subdir, name), variant_dir = os.path.join(subdir, BUILD_DIR), duplicate = 0)

# Build libraries.
core = call('core')
Export('core')
microsim_core = call('microsim-core')
Export('microsim_core')
microsim_simulator = call('microsim-simulator')
Export('microsim_simulator')
microsim_calibrator = call('microsim-calibrator')
Export('microsim_calibrator')
microsim_uk = call('microsim-uk')
Export('microsim_uk')

if mymode == 'debug':
   top_dir = Dir('#').abspath   
   env.Append(CXXFLAGS=['-isystem' + os.path.join(top_dir, 'googletest/include')])
   # Test tools.
   testing = call('testing')
   Export('testing')
   gtest = call('googletest', 'SConscript-gtest')
   Export('gtest')
   GTEST_LIBS = ['pthread']
   Export('GTEST_LIBS')
   # Build tests.
   call('tests')
   call('microsim-core-tests')
   call('microsim-calibrator-tests')
   call('microsim-simulator-tests')
   call('microsim-uk-tests')


# Integration test and Brexit model.
call('microsim-runners')