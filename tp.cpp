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
    unsigned virtual runL() = 0;
    virtual ~Work() {};
};

class ThreadPool {
public:
    static void* worker(void *arg);
    static ThreadPool* getInstance() {
        static ThreadPool *tp = new ThreadPool();
        return tp;
    };
    ~ThreadPool() { };
    
    static void queWork(Work *w) {
      //pthread_mutex_lock(&mutex); // ?? When it can be needed 
      ThreadPool *t = getInstance();
      t->wq.push(w);
      //pthread_mutex_unlock(&mutex);
      
      sem_post(&t->s);
    };
private:
    ThreadPool() {
        init_threadpool();
    };

    void init_threadpool() {
        sem_init(&s,0,0);
        mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        
        for(int i = 0;i<SIZE;i++) {
            pthread_create(&th[i],NULL,&ThreadPool::worker,this);
        }
    };
    
    pthread_t th[SIZE];
    sem_t s;
    pthread_mutex_t mutex;
    queue<Work*> wq;
};

void* ThreadPool::worker(void *arg) {
   ThreadPool *tp = (ThreadPool*)arg;
   while(1) {
      sem_wait(&(tp->s));
      Work *w=0;

      pthread_mutex_lock(&tp->mutex);
      if(!tp->wq.empty()) {   
         w = tp->wq.front();
         tp->wq.pop();
      }
      pthread_mutex_unlock(&tp->mutex);
         
      if (w) 
         w->runL();
   }
   return 0;
};

class MyWork: public Work {
public:
    MyWork(int i):tid(i) {}
    ~MyWork() {}
    unsigned runL() {
        cout  << tid << " th" << endl;
        usleep(tid*1000000);
        return 0;
    }
private:
    int tid;
};

int main() {
    Work *wo[10];
    for (int i = 0;i<10; i++) {
        wo[i] = new MyWork(i);
        ThreadPool::queWork(wo[i]);
    }
    usleep(50000000);

    for (int i = 0;i<10; i++)
        delete wo[i];
    return 0;
};
