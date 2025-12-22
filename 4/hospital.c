#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define N 30
#define M 3
const int TOTAL_PATIENTS = 40;

sem_t max_patients;
sem_t mutex_doctor;
sem_t mutex_patient;
sem_t patients;
sem_t doctors;
sem_t queue_mutex;

int DState[M];
int PState[N];
int patient_counter = 0;
int treated_patients = 0;
int waiting_room[N];
int wr_front = 0, wr_rear = 0;

void posix_error(int code, char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(EXIT_FAILURE);
}

void Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
    int rv = pthread_create(thread, attr, start_routine, arg);
    if (rv != 0) {
        posix_error(rv, "pthread_create");
    }
}

void Pthread_join(pthread_t thread, void **retval) {
    int rv = pthread_join(thread, retval);
    if (rv != 0) {
        posix_error(rv, "pthread_join");
    }
}

void* patient(void* arg) {
    int patient_id = (int)(long)arg;
    
    printf("patient %d: arrive\n", patient_id + 1);
    
    if(sem_trywait(&max_patients) != 0) {
        printf("patient %d: leave with no number\n", patient_id + 1);
        return NULL;
    }
    
    sem_wait(&mutex_patient);
    int my_number = patient_counter++;
    PState[my_number] = 0;
    sem_post(&mutex_patient);
    
    printf("patient %d: get number %d\n", patient_id + 1, my_number + 1);
    
    sem_wait(&queue_mutex);
    waiting_room[wr_rear] = my_number;
    wr_rear = (wr_rear + 1) % N;
    sem_post(&queue_mutex);
    
    sem_post(&patients);
    
    printf("patient %d: waiting\n", patient_id + 1);
    
    while(1) {
        sem_wait(&mutex_patient);
        if(PState[my_number] == -1) {
            sem_post(&mutex_patient);
            break;
        }
        sem_post(&mutex_patient);
        usleep(100000);
    }
    
    printf("patient %d: leave\n", my_number + 1);
    
    return NULL;
}

void* doctor(void* arg) {
    int doctor_id = (int)(long)arg;
    
    while(treated_patients < N) {
        sem_wait(&patients);
        
        if(treated_patients >= N) {
            sem_post(&patients);
            break;
        }
        
        sem_wait(&doctors);
        
        sem_wait(&queue_mutex);
        if(wr_front == wr_rear) {
            sem_post(&queue_mutex);
            sem_post(&doctors);
            sem_post(&patients);
            continue;
        }
        
        int current_patient = waiting_room[wr_front];
        wr_front = (wr_front + 1) % N;
        sem_post(&queue_mutex);
        
        sem_wait(&mutex_doctor);
        DState[doctor_id] = 1;
        sem_post(&mutex_doctor);
        
        sem_wait(&mutex_patient);
        PState[current_patient] = 1;
        sem_post(&mutex_patient);
        
        printf("doctor %d: start treat patient %d\n", doctor_id + 1, current_patient + 1);
        
        int treatment_time = rand() % 100000;
        usleep(treatment_time);
        
        printf("doctor %d: complete treat patient %d\n", doctor_id + 1, current_patient + 1);
        
        sem_wait(&mutex_patient);
        treated_patients++;
        PState[current_patient] = -1;
        sem_post(&mutex_patient);
        
        sem_wait(&mutex_doctor);
        DState[doctor_id] = 0;
        sem_post(&mutex_doctor);
        
        sem_post(&doctors);
    }
    
    printf("doctor %d: done\n", doctor_id + 1);
    return NULL;
}

int main() {
    pthread_t doctor_threads[M];
    pthread_t patient_threads[TOTAL_PATIENTS];
    
    srand(time(NULL));
    
    sem_init(&max_patients, 0, N);
    sem_init(&mutex_doctor, 0, 1);
    sem_init(&mutex_patient, 0, 1);
    sem_init(&patients, 0, 0);
    sem_init(&doctors, 0, M);
    sem_init(&queue_mutex, 0, 1);
    
    for(int i = 0; i < M; i++) {
        DState[i] = 0;
    }
    for(int i = 0; i < N; i++) {
        PState[i] = -1;
        waiting_room[i] = -1;
    }
    
    for(int i = 0; i < M; i++) {
        Pthread_create(&doctor_threads[i], NULL, doctor, (void *)(long)i);
    }
    
    for(int i = 0; i < TOTAL_PATIENTS; i++) {
        Pthread_create(&patient_threads[i], NULL, patient, (void *)(long)i);
        
        int interval = rand() % 100000;
        usleep(interval);
    }

    for(int i = 0; i < TOTAL_PATIENTS; i++) {
        Pthread_join(patient_threads[i], NULL);
    }

    for(int i = 0; i < M - 1; i++) {
        sem_post(&patients);
    }

    for(int i = 0; i < M; i++) {
        Pthread_join(doctor_threads[i], NULL);
    }
    
    printf("today: %d patient, %d treated\n", 
           patient_counter < N ? patient_counter : N, treated_patients);
    
    return 0;
}