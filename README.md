# gs2 interpreter

This repo is for a work-in-progress gs2 interpreter written in C++.

## Building

Assuming you have [Meson](https://mesonbuild.com/), [Ninja](https://ninja-build.org/), [Boost](https://www.boost.org/) and a compiler with C++17 support, the gs2 interpreter can built like so:

```sh
$ meson build --prefix=$(pwd)/dist
$ ninja -C build install
# The interpreter should now be installed at ./dist/bin/gs2
```
