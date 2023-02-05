flags = ['-pthread']
test_flags = ['-DTEST_BUILD']

libwebvirtd = static_library(
  'webvirtd',
  'app.cpp',
  'syscaller.cpp',
  'util.cpp',
  'json.cpp',
  'config.cpp',
  'libvirt.cpp',
  'virt/connection.cpp',
  'virt/util.cpp',
  'views/domains.cpp',
  'http/router.cpp',
  'http/middleware.cpp',
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

if not get_option('disable_bin')
  executable('webvirtd',
             'main.cpp',
             dependencies : deps,
             cpp_args : flags,
             install : true)
endif

if not get_option('disable_tests')
  main_test = executable(
    'main.test',
    'main.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('main test', main_test)

  util_test = executable(
    'util.test',
    'util.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('util test', util_test)

  app_test = executable(
    'app.test',
    'app.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('app test', app_test)

  config_test = executable(
    'config.test',
    'config.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('config test', config_test)

  json_test = executable(
    'json.test',
    'json.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('json test', json_test)
endif

subdir('http')
subdir('virt')
