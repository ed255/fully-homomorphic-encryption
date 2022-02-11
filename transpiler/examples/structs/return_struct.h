#ifndef TRANSPILER_EXAMPLES_STRUCTS_RETURN_STRUCT_H_
#define TRANSPILER_EXAMPLES_STRUCTS_RETURN_STRUCT_H_

// Very basic example demonstrating the ability to specify structs as return
// values.
struct Embedded {
  short a;
  char b;
  int c;
};

struct ReturnStruct {
  char a;
  Embedded b;
  char c;
};

// Builds a ReturnStruct out of the argument values and returns it.
ReturnStruct ConstructReturnStruct(char a, Embedded b, char c);

#endif  // TRANSPILER_EXAMPLES_STRUCTS_RETURN_STRUCT_H_
