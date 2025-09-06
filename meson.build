project('Imavromage', 'cpp', version: '0.0.1', default_options: ['cpp_std=c++23'])

zlib_dep = dependency('zlib', required: true)
def_dep = dependency('libdeflate', required: true)

subdir('src')
subdir('cli')
subdir('tests')

install_subdir('include', install_dir: get_option('includedir'))
