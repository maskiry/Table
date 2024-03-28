#include "table.h"

Table::Table(int msize): 
msize(msize), csize(0), hash(msize) {
    ks1 = new KeySpace1(0,0,nullptr,nullptr);
    last=ks1;
    try {
        ks2 = new KeySpace2[msize];
    }
    catch (std::bad_alloc& ex) {
        std::cout << "Caught bad_alloc: " << ex.what() << std::endl;
        return;
    }
    itr = new MyIter();
    lengths = new int[5]{5,5,5,7,7};
}

Table::Table(const Table& other):
msize(other.msize), csize(0), hash(other.msize){
    ks1 = new KeySpace1(0,0,nullptr,nullptr);
    last=ks1;
    try {
        ks2 = new KeySpace2[other.msize];
    }
    catch (std::bad_alloc& ex) {
        std::cout << "Caught bad_alloc: " << ex.what() << std::endl;
        return;
    }
    itr = new MyIter();
    lengths = new int[5];

    lengths[0]=other.lengths[0];lengths[1]=other.lengths[1];
    lengths[1]=other.lengths[1];lengths[3]=other.lengths[3];
    lengths[4]=other.lengths[4];
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

Table::Table(Table&& other):
ks1(other.ks1), ks2(other.ks2), msize(other.msize), csize(other.csize), 
hash(other.msize), lengths(other.lengths), itr(other.itr), last(other.last){
    other.ks1=nullptr;
    other.ks2=nullptr;
    other.lengths=nullptr;
    other.itr=nullptr;
    other.last=nullptr;
}

Table::~Table(){
    KeySpace1 *rem = ks1;
    KeySpace1 *next=ks1->next;
    while (next){
        delete rem;
        delete next->item;
        rem=next;
        next = next->next;
    };
    delete rem;
    delete [] ks2;
    delete lengths;
    delete itr;
}

void Table::clear(){
    KeySpace1 *rem = ks1->next;
    KeySpace1 *next;
    ks1->next=nullptr;
    while (rem){
        next = rem->next;
        delete rem->item;
        delete rem;
        rem=next;
    };
    for (int i=0; i<msize; i++){
        ks2[i].busy=0;
        ks2[i].item=nullptr;
        ks2[i].key="";
    }
    lengths[0]=5;lengths[1]=5;lengths[2]=5;lengths[3]=7;lengths[4]=7;
    csize=0;
    // itr=nullptr;
}

KeySpace1* Table::find_in_KS1(int key){
    KeySpace1* ans = nullptr;
    KeySpace1* actual=ks1;
    bool flag=(actual->key==key);
    while (!flag && actual->next){
        actual = actual->next;
        flag=(actual->key==key);
    }
    if (flag) ans=actual;
    return ans;
};

KeySpace2* Table::find_in_KS2(string key){
    KeySpace2* ans = nullptr;
    int init_pos = hash_func(key);
    int pos=init_pos;
    bool flag=(ks2[init_pos].key==key && ks2[init_pos].busy==1);
    while (!flag && (pos+1)%msize!=init_pos && ks2[(pos+1)%msize].busy!=0){
        pos=(pos+1)%msize;
        flag=(ks2[pos].key==key);
    }
    if (flag) ans=&ks2[pos];
    return ans;
};

int Table::hash_func(const string& s){
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
    if (par && !find_in_KS1(par)){
        std::cout << "No such parent: " << par << "\n";
        return -1;
    }
    if (csize == msize){
        std::cout << "Not enough place\n";
        return -1; 
    }
    if (find_in_KS1(key1) || find_in_KS2(key2)){
        // std::cout << find_in_KS1(key1) << "; " << find_in_KS2(key2) << "\n";
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


    Information *info = new Information(num_info, str_info);
    Item *it = new Item(key1, key2, info);

    it->p1 = add_in_KS1(key1, par, it);
    it->p2 = add_in_KS2(key2, it);
    csize++;

    return 0;
}

KeySpace1* Table::add_in_KS1(int key, int par, Item *it){
    KeySpace1 *ksp1 = new KeySpace1(key, par, nullptr, it);
    last->next=ksp1;
    last=ksp1;
    return ksp1;
}

KeySpace2* Table::add_in_KS2(string key, Item *it){
    int init_pos = hash_func(key);
    int pos=init_pos;
    while (ks2[pos].busy==1){
        pos=(pos+1)%msize;
        if (pos==init_pos) continue;
    }
    ks2[pos].key=key;
    ks2[pos].item=it;
    ks2[pos].busy=1;
    return ks2+pos;
}

int Table::del(int key1, string key2){
    KeySpace1* unit1 = find_in_KS1(key1);
    KeySpace2* unit2 = find_in_KS2(key2);
    if ((!unit1 && !unit2) || (unit1->item!=unit2->item)){
        std::cout << "no such key: " << key1 << ", " << key2 << "\n";
        return -1;
    }
    lengths[0]=5;lengths[1]=5;lengths[2]=5;lengths[3]=7;lengths[4]=7;

    unit2->item=nullptr;
    unit2->busy=2;
    unit2->key="";
    delete unit1->item;
    KeySpace1* actual=ks1;
    while (actual->next!=nullptr){
        if (actual->next==unit1){
            KeySpace1* temp;
            temp=(actual->next);
            actual->next=temp->next;
            delete temp;
        }
        else{
            int l;
            l=len_num(actual->next->par);
            if (l>lengths[0]) lengths[0]=l;
            l=len_num(actual->next->key);
            if (l>lengths[1]) lengths[1]=l;
            if (actual->next->item->key2.size()>lengths[2]) lengths[2]=actual->next->item->key2.size();
            l = len_num(actual->next->item->info->number);
            if (l>lengths[3]) lengths[3]=l;
            l = (actual->next->item->info->str).size();
            if (l>lengths[4]) lengths[4]=l;
            actual=actual->next;
        }
        if (actual->par==key1) actual->par=0;
    }


    csize--;

    return 0;
};

int Table::del(int key1){
    KeySpace1* unit1 = find_in_KS1(key1);
    if (!unit1){
        std::cout << "no such key\n";
        return -1;
    }
    KeySpace2* unit2 = (unit1->item)->p2;
    string key2=unit2->key;
    return del(key1,key2);
};

int Table::del(string key2){
    KeySpace2* unit2 = find_in_KS2(key2);
    if (!unit2){
        std::cout << "no such key\n";
        return -1;
    }
    KeySpace1* unit1 = (unit2->item)->p1;
    int key1 = unit1->key;

    return del(key1,key2);
};

const int* Table::get_lengths() const{
    return lengths;
}

MyIter& Table::get_iter() const{
    if (!ks1->next) *itr = MyIter(0,"", nullptr,nullptr,nullptr);
    else *itr = MyIter(*ks1->next->item);
    return *(itr);
}

Table Table::slice_by_parent(int a, int b){
    Table out(msize);
    KeySpace1* actual=ks1->next;
    while (actual){
        if (actual->par>=a && actual->par<b){
            Item* it = actual->item;
            int code;
            if (out.find_in_KS1(actual->par)) out.add(actual->par, it->key1, it->key2, it->info->number, it->info->str);
            else out.add(0, it->key1, it->key2, it->info->number, it->info->str);
        }
        actual = actual->next;
    }
    return out;
}

std::ostream& operator << (std::ostream& out, MyIter& iter){
    out << "[" << iter.get_info().number << "; "<< iter.get_info().str << "]\n";
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
        out << std::setw(l[2]) << std::left << act.get_key2();
        out << " | ";
        out << std::setw(l[3]+precsision+1) << std::left << std::fixed << 
               std::setprecision(precsision) << act.get_info().number;
        out << " | ";
        out << std::setw(l[4]) << std::left << act.get_info().str;
        out << " |\n";
        act.next();
    }

    for (int i=0; i<l[0]+l[1]+l[2]+l[3]+l[4]+precsision+1+16; i++){
        out << "-";
    }
    out << "\n";



    return out; 
};