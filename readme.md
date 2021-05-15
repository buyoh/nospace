
# Nospace

nospace to whitespace

nospace とは、esolang である whitespace を改行、タブ・半角スペース等の空白無しで記述することを目標にした言語及びコンパイラです。

whitespace よりは可読性がある謎の言語 nospace の記法については docs を参照してください。

勢いで書いたため、保守性が無い残念なソースコードになっています。

----

Nospace is a programming language that compiles to the Whitespace
esolang, but does not require any whitespace characters such as space,
tab, or line feed.

## Environment

- WSL
- ruby 2.4
- g++7 std=c++17

## Running

### Compile

`make`

### Test

`ruby test.rb`

## Documentation

See `./docs`.
