#include <sys/ipc.h>
#include <sys/shm.h>
#include <map>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sharemap.h"
#include <Python.h>

namespace PyShmSession
{

int Map::BLOCKSIZE;
int Map::MAPSIZE;

Map::Map(void* base, int blocksize, int maxitems):
    __size(*(int*)base), 
    __freelist(*(int*)(base + sizeof(int))),
    __hash((int*)(base + sizeof(int)*2)),
    __timeslice((int*)(base + sizeof(int)*2 + sizeof(int) * maxitems)),
    __blocks((MemBlock*)(base + sizeof(int)*2 + sizeof(int)*maxitems + 60 * sizeof(int)))
{
    BLOCKSIZE = blocksize;
    MAPSIZE = maxitems;
}

bool Map::init()
{
    //初始化内存块
    for(int i=0; i < MAPSIZE-1; i++){
        __blocks[i].pre = i - 1;
        __blocks[i].next = i+1;
        __blocks[i].timelinkNext = -1;
        __blocks[i].timelinkPre = -1;
        __blocks[i].khash = -1;
    }
    __blocks[MAPSIZE-1].next = -1;
    __blocks[MAPSIZE-1].pre = MAPSIZE - 2;
    __freelist = 0;
    __size = 0;
    for(int i = 0; i < MAPSIZE; i++){
        __hash[i] = -1;
    }
    for(int i = 0; i < 60; i++){
        __timeslice[i] = -1;
    }
    return true;
}

int Map::hashint(const char* data, int len)
{
    //hail to Daniel J.Bernstein
    unsigned int hash = 5381;
    for(int i = 0; i < len; i++)
    {
        hash = ((hash << 5) + hash) + (unsigned char)data[i];
    }
    return hash % MAPSIZE;
}

bool Map::get(const char* key, int klen, unsigned char* data, int len)
{
    if(klen + len + 1 > BLOCKSIZE)
        return false;
    int index = __hash[hashint(key,klen)]; 
    if(index == -1)
        return false;
    else {
        while(index >= 0){
            int bklen = strlen((const char*)&(__blocks[index].data));
            if(memcmp(key, __blocks[index].data, bklen)==0){
                if(len > BLOCKSIZE-bklen-1)
                    return false;
                else {
                    unsigned char* pd = &(__blocks[index].data[bklen + 1]);
                    memcpy(data, pd, len);
                    return true;
                }
            } else {
                index = __blocks[index].next;
            }
        }
        return false;
    }
}

bool Map::touch(const char* key, int klen)
{
    if(klen + 1 > BLOCKSIZE)
        return false;
    int index = __hash[hashint(key,klen)]; 
    if(index == -1)
        return false;
    else {
        while(index >= 0){
            int bklen = strlen((const char*)&(__blocks[index].data));
            if(memcmp(key, __blocks[index].data, bklen)==0){
                //调整时间链 
                delTimeLink(index);
                addTimeLink(index, 0);
                return true;
            } else {
                index = __blocks[index].next;
            }
        }
        return false;
    }
}

void Map::recycle(int ncycles)
{
    if(ncycles <= 0)
        return;
    if(ncycles >= 60){
        for(int i = 0; i < 60; i++){
            while(__timeslice[i] >= 0){
                release(__timeslice[i]);
            }
        }
        return;
    }
    for(int i = 59; i > 59 - ncycles; i--){
        while(__timeslice[i] >= 0){
            release(__timeslice[i]);
        }
    }
    for(int i = 59 - ncycles; i >= 0; i--){
        if(__timeslice[i] >= 0){
            __timeslice[i + ncycles] = __timeslice[i];
            __blocks[__timeslice[i]].timelinkPre -= ncycles;
            __timeslice[i] = -1;
        }
    }
}

bool Map::add(const char* key, int klen, const unsigned char* data, int len)
{
    if(__freelist < 0)
        return false;
    if(len + klen +1 > BLOCKSIZE)
        return false;
    int index = __freelist;
    int hash = hashint(key, klen);
    addHashLink(index, hash);
    addTimeLink(index, 0);
    memcpy(__blocks[index].data, key, klen);
    __blocks[index].data[klen] = 0;
    memcpy(&(__blocks[index].data[klen + 1]), data, len);
    __size += 1;
    return true;
}

void Map::del(const char* key, int len)
{
    if(len > BLOCKSIZE)
        return;
    int hash = hashint(key, len);
    int index = __hash[hash];
    //
    while(index>= 0){
        if(memcmp(__blocks[index].data, key, len) == 0){
            release(index);
            break;
        }
        index = __blocks[index].next;
    }
}

bool Map::update(const char* key, int klen, const unsigned char* data, int len)
{
    char key_[128];
    char data_[1024];
    memcpy((unsigned char*)key_, key, klen);
    key_[klen] = 0;
    memcpy((unsigned char*)data_, data, len);
    data_[len] = 0;
    FILE* fp = fopen("testout", "w");
    fprintf(fp, "%s,%d,%s,%d\n", key_, klen, data_, len);
    fclose(fp);
    if(klen + len + 1 > BLOCKSIZE)
        return false;
    int index = __hash[hashint(key,klen)]; 
    if(index == -1)
        return false;
    else {
        while(index >= 0){
            int bklen = strlen((const char*)&(__blocks[index].data));
            if(memcmp(key, __blocks[index].data, bklen)==0){
                if(len > BLOCKSIZE-bklen-1)
                    return false;
                else {
                    unsigned char* pd = &(__blocks[index].data[bklen + 1]);
                    memcpy(pd, data, len);
                    return true;
                }
            } else {
                index = __blocks[index].next;
            }
        }
        return false;
    }
}

int Map::size()
{
    return __size;
}

}

