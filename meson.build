project('webvirtd', 'cpp',
  version : '0.1.0',
  default_options : [
    'warning_level=3',
    'cpp_std=c++17',
])

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')
gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')
gmock_main_dep = gtest_proj.get_variable('gmock_main_dep')

jsoncpp_proj = subproject('jsoncpp')
jsoncpp_dep = jsoncpp_proj.get_variable('jsoncpp_dep')

fmt_proj = subproject('fmt')
fmt_dep = fmt_proj.get_variable('fmt_header_only_dep')

pugixml_proj = subproject('pugixml', default_options : [
  'tests=false',
])
pugixml_dep = pugixml_proj.get_variable('pugixml_dep')

boost_dep = dependency(
  'boost',
  modules : ['program_options', 'system'],
  required : true
)

libvirt_dep = dependency(
  'libvirt',
  required : true
)

deps = [
  boost_dep,
  libvirt_dep,
  jsoncpp_dep,
  fmt_dep,
  pugixml_dep,
]

test_deps = [
  gtest_dep,
  gtest_main_dep,
  gmock_dep,
] + deps

subdir('src')

bash = find_program('bash')
root = meson.source_root()
filtered_lcov = root + '/scripts/filtered-lcov.sh'
build_dir = meson.current_build_dir()

run_target('filter-coverage',
  command : [
    bash, filtered_lcov, build_dir,
    '**/src/libvirt.cpp',
    '**/src/syscaller.cpp',
  ],
)
