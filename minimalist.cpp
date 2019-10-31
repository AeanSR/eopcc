def foo(s){
  vector[s] a;
  vector[s] b;
  vector[s] c;
  vector[sizeof(c)/2] d;
  extern vector[10] x;
 
  x = a;
  c = (a + b) + (d = c + d);
}

int s = 0;
foo(s);
foo(20);
