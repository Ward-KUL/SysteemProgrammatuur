#include "connmgr.h"
#include <unistd.h>

int main(void){
    connmgr_listen(5678);
    printf("Swa?\n");
    sleep(7);
    connmgr_free();
    return 0;
}