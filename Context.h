#ifndef CONTEXT_H
#define CONTEXT_H
class Thread;
#include <string>
#include "maxlist.h"
using namespace std;
class Context {
private:
  maxlist LocalVARS;
  int line;
  int t;
  maxlist Code;
public:
  Context() {}
  Context(int l, maxlist code, int thread);
  /*
    return values:
    0   - OK
    1   - Unmatched (
    2   - Unmatched )
    3   - Unmatched "
    4   - EOL
    5   - Undefined Access
    10  - Unknown Operator
    12  - Attempt to call non-function object
    13  - Attempt to access property of a primitive
    255 - Context returned
   */
  void set_variable(string name, string value, string type);
  string get_variable(string);
  int get_line_number();
  string get_line();
  void set_line(int l);
  Thread parent();
  void next_line();
  int run();
};
#endif
