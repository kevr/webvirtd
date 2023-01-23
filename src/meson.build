flags = ['-pthread']

libwebvirtd = static_library(
  'libwebvirtd',
  'syscaller.cpp',
  'util.cpp',
  'http/processor.cpp',
  'http/handlers.cpp',
  'http/io_service.cpp',
  'stubs/io_service.cpp',
  dependencies : deps,
)
webvirtd_dep = declare_dependency(
  link_with : [libwebvirtd],
)
deps = [webvirtd_dep] + deps
test_deps = [webvirtd_dep] + test_deps

executable('webvirtd',
           'main.cpp',
           dependencies : deps,
           install : true)

main_test = executable(
  'main.test',
  'main.test.cpp',
  dependencies : test_deps,
  cpp_args : flags,
)
test('main test', main_test)

util_test = executable(
  'util.test',
  'util.test.cpp',
  dependencies : test_deps,
  cpp_args : flags,
)
test('util test', util_test)

subdir('http')
