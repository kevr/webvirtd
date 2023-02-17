flags = ['-pthread']
test_flags = ['-DTEST_BUILD']

conf_data = configuration_data()
conf_data.set('version', get_option('version'))
configure_file(input : 'version.hpp.in',
               output : 'version.hpp',
               configuration : conf_data)

sources = [
  'app.cpp',
  'syscaller.cpp',
  'util.cpp',
  'json.cpp',
  'config.cpp',
  'libvirt.cpp',
  'signal.cpp',
  'logging.cpp',
  'views/domains.cpp',
  'views/host.cpp',
  'virt/network.cpp',
  'virt/domain.cpp',
  'virt/connection_pool.cpp',
  'virt/connection.cpp',
  'virt/util.cpp',
  'http/router.cpp',
  'http/middleware.cpp',
  'http/handlers.cpp',
  'http/io_service.cpp',
  'http/util.cpp',
  'stubs/io_service.cpp',
]

if not get_option('disable_tests')
  flags += ['-g']

  libwebvirtd_test = static_library(
    'webvirtd_test',
    'stubs/libvirt.cpp',
    sources,
    dependencies : deps,
    cpp_args : flags + test_flags,
  )

  webvirtd_test_dep = declare_dependency(
    link_with : [libwebvirtd_test],
  )

  test_deps = [webvirtd_test_dep] + test_deps
endif

if not get_option('disable_bin')
  libwebvirtd = static_library(
    'webvirtd',
    sources,
    dependencies : deps,
    cpp_args : flags,
  )

  webvirtd_dep = declare_dependency(
    link_with : [libwebvirtd],
  )

  deps = [webvirtd_dep] + deps
endif

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

  logging_test = executable(
    'logging.test',
    'logging.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('logging test', logging_test)

  signal_test = executable(
    'signal.test',
    'signal.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('signal test', signal_test)
endif

subdir('http')
subdir('virt')
subdir('views')
