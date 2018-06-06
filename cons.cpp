#include <iostream>
#include <stdio.h>
#include "monitor.h"

int main()
{
    Monitor monitor1("setup1.txt");

    monitor1.Initialize();

    for(int i=0;i<15;i++){
        monitor1.Lock();
        cout<<"I got: "<<monitor1.Pop()<<endl;
        monitor1.Unlock();
    }

    return 0;
}