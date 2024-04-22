使用 `make` 命令进行编译。

```sh
$ make mcc
```

生成词法分析符号

```sh
./mcc --emit-tokens example.c > tokens.txt
```

生成抽象语法树

```sh
./mcc --emit-ast example.c > ast.txt
```

生成语义检查信息

```sh
./mcc --emit-sema example.c > sema.txt
```

生成llvm ir

```sh
./mcc --emit-ir example.c > tmp.ll
```

编译ir并执行

```sh
clang tmp.ll -o tmp
./tmp
```
