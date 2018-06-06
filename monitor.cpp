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
    cout << "Lock -> Czekam na odpowiedni stan" << endl;
    while (GetState() != IDLE && GetState() != UNLOCKED)
    {
    };
    if (GetState() == IDLE || GetState() == UNLOCKED)
    {
        SetState(LOCKED);
        cout << "Lock -> State = LOCKED" << endl;
        if (hasToken == false)
        {
            messages.insert(messages.begin(), CreateMessage(WANT_CS, ALL, GetId(), to_string(Rn[GetId()])));
            SetState(REQ_CS);
            cout << "Lock -> Wysłałem wiadomość, że chcę CS" << endl;
        }
        else
        {
            SetState(CS);
            cout << "Lock -> Mam CS" << endl;
        }
    }
}

//void unlock   //zwalnia sekcję krytyczną
void Monitor::Unlock()
{
    if (GetState() == CS || GetState() == REQ_CS || GetState() == LOCKED)
    {
        SetState(IDLE);
        cout << "Unlock -> State = IDLE" << endl;
    }
    cout<<"Test"<<endl;
    if (!IsEmptyToken())
    {
        cout<<"Test3"<<endl;
        messages.insert(messages.begin(), CreateMessage(TOKEN, GetFromToken(), GetId(), VectorToString(Token)));
        cout << "Unlock -> Przesyłam token" << endl;
        ClearToken();
    }
    else
    {
        cout << "Unlock -> Token został u mnie" << endl;
    }
}
//void put  //wkłada element
void Monitor::Put(int n)
{
    sleep(1);
    cout << "Put -> Czekam na state lub wolne miejsce"<< endl;
    while (GetState() != CS || count == 10)
    {
    }
    messages.insert(messages.begin(), CreateMessage(PUT, ALL, GetId(), to_string(n)));
    cout << "Put -> włożyłem element: " << n << " Stan: " << n << endl;
}
//void pop      //zdejmuje element

int Monitor::Pop()
{
    cout << "Pop -> Czekam na state lub cos w zbiorze" << endl;
    while (GetState() != CS || count == -1)
    {
    }
    messages.insert(messages.begin(), CreateMessage(POP, ALL, GetId(), ""));
    count--;
    cout << "Pop -> Wyjąłem: " << pool[count + 1] << " Stan: " << count << endl;

    return pool[count + 1];
}

Monitor::Monitor(string fileName)
{
    pool = new int[10];
    SetState(IDLE);
    SetParticipants(fileName);
    if (GetId() == 0)
    {
        hasToken = true;
    }
}

Monitor::~Monitor()
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

    cout << "Moje ID: " << GetId() << endl;
    //ADD NEW MESSAGE
    //messages.insert(messages.begin(), CreateMessage(NEW, ALL, Id, "message"));
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
        iss >> mess.Flag >> mess.DestId >> mess.SourceId >> mess.Content;
        cout << "Odebrałem " << mess.Flag << " " << mess.DestId << " " << mess.SourceId << " " << mess.Content << endl;

        switch (mess.Flag)
        {
        case WANT_CS:
            cout << "Listen -> WANT_CS od " << mess.SourceId << endl;
            if (Rn[mess.SourceId] < stoi(mess.Content))
            {
                Rn[mess.SourceId] = stoi(mess.Content);
            }
            AddToToken(mess.SourceId);
            break;
        case TOKEN:
            cout << "Listen -> TOKEN od " << mess.SourceId << endl;
            SetState(CS);
            StringToVector(mess.Content);
            break;
        case NEW:
            cout << "Listen -> NEW od " << mess.SourceId << endl;
            //dodawanie do tabeli połączeń
            break;

        case NEW_RESPONSE:
            cout << "Listen -> NEW_RESPONSE od " << mess.SourceId << endl;
            //nie wiem czy potrzebne
            break;

        case PUT:
            count++;
            pool[count] = stoi(mess.Content);
            cout << "Listen -> PUT od " << mess.SourceId << " Dodano: " << pool[count] << " Na miejscu: " << count << endl;
            break;
        case POP:
            count--;
            cout << "Listen -> TOKEN od " << mess.SourceId << " Zabrano: " << pool[count + 1] << " Stan: " << count << endl;
            break;
        }
        s_send(subscriber, "");
    }
    pthread_exit(NULL);
}
void Monitor::Send()
{
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_REQ);
    for (auto const &x : connections)
    {
        publisher.connect(x.second.Send);
    }
    zmq::message_t message(512);
    zmq::message_t retMess(25);
    while (GetState() != END)
    {
        sleep(1);
        while (!messages.empty())
        {
            Message mess = messages.back();
            messages.pop_back();
            snprintf((char *)message.data(), 512,
                     "%d %d %d %s", mess.Flag, mess.DestId, mess.SourceId, mess.Content.c_str());

            for (int i = 0; i < connections.size(); i++)
            {
                if (i != GetId())
                {
                    cout<<"Wyslalem wiadomosci do: "<<i<<endl;
                    publisher.send(message);
                    publisher.recv(&retMess);
                }
            }
            cout << "Wysyłam " << mess.Flag << " " << mess.DestId << " " << mess.SourceId << " " << mess.Content << endl;
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

void Monitor::AddToToken(int n)
{
    Token.insert(Token.begin(), n);
}

int Monitor::GetFromToken()
{
    int n = Token.back();
    Token.pop_back();
    return n;
}

bool Monitor::IsEmptyToken()
{
    cout<<"Test2"<<endl;
    return Token.empty();
}

string Monitor::VectorToString(vector<int> vec)
{
    string str;
    while (!IsEmptyToken())
    {
        str.append(to_string(GetFromToken()));
        str.append(" ");
    }
}
void Monitor::StringToVector(string str)
{
    for (int i = 0; i < str.size(); i++)
    {
        AddToToken(str[i]);
        i++;
    }
    hasToken = true;
}
void Monitor::ClearToken()
{
    Token.clear();
    hasToken = false;
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