#include <iostream>
#include <stdio.h>
#include "monitor.h"

int main()
{
    Monitor monitor0("setup0.txt");

    monitor0.Initialize();

    for(int i=0;i<15;i++){
        monitor0.Lock();
        monitor0.Put(i);
        cout<<"I put: "<<i<<endl;
        monitor0.Unlock();
    }

    return 0;
}