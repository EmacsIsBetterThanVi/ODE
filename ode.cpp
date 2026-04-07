#include <iostream>
#include <stack>
#include <fstream>
#include <signal.h>
#include <string>
#include <glob.h>
#include <unistd.h>
#include <cstdlib>
class Thread;
class Context;
#include "maxlist.h"
#include "Context.h"
#include "Thread.h"
using namespace std;
bool stop = false;
string ode_dir;
maxlist GlobalVARS;
int thread_count=0;
string get_G_variable(string name){
  int i = GlobalVARS.iget(name);
  if (i == 0) {
    longjmp()
  }
  return ThreadVARS.dget(i);
}
//TODO: DEFINE VARIABLE SETTERS
void set_G_variable(string name, string value, string type){
}
Thread threads[100];
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
    GlobalVARS = maxlist(200000);
    bool INFO = true;
    bool OPS = false;
    if (argc >= 2) {
      INFO = false;
      if (argv[1][0] == '-') {
        OPS=true;
        INFO = strchr(argv[1], 'v') != NULL;
      }
    }
  if (INFO){
    cout << "ODE Binary 0.0.0\n";
    cout << "LOADING BUILT IN CLASSES\n";
  }
  if (INFO) cout << "DONE\n";
    int error = 0;
    ode_dir = string(getenv("HOME"))+"/.ode/";
    if (INFO) cout << "ODE DIRECTORY: " << ode_dir << "\n";
    if(argc==1 || (argc == 2 && OPS)){
      thread_init("REPL");
      stop = false;
      while (error == 0 && !stop) {
	error=exe();
      }
    } else{
        thread_init((OPS) ? argv[2] : argv[1]);
        stop=false;
        while(error==0 && !stop){
            error=exe();
        }
    }
    return error;
}
