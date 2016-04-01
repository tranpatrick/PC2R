#ifndef _SERVER_H_
#define _SERVER_H_

/* Gère les timers pour les phases */
void* thread_timer(void *arg);

/* Gère la phase de résolution */
void *thread_resolution(void *arg);

/* Gère la phase d'enchere */
void* thread_enchere(void *arg);

/* Gère la phase de réflexion */
void* thread_reflexion(void *arg);

/* gère la reception des messages d'un client */
void* thread_reception(void *arg);

/* traitement à effectuer pour chaque client */
void* client_thread(void *arg);

#endif

