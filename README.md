<img src="img/Logo.svg" alt="Scope Logo"/>

> **Warning**
>
> What started off as a small project has become something bloated and badly-written.
> I am in the process of re-writing Scope, so until then, be aware of bugs!

*Scope* is an interpreted, object oriented programming language that I am in the process of creating.

<img src="img/HelloWorld.png" alt="Hello World in Scope" width="332"/>

## Cool Features

*Temporary variables are boring...*

<img src="img/Swap.png" alt="Hello World in Scope" width="453"/>

*Function bloat...*

<img src="img/Func.png" alt="Hello World in Scope" width="604"/>

*I don't need `i`...* (temporarily disabled)

<img src="img/Repeat.png" alt="Hello World in Scope" width="382"/>

*Initilizing an array is annoying...*

<img src="img/With.png" alt="Hello World in Scope" width="678"/>

## Installation

```bash
# Clone!
$ git clone https://github.com/EliteAsian123/Scope.git

# If you don't have these installed
$ sudo apt install gcc
$ sudo apt install flex
$ sudo apt install bison
```

## Building

```bash
# These warnings are normal
$ sh build.sh
src/scope.y: warning: 1 shift/reduce conflict [-Wconflicts-sr]
src/output/scope.tab.c: In function ‘yyparse’:
src/output/scope.tab.c:1486:16: warning: implicit declaration of function ‘yylex’ [-Wimplicit-function-declaration]
 1486 |       yychar = yylex ();
      |                ^~~~~
```

## Testing

```bash
$ sh tests.sh
Testing `Tests/Arrays.scope`
Testing `Tests/Floats.scope`
Testing `Tests/For.scope`
...
Done!
```
