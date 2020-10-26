/*
    EOPFN: sample program of func-call in EOPC language.

    zhaoyongwei <zhaoyongwei@ict.ac.cn>
    2019, Aug. 6
*/

// ======================================================
// define a function. a function can be instantiated
// multiple times, in different contexts.
print "=== func basic ===================================";
def foo(a, b) { // type of args `a` and `b` depends.
  int c;        // you can declare new variables.
  int i;
  for (i = 0; i < 20; i++)
    c += b[0] * 2;
  print c;
  a[0][0] = c;     // modify data by reference.
}

vector[10][20] a;
vector[20][20] b;
vector[10][20] c;
vector[20] bias;

// you can call `foo` in an asynchronized manner.
// this will return a handler and you must hold it with
// an `except` variable definition.
except h = async foo(a, bias);

/*
  async foo(a, bias); // <---- discard the handler will
                               raise an exposed `rv-value`
                               type error.
*/

// when async-called, the processor may not execute the
// function immediately. it will record the call, and go
// further to see if there is any vector instructions to
// fill the vector execution unit, while the function is
// executing in the scalar execution unit.

mm c, b, a;  // <---- and here it is.

// the processor happily fill the `mm` instruction into
// its vector execution unit. and once the vector unit got
// enough instructions to keep itself busy, it will start
// the execution of async function call in another thread,
// called "except thread".

// now you cannot determine which one will finish earlier,
// `foo` or `mm`. you cannot use the result in `foo` until
// you checked its running state.

/*
  c *= 1 + a[0][0]; // <---- use of `a` before synchronize
                             will run into undeterministic
                             behavior.
*/

// you check the state of async calls by the handler it
// provided when you call it. with `await` statement, the
// processor blocks its main thread to wait the ending
// of its except thread.

await h; // synchronize. after this statement, `foo` is
         // guaranteed finished.

c *= 1 + a[0][0]; // <---- ok.

// you can also call a function in synchronized manner.

foo(a, bias); // <---- this will be executed in main thread,
              //       as if the function body is inlined
              //       into here.
// ======================================================

print "=== undet vector =================================";
// since the async call is executed simutaneously with
// main thread, only one of the two threads could use
// vector execution units.
// in practice, one never allocate vectors in async calls.

def bar() {
  vector[10] a; // <---- allocate vectors.
  a += 1;       // <---- vector instructions.
}

bar(); // <---- ok.
except h = async bar(); // <---- ok.
vector[10] a_on_main; // <---- will collide with `bar::a`!
//a_on_main -= 1; // <---- undeterministic behavior!
await h;

// ======================================================

print "=== lambda func ==================================";
// you can also do some tricky things with functions.
// eopc functions are more like lambdas!

int x = 0;

def buz(y) {
  def qux(z) {  // <---- nested function define.
    x += y * z; // <---- referencing out-scope variables.
    print z;
  }
  int i;
  for (i = 0; i < y; i++)
    qux(i);
}

buz(10);

// ======================================================

// no recursive support.

/*
  def recurse(j) {
    print j;
    if (j) recurse(j-1); // <---- but recursive call is not
  }                               supported, since we do not
                                  have a real call stack.
*/

// ======================================================

print "=== multi except =================================";
// async call cannot be nested. we have only two threads,
// there are no third thread to support except in except.

def func_contains_exception() {
  except h = async buz(x); // <-- ok.
  await h;
}

func_contains_exception(); // <-- ok.

/*
  except h = async func_contains_exception(); // <-- error.

//  121:12:note: in instantiation of func:func_contains_exception()
//  with arguments []
//      except h = async func_contains_exception();

//  115:14:error: cannot make async function calls from another
//  async function call.
//      except h = async buz(x);

*/

// but continous async calls is ok.

except h1 = async buz(12);
except h2 = async buz(13);
await h1; // <---- first issued async call finishes first.
await h2; //       but await on h2 first is ok. when `await h2`
          //       returns, following `await h1` returns
          //       immediately.

// ========================================================
print "=== redef ========================================";
def redef_kernel() { print "kernel 1"; } // <---- first def, fine
def use_kernel() { redef_kernel(); }     // <---- call last one
use_kernel();
// print: kernel 1
def redef_kernel() { print "kernel 2"; } // <---- redef, fine but warned
// 165:1:warning: function definition hides previous defined symbol in current scope.
//         def redef_kernel() { print "kernel 2"; } // <---- redef, warning
//         ^
// 
// 161:1:note: previous defined from here:
//         def redef_kernel() { print "kernel 1"; } // <---- first def, fine
//         ^
use_kernel();
// print: kernel 2
redef redef_kernel() { print "kernel 3"; } // <---- redef, fine
use_kernel();
// print: kernel 3

// ================================================
print "=== __func__, __args__ ===========================";
def inner(a, b, c) {
  print a, b, c; // <-- use args
  print __args__; // <-- same.
  print __func__, __args__; // <-- pretty.
}
inner("a", 2, 3.5);

def outter(a, b, c) {
  inner(__args__); // <-- packed arg forward.
}
outter("b", 3, 4.5);

def bind(b, c) {
  inner("c", __args__); // <-- pack two args.
}
bind(4, 5.5);
