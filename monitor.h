#include <atomic>
#include <pthread.h>
#include <vector>
#include <map>
#include "message.h"
#include "connection.h"

using namespace std;

class Monitor
{
  public:
    void Lock();   //rząda sekcji krytycznej
    void Unlock(); //zwalnia sekcję krytyczną
    void Put();    //wkłada element
    void Pop();    //zdejmuje element
    inline Monitor(string fileName);
    inline ~Monitor();
    void Initialize();

  private:
    static void *CallSend(void *p)
    {
        static_cast<Monitor *>(p)->Send();
        return NULL;
    };
    static void *CallListen(void *p)
    {
        static_cast<Monitor *>(p)->Listen();
        return NULL;
    };
    void SetParticipants(string fileName);
    void Listen();
    void Send();
    int GetState();
    void SetState(int n);
    int GetId();
    void SetId(int n);
    Message CreateMessage(int flag, int destID, int sourceId, string content);
    atomic<int> state;
    pthread_t threads[2];
    int Id;
    int Nmb;
    int *pool;
    int i_get;
    int i_put;
    int count;
    vector<Message> messages;
    map<int, Connection> connections;
    map<int, int> Rn;
    //vector<int> Rn;
};