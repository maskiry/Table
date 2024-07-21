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


Cache::Cache(FILE *ks1, FILE *ks2, FILE *info, FILE *item, size_t size): 
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
    // std::cout << "        ***begin choose bank***\n";
    int temp;
    for (int i=0; i<4; i++){
        temp=(LRU[idx][mode-1]&replace[i][0])^replace[i][1];
        // std::cout << "          ***i = " << i << "; LRU[" << idx << "][" << mode-1 << "] = " << LRU[idx][mode-1] << "; temp = " << temp << "***\n";
        if (temp==0) {
            LRU[idx][mode-1]=(LRU[idx][mode-1]&access[i][0])|access[i][1];
            // std::cout << "          ***choosed bank = " << i << "; new LRU[" << idx << "][" << mode-1 << "] = " << LRU[idx][mode-1] << "***\n";
            return i;
        }
    }
    return 0;
}

int Cache::update_cache(int offset, int main_class, int mode){
    // std::cout << "      ***begin update_cache; mode = " << mode << "; main_class = " << main_class << "***\n";
    int offset_next;
    std::vector<int> story_ks1(bank_size, 4), story_ks2(bank_size, 4), story_item(bank_size, 4);
    if (main_class==1){//ks1->item, info->ks2
        for (int i=0; i<extra_rec; i++){
            // std::cout << "      ***iteration " << i << "; offset for main_class = " << offset << "***\n";
            int offset_idx=(offset)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_ks1[offset_idx]==offset) return 0;
            }
            if ((--story_ks1[offset_idx])<0) return 1;
            if (offset<0) return -1;
            int bank_idx=choose_bank(offset_idx, 1);
            if (update_one_rec(offset, offset_idx, ks1, banks[bank_idx]->adrs_ks1, banks[bank_idx]->ks1, mode)) return -1;
            offset = banks[bank_idx]->ks1[offset_idx].offset_item;
            offset_next=banks[bank_idx]->ks1[offset_idx].offset_next;
            // std::cout << "        ***bank_idx = " << i << "; offset for item = " << offset << "***\n";

            offset_idx=(offset)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_item[offset_idx]==offset) return 0;
            }
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_info[offset_idx]==offset) return 0;
            }
            if ((--story_item[offset_idx])<0) return 1;
            if (offset<0) return -1;
            bank_idx=choose_bank(offset_idx,3);
            if (update_one_rec(offset, offset_idx, item, banks[bank_idx]->adrs_item, banks[bank_idx]->item, mode)) return -1;
            if (update_one_rec(offset, offset_idx, info, banks[bank_idx]->adrs_info, banks[bank_idx]->info, mode)) return -1;
            offset = banks[bank_idx]->item[offset_idx].offset_p2;

            offset_idx=(offset)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_ks2[offset_idx]==offset) return 0;
            }
            if ((--story_ks2[offset_idx])<0) return 1;
            if (offset<0) return -1;
            bank_idx=choose_bank(offset_idx,2);
            if (update_one_rec(offset, offset_idx, ks2, banks[bank_idx]->adrs_ks2, banks[bank_idx]->ks2, mode)) return -1;
            
            if (offset_next==-1) return 1;
            else offset=offset_next;
        }
        return 0;
    }
    else if (main_class==2){//ks2->item, info->ks1
        for (int i=0; i<extra_rec; i++){
            // std::cout << "      ***iteration " << i << "; offset for main_class = " << offset << "***\n";
            int offset_idx=(offset)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_ks2[offset_idx]==offset) return 0;
            }
            if ((--story_ks2[offset_idx])<0) return 1;
            if (offset<0) return -1;
            int bank_idx=choose_bank(offset_idx,2);
            if (update_one_rec(offset, offset_idx, ks2, banks[bank_idx]->adrs_ks2, banks[bank_idx]->ks2, mode)) return -1;
            offset = banks[bank_idx]->ks2[offset_idx].offset_item;
            offset_next=(offset+1)%bank_size;

            offset_idx=(offset)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_item[offset_idx]==offset) return 0;
            }
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_info[offset_idx]==offset) return 0;
            }
            if ((--story_item[offset_idx])<0) return 1;
            if (offset<0) return -1;
            bank_idx=choose_bank(offset_idx,3);
            if (update_one_rec(offset, offset_idx, item, banks[bank_idx]->adrs_item, banks[bank_idx]->item, mode)) return -1;
            if (update_one_rec(offset, offset_idx, info, banks[bank_idx]->adrs_info, banks[bank_idx]->info, mode)) return -1;
            offset = banks[bank_idx]->item[offset_idx].offset_p2;

            offset_idx=(offset)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_ks1[offset_idx]==offset) return 0;
            }
            if ((--story_ks1[offset_idx])<0) return 1;
            if (offset<0) return -1;
            bank_idx=choose_bank(offset_idx, 1);
            if (update_one_rec(offset, offset_idx, ks1, banks[bank_idx]->adrs_ks1, banks[bank_idx]->ks1, mode)) return -1;

            offset = offset_next;
        }
        return 0;
    }
    else if (main_class==3){//item, info->ks1,ks2
        for (int i=0; i<extra_rec; i++){
            // std::cout << "      ***iteration " << i << "; offset for main_class = " << offset << "***\n";
            int offset_idx=(offset)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_item[offset_idx]==offset) return 0;
            }
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_info[offset_idx]==offset) return 0;
            }
            if ((--story_item[offset_idx])<0) return 1;
            if (offset<0) return -1;
            int bank_idx=choose_bank(offset_idx,3);
            if (update_one_rec(offset, offset_idx, item, banks[bank_idx]->adrs_item, banks[bank_idx]->item, mode)) return -1;
            if (update_one_rec(offset, offset_idx, info, banks[bank_idx]->adrs_info, banks[bank_idx]->info, mode)) return -1;
            offset = banks[bank_idx]->item[offset_idx].offset_p1;
            int offset2 = banks[bank_idx]->item[offset_idx].offset_p2;

            offset_idx=(offset)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_ks1[offset_idx]==offset) return 0;
            }
            if ((--story_ks1[offset_idx])<0) return 1;
            if (offset<0) return -1;
            bank_idx=choose_bank(offset_idx, 1);
            if (update_one_rec(offset, offset_idx, ks1, banks[bank_idx]->adrs_ks1, banks[bank_idx]->ks1, mode)) return -1;
            offset_next=banks[bank_idx]->ks1[offset_idx].offset_next;

            offset_idx=(offset2)%bank_size;
            for (int j=0; j<4; j++){
                if (banks[j]->adrs_ks2[offset_idx]==offset) return 0;
            }
            if ((--story_ks2[offset_idx])<0) return 1;
            if (offset<0) return -1;
            bank_idx=choose_bank(offset_idx,2);
            if (update_one_rec(offset, offset_idx, ks2, banks[bank_idx]->adrs_ks2, banks[bank_idx]->ks2, mode)) return -1;
            
            if (offset_next==-1) return 1;
            else offset=offset_next;
        }
        return 0;
    }
    std::cout << "warning in upate_cahce: something went wrong: reached end of func\n";
    return -3;
};

