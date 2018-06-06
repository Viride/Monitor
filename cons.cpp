#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "monitor.h"

int main()
{
    Monitor monitor1("setup1.txt");

    monitor1.Initialize();

    // for(int i=0;i<15;i++){
    //     monitor1.Lock();
    //     cout<<"I got: "<<monitor1.Pop()<<endl;
    //     monitor1.Unlock();
    // }
    monitor1.Lock();
    cout << "I pop: " << monitor1.Pop() << endl;
    monitor1.Unlock();
    sleep(2);
    monitor1.Lock();
    cout << "I pop: " << monitor1.Pop() << endl;
    monitor1.Unlock();
    sleep(2);
    monitor1.Lock();
    cout << "I pop: " << monitor1.Pop() << endl;
    monitor1.Unlock();
    sleep(2);
    monitor1.Lock();
    monitor1.Put(4);
    cout << "I put: " << 4 << endl;
    monitor1.Unlock();

    return 0;
}