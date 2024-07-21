#include "table.h"

Table::Table(int msize): 
msize(msize), csize(0), hash(msize) {
    id=counter++;
    name_info = "Information_"+std::to_string(id);
    info=fopen(name_info.c_str(), "w+b");
    name_item = "Item_"+std::to_string(id);
    item=fopen(name_item.c_str(), "w+b");
    name_ks1 = "KeySpace1_"+std::to_string(id);
    ks1=fopen(name_ks1.c_str(), "w+b");
    name_ks2 = "KeySpace2_"+std::to_string(id);
    ks2=fopen(name_ks2.c_str(), "w+b");
    cache = new Cache(ks1, ks2, info, item, 2);
    offset_ks1=0;
    offset_last=0;

    KeySpace1 ksp1(0,0,-1,-1);
    rewind(ks1);
    for (int i=0; i<msize+1; i++){
        fwrite(&ksp1, sizeof(KeySpace1), 1, ks1);
    }

    KeySpace2 ksp2("", -1);
    rewind(ks2);
    for (int i=0; i<msize; i++){
        fwrite(&ksp2, sizeof(KeySpace2), 1, ks2);
    }

    Item itm(0, "", -1);
    rewind(item);
    for (int i=0; i<msize; i++){
        fwrite(&itm, sizeof(Item), 1, item);
    }

    Information inf(0, "");
    rewind(info);
    for (int i=0; i<msize; i++){
        fwrite(&inf, sizeof(Information), 1, info);
    }

    fs_ks1=new FreeSpace(-1, nullptr);
    fs_item=new FreeSpace(-1, nullptr);
    fs_info=new FreeSpace(-1, nullptr);
    FreeSpace *temp;
    for (int i=msize; i>0; i--){
        temp = new FreeSpace(i, fs_ks1);
        fs_ks1=temp;
    }
    for (int i=msize-1; i>=0; i--){
        temp = new FreeSpace(i, fs_item);
        fs_item=temp;
        temp = new FreeSpace(i, fs_info);
        fs_info=temp;
    }
    itr = new MyIter();
    lengths = new int[5]{5,5,5,7,7};
}

Table::Table(const Table& other):
msize(other.msize), csize(0), hash(other.msize){
    id=counter++;
    name_info = "Information_"+std::to_string(id);
    info=fopen(name_info.c_str(), "w+b");
    name_item = "Item_"+std::to_string(id);
    item=fopen(name_item.c_str(), "w+b");
    name_ks1 = "KeySpace1_"+std::to_string(id);
    ks1=fopen(name_ks1.c_str(), "w+b");
    name_ks2 = "KeySpace2_"+std::to_string(id);
    ks2=fopen(name_ks2.c_str(), "w+b");
    cache = new Cache(ks1, ks2, info, item, 5);
    offset_ks1=0;
    offset_last=0;

    KeySpace1 ksp1(0,0,-1,-1);
    rewind(ks1);
    for (int i=0; i<msize+1; i++){
        fwrite(&ksp1, sizeof(KeySpace1), 1, ks1);
    }

    KeySpace2 ksp2("", -1);
    rewind(ks2);
    for (int i=0; i<msize+1; i++){
        fwrite(&ksp2, sizeof(KeySpace2), 1, ks2);
    }

    Item itm(0, "", -1);
    rewind(item);
    for (int i=0; i<msize+1; i++){
        fwrite(&itm, sizeof(Item), 1, item);
    }

    Information inf(0, "");
    rewind(info);
    for (int i=0; i<msize+1; i++){
        fwrite(&inf, sizeof(Information), 1, info);
    }

    fs_ks1=new FreeSpace(-1, nullptr);
    fs_item=new FreeSpace(-1, nullptr);
    fs_info=new FreeSpace(-1, nullptr);
    FreeSpace *temp;
    for (int i=msize; i>0; i--){
        temp = new FreeSpace(i, fs_ks1);
        fs_ks1=temp;
    }
    for (int i=msize-1; i>=0; i--){
        temp = new FreeSpace(i, fs_item);
        fs_item=temp;
        temp = new FreeSpace(i, fs_info);
        fs_info=temp;
    }
    itr = new MyIter();
    lengths = new int[5]{5,5,5,7,7};
    

    Information inform;
    msize=other.msize;
    hash=other.hash;
    MyIter other_iter = other.get_iter();
    while (!other_iter.finish){
        other.cache->read_record(other_iter.offset_p1, ksp1);
        other.cache->read_record(other_iter.offset_info, inform);
        add(ksp1.par, ksp1.key, other_iter.key2.to_str(), inform.number, inform.str.to_str());
        other_iter.next();
    }
    
}

