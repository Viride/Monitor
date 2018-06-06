#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "monitor.h"

int main()
{
    Monitor monitor0("setup3.txt");

    monitor0.Initialize();

    for (int i = 0; i < 15; i++)
    {
        sleep(1);
        monitor0.Lock();
        monitor0.Put(i);
        cout << "Producent -> put: " << i << endl;
        monitor0.Unlock();
    }    
    return 0;
}