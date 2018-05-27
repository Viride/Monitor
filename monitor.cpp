#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
//#include "zhelpers.hpp"
#include <zmq.hpp>
//#include <zmqpp/zmqpp.hpp>
#include "message.h"

#define IDLE 0
#define LOCKED 1
#define REQ_CS 2
#define CS 3
#define UNLOCKED 4
#define END 5
using namespace std;

class Monitor{

    public:

    
    //void lock     //rząda sekcji krytycznej
    void lock(){
        state = LOCKED;
    }

    //void unlock   //zwalnia sekcję krytyczną
    void unlock (){
        state = UNLOCKED;
    }
    //void put  //wkłada element
    void put(){

    }
    //void pop      //zdejmuje element
    void pop(){

    }

    void listen(){
        int i=0;
        Message mess;
        while(i<10){
            sleep(1);
            i++;
            printf("%d\n", i);
            subscriber.receive(&message_recv);
            std::istringstream iss(static_cast<char*>(update.data()));
            iss >> mess.Flag >> mess.DestId >> mess.SourceId;
            printf("%s\n", message_recv)
        }
    }

    Monitor(){
        pool = new int[10];        
        Monitor::subscriber = zmqpp::socket(context, zmqpp::socket_type::subscribe);
        Monitor::publisher = zmqpp::socket(context, zmqpp::socket_type::publish);
        subscriber.connect("tcp://localhost:5555");
        subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
        subscriber
        publisher.bind("tcp://*:5555");

        //subscriber(context, zmqpp::socket_type::subscribe);
        //publisher(context, zmqpp::socket_type::publish);
        pid_t pid = fork();
        printf("%d\n", pid);
        if(pid == 0){
            listen();
        }
    }

    private:
    zmqpp::context context;
    zmqpp::socket subscriber, publisher;
    zmqpp::message message_recv, message_send;
    int *pool;
    int i_get;
    int i_put;
    int count;
    int state;
    
    vector<int> Rn;

};

int main(){
    Monitor monitor;
    zmqpp::context context;
    zmqpp::socket subscriber(context, zmqpp::socket_type::subscribe);
    zmqpp::socket publisher(context, zmqpp::socket_type::publish);
    printf("Jestem w mainie\n");
    sleep(3);
    for(int i=0 i<10;i++){
        snprintf ((char *) message.data(), 20 ,
            "%d %d %d", 10, 10, 10);
        publisher.send(message);
    }
    return 0;
}