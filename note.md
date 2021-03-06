
# note

- 言語名 `nospace`．空白を一切入れなくても成立するsyntaxを目指す．

# progress

### progress 1

print a number.

### progress 2

add two numbers and print the answer.

### progress 3

add some numbers and print the answer.

### progress 4

calculate(add/multiply) some numbers and print the answer.

### progress 5

calculate(add/sub/mul/div/mod) some numbers and print the answer.

 whitespace interpreter(ws.rb)の負数表現が間違っていたので(ws.rb)側の修正を含む．

### progress 6

calculate(you can use brackets) some statements and print the answers.

### progress 7

`-` as unary operation. `-(2+3)` is valid.

### progress 8

use a variable `abc`. `abc=5;abc+2;` is valid.

### progress 9

use a function `__xyz()`. `__xyz()` always return `999`. `__xyz();` is valid.

### progress 10

use some variables. `a=2;b=a+3;c=a*b;c*5;` is valid.

### progress 11

use build-in function `__puti` and `__putc`. `__puti(3+4);` says `7`.

### progress 12

use user-defined function. must define `main` function. all variables are global.
`func:main(){__puti(9);};` is valid. but `return` is invalid.
`func:f(){__puti(20);};func:main(){f();};` is valid.

### progress 13

must use variable declaration. `let:x;`. 

local variable. 

```
func:foo(){
    let:a;
    a=7;
    __puti(a);
}
func:main(){
    let:a;
    a=5;
    __puti(a);
    foo();
    __puti(a);
}
```

global variable.

```
let:a;
func:foo(){
    a=7;
}
func:main(){
    a=5;
    __puti(a);
    foo();
    __puti(a);
}
```

### progress 14

use `if` and `while` statement. 0 means false, otherwise means true.
`if(x-5){__puti(1);}`.

in global scope, you can descript `let`, `func`.
in func scope, you can descript `let`, expressions.
in statement scope(`if` `while`), you can descript expressions.


### progress 15

`else`, `elsif`.
`if(cond1){}elsif(cond2){}else{}`.


### progress 16

`__geti()`.


### progress 17

operation `<`, `==`. 0 means false, 1 means true.


### progress 18

operation `!=`, `<=`, `>`, `>=`.

- ラベルに関する致命的なバグ複数の修正など


### progress 19

reference, dereference. `&var`, `*var`.


### progress 20

function arguments. `func:foo(a1,a2,a3){}`.


### progress 21

return value. `func:succ(a){return:a+1;}`.


### progress 22

comment out. `# comment`


### progress 23

char literal `'a'` as integer. escape `\n`(LF), `\t`(TAB), `\s`(SPACE), `\'`, `\\`.
`__getc()`．
`__getiv(ptr)`, `__getcv(ptr)`. return value.

remove `__xyz()`


### progress 24

use array.`let:x[5];`. `x` means `x[0]`.
`(*p)[2]`.

invalid `*(&a[1]) = 1`. (特に必要ないかな)


### progress 25

initialize.
`let:a,b,c[3];`


### progress 26

operation `!`, `&&`, `||`

### progress 26.1

optimization `if (a<=b)`, `if (a>=b)`, `if (a!=b)`.


### progress 27

`let:a(1);`
`let:x[5](1,2,3,4,5);` as array.
`let:dp[5](0);` is valid.


### progress 28

operator `+=`,`-=`,`*=`,`/=`,`%=`



## feature

### progress x1

string. `"abc"`.
`let:str("abc");` equal to `let:str('a','b','c');`.


### progress x2

prototype
`func:f(a,b,c);`


### progress x3

struct
```
struct: MyPair {
    let: a;
    let: b;
}

let: p<= MyPair;

func:f(){
    p.a = 0;
}

```


