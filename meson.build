project('webvirtd', 'cpp',
  version : '0.0.0',
  default_options : [
    'warning_level=3',
    'cpp_std=c++17',
  ]
)

# Configure the version via -Dversion
message('Dynamic version: ' + get_option('version'))

# Google Test
gtest_dep = dependency('gtest', main : true, required : false)
if not gtest_dep.found()
  gtest_proj = subproject('gtest')
  gtest_dep = [
    gtest_proj.get_variable('gtest_dep'),
    gtest_proj.get_variable('gtest_main_dep')
  ]
endif

# Google Mock
gmock_dep = dependency('gmock', main : true, required : false)
if not gmock_dep.found()
  gmock_dep = [
    gtest_proj.get_variable('gmock_dep'),
    gtest_proj.get_variable('gmock_main_dep')
  ]
endif

# Third-party JSON library
jsoncpp_dep = dependency('jsoncpp', required : false)
if not jsoncpp_dep.found()
  jsoncpp_proj = subproject('jsoncpp', default_options : [
    'default_library=static',
  ])
  jsoncpp_dep = jsoncpp_proj.get_variable('jsoncpp_dep')
endif

# Third-party formatting library
fmt_dep = dependency('fmt', required : false)
if not fmt_dep.found()
  fmt_proj = subproject('fmt', default_options : [
    'default_library=static',
  ])
  fmt_dep = fmt_proj.get_variable('fmt_header_only_dep')
endif

# Third-party XML processing library
pugixml_dep = dependency('pugixml', required : false)
if not pugixml_dep.found()
  pugixml_proj = subproject('pugixml', default_options : [
    'tests=false',
    'default_library=static',
  ])
  pugixml_dep = pugixml_proj.get_variable('pugixml_dep')
endif

# Boost library provided by user
boost_dep = dependency(
  'boost',
  modules : ['program_options', 'system'],
  required : true
)

# Libvirt library provided by user
libvirt_dep = dependency(
  'libvirt',
  required : true
)

# Base dependencies for all tests and binaries
base_deps = [
  boost_dep,
  jsoncpp_dep,
  fmt_dep,
  pugixml_dep,
]

# Particular dependencies for executables
deps = base_deps + [libvirt_dep]

# Particular dependencies for tests
test_deps = [
  gtest_dep,
  gmock_dep,
] + base_deps

# Produce various variables we need in derivative build configs
bash = find_program('bash')
root = meson.source_root()
filtered_lcov = root + '/scripts/filtered-lcov.sh'
build_dir = meson.current_build_dir()

# Process 'src' directory
subdir('src')

# Meson build configured webvirtd.service
service_data = configuration_data()
service_data.set('prefix', get_option('prefix'))
configure_file(input : 'res/webvirtd.service.in',
               output : 'webvirtd.service',
               configuration : service_data,
               install_dir : 'lib/systemd/system')

meson.add_install_script('scripts/install-user.sh')

# A custom run_target which filters HTML coverage to exclude
# some code unreachable by tests
run_target('filter-coverage',
  command : [
    bash, filtered_lcov, build_dir,
    '**/src/libvirt.cpp',
    '**/src/stubs/libvirt.cpp',
    '**/src/syscall.cpp',
  ],
)
