#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "monitor.h"

int main()
{
    Monitor monitor0("setup0.txt");

    monitor0.Initialize();

    // for (int i = 0; i < 15; i++)
    // {
    //     monitor0.Lock();
    //     monitor0.Put(i);
    //     cout << "I put: " << i << endl;
    //     monitor0.Unlock();
    // }
    monitor0.Lock();
    monitor0.Put(1);
    cout << "I put: " << 1 << endl;
    monitor0.Unlock();
    sleep(2);
    monitor0.Lock();
    monitor0.Put(2);
    cout << "I put: " << 2 << endl;
    monitor0.Unlock();
    sleep(2);
    monitor0.Lock();
    monitor0.Put(3);
    cout << "I put: " << 3 << endl;
    monitor0.Unlock();
    sleep(2);
    monitor0.Lock();
    cout << "I pop: " << monitor0.Pop()<< endl;
    monitor0.Unlock();
    return 0;
}