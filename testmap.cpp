#include "sharemap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <time.h>
#include "pyshmsession.h"

using namespace PyShmSession;

int main(int argn, char* argv[])
{
    if(argn < 2)
        return 0;
    if(!init()){
        printf("fail to init.\n");
        return 0;
    }
    if(std::string("add") == argv[1] && argn == 4){
        add(argv[2], strlen(argv[2]), (unsigned char*)argv[3], strlen(argv[3])+1);
    }
    else if(std::string("del") == argv[1] && argn == 3){
        remove(argv[2], strlen(argv[2]));
    }
    else if(std::string("update") == argv[1] && argn==4){
        update(argv[2], strlen(argv[2]), (unsigned char*)argv[3], strlen(argv[3])+1);
    }
    else if(std::string("get") == argv[1] && argn == 3){
        unsigned char data[128];
        if(get(argv[2], strlen(argv[2]), data, 128))
            printf("%s\n", (char*)data);
        else
            printf("fail to get\n");

    }
    else if(std::string("touch") == argv[1] && argn == 3){
        if(!touch(argv[2], strlen(argv[2])))
            printf("fail to touch\n");

    }
    else if(std::string("recycle") == argv[1] && argn == 3){
        int cycles = atoi(argv[2]);
        recycle(cycles);
    }
    printf("size is %d\n", size());
    return 0;
}

