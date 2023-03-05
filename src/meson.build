flags = [
  '-isystem', root + '/src',
  '-pthread',
]
test_flags = ['-DTEST_BUILD']

conf_data = configuration_data()
conf_data.set('version', get_option('version'))
configure_file(input : 'version.hpp.in',
               output : 'version.hpp',
               configuration : conf_data)

sources = [
  'app.cpp',
  'syscall.cpp',
  'libvirt.cpp',
  'util/config.cpp',
  'util/json.cpp',
  'util/logging.cpp',
  'util/signal.cpp',
  'util/util.cpp',
  'views/domains.cpp',
  'views/host.cpp',
  'data/domain.cpp',
  'data/host.cpp',
  'virt/events/lifecycle.cpp',
  'virt/events/callbacks/lifecycle.cpp',
  'virt/event_callback.cpp',
  'virt/events.cpp',
  'virt/event.cpp',
  'virt/network.cpp',
  'virt/domain.cpp',
  'virt/connection_pool.cpp',
  'virt/connection.cpp',
  'virt/util.cpp',
  'ws/pool.cpp',
  'ws/client.cpp',
  'ws/connection.cpp',
  'http/router.cpp',
  'http/middleware.cpp',
  'http/server.cpp',
  'http/client.cpp',
  'http/connection.cpp',
  'http/io_context.cpp',
  'http/util.cpp',
  'thread/worker_pool.cpp',
  'thread/worker.cpp',
  'stubs/io_context.cpp',
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

  app_test = executable(
    'app.test',
    'app.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('app test', app_test)
endif

subdir('util')
subdir('virt')
subdir('http')
subdir('views')
