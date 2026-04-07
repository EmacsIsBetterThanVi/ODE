#ifndef ODE_H
#define ODE_H
#include <string>
#include "maxlist.h"
#include "Context.h"
#include "Thread.h"
using namespace std;
extern bool stop;
extern void thread_init(string file);
extern string ode_dir;
extern maxlist GlobalVARS;
extern int thread_count;
extern string get_G_variable(string name);
extern void set_G_variable(string name, string value, string type);
extern Thread threads[100];
#endif
