#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <ucontext.h>
#include <stdlib.h>
#include <list.h>
#include <ucontext.h>


//Thread principal
//Pour passer argc et argv en un seul argument: type mythread_args_t
typedef struct {
  int argc;
  char **argv;
} mythread_args_t;

//Pour récupérer argc et argv de la structure mythread_args_t
void mythread_decode_args (mythread_args_t *args, int *argc, char ***argv);

//args est un pointeur vers une structure mythread_args_t
extern int mythread_main(void *args);




//******************** !! THREADS !! ********************//
//Etats possibles d'un thread
enum mythread_state {ready, join_wait, cond_wait, terminated};
enum mythread_depend_state {depend_of_none=0,depend_of_join=1,depend_of_mutex=2};
enum mythread_priority {high=2, medium=1, low=0};

//Attribut pour la création du thread
typedef struct mythread_attr_s {
  enum mythread_priority priority;
} mythread_attr_t;

//Type mythread_t
LIST_TYPE(mythread,
  struct ucontext context;
  enum mythread_state state;
  enum mythread_depend_state depending_threads;
  mythread_t join;
  void **value_ptr;
  mythread_attr_t attr;
);




//Gestion des threads
//--------------------------------------------------------------------------
int  mythread_create (mythread_t *thread, const mythread_attr_t *, void *(*func)(void *), void *args);
mythread_t  mythread_self (void);
void mythread_yield (void);
int  mythread_join (mythread_t, void **value_ptr);
void mythread_exit (void* value_ptr);
//--------------------------------------------------------------------------



//******************** !! MUTEX !! ********************//
struct mythread_mutex_s
{
  int locked;
  mythread_t owner;
  mythread_list_t request;
};

typedef struct mythread_mutex_s mythread_mutex_t;

typedef unsigned mythread_mutexattr_t;


// Gestion des mutex
//--------------------------------------------------------------------------
int mythread_mutex_init (mythread_mutex_t *, const mythread_mutexattr_t *);
int mythread_mutex_destroy (mythread_mutex_t *mutex);
int mythread_mutex_lock (mythread_mutex_t *);
int mythread_mutex_unlock (mythread_mutex_t *);
//--------------------------------------------------------------------------



//******************** !! CONDITIONS !! ********************//
struct mythread_cond_s
{
  mythread_list_t sleep;
};

typedef struct mythread_cond_s mythread_cond_t;

typedef unsigned mythread_condattr_t;


// Gestion des conditions
//--------------------------------------------------------------------------
int mythread_cond_init      (mythread_cond_t *, const mythread_condattr_t *);
int mythread_cond_wait      (mythread_cond_t *, mythread_mutex_t *);
int mythread_cond_signal    (mythread_cond_t *);
int mythread_cond_broadcast (mythread_cond_t *cond);
int mythread_cond_destroy   (mythread_cond_t *);
//--------------------------------------------------------------------------

#endif /* MYTHREAD_H */