Table::Table(Table&& other):
ks1(other.ks1), ks2(other.ks2), info(other.info), item(other.item), msize(other.msize), 
csize(other.csize), name_info(other.name_info), name_item(other.name_item), 
name_ks1(other.name_ks1), name_ks2(other.name_ks2), hash(other.hash), lengths(other.lengths), 
itr(other.itr), offset_ks1(other.offset_ks1), offset_last(other.offset_last), cache(other.cache){
    other.ks1=nullptr;
    other.ks2=nullptr;
    other.lengths=nullptr;
    other.itr=nullptr;
    other.item=nullptr;
    other.info=nullptr;
    fs_ks1=other.fs_ks1;
    other.fs_ks1=nullptr;
    fs_item=other.fs_ks1;
    other.fs_item=nullptr;
    fs_info=other.fs_ks1;
    other.fs_info=nullptr;
    other.cache=nullptr;
}

Table::~Table(){
    cache->save_cache();

    free_mem();
    //remove or not...
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
}

void Table::free_mem(){
    fclose(ks1);
    fclose(ks2);
    fclose(info);
    fclose(item);

    FreeSpace* act=fs_ks1;
    while (act->next!=nullptr){
        FreeSpace* rem=act;
        act=act->next;
        delete rem;
    }
    fs_ks1=act;
    fs_ks1->offset=-1;
    act=fs_item;
    while (act->next!=nullptr){
        FreeSpace* rem=act;
        act=act->next;
        delete rem;
    }
    fs_item=act;
    fs_info->offset=-1;
    act=fs_info;
    while (act->next!=nullptr){
        FreeSpace* rem=act;
        act=act->next;
        delete rem;
    }
    fs_info=act;
    fs_info->offset=-1;
    csize=0;
}

void Table::clear(){
    free_mem();

    info=fopen(name_info.c_str(), "w+b");
    item=fopen(name_item.c_str(), "w+b");
    ks1=fopen(name_ks1.c_str(), "w+b");
    ks2=fopen(name_ks2.c_str(), "w+b");
    cache->reset_cache(ks1, ks2, info, item);
    offset_ks1=0; offset_last=0;
    *itr = MyIter(0,"", -1,-1,-1);

    KeySpace1 ksp1(0,0,-1,-1);
    rewind(ks1);
    for (int i=0; i<msize+1; i++){
        fwrite(&ksp1, sizeof(KeySpace1), 1, ks1);
    }
    
    KeySpace2 ksp2("", -1);
    rewind(ks2);
    for (int i=0; i<msize; i++){
        fwrite(&ksp2, sizeof(KeySpace2), 1, ks2);
    }
    
    Item itm(0, "", -1);
    rewind(item);
    for (int i=0; i<msize; i++){
        fwrite(&itm, sizeof(Item), 1, item);
    }
    
    Information inf(0, "");
    rewind(info);
    for (int i=0; i<msize; i++){
        fwrite(&inf, sizeof(Information), 1, info);
    }
    
    FreeSpace *temp;
    for (int i=msize; i>0; i--){
        temp = new FreeSpace(i, fs_ks1);
        fs_ks1=temp;
    }
    for (int i=msize-1; i>=0; i--){
        temp = new FreeSpace(i, fs_item);
        fs_item=temp;
        temp = new FreeSpace(i, fs_info);
        fs_info=temp;
    }
    
    lengths[0]=5;lengths[1]=5;lengths[2]=5;lengths[3]=7;lengths[4]=7;
}

