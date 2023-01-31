
if not get_option('disable_tests')
  server_test = executable(
    'server.test',
    'server.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('http server test', server_test)

  client_test = executable(
    'client.test',
    'client.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('http client test', client_test)

  router_test = executable(
    'router.test',
    'router.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('http router test', router_test)
endif
