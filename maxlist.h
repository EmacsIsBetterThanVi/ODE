#ifndef MAXLIST_H
#define MAXLIST_H
#include <string>
using namespace std;
class maxlist{
private:
    int maxsize;
    int size;
    string* data;
public:
  maxlist(int ms=256);
  int get_size();
  void add(string);
  void remove(int);
  void insert(int, string);
  void replace(int, string);
  string dget(int);
  int iget(string);
  void empty();
};
#endif
