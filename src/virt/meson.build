if not get_option('disable_tests')
  virt_connection_test = executable(
    'connection.test',
    'connection.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('virt connection test', virt_connection_test)
endif
