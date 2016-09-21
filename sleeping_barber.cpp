#include<iostream>
#include<semaphore.h>
#include<pthread.h>
#include<queue>
using namespace std;

#define totalbarbers 3
#define maxcapacity 10
#define maxcustomers 50
#define totalfilmfare 4

sem_t capacity; // for capacity of customers
sem_t customers; // for customer wait
sem_t barbers;  // for barber wait
sem_t filmfare; // for filmfare
sem_t customer[maxcustomers]; // for all customers
sem_t barber[totalbarbers]; // for all barbers
sem_t paymoney; // for customer to pay money to barber
sem_t submit_money; // for barber to submit money to cash counter

pthread_mutex_t customer_queue_mutex;
pthread_mutex_t barber_queue_mutex;
pthread_mutex_t print_mutex;

queue<int> customer_queue;  // Customer Queue who are reading filmfare and waiting for HairCut
queue<int> barber_queue;    // Barber Queue for Submitting cash to cash counter


// Initialize Semaphores and mutex
void initialize()
{
    pthread_mutex_init(&print_mutex,NULL);
    pthread_mutex_lock(&print_mutex);
    cout<<"Initializing variables ... \n";
    pthread_mutex_unlock(&print_mutex);

    sem_init(&capacity, 0, maxcapacity);
    sem_init(&filmfare, 0, totalfilmfare);

    for(int i=0; i<maxcustomers; i++)
        sem_init(&customer[i], 0, 0);

    sem_init(&paymoney, 0, 0);
    sem_init(&customers, 0, 0);

    for(int i=0; i<totalbarbers; i++)
        sem_init(&barber[i], 0, 0);

    sem_init(&barbers, 0, 0);
    sem_init(&submit_money, 0, 0);

    pthread_mutex_init(&customer_queue_mutex, NULL);
    pthread_mutex_init(&barber_queue_mutex, NULL);

}


//*******************************Print Functions ****************************

void read_os(int customer_no){

    pthread_mutex_lock(&print_mutex);
    cout<<"\nCustomer "<<customer_no+1<<" is reading OS Book.\n";
    pthread_mutex_unlock(&print_mutex);
}

void read_filmfare(int customer_no){

    pthread_mutex_lock(&print_mutex);
    cout<<"\nCustomer "<<customer_no+1<<" is reading Filmfare.\n";
    pthread_mutex_unlock(&print_mutex);
}

void sleeping(int barber_no)
{
    pthread_mutex_lock(&print_mutex);
    cout<<"Barber "<<barber_no+1<<" is sleeping.\n";
    pthread_mutex_unlock(&print_mutex);
}

void getting_haircut(int customer_no){

    pthread_mutex_lock(&print_mutex);
    cout<<"Customer "<<customer_no+1<<" is getting HairCut.\n";
    pthread_mutex_unlock(&print_mutex);
}

void leave_shop(int customer_no){

    pthread_mutex_lock(&print_mutex);
    cout<<"Customer "<<customer_no+1<<" left the shop.\n";
    pthread_mutex_unlock(&print_mutex);
}

void pay_money(int customer_no){

    pthread_mutex_lock(&print_mutex);
    cout<<"Customer "<<customer_no+1<<"paying for Haircut.\n";
    pthread_mutex_unlock(&print_mutex);
}

void money_submit(int barber_no){

    pthread_mutex_lock(&print_mutex);
    cout<<"Barber "<<barber_no+1<<" submitted money to Cash Counter.\n";
    pthread_mutex_unlock(&print_mutex);
}


//***********************************Runner Functions ********************************

void *Customer(void *CId) {

    long CustomerId=(long) CId;
    pthread_mutex_lock(&print_mutex);
    cout<<"\nCreating Customer "<<CustomerId+1<<"\n";

    pthread_mutex_unlock(&print_mutex);
    //wait if full capacity
    sem_wait(&capacity);


    //read OS Book
    read_os(CustomerId);
    // waiting for filmfare
    sem_wait(&filmfare);
    read_filmfare(CustomerId);

    pthread_mutex_lock(&customer_queue_mutex);
    customer_queue.push(CustomerId);
    sem_post(&customers);
    pthread_mutex_unlock(&customer_queue_mutex);

    sem_wait(&customer[CustomerId]);
    getting_haircut(CustomerId);
    sem_post(&filmfare);
    sem_post(&paymoney);
    pay_money(CustomerId);
    sem_post(&capacity);
    // Customer leave Shop after paying money
    leave_shop(CustomerId);


}



// Barber runner Function
void *Barber(void* BId){

    long BarberId=(long)BId;
    int front_customer;

    while(true){
                // Barber is Sleeping initially
                sleeping(BarberId);

                // wait for customers to come
                sem_wait(&customers);

                // get the customer from queue
                pthread_mutex_lock(&customer_queue_mutex);
                front_customer=customer_queue.front();
                customer_queue.pop();
                pthread_mutex_unlock(&customer_queue_mutex);

                // call the customer who is reading filmfare for longest time
                sem_post(&customer[front_customer]);
                //*************************
                // Customer getting Haircut
                //*************************
                pthread_mutex_lock(&print_mutex);
                cout<<"Barber "<<BarberId+1<<" is cutting hair of customer "<<front_customer+1<<"\n";
                pthread_mutex_unlock(&print_mutex);

                //Waiting for customer to pay money
                sem_wait(&paymoney);

                //Add Barber to queue
                pthread_mutex_lock(&barber_queue_mutex);
                barber_queue.push(BarberId);
                sem_post(&barbers);
                pthread_mutex_unlock(&barber_queue_mutex);

                sem_wait(&barber[BarberId]);
                sem_post(&submit_money);
                money_submit(BarberId);

        }
}

//CashCounter runner function
void *CashCounter(void *args){

    int front_barber;

    while(true){

            // Wait for barbers to come
            sem_wait(&barbers);
            // Take Barber from Queue
            pthread_mutex_lock(&barber_queue_mutex);
            front_barber=barber_queue.front();
            barber_queue.pop();
            pthread_mutex_unlock(&barber_queue_mutex);

            sem_post(&barber[front_barber]);

            // wait for barber to submit money
            sem_wait(&submit_money);

    }
}

//*****************************Main function****************************

int main(){

    // initialize all semaphores and mutex
    initialize();
    // Declare threads for barbers, customers and cashcounter
    pthread_t barber_threads[totalbarbers];
    pthread_t cust_threads[maxcustomers];
    pthread_t cash_thread;



    long i=0;

    pthread_mutex_lock(&print_mutex);
    cout<< "\nCreating Barber Threads\n";
    pthread_mutex_unlock(&print_mutex);

	while (i < totalbarbers){

             pthread_create(&barber_threads[i],NULL,Barber,(void *)i);
             i++;
 	}

 	pthread_create(&cash_thread,NULL,CashCounter,NULL);

    pthread_mutex_lock(&print_mutex);
    cout<< "\nCreating Customer Threads\n";
    pthread_mutex_unlock(&print_mutex);

    i=0;char ch;


	do {
            cin>>ch;
            pthread_create(&cust_threads[i],NULL,Customer,(void *)i);
            i++;

    }while(ch=='c' && i<maxcustomers);

}
