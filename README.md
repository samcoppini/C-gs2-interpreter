# gs2 interpreter

This repo is for a work-in-progress gs2 interpreter written in C++.

## Building

Assuming you have [Meson](https://mesonbuild.com/), [Ninja](https://ninja-build.org/) and a compiler with C++17 support, the gs2 interpreter can built like so:

```sh
$ meson build --prefix=$(pwd)/dist
$ ninja -C build install
# The interpreter should now be installed at ./dist/bin/gs2
```

## Implementation progress

This interpreter is currently unfinished. Of gs2's many, many commands, only the following are implemented:

| Byte | Name | Description |
|--------|--------------------|-------------------------|
| `00` | nop | Does nothing |
| `01` | push unsigned byte | Pushes an unsigned 8-bit number to the stack |
| `02` | push signed short | Pushes a signed 16-bit number to the stack |
| `03` | push signed long | Pushes a signed 32-bit number to the stack |
| `04` | string | Starts a series of string |
| `05` | regular string | Pushes a series of strings to the stack |
| `06` | array string | Pushes an array of strings to the stack |
| `07` | single char string | Pushes a single character string to the stack |
| `08` | { | Opens a block |
| `09` | } | Closes a block |
| `0a` | new-line | Pushes a string containing a single newline to the stack |
| `0b` | empty-list | Pushes an empty list to the stack |
| `0c` | empty-block | Pushes an empty block to the stack |
| `0d` | space | Pushes a string containing a single space to the stack |
| `10` - `1a` | push small number | Pushes the numbers `0`-`10` to the stack |
| `1b` | 100 | Pushes the number `100` to the stack |
| `1c` | 1000 | Pushes the number `1000` to the stack |
| `1d` | 16 | Pushes the number `16` to the stack |
| `1e` | 64 | Pushes the number `64` to the stack |
| `1f` | 256 | Pushes the number `256` to the stack |
| `20` | negate / reverse / eval | Pops the stack, then based on the value, do the following:<br/>**number**: negate the number<br/>**list**: reverse the list<br/>**block**: execute the block |
| `2a` | double / lines | Pops the stack, then based on the value, do the following:<br/>**number**: double the number and push it to the stack<br/>**list**: split a string on newlines, and push the list of strings to the stack |
| `30` | add / catenate | Pops the top two values of the stack, then depending on the types, do the following:<br/> **two numbers**: push the sum of the two numbers<br/> **two lists**: pushes the combined lists<br/>**two blocks**: pushes the combined blocks<br/>**list and block/number**: adds the block/number to the list and push it to the stack |
| `56` | read-num | Pops the stack, then push a number read from the string that was popped |
| `57` | read-nums | Pops the stack, then push a list of numbers read from the string that was popped |

All other commands are not yet implemented.
