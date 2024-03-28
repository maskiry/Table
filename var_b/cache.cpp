#include "cache.h"

Bank::Bank():bank_size(0), ks1(nullptr), ks2(nullptr), item(nullptr), info(nullptr),
    adrs_ks1(nullptr),adrs_ks2(nullptr),adrs_item(nullptr),adrs_info(nullptr){};

Bank::Bank(size_t size):bank_size(size){
    ks1=new KeySpace1[size];
    adrs_ks1 = new int[size];
    
    ks2=new KeySpace2[size];
    adrs_ks2 = new int[size];
    
    item=new Item[size];
    adrs_item = new int[size];
    
    info=new Information[size];
    adrs_info = new int[size];
    
    for (int i=0; i<size; i++){
        adrs_ks1[i]=-1;
        adrs_ks2[i]=-1;
        adrs_item[i]=-1;
        adrs_info[i]=-1;
    }

};

Bank::~Bank(){
    delete [] ks1;
    delete [] adrs_ks1;
    delete [] ks2;
    delete [] adrs_ks2;
    delete [] item;
    delete [] adrs_item;
    delete [] info;
    delete [] adrs_info;
}


Cache::Cache(FILE *ks1, FILE *ks2, FILE *info, FILE *item, size_t size=10): 
    bank_size(size), extra_rec(2), ks1(ks1), ks2(ks2), info(info), item(item), LRU(nullptr), banks(nullptr) {
    all_req=0;
    external_mem_req=0;
    banks = new Bank*[4];
    for (int i=0; i<4;i++) banks[i] = new Bank(size);
    LRU = new int*[size];
    for (int i=0; i<size; i++) {
        LRU[i]=new int[3];
        LRU[i][0]=0;
        LRU[i][1]=0;
        LRU[i][2]=0;
    }

};


Cache::~Cache(){
    for (int i=0; i<bank_size; i++) delete [] LRU[i];
    delete [] LRU;
    for (int i=0; i<4;i++) delete banks[i];
    delete [] banks;
};

int Cache::choose_bank(int idx, int mode){//1,2,3-ks1,ks2,item
    int temp;
    for (int i=0; i<4; i++){
        temp=(LRU[idx][mode-1]&replace[i][0])^replace[i][1];
        if (temp==0) {
            LRU[idx][mode-1]=(LRU[idx][mode-1]&access[i][0])|access[i][1];
            return i;
        }
    }
    return 0;
}

