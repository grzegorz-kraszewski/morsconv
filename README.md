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
* `DOT` – string to be used for Morse code short symbol (dot). Default is ".".
* `DASH` – string to be used for Morse code long symbol (dash). Default is "-".

### `count`
Counts all the elements of Morse code for given text. Prints 5 integer numbers separated by spaces and ended by a newline. Meaning of these numbers is as follows:
1. Number of dots.
2. Number of dashes.
3. Number of symbol pauses (inside a character).
4. Number of character pauses (between characters in a word).
5. Number of word pauses (between words).
   
### `8svx`
Generates and writes an audio file in IFF 8SVX format (8-bit, mono). Supported output control arguments:
* `TO` – path to the output file (with filename). This argument is required.
* `SAMPLERATE` = `SR` – sampling rate of the audio in Hz. Allowed range is from 1000 to 65535. Default value is 8000. 
* `WPM` – speed of the code in words per minute (based on PARIS). Allowed range is 5 to 100 including. Default value is 20.
* `PITCH` – tone pitch in Hz. Allowed range is from 100 to 8000. Default value is 500. Note that sampling rate specified must be at least twice the tone pitch. Sampling theorem is not fulfilled otherwise and such combination of parameters is rejected.
> [!TIP]
> While for pure sine wave sampling rate two times higher than the wave frequency is enough, 8-bit quantized sine wave is no more pure. Low sampling rates and high tone frequencies create unpleasant alias tones. In practice using sampling rates 4 or more times higher than the tone frequency is recommnended. Note also that for sampling rate exactly twice the tone frequency, sine is sampled at zero crossings only, the result is silence.

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
```
> MorsConv "foo bar" MODE=8svx TO=RAM:code.8svx
```
Generates audio sampled at 8 kHz, at 20 wpm speed, using 500 Hz tone. Writes it to IFF 8SVX file `code.8svx` in RamDisk.
```
> MorsConv "foo bar" MODE=8SVX SR=44100 WPM=12 PITCH=440 TO=Work:Sounds/foobar.8svx
```
Generates audio sampled at 44.1 kHz at 12 wpm. Tone is 440 Hz. File is written to `foobar.8sfx` file located in `Work:Sounds` directory.
