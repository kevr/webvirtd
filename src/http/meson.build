
if not get_option('disable_tests')
  server_test = executable(
    'server.test',
    'server.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('http server test', server_test)
endif
