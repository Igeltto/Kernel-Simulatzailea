#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>

#define TENP_KOP 2

// Hemen prozesu queue-a definitzen dugu
#define SIZE 100

void enqueue();
void dequeue();
bool isEmpty();
bool isFull();
void putLast();

int front = -1;
int rear = -1;

struct PCB
{
   int pid;
   int denbora;
};

struct PCB queue[SIZE];

// Hemen Machine barruko guztia definitzen dugu.
struct Machine
{
   struct CPU *lehena;
   int machineid;
};

struct CPU
{
   struct CPU *hurrengoa;
   struct Core *lehena;
   int CPUid;
};

struct Core
{
   struct Core *hurrengoa;
   struct Haria *lehena;
   int coreid;
};

struct Haria
{
   long freq;
   int tid;
   struct Haria *hurrengoa;
};

sem_t semaphore1, semaphore2, semaphore3, semaphore4;
pthread_mutex_t mutexClockTimer;
pthread_cond_t cond;
pthread_cond_t cond2;

int pid;
int pid_child;

int pidBer = 0;
int doneClk = 0;
int tickSch = 0;
int tickPG = 0;
// int denbora = 0;

void *scheduler()
{
   while (1)
   {
      // Oraingoz schedulerrak ez du ezer egingo
      sem_wait(&semaphore1);
      // egin beharrekoa egiten du hemen
      // printf("Aupa, honera iritsi naiz SCHEDULER\n");
      if (!isEmpty())
      {
         struct PCB pcb = queue[0];
         int denbora = pcb.denbora;
         int pidSch = pcb.pid;
         if (pidSch == -1)
         {
            while (rear == 0)
            {
               printf("Prozesu nulua naiz\n");
            }
         }
         else
         {
            printf("%d-garren prozesua naiz\n", pidSch);
            printf("%d prozesu daude ilaran\n", rear);
            printf("%d tick behar ditut\n", denbora);
            while (denbora > 0)
            {
               denbora = denbora - 1;
            }
            dequeue();
         }
      }
      sem_post(&semaphore1);
   }
}

void *processGenerator()
{

   if (isEmpty())
   {
      // pid_child = fork();
      // pid = getpid();
      /*pid_child == 0  &&*/

      struct PCB nullProzesua = {-1, -1};
      enqueue(&nullProzesua);
   }

   while (1) // prozesua egin den orduan hemendik atera behar da
   {
      sem_wait(&semaphore2);
      if (!isFull())
      {
         // pid_child = fork();
         // pid = getpid();
         if (queue[0].pid == -1)
         {
            struct PCB berria = {0, 50000000};
            enqueue(&berria);
            putLast();
         }
         else
         {
            struct PCB berria = {pidBer, 50000000};
            enqueue(&berria);
            pidBer = pidBer + 1;
         }
      }
      sem_post(&semaphore2);
   }
}

void *timerSch(void *arg) // bakoitzaren tid lortu behar det jakiteko zein timer bidali schedulerra eta zein PCra
{

   // printf("timer funtziora iritsi naiz \n");
   int freq = (*(struct Haria *)arg).freq;
   int tid = (*(struct Haria *)arg).tid; //(struct tid *)arg) esto es para hacer cast del tipo struct tid
   // printf("%d-garren haria naiz");
   pthread_mutex_lock(&mutexClockTimer);
   printf("%d-garren haria naiz\n", tid);
   printf("%d da frekuentzia\n", freq);
   while (1)
   {
      doneClk++;
      tickSch++;
      // printf("%d-garren ticka naiz \n", tick);
      if (tickSch == freq)
      {
         sem_post(&semaphore1);
         sem_wait(&semaphore1);
         // printf("Honera iritsi naiz TIMER\n");
         tickSch = 0;
      }

      // denbora++;
      // printf("%d tick-a egin dut\n", denbora);
      pthread_cond_signal(&cond);
      pthread_cond_wait(&cond2, &mutexClockTimer);
   }
}

