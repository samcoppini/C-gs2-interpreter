# Implementation Progress

This interpreter does not yet support all of the commands in gs2. Below is a table of which commands are supported, with a description of what they do.

In the "Stack before" and "Stack after", the following notation is used:
* *α*, *β*, and *γ* represent any stack value. 
* *κ* represents a constant
* *A*-*Z* represent blocks
* *a*-*z* represent integers
* {*α*} and {*β*} represent lists

| Byte | Name | Stack before | Stack after | Description |
|------|------|--------------|-------------|-------------|
| `00` | nop | | | Does nothing |
| `01` | push unsigned byte | | *a* | Pushes an unsigned 8-bit number to the stack. |
| `02` | push signed short | | *a* | Pushes a signed 16-bit number to the stack. |
| `03` | push signed long | | *a* | Pushes a signed 32-bit number to the stack. |
| `04` | string | N/A | N/A | Begins a string command. Multiple strings are allowed in the same command, delimited by `07`. What happens to the strings is based on end byte, which may be `05`, `06`, `9b`, `9c`, `9d`, `9e` or `9f`. |
| `05` | regular string | | {*abc*}, {*def*}, ... | Pushes a series of strings to the stack. | 
| `06` | array string | | {{*abc*}, {*def*}, ...} | Pushes an array of strings to the stack. |
| `07` | single char string | | {*a*} | Pushes a single character string to the stack. |
| `08` / `09` | block | | *A* | `08` opens a block, `09` can optionally close the block. This just pushes a block to the stack. |
| `0a` | new-line | | *κ* | Pushes a string containing only a newline character. |
| `0b` | empty-list | | *κ* | Pushes an empty list to the stack. |
| `0c` | empty-block | | *κ* | Pushes an empty block to the stack. |
| `0d` | space | | *κ* | Pushes a string containing only a space character. |
| `10` - `1a` | push small number | | *κ* | Pushes the numbers 0 through 10 to the stack. | 
| `1b` | 100 | | *κ* | Pushes 100 to the stack. |
| `1c` | 1000 | | *κ* | Pushes 1000 to the stack. |
| `1d` | 16 | | *κ* | Pushes 16 to the stack. |
| `1e` | 64 | | *κ* | Pushes 64 to the stack. |
| `1f` | 256 | | *κ* | Pushes 256 to the stack. |
| `20` | negate | *a* | -*a* | Negates a number. |
| `20` | reverse | {*α*, *β*, *γ*} | {*γ*, *β*, *α*} | Reverses a list. |
| `20` | eval | *A* | | Evaluates a block. |
| `21` | bnot | *a* | ~*a* | Bitwise negates a two's complement integer. |
| `21` | head | {*α*, *β*, *γ*} | *α* | Extracts the first element of a list. |
| `22` | not | *a* | *b* | Pushes 1 if the number is 0, or 0 otherwise. |
| `22` | tail | {*α*, *β*, *γ*} | {*β*, *γ*} | Removes the first element of a list. |
| `23` | abs | *a* | \|*a*\| | Pushes the absolute value of a number. |
| `23` | init | {*α*, *β*, *γ*} | {*α*, *β*} | Removes the last element of a list. |
| `24` | digits | *a* | {*b*, *c*, ...} | Converts a number to a list of its (base-10) digits. e.g. 123 becomes {1, 2, 3}. |
| `24` | last | {*α*, *β*, *γ*} | *γ* | Extracts the last element of a list. |
| `2a` | double | *a* | 2*a* | Multiplies a number by 2. |
| `2a` | lines | {*α*, *β*, *γ*, ...} | {{*α*, *β*, *γ*} ...} | Splits a list into sublists, breaking at each newline character. |
| `2e` | range | *a* | { 0, 1, ..., *a* - 1} | Pushes a list of the numbers 0 through *a* - 1. |
| `2e` | length | { *α*, ... } | *a* | Pushes the length of a list. |
| `2f` | range1 | *a* | { 1, ..., *a* } | Pushes a list of the numbers 1 through *a*. |
| `30` | add | *a* *b* | *c* | Adds two numbers together |
| `30` | catenate | *A* *B* | *AB* | Combines two blocks | 
| `30` | catenate | {*α*, ...} {*β*, ...} | {*α*, ..., *β*, ...} | Combines two lists. | 
| `30` | catenate | *α* {*β*, ...} |  {*α*, *β*, ...} | Adds an element to the beginning of a list. |
| `30` | catenate |  {*α*, ...} *β* |  {*α*, ..., *β*} | Adds an element to the end of a list. |
| `34` | mod | *a* *b* | *c* | Perform a modulo operation |
| `34` | step | {*α*, *β*, ...} *n* | {*γ*, ...} | Take every *n*th element of a list. |
| `34` | clean-split | {*α*, ...} {*β*, ...} | {{*γ*, ...}, ...} | Splits a list {*α*, ...} into sublists, breaking at occurrences of {*β*, ...}, removing empty lists. |
| `34` |  map | {*α*, ...} *A* | {*β*, ...} | Apply *A* to each member of a list. |
| `40` | dup |  *α* |  *α* *α* | Duplicates the top element of the stack. |
| `41` | dup2 | *α* *β* |  *α* *β*  *α* *β* | Duplicates the top two elements of the stack. |
| `50` | pop | *α* | | Removes the top element of the stack. |
| `51` | pop2 | *α* *β* | | Removes the top two elements of the stack. |
| `52` | show | *α* | {*a*, *b*, ...} | Converts an element to a string. |
| `54` | show-lines | {*α*, *β*, ...} | {*a*, *b*, ..., *κ*, *c*, *d*, ...} | Converts every element of a list to a string, and combine the strings with a newline. |
| `55` | show-words | {*α*, *β*, ...} | {*a*, *b*, ..., *κ*, *c*, *d*, ...} | Converts every element of a list to a string, and combine the strings with a space. |
| `56` | read-num | {*a*, *b*, ...} | *c* | Finds the first integer in a string, and push it to the stack. |
| `57` | read-nums | {*a*, *b*, ...} | {*c*, *d*, ...} | Finds all of the integers in a string, and push the list of numbers to the stack. |
| `58` | show-line | *α* | {*a*, *b*, ..., *κ* } | Converts a value to a string, and append a newline. |
| `59` | show-space | *α* | {*a*, *b*, ..., *κ* } | Converts a value to a string, and append a space. |
| `64` | even | *a* | *b* | Pushes 1 if the number is even, or 0 otherwise. |
| `64` | sum | {*a*, *b*, ...} | *c* | Pushes the sum of the numbers in a list. |
| `65` | odd | *a* | *b* | Pushes 1 if the number is odd, or 0 otherwise. |
| `65` | product | {*a*, *b*, ...} | *c* | Pushes the product of the numbers in a list. |
| `84` | uppercase-alphabet | | *κ* | Pushes a string of all the uppercase letters. |
| `85` | lowercase-alphabet | | *κ* | Pushes a string of all the lowercase letters. |
| `b2` | counter | | *a* | Pushes the number of times the counter command has been executed. |
