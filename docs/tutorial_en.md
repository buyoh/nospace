# Nospace Tutorial

## Hello World

```
func: main() {
    __putc('H');
    __putc('e');
    __putc('l');
    __putc('l');
    __putc('o');
    __putc('\n');
}
```

- Functions can be defined with `func`
- You must define the `main` function
- The `main` is executed
- `__putc()` outputs characters
- Functions starting with two underscores `_` are built-in functions
- `'a'` is an ASCII character
  - `\` is the escape character
  - Backslash is `\\`
  - Line feed (LF) `\n`
  - Space is `\s`
  - Tab is `\t`
  - `'` is `\'`

## Fibonacci

```
# calculate fibonacci(n)
func: fibo(n) {
    if (n < 0) {
        return: 0;
    }
    elsif (n == 0 || n == 1) {
        return: 1;
    }
    else {
        return: fibo(n-1) + fibo(n-2);
    }
}
func: main() {
    let: n;
    n = __geti();
    __puti(fibo(n));
    __putc('\n');
}
```

- Return a value from a function with `return`
  - Returns 0, if omitted
- `if`
- `elsif` instead of `else if`
- Start a line comment with `#`
- Declare variables with `let`
- Variables declared within a function are locally-scoped
- Variables declared outside a function are globally-scoped
- Variables with the same name cannot be declared in the same scope
  - Same for functions
- Read numbers with `__geti()`
  - The implementation varies by interpreter: for example, the reference
    Haskell interpreter uses line breaks to separate numbers

## Swap

```
func: swap(p, q) {
    let: t;
    t = *p;
    *p = *q;
    *q = t;
}
func: main() {
    let: a(1), b(2);
    swap(&a, &b);
    __puti(a);
    __puti(b);
}
```

- `&a` gets the address of the variable `a`
- `*p` dereferences `p`
- `let: a(1)` initializes the variable `a` to `1`

## Rotate Array

```
func:swap(p,q){let:t;t=*p;*p=*q;*q=t;}
func: rotate(begin, end) {
    end -= 1;
    while (begin < end) {
        swap(end - 1, end);
        end -= 1;
    }
}
func: main(){
    let: arr[4];
    arr[0] = __getc();
    arr[1] = __getc();
    arr[2] = __getc();
    arr[3] = __getc();
    rotate(&arr, &arr+4);
    __putc(arr[0]);
    __putc(arr[1]);
    __putc(arr[2]);
    __putc(arr[3]);
}
```

- `while`
- Arrays can be declared like `let:arr[4];`
  - The array size must be constant
  - A later update of the language may allow variable array sizes
- `arr[1]` refers to the element at index 1
- `arr` is synonymous with `arr[0]`
  - This differs from the C programming language
- The main feature of the language is that it requires no spaces, tabs,
  or line feeds

## Sorting (Quick Sort)

```
func: swap(p, q) {
    let: t;
    t = *p; *p = *q; *q = t;
}
func: qsort(begin, end) {
    if (end - begin <= 1) { return; }
    let: pv(begin), it(begin + 1);
    while (it < end) {
        if (*pv > *it) {
            swap(pv + 1, it);
            swap(pv, pv + 1);
            pv += 1;
        }
        it += 1;
    }
    qsort(begin, pv);
    qsort(pv+1, end);
}
func: main() {
    let: arr[9](3,1,4,1,5,9,2,6,5);
    qsort(&arr, &arr+9);
    let: i(0);
    while (i < 9) {
        __puti(arr[i]);
        __putc(' ');
        i += 1;
    }
}
```

- Arrays can also be initialized in a `let` declaration
- `a[3](1,2)` initializes `a[0]` to `1` and `a[1]` to `2`
  - `a[2]` is declared, but has an undefined value
- The initialization value does not need to be constant