void *timerPG(void *arg)
{
   // int freq = (*(struct Haria *)arg).freq;
   // int tid = (*(struct Haria *)arg).tid;
   int freq = rand() % 1000;
   pthread_mutex_lock(&mutexClockTimer);
   while (1)
   {
      doneClk++;
      tickPG++;
      if (tickPG == freq)
      {
         sem_post(&semaphore2);
         sem_wait(&semaphore2);
         tickPG = 0;
         freq = rand() % 10000000;
      }
      // printf("Honera iritsi naiz ORANGUTAN\n");
      pthread_cond_signal(&cond);
      pthread_cond_wait(&cond2, &mutexClockTimer);
   }
}

void *erlojua()
{

   // printf("Erlojura iritsi naiz\n");
   while (1)
   {
      pthread_mutex_lock(&mutexClockTimer);
      while (doneClk < TENP_KOP)
         pthread_cond_wait(&cond, &mutexClockTimer);
      doneClk = 0;
      pthread_cond_broadcast(&cond2);
      pthread_mutex_unlock(&mutexClockTimer);
   }
}

void enqueue(void *arg)
{
   struct PCB insert_item = *(struct PCB *)arg;
   if (rear == SIZE - 1)
   {
      printf("Overflow\n");
   }
   else
   {
      if (front == -1)
      {
         front = 0;
         printf("%d element inserted in the Queue\n", insert_item.pid);
         rear = rear + 1;
         queue[rear] = insert_item;
      }
      else
      {
         printf("Element inserted in the Queue\n");
         rear = rear + 1;
         queue[rear] = insert_item;
      }
   }
}

void dequeue()
{
   if (front == -1 || front > rear)
   {
      printf("Underflow \n");
   }
   else
   {
      printf("Element deleted from the Queue: %d\n", queue[front].pid);
      for (int i = 0; i <= rear; i++)
      {
         queue[i] = queue[i + 1];
      }
      rear = rear - 1;
   }
}

bool isEmpty()
{
   if (rear == -1)
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool isFull()
{
   if (rear == SIZE - 1)
   {
      return true;
   }
   else
   {
      return false;
   }
}

void putLast()
{
   struct PCB pcbLast = queue[0];
   dequeue();
   queue[rear] = pcbLast;
   rear = rear + 1;
}

int main(int argc, char *argv[])
{

   if (argc > 1)
   {
      printf("Sistema ondo hasieratu da\n");

      long freq = strtol(argv[1], NULL, 10);
      sem_init(&semaphore1, 0, 1);
      sem_init(&semaphore2, 0, 1);
      pthread_mutex_init(&mutexClockTimer, NULL);
      pthread_cond_init(&cond, NULL);
      pthread_cond_init(&cond2, NULL);
      pthread_t t1, t2, erl, sc, pg;

      struct Haria hari1 = {freq, 1, NULL};
      struct Haria hari0 = {freq, 0, &hari1};

      struct Core core = {NULL, &hari1, 0};

      struct CPU cpu = {NULL, &core, 0};

      struct Machine machine = {&cpu, 0};

      if (pthread_create(&erl, NULL, &erlojua, NULL) != 0)
         perror("Failed to create threads"); // Hariak sortzeko momentuan existitzen badira dagoeneko, honek "errorea" bueltatuko du
      if (pthread_create(&t1, NULL, &timerSch, (void *)&hari0) != 0)
         perror("Failed to create threads");
      if (pthread_create(&t2, NULL, &timerPG, (void *)&hari1) != 0)
         perror("Failed to create threads");
      if (pthread_create(&sc, NULL, &scheduler, NULL) != 0)
         perror("Failed to create threads");
      if (pthread_create(&pg, NULL, &processGenerator, NULL) != 0)
         perror("Failed to create threads");

      if (pthread_join(t1, NULL) != 0)
         perror("Failed to join threads");
      if (pthread_join(t2, NULL) != 0)
         perror("Failed to join threads");
      if (pthread_join(erl, NULL) != 0)
         perror("Failed to join threads");
      if (pthread_join(sc, NULL) != 0)
         perror("Failed to join threads");
      if (pthread_join(pg, NULL) != 0)
         perror("Failed to join threads");

      sem_destroy(&semaphore1);
      sem_destroy(&semaphore2);
      pthread_mutex_destroy(&mutexClockTimer);
      pthread_cond_destroy(&cond);
      pthread_cond_destroy(&cond2);
   }
}