#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#ifndef KEYSPACE_H
#define KEYSPACE_H 1

using std::string;
class MyString{
private:
    char data[101];
    int len;
public:
    MyString(){
        for (int i=0; i<101; i++) data[i]='\0';
        len=0;
    }
    MyString(const char* other){
        for (int i=0; i<101; i++) data[i]='\0';
        len=0;
        while (len<100 && other[len]!='\0'){
            data[len]=other[len];
            len++;
        }
    }
    MyString(const MyString& other){
        for (int i=0; i<101; i++) data[i]='\0';
        len=0;
        while (len<100 && other.data[len]!='\0'){
            data[len]=other.data[len];
            len++;
        }
    }
    MyString(MyString&& other){
        for (int i=0; i<101; i++) data[i]='\0';
        len=0;
        while (len<100 && other.data[len]!='\0'){
            data[len]=other.data[len];
            len++;
        }
    }
    MyString(std::string& other){
        for (int i=0; i<101; i++) data[i]='\0';
        len=0;
        while (len<100 && len<other.length()){
            data[len]=other[len];
            len++;
        }
    }

    MyString& operator = (std::string& other){
        for (int i=0; i<101; i++) data[i]='\0';
        len=0;
        while (len<100 && len<other.length()){
            data[len]=other[len];
            len++;
        }
        return *this;
    }
    MyString& operator = (const char* other){
        for (int i=0; i<101; i++) data[i]='\0';
        len=0;
        while (len<100 && other[len]!='\0'){
            data[len]=other[len];
            len++;
        }
        return *this;
    }
    MyString& operator = (const MyString& other){
        if (this!=&other){
            for (int i=0; i<101; i++) data[i]='\0';
            len=0;
            while (len<100 && other.data[len]!='\0'){
                data[len]=other.data[len];
                len++;
            }
        }
        return *this;
    }
    MyString& operator = (MyString&& other){
        if (this!=&other){
            for (int i=0; i<101; i++) data[i]='\0';
            len=0;
            while (len<100 && other.data[len]!='\0'){
                data[len]=other.data[len];
                len++;
            }
        }
        return *this;
    }

    const int size() const{
        return len;
    }
    std::string to_str(){
        return std::string(data);
    }

    char& operator [](int idx){
        if (idx<len) return data[idx];
        throw std::out_of_range("idx>len");
    }
    const char& operator [](int idx) const{
        if (idx<len) return data[idx];
        throw std::out_of_range("idx>len");
    }

    ~MyString(){
    }
};


class Table;
class MyIter;
struct KeySpace1;
struct KeySpace2;
class Cache;
class Bank;

struct Information{
    float number;
    MyString str;
    Information(): number(0), str(""){};
    Information(const Information& other):number(other.number), str(other.str){};
    Information(Information&& other):number(std::move(other.number)), str(other.str){};
    Information(float num, MyString s):
        number(num), str(s){};
    Information& operator = (const Information& other){
        if (this!=&other){
            number=other.number;
            str=other.str;
        }
        return *this;
    }
    Information& operator = (Information&& other){
        if (this!=&other){
            number=other.number;
            other.number=0;
            str=other.str;
        }
        return *this;
    }
};

struct Item{
    friend KeySpace1;
    friend KeySpace2;
    friend MyIter;
    friend Table;
    friend Cache;
    friend Bank;
    private:
        int offset_p1;
        int offset_p2;
        int offset_info;
        int key1;
        MyString key2;
        Item& operator = (const Item& other) {
            if (this!=&other){
                offset_p1=other.offset_p1;
                offset_p2=other.offset_p2;
                offset_info=other.offset_info;
                key1=other.key1;
                key2=other.key2;
            }
            return *this;
        };
        Item& operator = (Item&& other){
            if (this!=&other){
                offset_p1=other.offset_p1;
                other.offset_p1=-1;
                offset_p2=other.offset_p2;
                other.offset_p2=-1;
                offset_info=other.offset_info;
                other.offset_info=-1;
                key1=other.key1;
                key2=std::move(other.key2);
            }
            return *this;
        };
        Item(): key1(0), key2(""), offset_info(-1), offset_p1(-1), offset_p2(-1){};
        Item(const Item& other): offset_p1(other.offset_p1), offset_p2(other.offset_p2), 
                offset_info(other.offset_info), key1(other.key1), key2(other.key2) {};
        Item(Item&& other): offset_p1(other.offset_p1), offset_p2(other.offset_p2), 
                offset_info(other.offset_info), key1(other.key1), key2(other.key2) {
            other.offset_p1=-1;
            other.offset_p2=-1;
            other.offset_info=-1;
            other.key1=-1;
            other.key2="";
        };
        Item(int key1, MyString key2, int offset_info):
            key1(key1), key2(key2), offset_info(offset_info), offset_p1(-1), offset_p2(-1) {};
        ~Item(){}

};

struct KeySpace1{
    friend Item;
    friend MyIter;
    friend Table;
    friend Cache;
    friend Bank;
    private:
        int offset_next;
        int offset_item;
        int key;
        int par;
        KeySpace1(int key, int par, int offset_next, int offset_item): 
            key(key), par(par), offset_next(offset_next), offset_item(offset_item) {};
        KeySpace1(): key(0), par(-1), offset_next(-1), offset_item(-1) {};
        KeySpace1(const KeySpace1& other){
            offset_next=other.offset_next;
            offset_item=other.offset_item;
            key=other.key;
            par=other.par;
        }
        KeySpace1& operator = (const KeySpace1& other) {
            if (this!=&other){
                offset_next=other.offset_next;
                offset_item=other.offset_item;
                key=other.key;
                par=other.par;
            }
            return *this;
        };
        KeySpace1& operator = (KeySpace1&& other){
            if (this!=&other){
                offset_next=other.offset_next;
                other.offset_next=-1;
                offset_item=other.offset_item;
                other.offset_item=-1;
                key=other.key;
                par=other.par;
            }
            return *this;
        }
};

struct KeySpace2{
    friend Item;
    friend Table;
    friend Cache;
    friend Bank;
    private:
        int busy;//0 - free, 1 - busy, 2 - was busy
        MyString key;
        int offset_item;
        KeySpace2(): busy(0), key(""), offset_item(-1) {};
        KeySpace2(MyString key, int offset_item): busy(0), key(key), offset_item(offset_item) {
            if (offset_item>=0) busy=1;
        }
        KeySpace2(const KeySpace2& other){
            key=other.key;
            offset_item=other.offset_item;
            busy=other.busy;
        }
        KeySpace2& operator = (const KeySpace2& other) {
            if (this!=&other){
                key=other.key;
                offset_item=other.offset_item;
                busy=other.busy;
            }
            return *this;
        };
        KeySpace2& operator = (KeySpace2&& other){
            if (this!=&other){
                key=other.key;
                offset_item=other.offset_item;
                other.offset_item = -1;
                busy=other.busy;
                other.busy=0;
            }
            return *this;
        }
};


#endif