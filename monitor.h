#include <atomic>
#include <pthread.h>
#include <vector>
#include "message.h"

using namespace std;

class Monitor
{
  public:
    void Lock();   //rząda sekcji krytycznej
    void Unlock(); //zwalnia sekcję krytyczną
    void Put();    //wkłada element
    void Pop();    //zdejmuje element
    inline Monitor();
    inline ~Monitor();
    void Initialize();

  private:
    int GetState();
    void SetState(int n);
    int GetId();
    void SetId(int n);
    void Listen();
    void Send();
    Message CreateMessage(int flag, int destID, int sourceId, string content);
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
    atomic<int> state;
    pthread_t threads[2];
    int Id;
    int *pool;
    int i_get;
    int i_put;
    int count;
    vector<Message> messages;

    vector<int> Rn;
};