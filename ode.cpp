#include <iostream>
#include <stack>
#include <fstream>
#include <signal.h>
#include "data.cpp"
#include <glob.h>
#include <string>
#include <unistd.h>
using namespace std;
bool stop = false;
string ode_dir;
maxlist GlobalVARS;
class Context {
private:
  maxlist LocalVARS;
  int line;
  int t;
  maxlist Code;
public:
  Context() {}
  Context(int l, maxlist code, int thread) {
    LocalVARS = maxlist(200000);
    Code = code;
    line = l;
    t = thread;
  }
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
    254 - Context Returned(function)
    255 - Context exited(Non-function)
   */
  int run();
  void set_variable(string, string, string);
  string get_variable(string);
  int get_line_number() { return line; }
  string get_line() { return Code.dget(line); }
  void set_line(int l) {
    line = l;
  }
};
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
  Thread(int id, string file) {
    this->id = id;
    contextStack = stack<Context>();
    
    contextStack.push(Context(0, code, this->id));
    line = 0;
    run = true;
  }
  string get_variable(string name) {
    int i = ThreadVARS.iget(name);
    if (i == 0) {
      return get_G_variable(name);
    }
    return ThreadVARS.dget(i);
  }
  bool is_running() { return run; }
  int get_line_number() { return contextStack.top().get_line_number(); }
  string get_line(){ return contextStack.top().get_line(); }
  int exec() {
    int v = contextStack.top().run();
    if (v == 255)
      line = contextStack.top().get_line_number()+1;
    if (v == 254) line++;
    if (v == 255 || v == 254) {
      contextStack.pop();
      contextStack.top().set_line(line);
      if (contextStack.empty()) run = false;
      return 0;
    }
    return v;
  }
};
Thread threads[100];
int thread_count=0;
int Context::run() {
  // Access my thread: threads[this->t];
}
string Context::get_variable(string name) {
  int i = LocalVARS.iget(name);
  if (i == 0) {
    return threads[this->t].get_variable(name);
  }
  return LocalVARS.dget(i);
}
Thread Context::parent(){
  return threads[this->t]
}
void control_C(int signum){
    stop=true;
    cerr << "\nProcess killed by user\n";
    exit(11);
}
void thread_init(string file) {
  
}
int exe(){
    bool ran_thread=false;
    for(int i=0; i<thread_count; i++){
        if(stop){
            break;
        }
        if(threads[i].is_running()){
            ran_thread=true;
            int error;
            try{
	      error = threads[i].exec();
            } catch(...){
                cerr << "Ode Crashed Executing Line "+to_string(threads[i].get_line_number())+" in thread " + to_string(i) + "\n" + threads[i].get_line()<<"\n";
                return 256;
            }
            if(error!=0){
                switch(error){
                    case 1:
                        cerr << "Syntax Error: Unmatched character ')' on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 2:
                        cerr << "Syntax Error: Unmatched character '(' on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 3:
                        cerr << "Syntax Error: Unmatched character '\"' on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 4:
                        cerr << "Syntax Error: EOL before termination on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 5:
                        cerr << "Variable Error: Request for nonexistant variable made on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 6:
                        cerr << "Variable Error: Illigal request for variable made on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 9:
                        cerr << "Syntax Error: Function decleration in code block starting at "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 10:
                        cerr << "Operator Error: Unknown operator used on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 12:
                        cerr << "Call Error: Attempt to call non-function object on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                    case 13:
                        cerr << "Call Error: Attempt to call non-object value on line "+ to_string(threads[i].get_line_number())+" in thread "+to_string(i)+"\n"+threads[i].get_line()<<"\n";
                        break;
                }
                return error;
            }
        }
    }
    if(!ran_thread){
        return 255;
    }
    return 0;
}
int main(int argc, char* argv[]){
    signal(SIGINT,control_C);
    if(argc==1){
        cerr << "MISSING ODE DIRECTORY";
        return 8;
    }
    GlobalVARS = maxlist(200000);
    bool INFO = true;
    if (argc >= 3) {
      INFO = false;
      if (strcmp(argv[2], "-v")) {
	INFO = true;
      }
    }    
    cout << "ODE Binary 0.0.0\n";
    cout << "LOADING BUILT IN CLASSES\n";
    cout << "DONE\n";
    int error = 0;
    ode_dir = argv[1];
    if(argc==2 || (argc == 3 && !INFO)){
      thread_init("REPL");
      stop = false;
      while (error == 0 && !stop) {
	error=exe();
      }
    } else{
        thread_init(argv[2]);
        stop=false;
        while(error==0 && !stop){
            error=exe();
        }
    }
    return error;
}
