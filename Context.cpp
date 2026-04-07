#include <string>
#include <stack>
#include "maxlist.h"
#include "Context.h"
using namespace std;
#include "ode.h"
#include "Thread.h"
Context::Context(int l, maxlist code, int thread) {
 LocalVARS = maxlist(200000);
 Code = code;
 line = l;
 t = thread;
}
int Context::get_line_number(){ return line; }
string Context::get_line(){ return Code.dget(line); }
void Context::set_line(int l){ line = l; }
string Context::get_variable(string name) {
  int i = LocalVARS.iget(name);
  if (i == 0) {
    return threads[this->t].get_variable(name);
  }
  return LocalVARS.dget(i);
}
Thread Context::parent() {
  return threads[this->t];
}
int Context::run(){
  return 0;
}
