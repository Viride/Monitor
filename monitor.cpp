#include <vector>
#include <unistd.h>
#include <sstream>
#include <atomic>
#include <pthread.h>

#include <sys/time.h>
#include <chrono>
#include <math.h>

#include "zhelpers.hpp"
#include "monitor.h"

#define IDLE 0
#define LOCKED 1
#define REQ_CS 2
#define CS 3
#define UNLOCKED 4
#define END 5

#define ALL 0

#define WANT_CS 1
#define TOKEN 2
#define NEW 3
#define NEW_RESPONSE 4
using namespace std;

//void lock     //rząda sekcji krytycznej
void Monitor::Lock()
{

    state = LOCKED;
}

//void unlock   //zwalnia sekcję krytyczną
void Monitor::Unlock()
{
    state = UNLOCKED;
}
//void put  //wkłada element
void Monitor::Put()
{
}
//void pop      //zdejmuje element
void Monitor::Pop()
{
}

void Monitor::Listen()
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
void Monitor::Send()
{
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_PUB);
    publisher.bind("tcp://*:5555");
    zmq::message_t message(512);

    while (GetState() != END)
    {
        while (!messages.empty())
        {
            Message mess = messages.back();
            messages.pop_back();
            snprintf((char *)message.data(), 512,
            "%d %d %d %s", mess.Flag, mess.DestId, mess.SourceId, mess.Content);
            publisher.send(message);
        }
        // printf("State: %d\n", GetState());
        // s_sendmore(publisher, "B");
        // s_send(publisher, "We would like to see this");
        // sleep(1);
    }
    pthread_exit(NULL);
}

inline Monitor::Monitor()
{
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    pool = new int[10];
    SetState(IDLE);
    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
    double id = time_span.count()*10000000000;     
    SetId((int)id);

}

inline Monitor::~Monitor()
{
    SetState(END);
}

void Monitor::Initialize()
{
    printf("Initialized\n");
    int rc = 0;
    cout<<"main() : creating thread, 0\n";
    rc = pthread_create(&threads[0], NULL, Monitor::CallListen, this);
    if (rc)
    {
        cout << "Error:unable to create thread," << rc << endl;
        exit(-1);
    }

    printf("main() : creating thread, 1\n");
    rc = pthread_create(&threads[1], NULL, Monitor::CallSend, this);
    if (rc)
    {
        cout << "Error:unable to create thread," << rc << endl;
        exit(-1);
    }

    //ADD NEW MESSAGE
}

int Monitor::GetState()
{
    return Monitor::state.load();
}
void Monitor::SetState(int n)
{
    Monitor::state.store(n);
}

int Monitor::GetId()
{
    return Monitor::Id;
}
void Monitor::SetId(int n)
{
    Monitor::Id = n;
}

int main()
{
    Monitor monitor;
    sleep(1);
    Monitor monitor2;
    Monitor monitor3;
    Monitor monitor4;
    // printf("%d\n", monitor4.GetId());
    
    return 0;
}