//#include <iostream>
#include <vector>
//#include <stdio.h>
#include <unistd.h>
#include <sstream>
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

    void listen()
    {
        zmq::context_t context(1);
        zmq::socket_t subscriber(context, ZMQ_SUB);
        subscriber.connect("tcp://localhost:5555");
        subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
        zmq::message_t message;
        printf("Zaczynam słuchać\n");
        while (state != END)
        {
            printf("State: %d\n", state);
            //Read envelope with address
            //subscriber.recv(&message);
            printf("Odbieram\t");
            std::string address = s_recv(subscriber);
            printf("Wciąż odbieram \t");
            std::string contents = s_recv(subscriber);
            printf("Odebrałem\n");
            std::cout << "[" << address << "] " << contents << std::endl;
        }
        printf("słuchałem\n");
    }
    void send()
    {
        zmq::context_t context(1);
        zmq::socket_t publisher(context, ZMQ_PUB);
        publisher.bind("tcp://*:5555");

        while (state != END)
        {
            printf("State: %d\n", state);
            printf("Przed wysłaniem\t");
            s_sendmore(publisher, "B");
            s_send(publisher, "We would like to see this");
            printf("Po wysłaniu\n");
            sleep(1);
        }
    }

    //zmq::socket_t publisher;
    //zmq::socket_t subscriber, publisher;

    Monitor()
    {
        pool = new int[10];
        state = IDLE;
        //context = zmq::context_t(1);
        //subscriber = zmq::socket_t(context, ZMQ_SUB);
        //publisher = zmq::socket_t(context, ZMQ_PUB);

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
        //Monitor::publisher = zmq::socket(context, zmqpp::socket_type::publish);
        // subscriber(context, zmqpp::socket_type::subscribe);
        // publisher(context, zmqpp::socket_type::publish);
    }
    ~Monitor()
    {
        printf("Odpaliłem się");
        state = END;
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

    volatile int state;

  private:
    //static zmq::context_t context;
    //zmq::message message_recv, message_send;
    int *pool;
    int i_get;
    int i_put;
    int count;

    vector<int> Rn;
};

int main()
{
    Monitor monitor;
    sleep(2);
    monitor.state = END;
    // free(monitor);
    printf("jestem\n");
    return 0;
}