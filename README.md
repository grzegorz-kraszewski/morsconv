# morsconv
Modular converter of text to various forms of Morse code.

AmigaOS 3.x, M68000

## Usage
`MorsConv <text to convert> <output controls>`

Avaliable control arguments are listed in descriptions of output modules. Current version uses `con` module by default.

## Available output modules
### `con`
Outputs Morse code as text to standard output, which is usually a shell window, `MorsConv` is started in. Using AmigaDOS redirection, output can be sent to a file, pipe or other device. Supported output control arguments:
* `DOT` specifies string to be used for Morse code short symbol (dot). Default is ".".
* `DASH` specifies string to be used for Morse code long symbol (dash). Default is "-".

## Examples
```
MorsConv "foo bar"
..-. --- ---   -... .- .-.
MorsConv "foo bar" DOT=o DASH="="
oo=o === ===   =ooo o= o=o
MorsConv "foo bar" DOT=di DASH=da
dididadi dadada dadada   dadididi dida didadi
```
