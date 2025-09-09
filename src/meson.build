src_files = [
  'ivmg.cpp', 
  'codecs/pam.cpp', 
  'codecs/png.cpp', 
  'codecs/qoi.cpp', 
  'core/features.cpp',
  'core/image.cpp'
]

ivmg_lib = library(
  'ivmg',
  sources: src_files,
  include_directories: include_directories('.', 'codecs', '../include'),
  dependencies: [def_dep],
  install: true,
  version: meson.project_version(),
  cpp_args: get_option('buildtype') == 'debug' ? ['-DDEBUG', '-march=native'] : ['-O3', '-march=native'],
)
