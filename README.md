# morsconv
Modular converter of text to various forms of Morse code.

AmigaOS 3.x, M68000

## Usage
`MorsConv <text to convert> [MODE=<mode>] <output controls>`

Avaliable control arguments are listed in descriptions of output modules. Output module is selected with `MODE` argument, which takes output module name as value. Module names are case insensitive.
When `MODE` argument is absent, `con` module is the default.

## Available output modules
### `con`
Outputs Morse code as text to standard output, which is usually a shell window, `MorsConv` is started in. Using AmigaDOS redirection, output can be sent to a file, pipe or other device. Supported output control arguments:
* `DOT` specifies string to be used for Morse code short symbol (dot). Default is ".".
* `DASH` specifies string to be used for Morse code long symbol (dash). Default is "-".
### `count`
Counts all the elements of Morse code for given text. Prints 5 integer numbers separated by spaces and ended by a newline. Meaning of these numbers is as follows:
1. Number of dots.
2. Number of dashes.
3. Number of symbol pauses (inside a character).
4. Number of character pauses (between characters in a word).
5. Number of word pauses (between words).

Number of characters sent can be calculated as: *number of character pauses* + *number of word pauses* + 1. Note that skipped (not representable in Morse code) characters are not counted here. Note also that ASCII space does not count as Morse character (it is just word pause).
## Examples
```
> MorsConv "foo bar"
..-. --- ---   -... .- .-.
> MorsConv "foo bar" DOT=o DASH="="
oo=o === ===   =ooo o= o=o
> MorsConv "foo bar" DOT=di DASH=da
dididadi dadada dadada   dadididi dida didadi
> MorsConv MODE=count "foo bar"
9 10 13 4 1
```
