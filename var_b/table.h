#include "cache.h"

#ifndef TABLE_h
#define TABLE_h 1

using std::string;


class MyIter
{
friend Table;
private:
    int offset_p1;
    int offset_p2;
    int offset_info;
    Cache *cache;
    int key1;
    MyString key2;
    bool finish;
    void* operator new(std::size_t size) {return ::operator new(size);}
    void operator delete(void* ptr) {::operator delete(ptr);}
    void* operator new[](std::size_t size) {return ::operator new[](size);}
    void operator delete[](void* ptr) {::operator delete[](ptr);}
public:
    MyIter(): key1(0), key2(""), offset_info(-1), offset_p1(-1), offset_p2(-1), finish(true), cache(nullptr){};
    MyIter(int key1, MyString key2, int offset_info, int offset_p1, int offset_p2):
            key1(key1), key2(key2), offset_info(offset_info), offset_p1(offset_p1), 
            offset_p2(offset_p2), cache(nullptr) {
                if (offset_p1==-1) finish=true;
                else finish=false;
            };
    MyIter(const Item& other):offset_p1(other.offset_p1), offset_p2(other.offset_p2), 
                            offset_info(other.offset_info), key1(other.key1), 
                            key2(other.key2), cache(nullptr) {
        if (offset_p1==-1)finish=true;
        else finish=false;
    }
    MyIter(const MyIter& other): offset_p1(other.offset_p1), offset_p2(other.offset_p2), 
        offset_info(other.offset_info), key1(other.key1), key2(other.key2), 
        finish(other.finish), cache(other.cache){};
    MyIter(MyIter&& other) = delete;

    void next(){
        if (!finish){
            KeySpace1 ksp1;
            cache->read_record(offset_p1, ksp1);
            if (ksp1.offset_next!=-1){
                cache->read_record(ksp1.offset_next, ksp1);
                Item it;
                cache->read_record(ksp1.offset_item, it);
                key1=it.key1;
                key2=it.key2;
                offset_p1=it.offset_p1;
                offset_p2=it.offset_p2;
                offset_info=it.offset_info;
            }
            else{
                key1=0;
                key2="";
                offset_p1=-1;
                offset_p2=-1;
                offset_info=-1;
                finish = true;
            }
        }
    };

    Information get_info(){
        Information inf;
        int code = cache->read_record(offset_info, inf);
        if (code) {
            throw  ("MyIter::get_info: cant write read from inf; code = " + std::to_string(code));
        }
        return inf;
    };
    int get_par(){
        KeySpace1 ksp1;
        cache->read_record(offset_p1, ksp1);
        return ksp1.par;
    }
    int get_key1(){ return key1;}
    MyString get_key2(){ return key2;}
    bool get_finish() {return finish;}

    MyIter& operator[](int k1){
        if (key1==k1) return *this;
        throw std::out_of_range("No such element");
    };
    MyIter& operator[](MyString k2){
        if (key2.to_str()==k2.to_str()) return *this;
        throw std::out_of_range("No such element");
    };
    MyIter& operator = (MyIter&& other){
        if (this!=&other) {
            offset_p1=other.offset_p1;
            other.offset_p1=-1;
            offset_p2=other.offset_p2;
            other.offset_p2=-1;
            offset_info=other.offset_info;
            other.offset_info=-1;
            key1=other.key1;
            key2=other.key2;
            finish = other.finish;
            cache=other.cache;
            other.cache=nullptr;
        }
        return *this;
    };
    MyIter& operator = (const MyIter& other) {
        if (this!=&other) {
            offset_p1=other.offset_p1;
            offset_p2=other.offset_p2;
            offset_info=other.offset_info;
            key1=other.key1;
            key2=other.key2;
            finish = other.finish;
            cache=other.cache;
        }
        return *this;
    };

    ~MyIter(){};
};

std::ostream& operator << (std::ostream& out, MyIter& iter);

struct FreeSpace{
    int offset;
    FreeSpace *next;
    FreeSpace(int offset, FreeSpace *n):offset(offset), next(n){};
};

class Table{
    friend MyIter;
    private:
        inline static unsigned int counter=0;
        unsigned int id;
        FILE *ks1;
        int offset_ks1, offset_last;
        FILE *ks2;
        FILE *info;
        FILE *item;
        MyIter *itr;
        string name_info, name_item, name_ks1, name_ks2;
        FreeSpace *fs_ks1, *fs_item, *fs_info;
        Cache *cache;

        int  msize;
        int hash;
        int  csize;

        int* lengths;//par,key1,key2,num,str
        KeySpace1 find_in_KS1(int key);
        KeySpace2 find_in_KS2(MyString key);
        int add_in_KS1(int key, int par, int offset_item);
        int add_in_KS2(MyString key, int offset_item);
        int hash_func(const MyString& s);
        int len_num(int n);
        void free_mem();
    public:
        Table(int msize);
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
        float get_stat(){
            return cache->external_mem_req/cache->all_req;
        }

        Table& operator = (const Table& other){
            if (this!=&other){
                KeySpace1 ksp1;
                Information inform;
                msize=other.msize;
                hash=other.hash;
                clear();
                MyIter other_iter = other.get_iter();
                while (!other_iter.finish){
                    other.cache->read_record(other_iter.offset_p1, ksp1);
                    other.cache->read_record(other_iter.offset_info, inform);
                    add(ksp1.par, ksp1.key, other_iter.key2.to_str(), inform.number, inform.str.to_str());
                    other_iter.next();
                }
            }
            return *this;
        };

        Table& operator = (Table&& other){
            if (this!=&other){
                free_mem();
                remove(name_ks2.c_str());
                remove(name_ks1.c_str());
                remove(name_info.c_str());
                remove(name_item.c_str());
                delete cache;
                delete fs_ks1;
                delete fs_item;
                delete fs_info;
                delete [] lengths;
                delete itr;

                id=other.id;
                ks1=other.ks1;
                other.ks1=nullptr;
                ks2=other.ks2;
                other.ks2=nullptr;
                info=other.info;
                other.info=nullptr;
                item=other.item;
                other.item=nullptr;
                itr=other.itr;
                other.itr=nullptr;
                cache=other.cache;
                other.cache=nullptr;
                fs_ks1=other.fs_ks1;
                other.fs_ks1=nullptr;
                fs_item=other.fs_item;
                other.fs_item=nullptr;
                fs_info=other.fs_info;
                other.fs_info=nullptr;
                lengths=other.lengths;
                other.lengths=nullptr;
                name_info=other.name_info;
                name_item=other.name_item;
                name_ks1=other.name_ks1;
                name_ks2=other.name_ks2;
                msize=other.msize;
                hash=other.hash;
                csize=other.csize;
                offset_ks1=other.offset_ks1;
                offset_last=other.offset_last;
            }
            return *this;
        };

        MyIter& operator [](int key1){
            KeySpace1 ksp1=find_in_KS1(key1);
            if (ksp1.offset_item>=0) {
                Item it;
                cache->read_record(ksp1.offset_item, it);
                *itr = MyIter(it);
                itr->cache=cache;
                return *(itr);
            };
            throw std::out_of_range("No such element");
        };
        MyIter& operator [](string key2){
            KeySpace2 ksp2=find_in_KS2(key2);
            if (ksp2.offset_item>=0) {
                Item it;
                cache->read_record(ksp2.offset_item, it);
                *itr = MyIter(it);
                itr->cache=cache;
                return *(itr);
            };
            throw std::out_of_range("No such element");
        };

};

std::ostream& operator << (std::ostream& out, Table& tb);








#endif