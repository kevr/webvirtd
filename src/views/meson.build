if get_option('tests')
  views_domains_test = executable(
    'domains.test',
    'domains.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('views domains test', views_domains_test)

  views_host_test = executable(
    'host.test',
    'host.test.cpp',
    dependencies : test_deps,
    cpp_args : flags + test_flags,
  )
  test('views host test', views_host_test)
endif
