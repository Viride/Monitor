#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "monitor.h"

int main()
{
    Monitor monitor1("setup2.txt");

    monitor1.Initialize();

    for(int i=0;i<15;i++){
        sleep(1);
        monitor1.Lock();
        cout<<"Consument -> I got: "<<monitor1.Pop()<<endl;
        monitor1.Unlock();
    }    

    return 0;
}