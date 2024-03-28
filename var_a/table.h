#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#ifndef TABLE_h
#define TABLE_h 1

using std::string;

struct KeySpace1;
struct KeySpace2;
struct Item;
class MyIter;
class Table;

struct Information{
    float number;
    string str;
    Information(): number(0), str(""){};
    Information(const Information& other):number(other.number), str(other.str){};
    Information(Information&& other):number(std::move(other.number)), str(std::move(other.str)){};
    Information(float num, string s):
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
            str=std::move(other.str);
        }
        return *this;
    }
};

struct Item{
    friend KeySpace1;
    friend KeySpace2;
    friend MyIter;
    friend Table;
    private:
        KeySpace1 *p1;
        KeySpace2 *p2;
        Information *info;
        int key1;
        string key2;
        Item& operator = (const Item& other) = delete;
        Item& operator = (Item&& other){
            if (this!=&other){
                p1=other.p1;
                other.p1=nullptr;
                p2=other.p2;
                other.p2=nullptr;
                info=other.info;
                other.info=nullptr;
                key1=other.key1;
                key2=std::move(other.key2);
            }
            return *this;
        };
        Item(): key1(0), key2(""), info(nullptr), p1(nullptr), p2(nullptr){};
        Item(const Item& other) = delete;
        Item(Item&& other): p1(other.p1), p2(other.p2), info(other.info), key1(other.key1), key2(other.key2) {
            other.p1=nullptr;
            other.p2=nullptr;
            other.info=nullptr;
            other.key1=0;
            key2="";
        };
        Item(int key1, string key2, Information *info):
            key1(key1), key2(key2), info(info), p1(nullptr), p2(nullptr) {};
        ~Item(){
            delete info;
        }

};

struct KeySpace1{
    friend Item;
    friend MyIter;
    friend Table;
    private:
        KeySpace1 *next;
        Item *item;
        int key;
        int par;
        KeySpace1& operator = (const KeySpace1& other) = delete;
        KeySpace1& operator = (KeySpace1&& other){
            if (this!=&other){
                next=other.next;
                other.next=nullptr;
                item=other.item;
                other.item=nullptr;
                key=other.key;
                par=other.par;
            }
            return *this;
        }
        KeySpace1(int key, int par, KeySpace1 *next, Item *itm): 
            key(key), par(par), next(next), item(itm) {};
};

struct KeySpace2{
    friend Item;
    friend Table;
    private:
        int busy;//0 - free, 1 - busy, 2 - was busy
        std::string key;
        Item *item;
        KeySpace2(): busy(0), key(""), item(nullptr) {};
        KeySpace2(std::string key, Item* item): busy(0), key(key), item(item) {
            if (item) busy=1;
        }
        KeySpace2& operator = (const KeySpace2& other) = delete;
        KeySpace2& operator = (KeySpace2&& other){
            if (this!=&other){
                key=std::move(other.key);
                item=other.item;
                other.item = nullptr;
                busy=other.busy;
                other.busy=0;
            }
            return *this;
        }
};

class MyIter
{
    friend Table;
private:
    KeySpace1 *p1;
    KeySpace2 *p2;
    Information *info;
    int key1;
    string key2;
    bool finish;
    void* operator new(std::size_t size) {return ::operator new(size);}
    void operator delete(void* ptr) {::operator delete(ptr);}
    void* operator new[](std::size_t size) {return ::operator new[](size);}
    void operator delete[](void* ptr) {::operator delete[](ptr);}
public:
    MyIter(): key1(0), key2(""), info(nullptr), p1(nullptr), p2(nullptr), finish(true){};
    MyIter(int key1, string key2, Information *info, KeySpace1* p1, KeySpace2* p2):
            key1(key1), key2(key2), info(info), p1(p1), p2(p2) {
                if (p1)finish=false;
                else finish=true;
            };
    MyIter(const Item& other):p1(other.p1), p2(other.p2), info(other.info), key1(other.key1), key2(other.key2) {
        if (p1)finish=false;
        else finish=true;
    }
    MyIter(const MyIter& other): p1(other.p1), p2(other.p2), 
        info(other.info), key1(other.key1), key2(other.key2), finish(other.finish){};
    MyIter(MyIter&& other) = delete;
    void next(){
        if (!finish){
            if (p1->next && p1->next->item){
                Item* nxt(p1->next->item);
                key1=nxt->key1;
                key2=nxt->key2;
                info=nxt->info;
                p1=nxt->p1;
                p2=nxt->p2;
                return;
            }
            else{
                key1=0;
                key2="";
                info=nullptr;
                p1=nullptr;
                p2=nullptr;
                finish=true;
                return;
            }
        }
    };

