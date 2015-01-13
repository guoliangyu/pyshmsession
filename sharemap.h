#include <utility>

namespace PyShmSession
{

struct MemBlock
{
    int khash;
    int pre;
    int next;
    int timelinkNext;
    int timelinkPre;
    unsigned char data[1];
};


class Map
{
    private:
        int& __size;
        int& __freelist;
        int *__hash;
        int *__timeslice;
        static int BLOCKSIZE;
        static int MAPSIZE;

        class BlockArray
        {
            private:
                void* __ptr;
            public:
                BlockArray(void* ptr):__ptr(ptr){}
                MemBlock& operator[](int i){
                    return *(MemBlock*)(__ptr + i * (sizeof(MemBlock) - 1 + BLOCKSIZE));
                }
        };
        BlockArray __blocks;
        int hashint(const char* data, int len);
        void delTimeLink(int index)
        {
            if(__blocks[index].timelinkNext >= 0)
                __blocks[__blocks[index].timelinkNext].timelinkPre = __blocks[index].timelinkPre;
            if(__blocks[index].timelinkPre >= 0)
                __blocks[__blocks[index].timelinkPre].timelinkNext = __blocks[index].timelinkNext;
            else
                __timeslice[-__blocks[index].timelinkPre-2] = __blocks[index].timelinkNext;
            __blocks[index].timelinkNext = -1;
            __blocks[index].timelinkPre = -1;
        }
        void addTimeLink(int index, int slice)
        {
            __blocks[index].timelinkNext = __timeslice[slice];
            __blocks[index].timelinkPre = -slice-2;
            if(__timeslice[slice] >= 0)
                __blocks[__timeslice[slice]].timelinkPre = index;
            __timeslice[slice] = index;
        }
        void delHashLink(int index)
        {
            if(__blocks[index].next >= 0)
                __blocks[__blocks[index].next].pre = __blocks[index].pre;
            if(__blocks[index].pre >= 0)
                __blocks[__blocks[index].pre].next = __blocks[index].next;
            else
                __hash[__blocks[index].khash] = __blocks[index].next;
            __blocks[index].next = __freelist;
            if(__freelist >= 0)
                __blocks[__freelist].pre = index;
            __freelist = index;
            __blocks[index].pre = -1;
            __blocks[index].khash = -1;
        }
        void addHashLink(int index, int hash)
        {
            if(__blocks[index].next >= 0)
                __blocks[__blocks[index].next].pre = __blocks[index].pre;
            if(__blocks[index].pre >= 0)
                __blocks[__blocks[index].pre].next = __blocks[index].next;
            else
                __freelist = __blocks[index].next;
            __blocks[index].next = __hash[hash];
            if(__hash[hash] >= 0)
                __blocks[__hash[hash]].pre = index;
            __hash[hash] = index;
            __blocks[index].pre = -1;
            __blocks[index].khash = hash;
        }
        void release(int index)
        {
            delHashLink(index);
            delTimeLink(index);
            __size -= 1;
        }
    public:
        Map(void* base, int blocksize, int maxitems);
        bool init();
        bool get(const char* key, int klen, unsigned char* data, int len);
        bool add(const char* key, int klen, const unsigned char* data, int len);
        void del(const char* key, int klen);
        bool update(const char* key, int klen, const unsigned char* data, int len);
        bool touch(const char* key, int klen);
        void recycle(int ncycles);
        int size();
};

}