int Cache::update_cache(int offset, int main_class, int mode){
    int offset_next;
    std::vector<int> story_ks1(bank_size, 4), story_ks2(bank_size, 4), story_item(bank_size, 4);
    if (main_class==1){//ks1->item, info->ks2
        for (int i=0; i<extra_rec; i++){
            int offset_idx=(offset)%bank_size;
            if ((--story_ks1[offset_idx])<0) return 1;
            int bank_idx=choose_bank(offset_idx, 1);
            if (update_one_rec(offset, offset_idx, ks1, banks[bank_idx]->adrs_ks1, banks[bank_idx]->ks1, mode)) return -1;
            offset = banks[bank_idx]->ks1[offset_idx].offset_item;
            offset_next=banks[bank_idx]->ks1[offset_idx].offset_next;

            offset_idx=(offset)%bank_size;
            if ((--story_item[offset_idx])<0) return 1;
            bank_idx=choose_bank(offset_idx,3);
            if (update_one_rec(offset, offset_idx, item, banks[bank_idx]->adrs_item, banks[bank_idx]->item, mode)) return -1;
            if (update_one_rec(offset, offset_idx, info, banks[bank_idx]->adrs_info, banks[bank_idx]->info, mode)) return -1;
            offset = banks[bank_idx]->item[offset_idx].offset_p2;

            offset_idx=(offset)%bank_size;
            if ((--story_ks2[offset_idx])<0) return 1;
            bank_idx=choose_bank(offset_idx,2);
            if (update_one_rec(offset, offset_idx, ks2, banks[bank_idx]->adrs_ks2, banks[bank_idx]->ks2, mode)) return -1;
            if (offset_next==-1) return 1;
        }
        return 0;
    }
    else if (main_class==2){//ks2->item, info->ks1
        for (int i=0; i<extra_rec; i++){
            int offset_idx=(offset)%bank_size;
            if ((--story_ks2[offset_idx])<0) return 1;
            int bank_idx=choose_bank(offset_idx,2);
            if (update_one_rec(offset, offset_idx, ks2, banks[bank_idx]->adrs_ks2, banks[bank_idx]->ks2, mode)) return -1;
            offset = banks[bank_idx]->ks2[offset_idx].offset_item;
            offset_next=(offset+1)%bank_size;

            offset_idx=(offset)%bank_size;
            if ((--story_item[offset_idx])<0) return 1;
            bank_idx=choose_bank(offset_idx,3);
            if (update_one_rec(offset, offset_idx, item, banks[bank_idx]->adrs_item, banks[bank_idx]->item, mode)) return -1;
            if (update_one_rec(offset, offset_idx, info, banks[bank_idx]->adrs_info, banks[bank_idx]->info, mode)) return -1;
            offset = banks[bank_idx]->item[offset_idx].offset_p2;

            offset_idx=(offset)%bank_size;
            if ((--story_ks1[offset_idx])<0) return 1;
            bank_idx=choose_bank(offset_idx, 1);
            if (update_one_rec(offset, offset_idx, ks1, banks[bank_idx]->adrs_ks1, banks[bank_idx]->ks1, mode)) return -1;
        }
        return 0;
    }
    else if (main_class==3){//item, info->ks1,ks2
        for (int i=0; i<extra_rec; i++){
            int offset_idx=(offset)%bank_size;
            if ((--story_item[offset_idx])<0) return 1;
            int bank_idx=choose_bank(offset_idx,3);
            if (update_one_rec(offset, offset_idx, item, banks[bank_idx]->adrs_item, banks[bank_idx]->item, mode)) return -1;
            if (update_one_rec(offset, offset_idx, info, banks[bank_idx]->adrs_info, banks[bank_idx]->info, mode)) return -1;
            offset = banks[bank_idx]->item[offset_idx].offset_p1;
            int offset2 = banks[bank_idx]->item[offset_idx].offset_p2;

            offset_idx=(offset)%bank_size;
            if ((--story_ks1[offset_idx])<0) return 1;
            bank_idx=choose_bank(offset_idx, 1);
            if (update_one_rec(offset, offset_idx, ks1, banks[bank_idx]->adrs_ks1, banks[bank_idx]->ks1, mode)) return -1;
            offset_next=banks[bank_idx]->ks1[offset_idx].offset_next;

            offset_idx=(offset2)%bank_size;
            if ((--story_ks2[offset_idx])<0) return 1;
            bank_idx=choose_bank(offset_idx,2);
            if (update_one_rec(offset, offset_idx, ks2, banks[bank_idx]->adrs_ks2, banks[bank_idx]->ks2, mode)) return -1;
            if (offset_next==-1) return 1;
        }
        return 0;
    }
    std::cout << "warning in upate_cahce: something went wrong: reached end of func\n";
    return -3;
};

void Cache::save_cache(){
    for (int i=0; i<4; i++){
        for (int j=0; j<bank_size; j++){
            if (banks[i]->adrs_ks1[j]>-1){
                fseek(ks1, banks[i]->adrs_ks1[j]*sizeof(KeySpace1), SEEK_SET);
                fwrite(banks[i]->ks1+j, sizeof(KeySpace1), 1, ks1);
                banks[i]->adrs_ks1[j]=-1;
            }
            if (banks[i]->adrs_ks2[j]>-1){
                fseek(ks2, banks[i]->adrs_ks2[j]*sizeof(KeySpace2), SEEK_SET);
                fwrite(banks[i]->ks2+j, sizeof(KeySpace2), 1, ks2);
                banks[i]->adrs_ks2[j]=-1;
            }
            if (banks[i]->adrs_item[j]>-1){
                fseek(item, banks[i]->adrs_item[j]*sizeof(Item), SEEK_SET);
                fwrite(banks[i]->item+j, sizeof(Item), 1, item);
                banks[i]->adrs_item[j]=-1;
            }
            if (banks[i]->adrs_info[j]>-1){
                fseek(info, banks[i]->adrs_info[j]*sizeof(Information), SEEK_SET);
                fwrite(banks[i]->info+j, sizeof(Information), 1, info);
                banks[i]->adrs_info[j]=-1;
            }
        }
    }
    for (int i=0; i<bank_size; i++) {
        LRU[i][0]=0;
        LRU[i][1]=0;
        LRU[i][2]=0;
    }
};

