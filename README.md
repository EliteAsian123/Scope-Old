![Scope Logo](logo.svg)

*Scope* is an interpreted, object oriented programming language that I am in the process of creating. So far, it isn't even object oriented, but I am working on it. I promise.

```cpp
#insert Standard/Core.scope

println("Hello, World!");
```

## Cool Features

*Temporary variables are boring...*
```cpp
int a = 512;
int b = 256;
swap(a, b); // Swap them around!

println((string) a); // Prints "256"
println((string) b); // Prints "512"
```

*Function bloat...*
```cpp
func void funcWithinFunc() {
    // Only acessible within this function!
    func void theFunc() {
        println("Hello, World!");
    }
    
    theFunc();
    theFunc();
}

funcWithinFunc(); // Prints "Hello, World!" twice!
```

*I don't need `i`... (Not implemented yet)*
```cpp
// No more i!
repeat(5) {
    println("Hello, World!");
}
```

*Static classes are barely classes... (Not implemented yet)*

```cpp
// They aren't!

utility Wow {
    func void wow() {
        println("Wow!");
    }
}

utility {
    func void hello() {
        println("Hello, World!");
    }
}

Wow.wow(); // "Wow!"
hello(); // "Hello, World!"
```

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


## Credits

- [`unescape.c`](https://github.com/yasuoka/unescape/blob/master/unescape.c): Copyright (c) 2019 YASUOKA Masahiko <yasuoka@yasuoka.net>


