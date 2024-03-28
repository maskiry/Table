#include "table.h"
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <fstream>

void split_string(string input, std::vector<string>& output, char sep=' '){
    string tmp="";
    output.clear();
    int start=0;
    while (input[start]==' '){
        start++;
    }
    int i=start;
    while (i<input.size()){
        if (input[i]!=' ') {
            tmp+=input[i];
            i++;
        }
        else{
            output.push_back(tmp);
            tmp="";
            while (input[i]==' '){
                i++;
            }
        }
    }
    if (tmp.size()>0) output.push_back(tmp);

}

int parse_str(string s, int& par, int& key1, string& key2, float& num_info, string& str_info){
    if (s.size()<7) {
        std::cout << "Not enough input args (must be 5)\n";
        return -1;
    };
    
    int counter=0, i=0;
    std::vector<string> split_str;
    split_string(s, split_str);
    if (split_str.size()!=5) {
        std::cout << "Input args must be 5\n";
        return -1;
    }
    try{
        par=std::stoi(split_str[0]);
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
        return -1;
    }
    try{
        key1=std::stoi(split_str[1]);
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
        return -1;
    }
    key2=split_str[2];
    try{
        num_info=std::stof(split_str[3]);
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
        return -1;
    }
    str_info=split_str[4];

    return 0;
}

int fill_tab(string adrs, Table& tb){
    std::ifstream in(adrs);
    string s, key2, str;
    int key1, par;
    float num;

    while (!in.eof()){
        std::getline(in, s);
        int code=parse_str(s, par, key1, key2, num, str);
        if (code){
            continue;
        }
        code=tb.add(par, key1, key2, num, str);
    }
    
    in.close();
    return 0;
}

void menu(Table& tb){
    int com, key1, par, code,a,b;
    float num;
    string s, key2, gr;
    gr="input command: 0 - out, 1 - add, 2 - del by key1, 3 - del by key2, 4 - del by key1 and key2,\n\
5 - find by key1, 6- find by key2, 7 - find by key1 and key2, 8 - slice by parent, 9 - output: \n";
    std::cout << gr;
    std::cin >> com;
    while (com){
        if (com==1){
            std::cout << "input par: ";
            std::cin >> par;
            std::cout << "input key1: ";
            std::cin >> key1;
            std::cout << "input key2: ";
            std::cin >> key2;
            std::cout << "input num: ";
            std::cin >> num;
            std::cout << "input string: ";
            std::cin >> s;
            code=tb.add(par, key1, key2, num, s);
        }
        else if (com==2){
            std::cout << "input key1: ";
            std::cin >> key1;
            tb.del(key1);
        }
        else if (com==3){
            std::cout << "input key2: ";
            std::cin >> key2;
            tb.del(key2);
        }
        else if (com==4){
            std::cout << "input key1: ";
            std::cin >> key1;
            std::cout << "input key2: ";
            std::cin >> key2;
            tb.del(key1, key2);
        }
        else if (com==5){
            std::cout << "input key1: ";
            std::cin >> key1;
            try{
                std::cout << tb[key1];
            }
            catch(const std::exception& e){
                std::cerr << e.what() << '\n';
            }
        }
        else if (com==6){
            std::cout << "input key2: ";
            std::cin >> key2;
            try{
                std::cout << tb[key2];
            }
            catch(const std::exception& e){
                std::cerr << e.what() << '\n';
            }
        }
        else if (com==7){
            std::cout << "input key1: ";
            std::cin >> key1;
            std::cout << "input key2: ";
            std::cin >> key2;
            try{
                std::cout << tb[key1][key2];
            }
            catch(const std::exception& e){
                std::cerr << e.what() << '\n';
            }
        }
        else if (com==8){
            std::cout << "input first edge: ";
            std::cin >> a;
            std::cout << "input second edge: ";
            std::cin >> b;
            Table tb2(tb.slice_by_parent(a,b));
            std::cout << tb2;
        }
        else if (com==9){
            std::cout << tb;
        }
        else if (com==10) std::cout << gr;
        std::cout << "\ninput command: ";
        std::cin >> com;
    }
    
}

int main(){
    int size;
    std::cin >> size;
    Table tb(size);
    fill_tab("input.txt", tb);
    std::cout << tb;
    Table tb2(tb.slice_by_parent(1,10));
    std::cout << tb2;
    int x=7;
    std::cout << "with key1=" << x << ": " << tb[x];
    std::cout << "with key2=w: " << tb[(string) "w"];
    std::cout << "with key1=20002463, key2=q3rjqhbf: " << tb[(string) "q3rjqhbf"][20002463];

    menu(tb);
    
    

    return 0;
}