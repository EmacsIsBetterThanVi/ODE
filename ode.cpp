#include "CTurtle.hpp"
#include <iostream>
#include <stack>
#include <fstream>
#include <signal.h>
#include "data.hpp"
#include <glob.h>
using namespace std;
maxlist VARIABLE=maxlist(200000);
stack<string> ode_this[300];
string ode_dir="";
stack<string> truepath;
string path="/";
bool stop=false;
void control_C(int signum){
    stop=true;
    cerr << "\nProcess killed by user\n";
    exit(11);
}
string replace(string const& s, char c, string with)
{
    string::size_type pos = s.find(c);
    if (pos != string::npos)
    {
        return s.substr(0, pos)+with+s.substr(pos+1, sizeof(s)-pos);
    }
    else
    {
        return s;
    }
}
string replace_all(string s, char c, string with){
    string tmp="";
    while(tmp!=s){
        tmp=s;
        s=replace(s, c, with);
    }
    return s;
}
string directory_interface(string action, string name){
    if(action=="find"){
        if(name[0]=='*'){
            glob_t glob_result;
            glob(ode_dir+"/ode/Classes/*",GLOB_TILDE,NULL,&glob_result);
            for(unsigned int i=0; i<glob_result.gl_pathc; ++i){
                string tmp = glob_result.gl_pathv[i];
                tmp=tmp.substr(tmp.find_last_of("/")+1);
                if(tmp==name) return "true";
            }
        } else if(name[0]=='/'){
            glob_t glob_result;
            glob(ode_dir+name.substr(0,name.find_last_of("/")+1)+"*",GLOB_TILDE,NULL,&glob_result);
            for(unsigned int i=0; i<glob_result.gl_pathc; ++i){
                string tmp = glob_result.gl_pathv[i];
                tmp=tmp.substr(tmp.find_last_of("/")+1);
                if(tmp==name) return "true";
            }
        } else {
        glob_t glob_result;
        glob(ode_dir+path+"/*",GLOB_TILDE,NULL,&glob_result);
        for(unsigned int i=0; i<glob_result.gl_pathc; ++i){
            string tmp = glob_result.gl_pathv[i];
            tmp=tmp.substr(tmp.find_last_of("/")+1);
            if(tmp==name) return "true";
        }
        }
        return "false";
    } else if(action=="move"){
        if(name==".."){
            truepath.pop();
            stack<string> tmp(truepath);
            path="";
            while(!tmp.empty()){
                path="/"+tmp.top()+path;
                tmp.pop();
            }
        }
        else if(directory_interface("find", name)=="true"){
            truepath.push(name);
            stack<string> tmp(truepath);
            path="";
            while(!tmp.empty()){
                path="/"+tmp.top()+path;
                tmp.pop();
            }
        } else {
            cerr << "NON-EXISTANT FILE:" << name << " DOES NOT EXIST\n";
            return "throw(error)";
        }
    } else if(action=="mkdir"){
        if(name[0]=='/') system("mkdir "+ode_dir+name);
        else system("mkdir "+ode_dir+path+name);
    } else if(action=="del"){
        if(name[0]=='/') system("rm -rf "+ode_dir+name);
        else system("rm -rf "+ode_dir+path+name);
    } else if(action=="list"){
        string value="{$class:array$name:null$type:string$value:[";
        glob_t glob_result;
        if(name[0]=='/') glob(ode_dir+"/*",GLOB_TILDE,NULL,&glob_result);
        else glob(ode_dir+path+"/*",GLOB_TILDE,NULL,&glob_result);
        for(unsigned int i=0; i<glob_result.gl_pathc; ++i){
            string tmp = glob_result.gl_pathv[i];
            tmp=tmp.substr(tmp.find_last_of("/")+1)
            value=value+tmp+",";
        }
        value=value.substr(0, value.size()-1)+"]}"
    } else{
        cerr << "INVALID INSTRUCTION: DIRECTORY_INTERFACE." << action << "(" << name << ")\n";
        return "throw(error)";
    }
    return "";
}
string file_interface(string action, string name, string text){
    fstream file;
    if(action.compare("read")==0){
        if(name[0]=='/') file.open(ode_dir+name,ios::in);
        else file.open(ode_dir+path+name,ios::in);
        if(!file){
            cerr << "NON-EXISTANT FILE:" << name << " DOES NOT EXIST\n";
            return "throw(error)";
        }
        string contents="";
        char c;
        while(true){
            file >> c;
            if(file.eof()){
                file.close();
                return contents;
            } else if(c=='`'){
                contents=contents+"\n";
            } else{
                contents=contents+c;
            }
        }
    } else if(action.compare("write")==0){
        if(name[0]=='/') file.open(ode_dir+name,ios::trunc);
        else file.open(ode_dir+path+name,ios::trunc);
        for(int i=0; i<sizeof(text); i++){
            if(text[i]=='`'){
                file << "`\n";
            }
            file << text[i];
        }
        file.close();
    } else if(action.compare("append")==0){
        if(name[0]=='/') file.open(ode_dir+name,ios::app);
        else file.open(ode_dir+path+name,ios::app);
        for(int i=0; i<sizeof(text); i++){
            if(text[i]=='`'){
                file << "`\n";
            }
            file << text[i];
        }
        file.close();
    } else{
        cerr << "INVALID INSTRUCTION: FILE_INTERFACE." << action << "(" << name << ", " << text << ")\n";
        return "throw(error)";
    }
    return "file_interface."+action+"("+name+","+text+")";
}
class ode_thread{
    private:
        stack<maxlist> past_tokens;
        stack<maxlist> past_type;
        stack<maxlist> past_order;
        stack<string> Stack;
        bool run;
        maxlist code=maxlist(200000);
        int line;
        int id;
        string cfile;
    public:
        ode_thread(){}
        ode_thread(string file, int thread_id){
            run=true;
            string script_line;
            cfile=file;
            fstream script;
            script.open(ode_dir+file,ios::in);
            string contents="";
            char c;
            while(!script.eof()){
                script >> c;
                if(c=='`'){
                    code.add(contents);
                    contents="";
                } else{
                    contents=contents+c;
                }
            }
            code.add(contents.substr(0, contents.size()-1));
            script.close();
            line=0;
            id=thread_id;
        }
        void push(maxlist tokenlist, maxlist typelist, maxlist order){
            past_tokens.push(tokenlist);
            past_type.push(typelist);
            past_order.push(order);
        }
        maxlist pop_tokens(){
            maxlist tmp=past_tokens.top();
            past_tokens.pop();
            return tmp;
        }
        maxlist pop_type(){
            maxlist tmp=past_type.top();
            past_type.pop();
            return tmp;
        }
        maxlist pop_order(){
            maxlist tmp=past_order.top();
            past_order.pop();
            return tmp;
        }
        bool is_running(){
            return run;
        }
        string get_line(){
            return code.dget(line);
        }
        string get_line(int num){
            return code.dget(num);
        }
        void next(){
            line++;
            return;
        }
        int get_lines(){
            return code.get_size();
        }
        string stack_interface(string action, string file, int line){
            if(action=="add"){
                Stack.push(file);
                Stack.push(to_string(line));
            } else if(action=="pop"){
                Stack.pop();
            } else if(action=="len"){
                return to_string(Stack.size());
            } else if(action=="read"){
                return Stack.top();
            } else{
                cerr << "INVALID INSTRUCTION: STACK_INTERFACE." << action << "(" << file << ", " << line << ")\n";
            }
            return "";
        }
        void garbage(string target){
            int ID=(Stack.size()/2)+1;
            string tmp2="";
            for(int k=0; k<sizeof(target); k++){
                if(target[k]==';'){
                    tmp2=replace(target, '$', to_string(id));
                    tmp2=replace(tmp2, '%', to_string(ID));
                    for(int i=0; i<VARIABLE.get_size(); i++){
                        if(VARIABLE.dget(i).find(tmp2) != string::npos){
                            VARIABLE.remove(i);
                            VARIABLE.remove(i);
                        }
                    }
                    tmp2="";
                }else tmp2=tmp2+target[k];
            }
            return;
        }
        void thread_interface(string action, string arg1, string arg2){
            if(action=="load"){
                fstream script(ode_dir+arg1, ios::in);
                string contents="";
                char c;
                while(!script.eof()){
                    script >> c;
                    if(c=='`'){
                        code.add(contents);
                        contents="";
                    } else{
                        contents=contents+c;
                    }
                }
                code.add(contents);
                script.close();
                line=stoi(arg2)-1;
            } else if(action=="jump"){
                line=stoi(arg1)-1;
            } else if(action=="sub"){
                stack_interface("add", cfile, line);
                if(arg2=="" || arg2==cfile) line=stoi(arg1)-1;
                else thread_interface("load", arg2, arg1);
            } else if(action=="return"){
                if(arg2=="ALLOW_RETURN=TRUE && RELOAD_OLD_DATA()"){
                if(Stack.empty()){
                    run=false;
                    garbage("t$;");
                } else {
                int tmp=stoi(stack_interface("read","",0));
                stack_interface("pop", "", 0);
                if(stack_interface("read","",0)==cfile) line=tmp;
                else thread_interface("load", stack_interface("read", "", 0), to_string(tmp));
                stack_interface("pop", "", 0);
                garbage("t$l%;t$i%;");
                }
                } else cerr<< "ILLIGAL RETURN: RETURNING THROUGH THREAD INTERFACE IS NOT A VALID METHOD\nUSE return("<<arg1<<") TO RETURN\n";
            } else{
                cerr << "INVALID INSTRUCTION: THREAD_INTERFACE." << action << "(" << arg1 << ", " << arg2 << ")\n";
            }
        }
        int get_line_number(){
            return line;
        }
        string get_file(){
            return cfile;
        }
};
int thread_count=0;
ode_thread threads[300];
string variable_interface(string opt, string arg1, string arg2){
    if (opt=="add"){
        VARIABLE.add(arg1);
    } else if (opt=="delete"){
        VARIABLE.remove(stoi(arg1));
    } else if(opt=="replace"){
        VARIABLE.replace(stoi(arg1), arg2);
    } else if(opt=="dget"){
        return VARIABLE.dget(stoi(arg1));
    } else if(opt=="iget"){
        return to_string(VARIABLE.iget(arg1));
    } else{
        cerr << "INVALID INSTRUCTION: VARIABLE_INTERFACE." << opt << "(" << arg1 << ", " << arg2 << ")\n";
    }
    return "";
}
string get_variable_type(string name){
    if(VARIABLE.iget("$"+name)!=0){
        int target=0;
        char level='';
        for(int i=VARIABLE.iget("$"+name)+1; i<VARIABLE.get_size(); (i++)++){
            if(VARIABLE.dget(i).find(";g") != string::npos && level==''){
                level='g';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)) != string::npos && (level=='' || level=='g')){
                level='t';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"i") != string::npos && (level=='' || level=='g' || level=='t')){
                level='i';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"f:"+threads[id].get_file()) != string::npos && (level=='' || level=='g' || level=='t' || level=='i')){
                level='f';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"f:"+threads[id].stack_interface("len", "", 0)) != string::npos && (level=='' || level=='g' || level=='t' || level=='i' || level=='f')){
                level='l';
                target=i;
            }
            if(VARIABLE.dget(i)[0]=='$'){
                target=i-1;
                level='n'
                break;
            }
        }
        if(level=='n'){
            return "throw(Illigal)";
        else return VARIABLE.dget(target).substr(0, VARIABLE.dget(target).find(";"));
    } else return "throw(Undefined)";
}
string get_variable(string name){
    if(VARIABLE.iget("$"+name)!=0){
        int target=0;
        char level='';
        for(int i=VARIABLE.iget("$"+name)+1; i<VARIABLE.get_size(); (i++)++){
            if(VARIABLE.dget(i).find(";g") != string::npos && level==''){
                level='g';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)) != string::npos && (level=='' || level=='g')){
                level='t';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"i") != string::npos && (level=='' || level=='g' || level=='t')){
                level='i';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"f:"+threads[id].get_file()) != string::npos && (level=='' || level=='g' || level=='t' || level=='i')){
                level='f';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"f:"+threads[id].stack_interface("len", "", 0)) != string::npos && (level=='' || level=='g' || level=='t' || level=='i' || level=='f')){
                level='l';
                target=i;
            }
            if(VARIABLE.dget(i)[0]=='$'){
                target=i-1;
                level='n'
                break;
            }
        }
        if(level=='n'){
            return "throw(Illigal)";
        else return VARIABLE.dget(target+1);
    } else return "throw(Undefined)";
}
int set_variable(string name, string type, string value, int id){
    if(VARIABLE.iget("$"+name)!=0){
        int target=0;
        char level='';
        for(int i=VARIABLE.iget("$"+name)+1; i<VARIABLE.get_size(); (i++)++){
            if(VARIABLE.dget(i).find(";g") != string::npos && level==''){
                level='g';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)) != string::npos && (level=='' || level=='g')){
                level='t';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"i") != string::npos && (level=='' || level=='g' || level=='t')){
                level='i';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"f:"+threads[id].get_file()) != string::npos && (level=='' || level=='g' || level=='t' || level=='i')){
                level='f';
                target=i;
            }
            if(VARIABLE.dget(i).find(";t"+to_string(id)+"f:"+threads[id].stack_interface("len", "", 0)) != string::npos && (level=='' || level=='g' || level=='t' || level=='i' || level=='f')){
                level='l';
                target=i;
            }
            if(VARIABLE.dget(i)[0]=='$'){
                target=i-1;
                level='n'
                break;
            }
        }
        if(level=='n'){
            VARIABLE.insert(target, value);
            VARIABLE.insert(target, type);
            return target;
        }
        else VARIABLE.replace(target+1, value);
    } else{
        VARIABLE.add("$"+name);
        VARIABLE.add(type);
        VARIABLE.add(value);
        return VARIABLE.dget("$"+name);
    }
}
void thread_init(string file){
    for(int i=0; i<thread_count; i++){
        if(!(threads[i].is_running())){
            threads[i]=ode_thread(file, i);
            return;
        }
    }
    threads[thread_count]=ode_thread(file, thread_count);
    thread_count++;
    return;
}
int syntax(string input, int id){
    maxlist tokens=maxlist();
    maxlist type=maxlist();
    maxlist priority=maxlist();
    string token="";
    stack<int> layers;
    bool quotes=false;
    for(int i=0; i<input.size(); i++){
        if(stop){
            return 11;
        }
        if(input[i]=='"'){
            token=token+'"';
            quotes=!quotes;
        } else if(quotes && input[i]=='\\'){
            token=token+input[i+1];
            i++;
        } else if(quotes){
            token=token+input[i];
        } else if(input[i]=='.'){
            if(layers.empty()) priority.add("0");
            else priority.add(to_string(layers.size()));
            tokens.add(token);
            type.add("OBJECT");
            token="";
        } else if(input[i]=='('){
            if(layers.empty()) priority.add("0");
            else priority.add(to_string(layers.size()));
            layers.push(i);
            tokens.add(token);
            type.add("METHOD");
            token="";
        } else if(input[i]==')'){
            if(layers.empty()) priority.add("0");
            else priority.add(to_string(layers.size()));
            if(layers.empty()) return 1;
            else layers.pop();
            tokens.add(token);
            type.add("ARGUMENT");
            token="";
        } else if(input[i]==','){
            if(layers.empty()) priority.add("0");
            else priority.add(to_string(layers.size()));
            tokens.add(token);
            type.add("ARGUMENT");
            token="";
        } else{
            token=token+input[i];
        }
    }
    if(quotes) return 3;
    if(!layers.empty()) return 2;
    if(token!="") return 4;
    bool end=false;
    for(int i=0; i<tokens.get_size(); i++){
        if(tokens.dget(i+1)==""){
            tokens.remove(i+1);
            priority.remove(i+1);
            type.remove(i+1);
        }
    }
    while(!end){
        if(stop){
            return 11;
        }
        int highest=-1;
        int highest_id=0;
        end=true;
        for(int i=0; i<priority.get_size(); i++){
            if(stoi(priority.dget(i+1))>highest){
                if(stoi(priority.dget(i+1))!=255){
                    highest=stoi(priority.dget(i+1));
                    highest_id=i+1;
                    end=false;
                }
            }
        }
        if(!end){
        if(type.dget(highest_id)=="ARGUMENT"){
            priority.replace(highest_id, to_string(255));
                if(!tokens.dget(highest_id)[0]=='"'){
                    bool number=false;
                    for(int k=0; k<10; k++){
                        if("0123456789"[k]==tokens.dget(highest_id)[0]) number=true;
                    }
                    if(!number){
                        if(VARIABLE.iget("$"+tokens.dget(highest_id))!=0){
                            string tmp = VARIABLE.dget(VARIABLE.iget("$"+tokens.dget(highest_id))+1);
                            string ID=to_string(stoi(threads[id].stack_interface("len", "", 0))/2);
                            if(tmp.find("t"+to_string(id)+"l"+ID)||tmp.find("g")||tmp.find("t"+to_string(id)+"i"+ID))tokens.replace(highest_id,VARIABLE.dget(VARIABLE.iget("$"+tokens.dget(highest_id))+2));
                            else{
                                bool permision=false;
                                for(int k=0; k<stoi(ID); k++){
                                    if(tmp.find("t"+to_string(id)+"i"+to_string(k))) permision=true;
                                }
                                if(permision) tokens.replace(highest_id,VARIABLE.dget(VARIABLE.iget("$"+tokens.dget(highest_id))+2));
                                else return 6;
                            }
                        } else return 5;
                    }
                }
        } else if(type.dget(highest_id)=="OBJECT"){
            priority.replace(highest_id, to_string(255));
            if(tokens.dget(highest_id)=="variable_interface");
            else if(tokens.dget(highest_id)=="stack_interface");
            else if(tokens.dget(highest_id)=="pen_interface");
            else if(tokens.dget(highest_id)=="file_interface");
            else if(tokens.dget(highest_id)=="thread_interface");
            else if(tokens.dget(highest_id)=="this"){
                ode_this[id].top();
            }
            else if(type.dget(highest_id-1)=="OBJECT"){
                if(tokens.dget(highest_id)=="name" || tokens.dget(highest_id)=="class" || tokens.dget(highest_id)=="type" || tokens.dget(highest_id)=="super" || tokens.dget(highest_id)=="value"){
                    string tmp=tokens.dget(highest_id-1);
                    string name=tmp.substr(tmp.find("$"+tokens.dget(highest_id)+":")+sizeof("$"+tokens.dget(highest_id)+":")-1, tmp.size()-tmp.find("$"+tokens.dget(highest_id)+":")-sizeof("$"+tokens.dget(highest_id)+":"));
                    tokens.replace(highest_id, name.substr(0, name.find("$")));
                } else {
                    string tmp=tokens.dget(highest_id-1);
                    string name=tmp.substr(tmp.find("$name:")+sizeof("$name:")-1, tmp.size()-tmp.find("$name:")-sizeof("$name:"));
                    name=name.substr(0, name.find("$"));
                    string value=tmp.substr(tmp.find("$"+name+"-"+tokens.dget(highest_id)+":")+("$"+tokens.dget(highest_id)+":").size(), tmp.size()-tmp.find("$"+tokens.dget(highest_id)+":")-("$"+tokens.dget(highest_id)+":").size());
                    if(value[0]=='{'){
                        value=value.substr(0, value.find("}$"+name)+1);
                    } else {
                        if(contains(value, "$")){
                            value=value.substr(0, value.find("$"));
                        } else {
                            value=value.substr(0, value.find("}"));
                        }
                    }
                    tokens.replace(highest_id,value.substr(0, value.find("$")));
                }
            }else{
                string value=get_variable(tokens.dget(highest_id));
                if(value=="throw(Undifined)") return 5;
                else if(value=="throw(Illigal)")return 6;
                tokens.replace(highest_id, value);
            }
        } else if(type.dget(highest_id)=="METHOD"){
            if(type.dget(highest_id-1)=="OBJECT"){
                if(tokens.dget(highest_id-1)=="variable_interface"){
                    tokens.remove(highest_id-1);
                    priority.remove(highest_id-1);
                    type.remove(highest_id-1);
                    highest_id--;
                    if(tokens.dget(highest_id)=="add"){
                        tokens.replace(highest_id, variable_interface("add",tokens.dget(highest_id+1), ""));
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                    }
                    else if(tokens.dget(highest_id)=="delete") {
                        tokens.replace(highest_id, variable_interface("delete",tokens.dget(highest_id+1), ""));
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                    }
                    else if(tokens.dget(highest_id)=="replace") {
                        tokens.replace(highest_id, variable_interface("replace",tokens.dget(highest_id+1), tokens.dget(highest_id+2)));
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                    }
                    else if(tokens.dget(highest_id)=="dget") {
                        tokens.replace(highest_id, variable_interface("dget",tokens.dget(highest_id+1), ""));
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                    }
                    else if(tokens.dget(highest_id)=="iget") {
                        tokens.replace(highest_id, variable_interface("iget",tokens.dget(highest_id+1), ""));
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                    } else{
                        variable_interface(tokens.dget(highest_id), "", "");
                        return 7;
                    }
                    type.replace(highest_id, "ARGUMENT");
                }
                else if(tokens.dget(highest_id-1)=="stack_interface"){
                    tokens.remove(highest_id-1);
                    priority.remove(highest_id-1);
                    type.remove(highest_id-1);
                    highest_id--;
                    if(tokens.dget(highest_id)=="add"){
                        tokens.replace(highest_id, threads[id].stack_interface("add",tokens.dget(highest_id+1), stoi(tokens.dget(highest_id+2))));
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                    } else if(tokens.dget(highest_id)=="pop"){
                        tokens.replace(highest_id, threads[id].stack_interface("pop","", 0));
                    } else if(tokens.dget(highest_id)=="len"){
                        tokens.replace(highest_id, threads[id].stack_interface("len","", 0));
                    }else if(tokens.dget(highest_id)=="len"){
                        tokens.replace(highest_id, threads[id].stack_interface("len","", 0));
                    } else if(tokens.dget(highest_id)=="read"){
                        tokens.replace(highest_id, threads[id].stack_interface("read","",0));
                    } else{
                        threads[id].stack_interface(tokens.dget(highest_id), "", 0);
                        return 7;
                    }
                    type.replace(highest_id, "ARGUMENT");
                }
                else if(tokens.dget(highest_id-1)=="pen_interface");
                else if(tokens.dget(highest_id-1)=="file_interface"){
                    if(tokens.dget(highest_id)=="read"){
                        string tmp=file_interface("read",tokens.dget(highest_id+1), "");
                        if(tmp=="throw(error)") return 8;
                        tokens.replace(highest_id, tmp);
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                        type.replace(highest_id, "ARGUMENT");
                    } else{
                        string tmp=file_interface(tokens.dget(highest_id),tokens.dget(highest_id+1),tokens.dget(highest_id+2));
                        if(tmp=="throw(error)") return 7;
                        tokens.replace(highest_id, tmp);
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                        tokens.remove(highest_id+1);
                        priority.remove(highest_id+1);
                        type.remove(highest_id+1);
                        type.replace(highest_id, "ARGUMENT");
                    }
                }
                else if(tokens.dget(highest_id-1)=="thread_interface"){
                    threads[id].thread_interface(tokens.dget(highest_id),tokens.dget(highest_id+1),tokens.dget(highest_id+2));
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    type.replace(highest_id, "ARGUMENT");
                }
                else{
                    string tmp=tokens.dget(highest_id-1);
                    string name=tmp.substr(tmp.find("$name:")+sizeof("$name:")-1, tmp.size()-tmp.find("$name:")-sizeof("$name:"));
                    name=name.substr(0, name.find("$"));
                    if(tmp.find()!=string::npos){
                        string value=tmp.substr(tmp.find("$"+name+"-"+tokens.dget(highest_id)+":")+("$"+tokens.dget(highest_id)+":").size(), tmp.size()-tmp.find("$"+tokens.dget(highest_id)+":")-("$"+tokens.dget(highest_id)+":").size());
                        if(value[0]=='{'){
                            string class_name=value.substr(value.find("$class:")+sizeof("$class:")-1,value.size()-value.find("$class:")-sizeof("$class:"));
                            if(class_name=="func"){
                                int argcount=0;
                                for(int i=highest_id+1; i<=tokens.get_size(); i++){
                                    if(priority.dget(i)!=255) break;
                                    set_variable(arg+to_string(argcount), "object;t"+to_string(id)+"l"+to_string((stoi(threads[id].stack_interface("len","",0))/2)+1), toknes.dget(i));
                                }
                                threads[id].thread_interface("sub",class_name,value.substr(value.find("$line:")+sizeof("$line:")-1, value.size()-value.find("$line:")-sizeof("$line:")));
                            } else return 12
                        } else return 13;
                    } else return 5;
                }
            } else{
                if(tokens.dget(highest_id)=="import"){
                    set_variable(tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2))
                    variable_interface("add", "CLASS;t"+to_string(id)+"f"+threads[i].get_file(), "");
                    variable_interface("add", tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2)+".ode", "");
                    tokens.replace(highest_id, tokens.dget(highest_id+1));
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    type.replace(highest_id,"ARGUMENT");
                } else if(tokens.dget(highest_id)=="export"){
                    string tmp = variable_interface("dget", to_string(stoi(variable_interface("iget","$"+tokens.dget(highest_id+1),""))+1), "");
                    string tmp2 = tmp.substr(0, tmp.find(';'));
                    VARIABLE.replace(stoi(variable_interface("iget","$"+tokens.dget(highest_id+1),""))+1, tmp2+";g");
                } else if(tokens.dget(highest_id)=="*"){
                    int tmp=stoi(variable_interface("iget","$"+ode_this[id].top(),""));
                    string tmp2=variable_interface("dget", to_string(tmp+2), "");
                    variable_interface("replace", to_string(tmp+2), tmp2.substr(0, tmp2.size()-1)+"$"+ode_this[id].top()+"-"+tokens.dget(highest_id+1)+":"+VARIABLE.dget(VARIABLE.iget("$"+tokens.dget(highest_id+1))+2)+"}");
                    VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1))+1);
                    VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1))+1);
                    VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1)));
                } else if(tokens.dget(highest_id)=="root"){
                    string tmp = variable_interface("dget", to_string(stoi(variable_interface("iget","$"+tokens.dget(highest_id+1),""))+1), "");
                    string tmp2 = tmp.substr(0, tmp.find(';'));
                    VARIABLE.replace(stoi(variable_interface("iget","$"+tokens.dget(highest_id+1),""))+1, tmp2+";t"+to_string(id));
                } else if(tokens.dget(highest_id)=="inherit"){
                    string tmp = variable_interface("dget", to_string(stoi(variable_interface("iget","$"+tokens.dget(highest_id+1),""))+1), "");
                    string tmp2 = tmp.substr(0, tmp.find(';'));
                    VARIABLE.replace(stoi(variable_interface("iget","$"+tokens.dget(highest_id+1),""))+1, tmp2+";t"+to_string(id)+"i"+to_string(stoi(threads[id].stack_interface("len","",0))/2));
                } else if(tokens.dget(highest_id)=="local"){
                    string tmp = variable_interface("dget", to_string(stoi(variable_interface("iget","$"+tokens.dget(highest_id+1),""))+1), "");
                    string tmp2 = tmp.substr(0, tmp.find(';'));
                    VARIABLE.replace(stoi(variable_interface("iget","$"+tokens.dget(highest_id+1),""))+1, tmp2+";t"+to_string(id)+"l"+to_string(stoi(threads[id].stack_interface("len","",0))/2));
                } else if(tokens.dget(highest_id)=="$"){
                    tokens.replace(highest_id, tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2));
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="find"){
                    tokens.replace(highest_id, to_string(tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2).find(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2))));
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="#"){
                    tokens.replace(highest_id, "\""+tokens.dget(highest_id+1)+"\"");
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="substr"){
                    tokens.replace(highest_id, "\""+(tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2).substr(stoi(tokens.dget(highest_id+2)),stoi(tokens.dget(highest_id+3))))+"\"");
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="join"){
                    tokens.replace(highest_id, "\""+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2)+tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2)+"\"");
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="return"){
                    threads[id].thread_interface("return", tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2), "ALLOW_RETURN=TRUE && RELOAD_OLD_DATA()");
                    tokens=threads[id].pop_tokens();
                    priority=threads[id].pop_order();
                    type=threads[id].pop_type();
                    for(int i=0; i<priority.get_size(); i++){
                        if(stoi(priority.dget(i+1))==254){
                            priority.replace(i, to_string(255));
                            type.replace(i,"ARGUMENT");
                            tokens.replace(i,tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2));
                        }
                    }
                } else if(tokens.dget(highest_id)=="and"){
                    if(tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2)=="true" && tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="true"){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, "\"true\"");
                    } else {
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, "\"false\"");
                    }
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="or"){
                    if(tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2)=="true" || tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="true"){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, "\"true\"");
                    } else {
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, "\"false\"");
                    }
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="not"){
                    if(tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2)=="true"){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, "\"false\"");
                    } else {
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, "\"true\"");
                    }
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="exit"){
                    return 255;
                } else if(tokens.dget(highest_id)=="thread_init"){
                    thread_init(tokens.dget(highest_id+1));
                }else if(tokens.dget(highest_id)=="func"){
                    set_variable(tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2),"func;t"+to_string(id)+"f"+threads[id].get_file(),to_string(threads[id].get_line_number()+1));
                    int layer=1;
                    int i;
                    for(i=threads[id].get_line_number(); i<threads[id].get_lines(); i++){
                        if(threads[id].get_line(i).substr(0,4)=="end(") layer--;
                        else if(threads[id].get_line(i).substr(0,3)=="if(") layer++;
                        else if(threads[id].get_line(i).substr(0,5)=="func(") return 9;
                        if(layer==0) break;
                    }
                    threads[id].thread_interface("jump", to_string(i+1), "");
                } else if(tokens.dget(highest_id)=="end" || tokens.dget(highest_id)=="//");
                else if(tokens.dget(highest_id)=="p"){
                    if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="=="){
                        if(tokens.dget(highest_id+1)==tokens.dget(highest_id+3)){
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"true\"");
                        } else {
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"false\"");
                        }
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="<="){
                        if(stoi(tokens.dget(highest_id+1))<=stoi(tokens.dget(highest_id+3))){
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"true\"");
                        } else {
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"false\"");
                        }
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)==">="){
                        if(stoi(tokens.dget(highest_id+1))>=stoi(tokens.dget(highest_id+3))){
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"true\"");
                        } else {
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"false\"");
                        }
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="<"){
                        if(stoi(tokens.dget(highest_id+1))<stoi(tokens.dget(highest_id+3))){
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"true\"");
                        } else {
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"false\"");
                        }
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)==">"){
                        if(stoi(tokens.dget(highest_id+1))>stoi(tokens.dget(highest_id+3))){
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"true\"");
                        } else {
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"false\"");
                        }
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="!="){
                        if(tokens.dget(highest_id+1)!=tokens.dget(highest_id+3)){
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"true\"");
                        } else {
                            type.replace(highest_id,"ARGUMENT");
                            tokens.replace(highest_id, "\"false\"");
                        }
                    } else return 10;
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                }
                else if(tokens.dget(highest_id)=="if"){
                    if(tokens.dget(highest_id).substr(1, tokens.dget(highest_id+1).size()-2)!="true"){
                        int layer=1;
                        int i;
                        for(i=threads[id].get_line_number(); i<threads[id].get_lines(); i++){
                            if(threads[id].get_line(i).substr(0,4)=="end(") layer--;
                            else if(threads[id].get_line(i).substr(0,3)=="if(") layer++;
                            else if(threads[id].get_line(i).substr(0,5)=="func(") return 9;
                            if(layer==0) break;
                        }
                        threads[id].thread_interface("jump", to_string(i+1), "");
                    }
                }else if(tokens.dget(highest_id)=="for"){
                    if(VARIABLE.iget("$for_at"+to_string(threads[id].get_line_number()))==0){
                        VARIABLE.add("$for_at"+to_string(threads[id].get_line_number()));
                        VARIABLE.add("bool;t"+to_string(id)+"l"+to_string(stoi(threads[id].stack_interface("len","",0))/2));
                        VARIABLE.add("true");
                        VARIABLE.add("$"+tokens.dget(highest_id+1));
                        VARIABLE.add("int;t"+to_string(id)+"l"+to_string(stoi(threads[id].stack_interface("len","",0))/2));
                        VARIABLE.add(tokens.dget(highest_id+2));
                    }
                    if(tokens.dget(highest_id+3).substr(1, tokens.dget(highest_id+1).size()-2)!="true"){
                        int layer=1;
                        int i;
                        for(i=threads[id].get_line_number(); i<threads[id].get_lines(); i++){
                            if(threads[id].get_line(i).substr(0,5)=="loop(") layer--;
                            else if(threads[id].get_line(i).substr(0,4)=="for(") layer++;
                            else if(threads[id].get_line(i).substr(0,6)=="while(") layer++;
                            else if(threads[id].get_line(i).substr(0,5)=="func(") return 9;
                            if(layer==0) break;
                        }
                        threads[id].thread_interface("jump", to_string(i+1), "");
                    }
                }else if(tokens.dget(highest_id)=="while"){
                    if(tokens.dget(highest_id).substr(1, tokens.dget(highest_id+1).size()-2)!="true"){
                        int layer=1;
                        int i;
                        for(i=threads[id].get_line_number(); i<threads[id].get_lines(); i++){
                            if(threads[id].get_line(i).substr(0,5)=="loop(") layer--;
                            else if(threads[id].get_line(i).substr(0,4)=="for(") layer++;
                            else if(threads[id].get_line(i).substr(0,6)=="while(") layer++;
                            else if(threads[id].get_line(i).substr(0,5)=="func(") return 9;
                            if(layer==0) break;
                        }
                        threads[id].thread_interface("jump", to_string(i+1), "");
                    }
                } else if(tokens.dget(highest_id)=="loop"){
                    int layer=1;
                    int i;
                    for(i=threads[id].get_line_number(); i>=0; i--){
                        if(threads[id].get_line(i).substr(0,5)=="loop(") layer++;
                        else if(threads[id].get_line(i).substr(0,4)=="for(") layer--;
                        else if(threads[id].get_line(i).substr(0,6)=="while(") layer--;
                        else if(threads[id].get_line(i).substr(0,5)=="func(") return 9;
                        if(layer==0) break;
                    }
                    threads[id].thread_interface("jump", to_string(i-1), "");
                } else if(tokens.dget(highest_id)=="eval"){
                    if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="+"){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, to_string(stoi(tokens.dget(highest_id+1))+stoi(tokens.dget(highest_id+3))));
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="-"){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, to_string(stoi(tokens.dget(highest_id+1))-stoi(tokens.dget(highest_id+3))));
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="*"){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, to_string(stoi(tokens.dget(highest_id+1))*stoi(tokens.dget(highest_id+3))));
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="/"){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, to_string(stoi(tokens.dget(highest_id+1))/stoi(tokens.dget(highest_id+3))));
                    } else if(tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+1).size()-2)=="%"){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, to_string(stoi(tokens.dget(highest_id+1))%stoi(tokens.dget(highest_id+3))));
                    } else return 10;
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                }else if(tokens.dget(highest_id)=="throw"){
                    return stoi(tokens.dget(highest_id+1));
                } else if(tokens.dget(highest_id)=="take_out_the_trash"){
                    threads[id].garbage(tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2));
                } else if(tokens.dget(highest_id)=="del"){
                    if(VARIABLE.iget("$"+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2))!=0){
                        string tmp = VARIABLE.dget(VARIABLE.iget("$"+tokens.dget(highest_id-1))+1);
                        string ID=to_string(stoi(threads[id].stack_interface("len", "", 0))/2);
                        if(tmp.find("t"+to_string(id)+"l"+ID)||tmp.find("g")||tmp.find("t"+to_string(id)+"i"+ID)){
                            VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2))+1);
                            VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2))+1);
                            VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2)));
                        } else{
                            bool permision=false;
                            for(int k=0; k<stoi(ID); k++){
                                if(tmp.find("t"+to_string(id)+"i"+to_string(k))) permision=true;
                            }
                            if(permision){
                                VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2))+1);
                                VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2))+1);
                                VARIABLE.remove(VARIABLE.iget("$"+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2)));
                            }
                        }
                        
                    } else return 5;
                } else if(tokens.dget(highest_id)=="null"){
                    if(VARIABLE.iget("$"+tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2))==0){
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, "\"true\"");
                    } else {
                        type.replace(highest_id,"ARGUMENT");
                        tokens.replace(highest_id, "\"false\"");
                    }
                    tokens.remove(highest_id+1);
                    priority.remove(highest_id+1);
                    type.remove(highest_id+1);
                } else if(tokens.dget(highest_id)=="log"){
                    string tmp=tokens.dget(highest_id+1).substr(1, tokens.dget(highest_id+1).size()-2);
                    tmp=replace_all(tmp, '*', "\n");
                    tmp=replace_all(tmp, '~', " ");
                    tmp=replace_all(tmp, '|', "\r");
                    cout <<tmp;
                } else if(tokens.dget(highest_id)=="clear"){
                    cout << flush;
                    system("clear");
                }else if(tokens.dget(highest_id)=="object"){
                    if(type.dget(highest_id+2)=="ARGUMENT"){
                        if(VARIABLE.iget("$"+tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2))!=0){
                            type.replace(highest_id+1, "OBJECT");
                            string tmp = VARIABLE.dget(VARIABLE.iget("$"+tokens.dget(highest_id-1))+1);
                            string ID=to_string(stoi(threads[id].stack_interface("len", "", 0))/2);
                            if(tmp.find("t"+to_string(id)+"l"+ID)||tmp.find("g")||tmp.find("t"+to_string(id)+"i"+ID)){
                                string value=tokens.dget(highest_id+1);
                                string tmp2=value.substr(value.find("$name:")+1);
                                value=value.substr(0, value.find("$name:"))+"$name:"+tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2)+tmp2.substr(tmp2.find("$"));
                                VARIABLE.replace(VARIABLE.iget("$"+tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2))+2, value);
                            } else{
                                bool permision=false;
                                for(int k=0; k<stoi(ID); k++){
                                    if(tmp.find("t"+to_string(id)+"i"+to_string(k))) permision=true;
                                }
                                if(permision){
                                    string value=tokens.dget(highest_id+1);
                                    string tmp2=value.substr(value.find("$name:")+1);
                                    value=value.substr(0, value.find("$name:"))+"$name:"+tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2)+tmp2.substr(tmp2.find("$"));
                                    VARIABLE.replace(VARIABLE.iget("$"+tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2))+2, value);
                                }
                            }
                        } else {
                            VARIABLE.add("$"+tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2));
                            string value=tokens.dget(highest_id+1);
                            string tmp2=value.substr(value.find("$name:")+1);
                            value=value.substr(0, value.find("$name:"))+"$name:"+tokens.dget(highest_id+2).substr(1, tokens.dget(highest_id+2).size()-2)+tmp2.substr(tmp2.find("$"));
                            string ID=to_string(stoi(threads[id].stack_interface("len", "", 0))/2);
                            VARIABLE.add(value.substr(value.find("$class:")+sizeof("$class:")-1, value.size()-value.find("$class:")-sizeof("$class:")).substr(0, value.substr(value.find("$class:")+sizeof("$class:")-1, value.size()-value.find("$class:")-sizeof("$class:")).find("$"))+";t"+to_string(id)+"i"+ID);
                            VARIABLE.add(value);
                        }
                        tokens.remove(highest_id+2);
                        priority.remove(highest_id+2);
                        type.remove(highest_id+2);
                        tokens.remove(highest_id);
                        priority.remove(highest_id);
                        type.remove(highest_id);
                    } else {
                        type.replace(highest_id+1, "OBJECT");
                        tokens.remove(highest_id);
                        priority.remove(highest_id);
                        type.remove(highest_id);
                    }
                } else{
                    cerr << "INVALID INSTRUCTION: " << tokens.dget(highest_id) << "(" << tokens.dget(highest_id+1) << "," << tokens.dget(highest_id+2) << ")";
                    return 7;
                }
                if(tokens.dget(highest_id)!="return") priority.replace(highest_id, to_string(255));
            }
        }
        }
    }
    return 0;
}
int exe(){
    bool ran_thread=false;
    for(int i=0; i<thread_count; i++){
        if(stop){
            break;
        }
        if(threads[i].is_running()){
            threads[i].next();
            ran_thread=true;
            int error;
            try{
                error = syntax(threads[i].get_line(), i);
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
    cout << flush;
    system("clear");
    if(argc==2){
    cout << "ODE Binary 0.85.625\n";
    sleep(1);
    cout << "INTERFACE VERSIONS\n";
    cout << "VARIABLE INTERFACE 1.0.0\n";
    cout << "PEN INTERFACE 0.0.0\n";
    cout << "STACK INTERFACE 1.0.0\n";
    cout << "DIRECTORY INTERFACE 1.0.0\n";
    cout << "FILE INTERFACE 1.0.0\n";
    cout << "THREAD INTERFACE 1.0.0\n";
    cout << "ODE INTERFACE 0.9.0\n";
    cout << "SYNTAX VERSION 0.9.5";
    sleep(1);
    cout << flush;
    system("clear");
    cout << "ODE Binary 0.85.625\n";
    cout << "LOADING BUILT IN CLASSES\n";
    }
    if(argc==2) cout << "DONE\n";
    sleep(1);
    cout << flush;
    system("clear");
    int error = 0;
    ode_dir=argv[1];
    if(argc==2){
        string command="";
        maxlist last=maxlist();
        while(command!="exit()"){
            stop=false;
            cout << "ode>";
            cin >> command;
            if(command[0]=='^'){
                command=last.dget(last.get_size()-stoi(command.substr(1)));
            }
	    if(command=="clear()"){
		last.empty();
            last.add(command);
            if(command[0]=='$'){
                thread_init("");
                error=syntax(command.substr(1,sizeof(command)-1), 0);
                stop=false;
                while(error==0 && !stop){
                    error=exe();
                }
            } else error=syntax(command,0);
            if(error!=0 && error!=255) cout << error << "\n";
        }
        return 255;
    } else if(strcmp(argv[2],"--command")==0){
        if(argv[3][0]=='$'){
            char command[sizeof(argv[3])-1];
            subarr(argv[3], command, 1,sizeof(argv[3])-1);
            thread_init("");
            error=syntax(command,0);
            stop=false;
            while(error==0 && !stop){
                error=exe();
            }
            return error;
        } else return syntax(argv[3],0);
    } else{
        thread_init(argv[2]);
        stop=false;
        while(error==0 && !stop){
            error=exe();
        }
    }
    return error;
}
