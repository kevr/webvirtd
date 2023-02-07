if not get_option('disable_tests')
  views_domains_test = executable(
    'domains.test',
    'domains.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('views domains test', views_domains_test)
endif