void Cache::reset_cache(FILE *ks1_new, FILE *ks2_new, FILE *info_new, FILE *item_new){
    ks1=ks1_new;
    ks2=ks2_new;
    item=item_new;
    info=info_new;
    for (int i=0; i<4; i++){
        for (int j=0; j<bank_size; j++){
            banks[i]->adrs_ks1[j]=-1;
            banks[i]->adrs_ks2[j]=-1;
            banks[i]->adrs_item[j]=-1;
            banks[i]->adrs_info[j]=-1;
        }
    }
    for (int i=0; i<bank_size; i++) {
        LRU[i][0]=0;
        LRU[i][1]=0;
        LRU[i][2]=0;
    }
};


int Cache::read_record(int offset, KeySpace1& var){
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=1;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_ks1[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                var=banks[i]->ks1[idx];
                return 0;
            }
        }
        external_mem_req+=1;
        if (j==0) code = update_cache(offset, main_class, 1);
        else {
            std::cout << "warning in read_record: no record with offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
    }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::read_record(int offset, KeySpace2& var){
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=2;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_ks2[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                var=banks[i]->ks2[idx];
                return 0;
            }
        }
        external_mem_req+=1;
        if (j==0) code = update_cache(offset, main_class, 1);
        else {
            std::cout << "warning in read_record: no record with offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
        }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::read_record(int offset, Item& var){
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=3;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_item[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                var=banks[i]->item[idx];
                return 0;
            }
        }
        external_mem_req+=1;
        if (j==0) code = update_cache(offset, main_class, 1);
        else {
            std::cout << "warning in read_record: no record with offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
    }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::read_record(int offset, Information& var){
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=3;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_info[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                var=banks[i]->info[idx];
                return 0;
            }
        }
        external_mem_req+=1;
        if (j==0) code = update_cache(offset, main_class, 1);
        else {
            std::cout << "warning in read_record: no record with offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
    }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::write_record(int offset, KeySpace1& var){
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=1;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_ks1[idx] || banks[i]->adrs_ks1[idx]==-1){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                banks[i]->ks1[idx]=var;
                banks[i]->adrs_ks1[idx]=offset;
                return 0;
            }
        }
        external_mem_req+=1;
        if (j==0) code = update_cache(offset, main_class, 2);
        else {
            std::cout << "warning in write_record: something went wrong: record wiht offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
        }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::write_record(int offset, KeySpace2& var){
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=2;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_ks2[idx] || banks[i]->adrs_ks2[idx]==-1){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                banks[i]->ks2[idx]=var;
                banks[i]->adrs_ks2[idx]=offset;
                return 0;
            }
        }
        external_mem_req+=1;
        if (j==0) code = update_cache(offset, main_class, 2);
        else {
            std::cout << "warning in write_record: something went wrong: record wiht offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
        }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::write_record(int offset, Item& var){
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=3;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_item[idx] || banks[i]->adrs_item[idx]==-1){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                banks[i]->item[idx]=var;
                banks[i]->adrs_item[idx]=offset;
                return 0;
            }
        }
        external_mem_req+=1;
        if (j==0) code = update_cache(offset, main_class, 2);
        else {
            std::cout << "warning in write_record: something went wrong: record wiht offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
        }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::write_record(int offset, Information& var){
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=3;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_info[idx] || banks[i]->adrs_info[idx]==-1){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                banks[i]->info[idx]=var;
                banks[i]->adrs_info[idx]=offset;
                return 0;
            }
        }
        external_mem_req+=1;
        if (j==0) code = update_cache(offset, main_class, 2);
        else {
            std::cout << "warning in write_record: something went wrong: record wiht offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
    }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};
