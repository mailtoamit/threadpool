#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<iostream>
#include<queue>
using namespace std;
class ThreadPool;
#define SIZE 5 //Thread pool size

class Work {
public:
    unsigned virtual runL(void *arg) = 0;
    virtual ~Work() {};
    void *ct;
};

class ThreadPool {
public:
    static void* worker(void *arg);
    static ThreadPool* getInstance() {
        static ThreadPool tp;
        return &tp;
    };
    ~ThreadPool() {
        void *ret;
        for(int i = 0;i<SIZE;i++) {
            pthread_join(th[i],&ret);
        }
    };
    
    static void queWork(Work *w) {
      ThreadPool *t = getInstance();
      pthread_mutex_lock(&t->qLock);
      t->wq.push(w);
      pthread_mutex_unlock(&t->qLock);
      
      sem_post(&t->s);
    };
private:
    ThreadPool() {
        init_threadpool();
    };
    void init_threadpool() {
        sem_init(&s,0,0);
        qLock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        dqLock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        
        for(int i = 0;i<SIZE;i++) {
            pthread_create(&th[i],NULL,&ThreadPool::worker,this);
        }
    };
    
    pthread_t th[SIZE];
    sem_t s;
    pthread_mutex_t qLock, dqLock;
    queue<Work*> wq;
    static int thr_cnt;
};

int ThreadPool::thr_cnt = 0;
void* ThreadPool::worker(void *arg) {
   ThreadPool *tp = (ThreadPool*)arg;
   while(1) {
      sem_wait(&(tp->s));
      Work *w=0;

      pthread_mutex_lock(&tp->dqLock);
      if(!tp->wq.empty()) {   
         w = tp->wq.front();
         tp->wq.pop();
      }
      pthread_mutex_unlock(&tp->dqLock);
         
      if (w) 
         w->runL(w->ct);
   }
   return 0;
};



///////Driver Code
class MyWork: public Work {
public:
    MyWork(void *arg) {
        ct = arg;
    }
    ~MyWork() {}
    unsigned runL(void *arg) {
        if(arg) {
            Work *w = new MyWork((void *)((long)arg/1000));
            ThreadPool::queWork(w);
            usleep((long)arg);
        }
        else
            usleep(1000000);
        cout  << this << " \n";
        return 0;
    }
};

int main()
{
    Work *wo[10];
    for (int i = 0;i<10; i++) {
        wo[i] = new MyWork((void*)50000);
        ThreadPool::queWork(wo[i]);
    }
    usleep(50000000);

    for (int i = 0;i<10; i++)
        delete wo[i];
    return 0;
};
