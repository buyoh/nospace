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

- `func` で関数を定義出来る
- `main` 関数を定義しなければならない
- `main` 関数が呼ばれて実行される
- `__putc()` は文字を出力する
- `_` 2つから始まる関数は組み込み関数
- `'a'` はasciiコード
  - `\` はエスケープ文字
  - バックスラッシュは `\\`
  - 改行(LF)は `\n`
  - 半角スペースは `\s`
  - タブは `\t`
  - `'` は `\'`

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

- `return` で関数の返り値を指定出来る
  - 省略した場合は常に0
- `if`
- `else if` ではなく `elsif`
- 行コメントアウトは `#`
- 変数宣言は `let`
- ローカルスコープは関数のみ
- 関数内のどこでも `let` を使うことが出来る
- 関数外で`let`を使うとグローバルスコープになる
- 同一スコープで同じ名前の変数の宣言は出来ない
  - 関数も同様
- `__geti()` で数字を読み込む
  - whitespace interpreter側の実装依存でhaskell版では改行区切り

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

- `&a` は変数 `a` のアドレスを取得
- `*p` は `p` をデリファレンスする
- `let` 宣言の時 `a(1)` で変数 `a` を `1` に初期化する

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
- 配列の宣言は `let:arr[4];`
  - 配列サイズは定数のみ指定可能
  - アップデートで変数が指定可能になるかも？
- `arr[1]` で 1番目の要素を参照する
- `arr` は `arr[0]` と同義
  - C言語と異なる
- 空白改行タブを一切含めること無く記述できるのが本言語の下らない特徴

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

- 配列も`let` 宣言の時に初期化出来る
- `a[3](1,2)` で `a[0]` を `1`，`a[1]`を `2` に初期化する
  - `a[2]` は確保されるが未定値
- 初期化の値は定数でなくても良い
