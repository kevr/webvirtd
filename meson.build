project('webvirtd', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

boost_dep = dependency(
  'boost',
  modules : ['program_options', 'system'],
  required : true
)

subdir('src')