KeySpace1 Table::find_in_KS1(int key){
    int cur=offset_ks1, code;
    KeySpace1 ans(0,0,-1,-1);
    KeySpace1 ksp1;
    while (cur>=0){
        code = cache->read_record(cur, ksp1);
        if (code) {
            std::cout << "find_in_KS1: cant get such record: offset = " << cur << "\n";
            return ans;
        }
        if (key==ksp1.key){
            ans=ksp1;
            break;
        }
        cur=ksp1.offset_next;
    }
    return ans;
};

KeySpace2 Table::find_in_KS2(MyString key){
    int cur=0, code;
    KeySpace2 ans("", -1);
    int init_pos = hash_func(key), pos;
    KeySpace2 ksp2;
    while (cur<msize){
        pos=(init_pos+cur)%msize;
        code = cache->read_record(pos, ksp2);
        if (code) {
            std::cout << "find_in_KS2: cant get such record: offset = " << pos << "\n";
            return ans;
        }
        if (key.to_str()==ksp2.key.to_str() && ksp2.busy==1){
            ans=ksp2;
            break;
        }
        else if (ksp2.busy==0){
            break;
        }
        cur++;
    }
    return ans;
};

int Table::hash_func(const MyString& s){
    int h=0;
    for (int i=0; i<s.size(); i++){
        h+=(int) s[i];
    }
    return h%hash;
}

int Table::len_num(int n){
    int len=0;
    if (n==0) return 1;
    while (n){
        n=n/10;
        len++;
    }
    return len;
}

int Table::add(int par, int key1, string key2, float num_info, string str_info){
    if (par && find_in_KS1(par).offset_item==-1){
        std::cout << "No such parent: " << par << "\n";
        return -1;
    }
    if (csize == msize){
        std::cout << "Not enough place\n";
        return -1; 
    }
    if (find_in_KS1(key1).offset_item!=-1 || find_in_KS2(key2).offset_item!=-1){
        std::cout << "key1 = " << key1 << "; key2 = " << key2;
        std::cout << ": Can't add, not unique\n";
        return -1;
    }
    if (key1==0){
        std::cout << "Key1 can't be 0\n";
        return -1;
    }
    int l;
    l=len_num(par);
    if (l>lengths[0]) lengths[0]=l;
    l=len_num(key1);
    if (l>lengths[1]) lengths[1]=l;
    if (key2.size()>lengths[2]) lengths[2]=key2.size();
    l = len_num((int) num_info);
    if (l>lengths[3]) lengths[3]=l;
    if (str_info.size()>lengths[4]) lengths[4]=str_info.size();

    int pos_info, code;
    Information inf(num_info, str_info);
    pos_info=fs_info->offset;
    if (pos_info==-1){
        string err_msg = "table.add(int, string, float, string): something went wrong (not enough place)";
        throw std::runtime_error(err_msg);
    }
    code = cache->write_record(pos_info, inf);
    if (code) {
        throw  ("add: cant write record into info; code = " + std::to_string(code));
    }

    int pos_item;
    Item it(key1, key2, pos_info);
    pos_item=fs_item->offset;

    it.offset_p1 = add_in_KS1(key1, par, pos_item);
    it.offset_p2 = add_in_KS2(key2, pos_item);

    FreeSpace* rem = fs_info;
    fs_info=fs_info->next;
    delete rem;
    rem = fs_item;
    fs_item=fs_item->next;
    delete rem;

    code = cache->write_record(pos_item, it);
    if (code) {
        throw  ("add: cant write record into item; code = " + std::to_string(code));
    }

    // cache->save_cache();
    csize++;

    return 0;
}

int Table::add_in_KS1(int key, int par, int offset_item){
    int pos_ks1, code;
    KeySpace1 ks1_last;
    KeySpace1 ksp1_new(key, par, -1, offset_item);
    pos_ks1=fs_ks1->offset;
    FreeSpace* rem = fs_ks1;
    fs_ks1=fs_ks1->next;
    delete rem;
    code = cache->read_record(offset_last, ks1_last);
    ks1_last.offset_next=pos_ks1;
    code = cache->write_record(offset_last, ks1_last);
    offset_last=pos_ks1;
    code = cache->write_record(pos_ks1, ksp1_new);
    if (code) {
        throw  ("add_in_KS1: cant write record into ks1; code = " + std::to_string(code));
    }
    return pos_ks1;
}

