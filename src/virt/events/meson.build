if get_option('tests')
  virt_lifecycle_event_test = executable(
    'lifecycle.test',
    'lifecycle.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('virt lifecycle_event test', virt_lifecycle_event_test)
endif
