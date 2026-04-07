#include <string.h>
#include "maxlist.h"
#include <stack>
#include "Context.h"
#include "Thread.h"
#include "ode.h"
using namespace std;
Thread::Thread(int id, string file) {
  this->id = id;
  contextStack = stack<Context>();
  
  contextStack.push(Context(0, code, this->id));
  run = true;
}
string Thread::get_variable(string name) {
  int i = ThreadVARS.iget(name);
  if (i == 0) {
    return get_G_variable(name);
  }
  return ThreadVARS.dget(i);
}
void Thread::set_variable(string name, string value, string type){
}
bool Thread::is_running() { return run; }
int Thread::get_line_number() { return contextStack.top().get_line_number(); }
string Thread::get_line(){ return contextStack.top().get_line(); }
int Thread::exec() {
  int v = contextStack.top().run();
  
  if (v == 255) {
    delete &(contextStack.top());
    contextStack.pop();
    if (contextStack.empty()) run = false;
    else contextStack.top().next_line();
    return 0;
  } else if (v==0) contextStack.top().next_line();
  return v;
}
