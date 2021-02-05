tests_src = files(
    'block-tests.cpp',
    'catch-main.cpp',
)

gs2_test = executable(
    'gs2-tests',
    tests_src,
    dependencies: [
        catch2_dep,
        gs2_dep,
    ],
)

test(
    'gs2-tests',
    gs2_test,    
)
