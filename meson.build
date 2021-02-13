project(
    'gs2-cpp',
    'cpp',
    version: '0.0.1',
    default_options: [
        'cpp_std=c++17',
        'warning_level=3',
        'werror=true',
    ]
)

boost_dep = dependency('boost')

catch2_proj = subproject('catch2')
catch2_dep = catch2_proj.get_variable('catch2_dep')

gs2_src = files(
    'src/block.cpp',
    'src/command.cpp',
    'src/commands.cpp',
    'src/gs2context.cpp',
    'src/list.cpp',
    'src/utils.cpp',
    'src/value.cpp',
)

gs2_inc = include_directories('inc')

gs2_lib = static_library(
    'gs2_lib',
    gs2_src,
    include_directories: gs2_inc,
    dependencies: [
        boost_dep,
    ],
)

gs2_dep = declare_dependency(
    link_with: gs2_lib,
    include_directories: gs2_inc,
    dependencies: [
        boost_dep,
    ],
)

gs2_exe = executable(
    'gs2',
    'src/main.cpp',
    dependencies: [
        gs2_dep,
    ],
    install: true,
)

subdir('test')
