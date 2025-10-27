import os
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

env['ENV']['CCACHE_BASEDIR'] = env.subst('$PROJECT_DIR')
ccache_dir = os.path.join(env.subst('$PROJECT_CACHE_DIR'), 'ccache')
os.makedirs(ccache_dir, exist_ok=True)

env['ENV']['CCACHE_DIR'] = ccache_dir

if env['CXX'] and 'ccache' not in env['CXX']:
    env['CXX'] = 'ccache ' + env['CXX']

if env['CC'] and 'ccache' not in env['CC']:
    env['CC'] = 'ccache ' + env['CC']
