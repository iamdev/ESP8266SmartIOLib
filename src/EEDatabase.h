#ifndef _EEDatabase_H
#define _EEDatabase_H
#include "EEPROM_EX.h"

template <class T>
class EEDatabase{
    public : 
        EEDatabase<T>(int ee_start,int ee_size);
        void moveNext();
        void moveStart();
        void insert(T &trx);
        void update(T &trx);
        T getFirst();
        T getLast();
        bool isEmpty();
        void format();
        void load();        
    protected : 
        int getNextIndex(int idx);
        int trx_start_index = 0;
        int trx_next_index = 0;
        int trx_cur_index = 0;        
        int start_idx_address;
        int start_trx_address;
        int storage_size;
        int max_trx;
};

template <class T>
inline
EEDatabase<T>::EEDatabase(int ee_start,int ee_size){
    int index_storage_size =(sizeof(trx_start_index)*2)+1;
    start_idx_address = ee_start;
    start_trx_address = ee_start + index_storage_size;
    storage_size = ee_size;
    max_trx = (int)(storage_size - index_storage_size)/sizeof(T);
}

template <class T>
inline
int EEDatabase<T>::getNextIndex(int idx){
  idx++;
  if(idx>max_trx)idx = 0;
  return idx;
}

template <class T>
inline
void EEDatabase<T>::moveNext(){
    trx_next_index = getNextIndex(trx_next_index);
    eeprom.write(start_idx_address+sizeof(trx_start_index),&trx_next_index,sizeof(trx_next_index));
    if(trx_next_index == trx_start_index){ 
        trx_start_index = getNextIndex(trx_start_index);    
        eeprom.write(start_idx_address,&trx_start_index,sizeof(trx_start_index));
    }

}

template <class T>
inline
void EEDatabase<T>::moveStart(){
    if(trx_next_index != trx_start_index){
        trx_start_index = getNextIndex(trx_start_index);
        eeprom.write(start_idx_address,&trx_start_index,sizeof(trx_start_index));
    }
}

template <class T>
inline
void EEDatabase<T>::insert(T &trx){
    trx_cur_index = trx_next_index;
    eeprom.write(start_trx_address + (trx_cur_index * sizeof(T)),&trx,sizeof(T));
    moveNext(); 
}

template <class T>
inline
void EEDatabase<T>::update(T &trx){
    trx_cur_index = trx_next_index;
    eeprom.write(start_trx_address + (trx_cur_index * sizeof(T)),&trx,sizeof(T));
}

template <class T>
inline
T EEDatabase<T>::getFirst(){
    T trx;
    eeprom.read(start_trx_address + (trx_start_index * sizeof(T)),&trx,sizeof(T));
    return trx;
}

template <class T>
inline
T EEDatabase<T>::getLast(){
    T trx;
    eeprom.read(start_trx_address + (trx_cur_index * sizeof(T)),&trx,sizeof(T));
    return trx;
}
template <class T>
inline
bool EEDatabase<T>::isEmpty(){
    return trx_start_index == trx_next_index;
}

template <class T>
inline
void EEDatabase<T>::format(){
    trx_cur_index = 0;
    trx_start_index = 0;
    trx_next_index = 0;
    eeprom.write(start_idx_address+sizeof(trx_start_index),&trx_next_index,sizeof(trx_next_index));   
    eeprom.write(start_idx_address,&trx_start_index,sizeof(trx_start_index));
}

template <class T>
inline
void EEDatabase<T>::load(){
    eeprom.read(start_idx_address,&trx_start_index,sizeof(trx_start_index));
    eeprom.read(start_idx_address+sizeof(trx_start_index),&trx_next_index,sizeof(trx_next_index));
}

#endif/* _EEDatabase_H*/
