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
    void lock()
    {
        state = LOCKED;
    }

    //void unlock   //zwalnia sekcję krytyczną
    void unlock()
    {
        state = UNLOCKED;
    }
    //void put  //wkłada element
    void put()
    {
    }
    //void pop      //zdejmuje element
    void pop()
    {
    }

    void *listen()
    {
        sleep(5);
        zmq::context_t context(1);
        zmq::socket_t subscriber(context, ZMQ_SUB);
        subscriber.connect("tcp://localhost:5555");
        subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
        zmq::message_t message;
        printf("Zaczynam słuchać\n");
        while (GetState() != END)
        {
            printf("State: %d\n", GetState());
            //Read envelope with address
            //subscriber.recv(&message);
            printf("Odbieram\t");
            std::string address = s_recv(subscriber);
            printf("Wciąż odbieram \t");
            std::string contents = s_recv(subscriber);
            printf("Odebrałem\n");
            std::cout << "[" << address << "] " << contents << std::endl;
        }
        pthread_exit(NULL);
    }
    void *send()
    {
        sleep(5);
        zmq::context_t context(1);
        zmq::socket_t publisher(context, ZMQ_PUB);
        publisher.bind("tcp://*:5555");

        while (GetState() != END)
        {
            printf("State: %d\n", GetState());
            printf("Przed wysłaniem\t");
            s_sendmore(publisher, "B");
            s_send(publisher, "We would like to see this");
            printf("Po wysłaniu\n");
            sleep(1);
        }
        pthread_exit(NULL);
    }

    Monitor()
    {
        pool = new int[10];
        SetState(IDLE);
        
        printf("Initialized\n");
        pid_t pid = fork();
        printf("%d\n", pid);
        if (pid == 0)
        {
            listen();
        }
        else
        {
            pid_t pid2 = fork();
            if (pid2 == 0)
            {
                send();
            }
        }
    }

    ~Monitor()
    {
        printf("Odpaliłem się");
        SetState(END);
        ///CZEKANIE AŻ PROCESY SIĘ SKOŃCZĄ
        // int status;
        // do
        // {
        //     status = wait();
        //     if (status == -1 && errno != ECHILD)
        //     {
        //         perror("Error during wait()");
        //         abort();
        //     }
        // } while (status > 0);
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
    //sleep(2);
    printf("%d\n", END);
    monitor.SetState(END);
    printf("Otrzymałem %d\n",monitor.GetState());
    // free(monitor);
    printf("jestem\n");
    return 0;
}