    Information& get_info(){return *info;};
    int get_par(){return p1->par;}
    int get_key1(){ return key1;}
    string get_key2(){ return key2;}
    bool get_finish() {return finish;}

    MyIter& operator[](int k1){
        if (key1==k1) return *this;
        throw std::out_of_range("No such element");
    };
    MyIter& operator[](string k2){
        if (key2==k2) return *this;
        throw std::out_of_range("No such element");
    };
    MyIter& operator = (MyIter&& other){
        if (this!=&other) {
            p1=other.p1;
            other.p1=nullptr;
            p2=other.p2;
            other.p2=nullptr;
            info=other.info;
            other.info=nullptr;
            key1=other.key1;
            key2=other.key2;
            finish = other.finish;
        }
        return *this;
    };
    MyIter& operator = (MyIter& other) = delete;

    ~MyIter(){};
};

std::ostream& operator << (std::ostream& out, MyIter& iter);

class Table{
    private:
        KeySpace1 *ks1;
        KeySpace1 *last;
        KeySpace2 *ks2;
        MyIter *itr;
        
        int  msize;
        int hash;
        int  csize;

        int* lengths;//key1,key2,num,str
        KeySpace1* find_in_KS1(int key);
        KeySpace2* find_in_KS2(string key);
        KeySpace1* add_in_KS1(int key, int par, Item *it);
        KeySpace2* add_in_KS2(string key, Item *it);
        int hash_func(const string& s);
        int len_num(int n);
    public:
        Table(int msize2);
        ~Table();
        Table(const Table& other);
        Table(Table&& other);
        void clear();
        int add(int par, int key1, string key2, float num_info, string str_info);
        int del(int key1, string key2);
        int del(int key1);
        int del(string key2);
        const int* get_lengths() const;
        MyIter& get_iter() const;
        Table slice_by_parent(int a, int b);

        Table& operator = (const Table& other){
            if (this!=&other){
                clear();
                delete [] ks2;
                try {
                    ks2 = new KeySpace2[other.msize];
                }
                catch (std::bad_alloc& ex) {
                    std::cout << "Caught bad_alloc: " << ex.what() << std::endl;
                    return *this;
                }
                lengths[0]=other.lengths[0];lengths[1]=other.lengths[1];
                lengths[1]=other.lengths[1];lengths[3]=other.lengths[3];
                msize=other.msize;
                hash=other.hash;
                MyIter other_iter = other.get_iter();
                while (!other_iter.finish){
                    Information *info = new Information(other_iter.info->number,
                        other_iter.info->str);
                    Item *it = new Item(other_iter.key1, other_iter.key2, other_iter.info);
                    it->p1 = add_in_KS1(other_iter.key1, other_iter.p1->par, it);
                    it->p2 = add_in_KS2(other_iter.key2, it);
                    csize++;
                }
                
            }
            return *this;
        };

        Table& operator = (Table&& other){
            if (this!=&other){
                clear();
                delete ks1;
                ks1=other.ks1;
                other.ks1=nullptr;
                delete [] ks2;
                ks2=other.ks2;
                other.ks2=nullptr;
                delete [] lengths;
                lengths=other.lengths;
                other.lengths=nullptr;
                delete itr;
                itr=other.itr;
                other.itr=nullptr;
                msize=other.msize;
                hash=other.hash;
            }
            return *this;
        };

        MyIter& operator [](int key1){
            KeySpace1* act=ks1;            
            KeySpace1* ans=find_in_KS1(key1);
            if (ans) {
                *itr = MyIter(*ans->item);
                return *(itr);
            };
            throw std::out_of_range("No such element");
        };
        MyIter& operator [](string key2){
            KeySpace2* ans=find_in_KS2(key2);
            if (ans) {
                *itr = MyIter(*ans->item);
                return *(itr);
            };
            throw std::out_of_range("No such element");
        };

};

std::ostream& operator << (std::ostream& out, Table& tb);








#endif