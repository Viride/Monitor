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
#define WAITING 6;

#define ALL 0

#define WANT_CS 1
#define TOKEN 2
#define NEW 3
#define NEW_RESPONSE 4
#define PUT 5
#define POP 6
#define IGNORE 7
using namespace std;

//void lock     //rząda sekcji krytycznej
void Monitor::Lock()
{
    sleep(1);
    cout << "Lock -> Czekam na odpowiedni stan" << endl;
    while (GetState() != IDLE && GetState() != UNLOCKED)
    {
        sleep(1);
        cout << "Lock -> while -> State: " << GetState() << endl;
    };
    if (GetState() == IDLE || GetState() == UNLOCKED)
    {
        SetState(LOCKED);
        cout << "Lock -> State = LOCKED" << endl;
        if (hasToken == false)
        {
            Rn[GetId()]++;
            messages.insert(messages.begin(), CreateMessage(WANT_CS, ALL, GetId(), to_string(Rn[GetId()])));
            SetState(REQ_CS);
            cout << "Lock -> Wysłałem wiadomość, że chcę CS" << endl;
            while (hasToken != true)
            {
                sleep(1);
                cout << "Lock -> czekam na token" << endl;
            }
            SetState(CS);
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
    sleep(1);
    if (GetState() == CS || GetState() == REQ_CS || GetState() == LOCKED)
    {
        SetState(IDLE);
        cout << "Unlock -> State = IDLE" << endl;
    }
    if (!IsEmptyToken())
    {
        cout << "Unlock -> Przesyłam token" << endl;
        int sendTo = GetTopFromToken();
        messages.insert(messages.begin(), CreateMessage(TOKEN, sendTo, GetId(), VectorToString(Token)));
        ClearToken();
    }
    else
    {
        cout << "Unlock -> Token został u mnie" << endl;
    }
}
void Monitor::Put(int n)
{
    sleep(1);
    cout << "Put -> Czekam na state lub wolne miejsce" << endl;
    while (GetState() != CS || count == 10)
    {
        sleep(1);
        cout << "Put -> while -> State: " << GetState() << " Count: " << count << endl;
    }
    messages.insert(messages.begin(), CreateMessage(PUT, ALL, GetId(), to_string(n)));
    pool[count] = n;
    count++;
    cout << "Put -> włożyłem element: " << n << " Stan: " << count << endl;
}

int Monitor::Pop()
{
    sleep(1);
    cout << "Pop -> Czekam na state lub cos w zbiorze" << endl;
    int i=0;
    while (GetState() != CS || count == 0)
    {
        sleep(1);
        cout << "Pop -> while -> State: " << GetState() << " Count: " << count << endl;
        if(GetState()==CS && i==0)
        {
            i=1;
            AddToToken(GetId());
            int sendTo = GetTopFromToken();
            messages.insert(messages.begin(), CreateMessage(TOKEN, sendTo, GetId(), VectorToString(Token)));
            ClearToken();
        }
    }
    
    messages.insert(messages.begin(), CreateMessage(POP, ALL, GetId(), ""));
    count--;
    cout << "Pop -> Wyjąłem: " << pool[count] << " Stan: " << count << endl;

    return pool[count];
}

Monitor::Monitor(string fileName)
{
    count = 0;
    for (int i = 0; i < 10; i++)
    {
        pool[i] = 0;
    }
    i_get = 0;
    i_put = 0;
    SetState(IDLE);
    SetParticipants(fileName);
    if (GetId() == 0)
    {
        hasToken = true;
    }
}

Monitor::~Monitor()
{
    sleep(4);
    SetState(END);
}

void Monitor::Initialize()
{
    int rc = 0;
    cout << "Intialize -> main() : creating thread, Listen\n";
    rc = pthread_create(&threads[0], NULL, Monitor::CallListen, this);
    if (rc)
    {
        cout << "Error:unable to create thread," << rc << endl;
        exit(-1);
    }
    printf("Intialize -> main() : creating thread, Send\n");
    rc = pthread_create(&threads[1], NULL, Monitor::CallSend, this);
    if (rc)
    {
        cout << "Error:unable to create thread," << rc << endl;
        exit(-1);
    }

    cout << "Intialize -> Moje ID: " << GetId() << endl;
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
    zmq::socket_t subscriber(context, ZMQ_PULL);
    subscriber.bind(connections[GetId()].Recv);
    int v = 0;
    Message mess;
    while (GetState() != END)
    {
        string message = s_recv(subscriber);
        mess = StringToMessage(message);
        cout << "Listen -> Odebrałem " << mess.Flag << " " << mess.DestId << " " << mess.SourceId << " " << mess.Content << endl;

        switch (mess.Flag)
        {
        case WANT_CS:
            cout << "Listen -> WANT_CS od " << mess.SourceId << endl;
            if (Rn[mess.SourceId] < stoi(mess.Content))
            {
                Rn[mess.SourceId] = stoi(mess.Content);
                AddToToken(mess.SourceId);
            }
            break;

        case TOKEN:
            if (mess.DestId == GetId())
            {
                cout << "Listen -> TOKEN od " << mess.SourceId << endl;
                hasToken = true;
                StringToVector(mess.Content);
                v = GetFromToken();
            }
            break;

        case NEW:
            cout << "Listen -> NEW od " << mess.SourceId << endl;
            break;

        case NEW_RESPONSE:
            cout << "Listen -> NEW_RESPONSE od " << mess.SourceId << endl;
            break;

        case PUT:
            cout << "Listen -> PUT od " << mess.SourceId << endl;
            pool[count] = stoi(mess.Content);
            count++;
            cout << "Listen -> PUT -> Dodano: " << pool[count - 1] << " Count: " << count << endl;
            break;

        case POP:
            cout << "Listen -> POP od " << mess.SourceId << endl;
            count--;
            cout << "Listen -> POP -> Zabrano: " << pool[count] << " Count: " << count << endl;
            break;

        case IGNORE:
            cout << "Listen -> Ignore message" << endl;
            break;
        }
    }
    pthread_exit(NULL);
}
void Monitor::Send()
{
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_PUSH);
    for (auto const &x : connections)
    {
        publisher.connect(x.second.Send);
    }
    // zmq::message_t message(512);
    // string text = "dummy";
    // zmq::message_t dummy2(512);
    // snprintf((char *)dummy2.data(), 512,
    //          "%d %d %d %s", IGNORE, GetId(), GetId(), text.c_str());
    string dummy = MessageToString(CreateMessage(IGNORE, GetId(), GetId(), "dummy"));
    while (GetState() != END)
    {
        sleep(1);
        while (!messages.empty())
        {
            Message mess = messages.back();
            messages.pop_back();
            // snprintf((char *)message.data(), 512,
            //          "%d %d %d %s", mess.Flag, mess.DestId, mess.SourceId, mess.Content.c_str());

            for (int i = 0; i < connections.size(); i++)
            {
                if (i == GetId())
                {
                    // cout << "Wyslalem wiadomosci do: " << i << endl;
                    s_send(publisher, dummy);
                    //publisher.send(dummy);
                }
                else
                {
                    // cout << "Wyslalem wiadomosci do: " << i << endl;
                    s_send(publisher, MessageToString(mess));
                    //publisher.send(message);
                }
            }
            cout << "Send -> Wysłałem " << mess.Flag << " " << mess.DestId << " " << mess.SourceId << " " << mess.Content << endl;
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

int Monitor::GetTopFromToken()
{
    int n = Token.back();
    return n;
}

bool Monitor::IsEmptyToken()
{
    return Token.empty();
}

string Monitor::VectorToString(vector<int> vec)
{
    string str;
    while (IsEmptyToken() == false)
    {
        str.append(to_string(GetFromToken()));
        str.append(";");
    }
    return str;
}

void Monitor::StringToVector(string str)
{
    int x = 0;
    string str2;
    while (str.size() > 0)
    {
        x = str.find_first_of(";");
        str2 = str.substr(0, x);
        AddToToken(stoi(str2));
        str.erase(0, x + 1);
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

string Monitor::MessageToString(Message message)
{
    string str;
    str = to_string(message.Flag);
    str.append(";");
    str.append(to_string(message.DestId));
    str.append(";");
    str.append(to_string(message.SourceId));
    str.append(";");
    str.append(message.Content);
    return str;
}

Message Monitor::StringToMessage(string str)
{
    Message mess;
    int x = str.find_first_of(";");
    string str2 = str.substr(0, x);
    mess.Flag = stoi(str2);
    str.erase(0, x + 1);
    x = str.find_first_of(";");
    str2 = str.substr(0, x);
    mess.DestId = stoi(str2);
    str.erase(0, x + 1);
    x = str.find_first_of(";");
    str2 = str.substr(0, x);
    mess.SourceId = stoi(str2);
    str.erase(0, x + 1);
    mess.Content = str;
    return mess;
}