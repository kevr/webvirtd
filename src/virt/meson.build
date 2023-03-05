if not get_option('disable_tests')
  virt_connection_test = executable(
    'connection.test',
    'connection.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('virt connection test', virt_connection_test)

  virt_domain_test = executable(
    'domain.test',
    'domain.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('virt domain test', virt_domain_test)

  virt_event_test = executable(
    'event.test',
    'event.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('virt event test', virt_event_test)

  virt_util_test = executable(
    'util.test',
    'util.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('virt util test', virt_util_test)
endif

subdir('events')