int Table::add_in_KS2(MyString key, int offset_item){
    int cur=0, ans=-1, code;
    int init_pos = hash_func(key), pos_ks2;
    pos_ks2=init_pos-1;
    KeySpace2 ksp2, ksp2_new(key, offset_item);
    do{
        pos_ks2=(pos_ks2+1)%msize;
        code = cache->read_record(pos_ks2, ksp2);
        if (code) {
            throw  ("add_in_KS2: cant read record from ks2; code = " + std::to_string(code));
        }
    } while (ksp2.busy==1);
    code = cache->write_record(pos_ks2, ksp2_new);
    if (code) {
        throw  ("add_in_KS2: cant write record into ks2; code = " + std::to_string(code));
    }
    return pos_ks2;
}

int Table::del(int key1, string key2){
    KeySpace1 unit1 = find_in_KS1(key1);
    KeySpace2 unit2 = find_in_KS2(key2);
    int code;
    if ((unit2.offset_item==-1 && unit1.offset_item==-1) || (unit1.offset_item!=unit2.offset_item)){
         std::cout << "no such key: \"" << key1 << "\", \"" << key2 << "\"\n";
         return -1;
    }
    Item cur_item;
    code = cache->read_record(unit2.offset_item, cur_item);
    
    FreeSpace *deleted_pos = new FreeSpace(unit2.offset_item, fs_item);
    fs_item=deleted_pos;
    deleted_pos = new FreeSpace(cur_item.offset_info, fs_info);
    fs_info=deleted_pos;
    lengths[0]=5;lengths[1]=5;lengths[2]=5;lengths[3]=7;lengths[4]=7;
    
    unit2.busy=2;
    unit2.key="";
    unit2.offset_item=-1;
    code = cache->write_record(cur_item.offset_p2, unit2);
    if (code) {
        throw  ("del: cant write record into ks2; code = " + std::to_string(code));
    }

    KeySpace1 ksp1_prev, ksp1;
    Information cur_inf;
    code = cache->read_record(offset_ks1, ksp1_prev);
    if (code) {
        throw  ("del: cant read record from ks1; code = " + std::to_string(code));
    }

    int prev=offset_ks1;
    offset_last=offset_ks1;
    while (ksp1_prev.offset_next!=-1){
        code = cache->read_record(ksp1_prev.offset_next, ksp1);
        if (code) {
            throw  ("del: cant read record from ks1 "+std::to_string(code));
        }
        if (ksp1.key==key1){
            deleted_pos = new FreeSpace(ksp1_prev.offset_next, fs_ks1);
            fs_ks1=deleted_pos;
            ksp1_prev.offset_next=ksp1.offset_next;
            code = cache->write_record(prev, ksp1_prev);
            if (code) {
                throw  ("del: cant write record into ks1; code = " + std::to_string(code));
            }
        }
        else{
            code = cache->read_record(ksp1.offset_item, cur_item);
            if (code) {
                throw  ("del: cant read record from item; code = " + std::to_string(code));
            }
            code = cache->read_record(cur_item.offset_info, cur_inf);
            if (code) {
                throw  ("del: cant read record from info; code = " + std::to_string(code));
            }
            int l;
            l=len_num(ksp1.par);
            if (l>lengths[0]) lengths[0]=l;
            l=len_num(cur_item.key1);
            if (l>lengths[1]) lengths[1]=l;
            if (cur_item.key2.size()>lengths[2]) lengths[2]=cur_item.key2.size();
            l = len_num(cur_inf.number);
            if (l>lengths[3]) lengths[3]=l;
            l = (cur_inf.str).size();
            if (l>lengths[4]) lengths[4]=l;

            if (ksp1.par==key1) {
                ksp1.par=0;
                code=cache->write_record(ksp1_prev.offset_next, ksp1);
                if (code) {
                    throw  ("del: cant write record into ks1; code = " + std::to_string(code));
                }
            }
            offset_last=ksp1_prev.offset_next;
            prev=ksp1_prev.offset_next;
            ksp1_prev=ksp1;
        }
    }
    csize--;

    return 0;
};

