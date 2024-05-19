using namespace std;
#include <string.h>
class maxlist{
    private:
        int maxsize;
        int size;
        string* data;
    public:
        maxlist(int ms=256){
            maxsize=ms;
            size=0;
            data=new string[0];
        }
        int get_size(){
            return size;
        }
        void add(string element){
            if(size<maxsize){
            size++;
            string old_data[size-1];
            for(int i=0; i<size-1; i++){
                old_data[i]=data[i];
            }
            delete[] data;
            data=new string[size];
            for(int i=0; i<size-1; i++){
                data[i]=old_data[i];
            }
            data[size-1]=element;
            }
            return;
        }
        void remove(int id){
            string old_data1[id-1];
            string old_data2[size-id];
            for(int i=0; i<id; i++){
                old_data1[i]=data[i];
            }
            for(int i=0; i<size-id; i++){
                old_data2[i]=data[i+id];
            }
            size--;
            delete[] data;
            data=new string[size];
            for(int i=0; i<id-1; i++){
                data[i]=old_data1[i];
            }
            for(int i=0; i<(size+1)-id; i++){
                data[i+id-1]=old_data2[i];
            }
            return;
        }
        void insert(int id, string element){
            string old_data1[id];
            string old_data2[size-id-1];
            for(int i=0; i<id; i++){
                old_data1[i]=data[i];
            }
            for(int i=0; i<size-id-1; i++){
                old_data2[i]=data[i+id];
            }
            size++;
            delete[] data;
            data=new string[size];
            for(int i=0; i<id; i++){
                data[i]=old_data1[i];
            }
            for(int i=0; i<size-id-1; i++){
                data[i+id+1]=old_data2[i];
            }
            data[id]=element;
            return;
        }
        void replace(int id, string element){
            data[id-1]=element;
            return;
        }
        string dget(int id){
            if(id<1 || id>size) return "";
            else return data[id-1];
        }
        int iget(string element){
            for(int i=0; i<size; i++){
                if(data[i]==element){
                    return i+1;
                }
            }
            return 0;
        }
	void empty(){
	    delete[] data;
	    data=new string[0];
	    size=0;
	}
};
void subarr(char arr[], char sub[], int start, int length){
    for(int i=0; i<length; i++){
        sub[i]=arr[i+start];
    }
}
bool contains(string str, string substring){
    return (str.find(substring)!=string::npos);
}
