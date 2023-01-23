executable('webvirtd',
           'main.cpp',
           dependencies : [boost_dep],
           install : true)

util_test = executable(
  'util.test',
  'util.test.cpp',
  dependencies : test_deps,
  cpp_args : flags,
)
test('util test', util_test)

subdir('http')
