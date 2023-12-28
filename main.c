#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t lock;
    int data;
    int is_ready;
} mutex_data;

mutex_data global_data = {
    .lock = PTHREAD_MUTEX_INITIALIZER, 
    .cond = PTHREAD_COND_INITIALIZER,
    .data = 0,
    .is_ready = 0,
};

void* DataProviderThread() {
    for (int i = 1; i < 10; i++) {
        printf("Поток-поставщик отправил задание № %d\n", i);
        provider_handler(&global_data, i);
        sleep(1);
    }
    return NULL;
}

void provider_handler(mutex_data *channel, int data) {
	
    pthread_mutex_lock(&channel->lock);
    if (channel->is_ready == 1) {
        return;
    }
	channel->data = data;
	channel->is_ready = 1;
	pthread_cond_signal(&channel->cond);
	pthread_mutex_unlock(&channel->lock); 
}

void* DataConsumerThread() {
    while (1) {
        int data = consumer_handler(&global_data);
        printf("Поток-потребитель получил задание № %d\n", data);
        if (data == 9) {
            break;
        }
    }
    return NULL;
}

int consumer_handler(mutex_data *channel) {
    pthread_mutex_lock(&channel->lock);
    while (channel->is_ready == 0) {
        pthread_cond_wait(&channel->cond, &channel->lock);
    }
    int data = channel->data;
    channel->is_ready = 0;
    pthread_mutex_unlock(&channel->lock); 
    return data;
}

int main() {
    pthread_t thread_provider;
    pthread_t thread_consumer;
    int result_provider = pthread_create(&thread_provider, NULL, DataProviderThread, NULL);
    if (result_provider != 0) {
        printf("ошибка создания потока: %d\n", result_provider);
        return result_provider;
    }
    int result_worker = pthread_create(&thread_consumer, NULL, DataConsumerThread, NULL);
    if (result_provider != 0) {
        printf("ошибка создания потока: %d\n", result_worker);
        return result_worker;
    }
    pthread_join(thread_provider, NULL);
    pthread_join(thread_consumer, NULL);
    return EXIT_SUCCESS;
}