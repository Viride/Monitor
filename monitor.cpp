//#include <iostream>
#include <vector>
//#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <atomic>
#include <pthread.h>
#include "zhelpers.hpp"
//#include <zmq.hpp>
//#include <zmqpp/zmqpp.hpp>
#include "message.h"

#define IDLE 0
#define LOCKED 1
#define REQ_CS 2
#define CS 3
#define UNLOCKED 4
#define END 5
using namespace std;

class Monitor
{

  public:
    //void lock     //rząda sekcji krytycznej
    void Lock()
    {
        state = LOCKED;
    }

    //void unlock   //zwalnia sekcję krytyczną
    void Unlock()
    {
        state = UNLOCKED;
    }
    //void put  //wkłada element
    void Put()
    {
    }
    //void pop      //zdejmuje element
    void Pop()
    {
    }

    void Listen()
    {
        zmq::context_t context(1);
        zmq::socket_t subscriber(context, ZMQ_SUB);
        subscriber.connect("tcp://localhost:5555");
        subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
        zmq::message_t message;
        while (GetState() != END)
        {
            //Read envelope with address
            //subscriber.recv(&message);
            std::string address = s_recv(subscriber);
            std::string contents = s_recv(subscriber);
            std::cout << "[" << address << "] " << contents << std::endl;
        }
        pthread_exit(NULL);
    }
    void Send()
    {
        zmq::context_t context(1);
        zmq::socket_t publisher(context, ZMQ_PUB);
        publisher.bind("tcp://*:5555");

        while (GetState() != END)
        {
            printf("State: %d\n", GetState());
            s_sendmore(publisher, "B");
            s_send(publisher, "We would like to see this");
            sleep(1);
        }
        pthread_exit(NULL);
    }

    static void *CallSend(void *p)
    {
        static_cast<Monitor*>(p)->Send();
        return NULL;
    }

    static void *CallListen(void *p)
    {
        static_cast<Monitor*>(p)->Listen();
        return NULL;
    }

    Monitor()
    {
        pool = new int[10];
        SetState(IDLE);
    }

    void Initialize()
    {
        printf("Initialized\n");
        int rc = 0;
        printf("main() : creating thread, 0\n");
        rc = pthread_create(&threads[0], NULL, CallListen, this);
        if (rc)
        {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }

        printf("main() : creating thread, 1\n");
        rc = pthread_create(&threads[1], NULL, CallSend, this);
        if (rc)
        {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }

    ~Monitor()
    {
        SetState(END);
    }
    int GetState()
    {
        return Monitor::state.load();
    }
    void SetState(int n)
    {
        Monitor::state.store(n);
    }

  private:
    atomic<int> state;
    pthread_t threads[2];
    int *pool;
    int i_get;
    int i_put;
    int count;

    vector<int> Rn;
};

int main()
{
    Monitor monitor;
    monitor.Initialize();
    sleep(5);
    // monitor.SetState(END);s
    printf("Otrzymałem %d\n", monitor.GetState());
    // printf("jestem\n");
    return 0;
}