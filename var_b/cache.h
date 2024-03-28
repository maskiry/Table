#include "keyspaces.h"

#ifndef CACHE_H
#define CACHE_H 1

class Cache;

class Bank
{
friend Cache;
private:
    size_t bank_size;
    KeySpace1* ks1;
    int* adrs_ks1;
    KeySpace2* ks2;
    int* adrs_ks2;
    Item* item;
    int* adrs_item;
    Information* info;
    int* adrs_info;
    Bank();
    Bank(size_t size);
    ~Bank();
};

class Cache
{
private:
    Bank **banks;
    size_t bank_size;
    size_t extra_rec;
    int **LRU;
    int access[4][2]={{7,0},{25,32},{42,20},{52,11}}, replace[4][2]={{56,56},{38,6},{21,1},{11,0}};
    FILE *ks1, *ks2, *info, *item;
    int choose_bank(int idx, int mode);

    template <class T>
    int update_one_rec(int offset, int offset_idx, FILE* f, int* adrs, T* rec, int mode/*1 - for read, 2 - for write*/){
        if (offset<0) {
            return -1;
        }
        fseek(f, 0, SEEK_END);
        if (ftell(f)<=offset*sizeof(*rec)) {
            std::cout << "error in update_one_rec: offset is bigger then msize (" << ftell(f) << "<=" << sizeof(*rec) << ")\n";
            return -1;
        }
        if (adrs[offset_idx]>-1){
            fseek(f, adrs[offset_idx]*sizeof(*rec), SEEK_SET);
            fwrite(rec+offset_idx, sizeof(*rec), 1, f);
        }
        if (mode==1){
            adrs[offset_idx]=offset;
            fseek(f, offset*sizeof(*rec), SEEK_SET);
            fread(&rec[offset_idx], sizeof(*rec), 1, f);
        }
        else {
            adrs[offset_idx]=-1;
        }

        return 0;
    }


    int update_cache(int offset, int main_class, int mode);
public:
    float all_req, external_mem_req;

    Cache(FILE *ks1, FILE *ks2, FILE *info, FILE *item, size_t size);
    ~Cache();

    int read_record(int offset, KeySpace1& var);
    int read_record(int offset, KeySpace2& var);
    int read_record(int offset, Item& var);
    int read_record(int offset, Information& var);
    int write_record(int offset, KeySpace1& var);
    int write_record(int offset, KeySpace2& var);
    int write_record(int offset, Item& var);
    int write_record(int offset, Information& var);

    void save_cache();
    void reset_cache(FILE *ks1, FILE *ks2, FILE *info, FILE *item);
};









#endif