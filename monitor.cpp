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
        printf("Przed wysłaniem\t");
        s_sendmore(publisher, "B");
        s_send(publisher, "We would like to see this");
        printf("Po wysłaniu\n");
        sleep(1);
    }
    //void pop      //zdejmuje element
    void pop()
    {
    }

    void listen()
    {
        int i=0;
        printf("Zaczynam słuchać\n");
        while(i<5)
        {
            //Read envelope with address
            printf("Odbieram\t");
            std::string address = s_recv(subscriber);
            printf("Wciąż odbieram \t");
            std::string contents = s_recv(subscriber);
            printf("Odebrałem\n");
            std::cout << "[" << address << "] " << contents << std::endl;
            i++;
        }
        printf("słuchałem\n");
    }

    zmq::context_t context;
    zmq::socket_t subscriber, publisher;

    Monitor() : context(1), subscriber(context, ZMQ_SUB), publisher(context, ZMQ_PUB)
    {
        pool = new int[10];
        //context = zmq::context_t(1);
        //subscriber = zmq::socket_t(context, ZMQ_SUB);
        //publisher = zmq::socket_t(context, ZMQ_PUB);
        subscriber.connect("tcp://localhost:5555");
        subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

        publisher.bind("tcp://*:5555");

        printf("Initialized\n");
        pid_t pid = fork();
        printf("%d\n", pid);
        if (pid == 0)
        {
            listen();
        }
        //Monitor::publisher = zmq::socket(context, zmqpp::socket_type::publish);
        // subscriber(context, zmqpp::socket_type::subscribe);
        // publisher(context, zmqpp::socket_type::publish);
    }

  private:
    
    //static zmq::context_t context;
    //zmq::message message_recv, message_send;
    int *pool;
    int i_get;
    int i_put;
    int count;
    int state;

    vector<int> Rn;
};

int main()
{
    Monitor monitor;
    // zmqpp::context context;
    // zmqpp::socket subscriber(context, zmqpp::socket_type::subscribe);
    // zmqpp::socket publisher(context, zmqpp::socket_type::publish);
    // printf("Jestem w mainie\n");
    // sleep(3);
    // for(int i=0 i<10;i++){
    //     snprintf ((char *) message.data(), 20 ,
    //         "%d %d %d", 10, 10, 10);
    //     publisher.send(message);
    // }
    sleep(2);
    printf("jestem\n");
    // monitor.put();
    // monitor.put();
    // monitor.put();
    // monitor.put();
    // monitor.put();
    // pid_t pid = fork();
    // printf("%d\n", pid);
    // if (pid == 0)
    // {
    //     printf("jestem1\n");

    //     zmq::context_t context(1);
    //     zmq::socket_t publisher(context, ZMQ_PUB);
    //     publisher.bind("tcp://*:5563");
    //     int i = 0;
    //     sleep(1);
    //     while (i < 5)
    //     {
    //         //  Write two messages, each with an envelope and content
    //         s_sendmore(publisher, "A");
    //         s_send(publisher, "We don't want to see this");
    //         s_sendmore(publisher, "B");
    //         s_send(publisher, "We would like to see this");
    //         sleep(1);
    //         i++;
    //     }
    //     printf("koniec1");
        
    // }
    // else
    // {
    //     printf("jestem2\n");

    //     //  Prepare our context and publisher
    //     zmq::context_t context(1);
    //     zmq::socket_t subscriber(context, ZMQ_SUB);
    //     subscriber.connect("tcp://localhost:5563");
    //     subscriber.setsockopt(ZMQ_SUBSCRIBE, "B", 1);
    //     int i=0;
    //     while (i<5)
    //     {

    //         //  Read envelope with address
    //         std::string address = s_recv(subscriber);
    //         //  Read message contents
    //         std::string contents = s_recv(subscriber);

    //         std::cout << "[" << address << "] " << contents << std::endl;
    //         i++;
    //     }
    //     printf("koniec2");
    //}

    return 0;
}