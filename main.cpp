#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

void ReadFile(){
    ifstream fin;
    fin.open("1684748905742_525.ndf", ios::in);
    if(!fin.is_open()){
        cout << "Open failed." <<endl;
        return ;
    }
    string buff;
    while(getline(fin, buff)) cout << buff << endl;
    fin.close();
}


int main(){
    ReadFile();
    return 0;
}