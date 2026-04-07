#ifndef THREAD_H
#define THREAD_H
#include <string.h>
#include "maxlist.h"
#include <stack>
using namespace std;
class Context;
class Thread {
private:
  stack<Context> contextStack;
  bool run;
  int line;
  int id;
  maxlist ThreadVARS;
  maxlist code;
public:
  Thread() {}
  Thread(int id, string file);
  string get_variable(string name);
  void set_variable(string name, string value, string type);
  bool is_running();
  int get_line_number();
  string get_line();
  int exec();
};
#endif
