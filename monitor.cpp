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
        messages.insert(messages.begin(), CreateMessage(TOKEN, GetFromToken(), GetId(), VectorToString(Token)));
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
    cout << "Put -> Czekam na state lub wolne miejsce" << endl;
    while (GetState() != CS || count >= 10)
    {
        sleep(1);
        cout << "Put -> while -> State: " << GetState() << " Count: " << count << endl;
    }
    messages.insert(messages.begin(), CreateMessage(PUT, ALL, GetId(), to_string(n)));
    cout << "Put -> włożyłem element: " << n << " Stan: " << n << endl;
}
//void pop      //zdejmuje element

int Monitor::Pop()
{
    sleep(1);
    cout << "Pop -> Czekam na state lub cos w zbiorze" << endl;
    while (GetState() != CS || count == -1)
    {
        sleep(1);
        cout << "Pop -> while -> State: " << GetState() << " Count: " << count << endl;
    }
    messages.insert(messages.begin(), CreateMessage(POP, ALL, GetId(), ""));
    count--;
    cout << "Pop -> Wyjąłem: " << pool[count + 1] << " Stan: " << count << endl;

    return pool[count + 1];
}

Monitor::Monitor(string fileName)
{
    pool = new int[10];
    count = 0;
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
    zmq::socket_t subscriber(context, ZMQ_PULL);
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
                AddToToken(mess.SourceId);
            }
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
            cout << "Listen -> PUT od " << mess.SourceId;
            pool[count] = stoi(mess.Content);
            count++;
            cout << " Dodano: " << pool[count] << " Na miejscu: " << count - 1 << endl;
            break;

        case POP:
            cout << "Listen -> TOKEN od " << mess.SourceId;
            count--;
            cout << " Zabrano: " << pool[count + 1] << " Stan: " << count << endl;
            break;

        case IGNORE:
            cout << "Ignore message" << endl;
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
    zmq::message_t message(512);
    string text = "dummy";
    zmq::message_t dummy(512);
    snprintf((char *)dummy.data(), 512,
             "%d %d %d %s", IGNORE, GetId(), GetId(), text.c_str());
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
                if (i == GetId())
                {
                    cout << "Wyslalem wiadomosci do: " << i << endl;
                    publisher.send(dummy);
                }
                else
                {
                    cout << "Wyslalem wiadomosci do: " << i << endl;
                    publisher.send(message);
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
    return Token.empty();
}

string Monitor::VectorToString(vector<int> vec)
{
    string str;
    while (IsEmptyToken() == false)
    {
        str.append(to_string(GetFromToken()));
        str.append(" ");
    }
    return str;
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
    string str2 = str.substr(0, x );
    mess.Flag = stoi(str2);
    str.erase(0, x+1);
    x = str.find_first_of(";");
    str2 = str.substr(0, x);
    mess.DestId = stoi(str2);
    str.erase(0, x+1);
    x = str.find_first_of(";");
    str2 = str.substr(0, x);
    mess.SourceId = stoi(str2);
    str.erase(0, x+1);
    mess.Content = str;
    return mess;
}