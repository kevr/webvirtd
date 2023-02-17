if not get_option('disable_tests')
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

  util_test = executable(
    'util.test',
    'util.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('util test', util_test)
endif
