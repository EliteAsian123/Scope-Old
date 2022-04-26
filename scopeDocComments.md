![Scope Logo](logo.svg)

*Scope* has a simple way of defining documentation for functions, variable, classes, etc. Start off by creating a comment starting with `/%` and ending with `%/` before the thing you want to document.
<pre>
<b>/%
The comment about this awesome function!
%/</b>
func float myAwesomeFunction(float a, bool b) {
    ret 3.1415f;
}
</pre>
> The comment about this awesome function!

## References

If you would like to reference a class, function, argument, etc., you can use the `@"abc.zyx"` syntax.
<pre>
<b>/%
The comment about this awesome function! The @"a" controls the awesomeness!
%/</b>
func float myAwesomeFunction(float a, bool b) {
    ret 3.1415f;
}
</pre>
> The comment about this awesome function! The `a` controls the awesomeness!

You can also reference other functions in the same way.
<pre>
func bool isItAwesome(string x, int y, bool z) {
    ret true;
}

<b>/%
The comment about this awesome function! The @"a" controls the awesomeness!
Make sure you use @"isItAwesome" first, or it could throw an error...
%/</b>
func float myAwesomeFunction(float a, bool b) {
    ret 3.1415f;
}
</pre>
> The comment about this awesome function! The `a` controls the awesomeness!<br/>
> Make sure you use `bool isItAwesome(string, int, ...)` first, or it could throw an error...

If an arugment and a function have the same name, simple use a `.` to differentiate them.
<pre>
func bool isItAwesome(string x, int y, bool z) {
    ret true;
}

<b>/%
This function does some tricky math to figure out if something is awesome.
@"isItAwesome" is the thing to test!
Make sure you use @".isItAwesome" first.
%/</b>
func bool myAwesomeFunction(bool isItAwesome) {
    ret !isItAwesome;
}
</pre>
> This function does some tricky math to figure out if something is awesome.<br/>
> `isItAwesome` is the thing to test!<br/>
> Make sure you use `bool isItAwesome(string, int, ...)` first.

## Argument Descriptions

If you want to descibe how arguments can be used, use this syntax:
<pre>
<b>/%
The comment about this awesome function!
<br/>
@"a": The awesomeness!<br/>
@"b": The awesome switch!
%/</b>
func float myAwesomeFunction(float a, bool b) {
    ret 3.1415f;
}
</pre>
> The comment about this awesome function!<br/><br/>
> `float a`: The awesomeness!<br/>
> `bool b`: The awesome switch!

All invalid argument names will be discarded completely.

## Markdown

Simple markdown *will* work in these comments!
<pre>
<b>/%
PI is **awesome**! Do *you* agree?
~~Pie stinks though.~~ `Pie is awesome!`
%/</b>
float pi = 3.1415f;
</pre>
> PI is **awesome**! Do *you* agree?<br/>
> ~~Pie stinks though.~~ `Pie is awesome!`

## Tags

Tags are stated at the top of the documentation comment and describe simple things like deprecation and unsafeness. Tags can optionally have short descriptions after them.

<pre>
<b>/%
@deprecated: Use @"fullPi" instead
@unsafe
@complexity: O(log(n))

PI up to 4 digits.
%/</b>
float pi = 3.1415f;
</pre>
> *Deprecated:* Use `fullPi` instead<br/>
> *Unsafe*<br/>
> *Complexity:* `O(log(n))`<br/><br/>
> PI up to 4 digits.

Here is a list of tags. All invalid tags will be complete discarded.

- `@deprecated`: States that the object is deprecated/obsolete.
- `@unsafe`: States that the object is not safe and shouldn't be used if security is a concern.
- `@unoptimized`: States that the function is slow and *definitely* shouldn't be used if speed is a concern.
- `@optimized`: States that the function is optimized and *should* be used if speed is a concern. This does not mean it should *always* be used.
- `@bug`: States that the object has a bug/issue associated with it. Use with `@unsafe` if it can cause a security concern.
- `@complexity`: States the "Big O notation" of the function.
- `@approx`: Marks the function as being an approximation, meaning the result could be slightly off from the real value.