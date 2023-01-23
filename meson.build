project('webvirtd', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')
gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')
gmock_main_dep = gtest_proj.get_variable('gmock_main_dep')

boost_dep = dependency(
  'boost',
  modules : ['program_options', 'system'],
  required : true
)

deps = [
  boost_dep,
]

test_deps = [
  gtest_dep,
  gtest_main_dep,
  gmock_dep,
] + deps

subdir('src')