int Table::del(int key1){
    int code;
    KeySpace1 unit1 = find_in_KS1(key1);
    if (unit1.offset_item==-1){
        std::cout << "no such key\n";
        return -1;
    }
    KeySpace2 unit2;
    Item itm;
    code = cache->read_record(unit1.offset_item, itm);
    if (code) {
        throw  ("del: cant read record from item; code = " + std::to_string(code));
    }
    code = cache->read_record(itm.offset_p2, unit2);
    if (code) {
        throw  ("del: cant read record from ks2; code = " + std::to_string(code));
    }
    string key2=unit2.key.to_str();
    return del(key1,key2);
};

int Table::del(string key2){
    int code;
    KeySpace2 unit2 = find_in_KS2(key2);
    if (unit2.offset_item==-1){
        std::cout << "no such key\n";
        return -1;
    }
    KeySpace1 unit1;
    Item itm;
    code = cache->read_record(unit2.offset_item, itm);
    if (code) {
        throw  ("del: cant read record from item; code = " + std::to_string(code));
    }
    code = cache->read_record(itm.offset_p1, unit1);
    if (code) {
        throw  ("del: cant read record from ks1; code = " + std::to_string(code));
    }
    int key1 = unit1.key;
    return del(key1,key2);
};

const int* Table::get_lengths() const{
    return lengths;
}

MyIter& Table::get_iter() const{
    if (csize==0) *itr = MyIter(0,"", -1,-1,-1);
    else {
        KeySpace1 ksp1;
        Item it;
        cache->read_record(offset_ks1, ksp1);
        cache->read_record(ksp1.offset_next, ksp1);
        cache->read_record(ksp1.offset_item, it);
        *itr = MyIter(it);
        itr->cache=cache;
    }
    return *(itr);
}

Table Table::slice_by_parent(int a, int b){
    Table out(msize);
    KeySpace1 actual;
    cache->read_record(offset_ks1, actual);
    while (actual.offset_next>=0){
        cache->read_record(actual.offset_next, actual);
        if (actual.par>=a && actual.par<b){
            Item it;
            Information inform;
            cache->read_record(actual.offset_item, it);
            cache->read_record(actual.offset_item, inform);
            if (out.find_in_KS1(actual.par).offset_item>=0) out.add(actual.par, it.key1, it.key2.to_str(), inform.number, inform.str.to_str());
            else out.add(0, it.key1, it.key2.to_str(), inform.number, inform.str.to_str());
        }
    }
    return out;
}

std::ostream& operator << (std::ostream& out, MyIter& iter){
    out << "[" << iter.get_info().number << "; "<< iter.get_info().str.to_str() << "]\n";
    return out;
};

std::ostream& operator << (std::ostream& out, Table& tb){
    int precsision=2;
    string head[5]={"par", "key1", "key2", "number", "string"};
    const int* l=tb.get_lengths(); 

    for (int i=0; i<l[0]+l[1]+l[2]+l[3]+l[4]+precsision+1+16; i++){
        out << "-";
    }
    out << "\n";
    out << "| ";
    for (int i=0; i<5; i++){
        if (i==3) out << std::setw(l[i]+precsision+1) << std::left << head[i];
        else out << std::setw(l[i]) << std::left << head[i];
        if (i<4) out << " | ";
        else out << " |\n";
    }
    for (int i=0; i<l[0]+l[1]+l[2]+l[3]+l[4]+precsision+1+16; i++){
        out << "-";
    }
    out << "\n";
    MyIter act=tb.get_iter();
    while (!act.get_finish()){
        out << "| ";
        out << std::setw(l[0]) << std::left << act.get_par();
        out << " | ";
        out << std::setw(l[1]) << std::left << act.get_key1();
        out << " | ";
        out << std::setw(l[2]) << std::left << act.get_key2().to_str();
        out << " | ";
        out << std::setw(l[3]+precsision+1) << std::left << std::fixed << 
               std::setprecision(precsision) << act.get_info().number;
        out << " | ";
        out << std::setw(l[4]) << std::left << act.get_info().str.to_str();
        out << " |\n";
        act.next();
    }

    for (int i=0; i<l[0]+l[1]+l[2]+l[3]+l[4]+precsision+1+16; i++){
        out << "-";
    }
    out << "\n";



    return out; 
};