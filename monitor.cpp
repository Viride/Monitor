#include <vector>
#include <unistd.h>
#include <sstream>
#include <atomic>
#include <map>
#include <utility>
#include <pthread.h>
#include <fstream>

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
#define PUT 5
#define POP 6
using namespace std;

//void lock     //rząda sekcji krytycznej
void Monitor::Lock()
{
    if (GetState() == IDLE || GetState() == UNLOCKED)
    {
        SetState(LOCKED);
        messages.insert(messages.begin(), CreateMessage(WANT_CS, ALL, Id, "message"));
        SetState(REQ_CS);
    }
}

//void unlock   //zwalnia sekcję krytyczną
void Monitor::Unlock()
{
    if (GetState() == CS || GetState() == REQ_CS || GetState() == LOCKED)
    {
        SetState(UNLOCKED);
    }
}
//void put  //wkłada element
void Monitor::Put()
{
    while (!GetState() == CS)
    {
    }
}
//void pop      //zdejmuje element
void Monitor::Pop()
{
    while (!GetState() == CS && count == sizeof(pool) / sizeof(pool[0]))
    {
    }
}

inline Monitor::Monitor(string fileName)
{
    pool = new int[10];
    SetState(IDLE);
    SetParticipants(fileName);
}

inline Monitor::~Monitor()
{
    SetState(END);
}

void Monitor::Initialize()
{
    printf("Initialized\n");
    int rc = 0;

    cout << "main() : creating thread, Listen\n";
    rc = pthread_create(&threads[0], NULL, Monitor::CallListen, this);
    if (rc)
    {
        cout << "Error:unable to create thread," << rc << endl;
        exit(-1);
    }
    printf("main() : creating thread, Send\n");
    rc = pthread_create(&threads[1], NULL, Monitor::CallSend, this);
    if (rc)
    {
        cout << "Error:unable to create thread," << rc << endl;
        exit(-1);
    }

    //ADD NEW MESSAGE
    messages.insert(messages.begin(), CreateMessage(NEW, ALL, Id, "message"));
}

void Monitor::SetParticipants(string fileName)
{
    ifstream file;
    file.open(fileName);
    int id, i = 0;
    string line;
    file >> id;
    getline(file, line);
    while (getline(file, line))
    {
        Connection connection;
        connection.Id = i;
        connection.Send = line;
        getline(file, line);
        connection.Recv = line;
        connections.insert(pair<int, Connection>(i, connection));
        i++;
    }
    file.close();
    SetId(id);
}

void Monitor::Listen()
{
    zmq::context_t context(1);
    zmq::socket_t subscriber(context, ZMQ_REP);
    subscriber.bind(connections[GetId()].Recv);
    zmq::message_t message;
    Message mess;
    while (GetState() != END)
    {
        subscriber.recv(&message);
        std::istringstream iss(static_cast<char *>(message.data()));
        //iss >> mess.Flag >> mess.DestId >> mess.SourceId >> mess.Content;
        iss >> mess.Flag >> mess.DestId >> mess.SourceId; // >> mess.Content;
        s_send(subscriber, "");
        cout << mess.Flag << " " << mess.DestId << " " << mess.SourceId << " " << mess.Content << endl;
    }
    pthread_exit(NULL);
}
void Monitor::Send()
{
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_REQ);
    for (auto const& x : connections)
    {
        publisher.connect(x.second.Send);
    }
    zmq::message_t message(512);
    while (GetState() != END)
    {
        sleep(1);
        while (!messages.empty())
        {
            Message mess = messages.back();
            messages.pop_back();
            // snprintf((char *)message.data(), sizeof(int)+sizeof(int)+sizeof(int)+sizeof(mess.Content)+1,
            //          "%d %d %d %s", mess.Flag, mess.DestId, mess.SourceId, mess.Content);
            snprintf((char *)message.data(), 512,
                     "%d %d %d", mess.Flag, mess.DestId, mess.SourceId);
            cout << "Wysyłam " << mess.Flag << " " << mess.DestId << " " << mess.SourceId << " " << mess.Content << endl;
            publisher.send(message);
            cout << "Wysłałem" << endl;
            publisher.recv(&message);
        }
    }
    pthread_exit(NULL);
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

Message Monitor::CreateMessage(int flag, int destId, int sourceId, string content)
{
    Message mess;
    mess.Flag = flag;
    mess.DestId = destId;
    mess.SourceId = sourceId;
    mess.Content = content;
    return mess;
}

int main()
{
    Monitor monitor("setup0.txt");
    Monitor monitor2("setup1.txt");
    // Monitor monitor3("setup2.txt");
    //Monitor monitor4("setup3.txt");
    //Monitor monitor5("setup4.txt");

    monitor.Initialize();
    cout<<endl<<"Zaczynam drugi: "<<endl;
    monitor2.Initialize();
    //monitor3.Initialize();
    //monitor4.Initialize();
    sleep(5);

    return 0;
}