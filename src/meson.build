src_files = ['ivmg.cpp', 'codecs/pam.cpp', 'codecs/png.cpp', 'codecs/qoi.cpp']

ivmg_lib = library(
  'ivmg',
  sources: src_files,
  include_directories: include_directories('.', 'codecs', '../include'),
  dependencies: [zlib_dep],
  install: true,
  version: meson.project_version(),
  cpp_args: get_option('buildtype') == 'debug' ? ['-DDEBUG'] : ['-O3'],
)