void Cache::save_cache(){
    for (int i=0; i<4; i++){
        for (int j=0; j<bank_size; j++){
            if ((banks[i]->adrs_ks1[j])>-1){
                fseek(ks1, (banks[i]->adrs_ks1[j])*sizeof(KeySpace1), SEEK_SET);
                fwrite(banks[i]->ks1+j, sizeof(KeySpace1), 1, ks1);
                //delete
                KeySpace1 aa;
                fseek(ks1, (banks[i]->adrs_ks1[j])*sizeof(KeySpace1), SEEK_SET);
                fread(&aa, sizeof(KeySpace1), 1, ks1);
                // std::cout << "save_cache: keyspace1.key = " << aa.key << "; next = " << aa.offset_next << "\n";
                //delete
                banks[i]->adrs_ks1[j]=-1;
            }
            if ((banks[i]->adrs_ks2[j])>-1){
                fseek(ks2, (banks[i]->adrs_ks2[j])*sizeof(KeySpace2), SEEK_SET);
                fwrite(banks[i]->ks2+j, sizeof(KeySpace2), 1, ks2);
                banks[i]->adrs_ks2[j]=-1;
            }
            if ((banks[i]->adrs_item[j])>-1){
                fseek(item, (banks[i]->adrs_item[j])*sizeof(Item), SEEK_SET);
                fwrite(banks[i]->item+j, sizeof(Item), 1, item);
                banks[i]->adrs_item[j]=-1;
            }
            if ((banks[i]->adrs_info[j])>-1){
                fseek(info, (banks[i]->adrs_info[j])*sizeof(Information), SEEK_SET);
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
    // std::cout << "    ***read_record (KS1) begins***\n";
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=1;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_ks1[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                var=banks[i]->ks1[idx];
                // std::cout << "    !!!record (KS1) with offset = " << offset << " read from bank " << i << " with index " << idx << "***\n";
                return 0;
            }
        }
        external_mem_req+=1;
        // std::cout << "    ***record didn't find; trying to update cache***\n";
        if (j==0) code = update_cache(offset, main_class, 1);
        else {
            std::cout << "warning in read_record (KS1): no record with offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
    }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::read_record(int offset, KeySpace2& var){
    // std::cout << "    ***read_record (KS2) begins***\n";
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=2;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_ks2[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                var=banks[i]->ks2[idx];
                // std::cout << "    !!!record (KS2) with offset = " << offset << " read from bank " << i << " with index " << idx << "***\n";
                return 0;
            }
        }
        external_mem_req+=1;
        // std::cout << "    ***record didn't find; trying to update cache***\n";
        if (j==0) code = update_cache(offset, main_class, 1);
        else {
            std::cout << "warning in read_record (KS2): no record with offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
        }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::read_record(int offset, Item& var){
    // std::cout << "    ***read_record (Item) begins***\n";
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=3;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_item[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                var=banks[i]->item[idx];
                // std::cout << "    !!!record (Item) with offset = " << offset << " read from bank " << i << " with index " << idx << "***\n";
                return 0;
            }
        }
        external_mem_req+=1;
        // std::cout << "    ***record didn't find; trying to update cache***\n";
        if (j==0) code = update_cache(offset, main_class, 1);
        else {
            std::cout << "warning in read_record (Item): no record with offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
    }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::read_record(int offset, Information& var){
    // std::cout << "    ***read_record (Information) begins***\n";
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=3;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_info[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                var=banks[i]->info[idx];
                // std::cout << "    !!!record (Information) with offset = " << offset << " read from bank " << i << " with index " << idx << "***\n";
                return 0;
            }
        }
        external_mem_req+=1;
        // std::cout << "    ***record didn't find; trying to update cache***\n";
        if (j==0) code = update_cache(offset, main_class, 1);
        else {
            std::cout << "warning in read_record (Information): no record with offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
    }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::write_record(int offset, KeySpace1& var){
    // std::cout << "    ***wirte_record (KS1) begins***\n";
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=1;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_ks1[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                banks[i]->ks1[idx]=var;
                banks[i]->adrs_ks1[idx]=offset;
                // std::cout << "    !!!record (KS1) with offset = " << offset << " wrote to bank " << i << " with index " << idx << "***\n";
                return 0;
            }
        }
        external_mem_req+=1;
        // std::cout << "    ***cache full; trying to update cache***\n";
        if (j==0) code = update_cache(offset, main_class, 2);
        else {
            std::cout << "warning in write_record (KS1): something went wrong: record wiht offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
        }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::write_record(int offset, KeySpace2& var){
    // std::cout << "    ***wirte_record (KS2) begins***\n";
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=2;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_ks2[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                banks[i]->ks2[idx]=var;
                banks[i]->adrs_ks2[idx]=offset;
                // std::cout << "    !!!record (KS2) with offset = " << offset << " wrote to bank " << i << " with index " << idx << "***\n";
                return 0;
            }
        }
        external_mem_req+=1;
        // std::cout << "    ***cache full; trying to update cache***\n";
        if (j==0) code = update_cache(offset, main_class, 2);
        else {
            std::cout << "warning in write_record (KS2): something went wrong: record wiht offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
        }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::write_record(int offset, Item& var){
    // std::cout << "    ***wirte_record (Item) begins***\n";
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=3;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_item[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                banks[i]->item[idx]=var;
                banks[i]->adrs_item[idx]=offset;
                // std::cout << "    !!!record (Item) with offset = " << offset << " wrote to bank " << i << " with index " << idx << "***\n";
                return 0;
            }
        }
        external_mem_req+=1;
        // std::cout << "    ***cache full; trying to update cache***\n";
        if (j==0) code = update_cache(offset, main_class, 2);
        else {
            std::cout << "warning in write_record (Item): something went wrong: record wiht offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
        }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};

int Cache::write_record(int offset, Information& var){
    // std::cout << "    ***wirte_record (Information) begins***\n";
    all_req+=1;
    int idx=offset%bank_size, code, main_class;
    for (int j=0; j<2; j++){
        main_class=3;
        for (int i=0; i<4; i++){
            if (offset == banks[i]->adrs_info[idx]){
                LRU[idx][main_class-1]=(LRU[idx][main_class-1]&access[i][0])|access[i][1];
                banks[i]->info[idx]=var;
                banks[i]->adrs_info[idx]=offset;
                // std::cout << "    !!!record (Information) with offset = " << offset << " wrote to bank " << i << " with index " << idx << "***\n";
                return 0;
            }
        }
        external_mem_req+=1;
        // std::cout << "    ***cache full; trying to update cache***\n";
        if (j==0) code = update_cache(offset, main_class, 2);
        else {
            std::cout << "warning in write_record (Information): something went wrong: record wiht offset "<< 
                    offset << "(didnt find after updating cache)\n";
            return -3;
        }
    }
    std::cout << "warning in read_record: something went wrong: reached end of func\n";
    return -3;
};
