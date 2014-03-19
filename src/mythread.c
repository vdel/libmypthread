/* 
 * mythread.c: librairie de Threads, coopératifs et préemptifs, avec ou sans ordonnancement.
 * 25/04/2008
 * Vincent DELAITRE et Aloïs BRUNEL
 */

#include <errno.h>
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <mythread.h>
#include <string.h>
#include <sys/time.h>

/* 
 *  Un sommaire des fonctions, pour plus de clarté 
 *
 *  0. Prototypes et types privés                             (l.56)
 *    1) Timer de la version préemptive   
 *    2) Ordonnanceur
 *    3) Général
 *
 *  I. Fonctions simples pour les threads                     (l.191)
 *    1) Fonctions auxiliaires (swap, etc...)                  
 *    2) Fonctions principales (create, join, yield, etc...)   
 *
 * II. Fonctions de gestion des mutex                         (l.401)
 *    1) mutex_init, mutex_destroy
 *    2) mutex_lock, mutex_unlock
 * 
 * III. Fonctions de gestion des conditions                   (l.491)
 *    1) cond_init, cond_destroy 
 *    2) cond_wait 
 *    3) cond_signal, cond_broadcast
 * 
 * IV. Ordonnanceur                                           (l.600)
 *    1) scheduler_init, scheduler_destroy
 *    2) scheduler_push, scheduler_pop
 *    3) scheduler_next_priority
 *
 */

// Constantes globales:
#define FONCTIONNEMENT   0       //0: Coopératif  -  1: Préemptif
                                 //En préemptif, on utilise le même timer que 
                                 //la fonction sleep et on ne peut plus utiliser
                                 //celle-ci. Pour absoluement utiliser sleep,
                                 //mettre VIRTUAL_TIMER à 1
#define VIRTUAL_TIMER    0       //(mais préférer VIRTUAL_TIMER = 0)
#define PREEMPT_INTERVAL 100     //Intervalle de préemption en microsecondes
#define ORDONNANCEMENT   1       //0: FIFO        -  1: Perso

int LOCKED = 0;    //TRUE ssi le thread utilise actuellement une fonction
                   //de la librairie.
int SWAP   = 0;    //Dans ce cas, en cas d'interruption du timer, on attend
                   //d'être sorti de la librairie pour faire le swap





/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                  Prototypes des fonctions privées                         */
/*                   types et structures privées                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
//Inititialise le timer
void mythread_settimer (int ms);

//Pour passer argc et argv en un seul argument
void mythread_encode_args (mythread_args_t *args, int *argc, char ***argv);

//Initialise les listes et crée le thread principal
void mythread_init (void *(*func)(void *), int argc, char **argv);

//Exécute le prochain thread
void mythread_swap ();

//Fonction appelée par swapcontext (utile pour le join)
void mythread_handler (void *(*func)(void *), void *args);

//******************** !! TIMER POUR LE PREEMPTIF !! ********************//
void
mythread_settimer (int ms)
{
  struct itimerval new;
  new.it_interval.tv_usec = 0;
  new.it_interval.tv_sec = 0;
  new.it_value.tv_usec = ms;
  new.it_value.tv_sec = 0;
  if(!VIRTUAL_TIMER)
    setitimer (ITIMER_REAL, &new, NULL);
  else
    setitimer (ITIMER_VIRTUAL, &new, NULL);
}

void
mythread_catchsignal (int sig)
{
  if(!FONCTIONNEMENT) return;
  if(LOCKED)        //Impossible de faire un swapcontext maintenant
  {                 //car on exécute une fonction de la librairie
    SWAP=1;         //on se souvient qu'il faudra le faire
    return;
  }
  if(SWAP) return;  //Déjà un swap en attente, on laisse tomber

  mythread_settimer (PREEMPT_INTERVAL);
  mythread_yield();
}

void LOCK()
{
  LOCKED=1;
}

void UNLOCK()
{
  if(SWAP)
  {
    mythread_settimer (PREEMPT_INTERVAL);
    SWAP=0;
    LOCKED=0;
    mythread_yield();
  }
  else
    LOCKED=0;
}

//******************** !! ORDONNANCEUR !! ********************//
// 3 paramètres à prendre en compte:
// Ancienneté de création
// Ancienneté d'exécution
// Attendu par des threads
// 
typedef struct mythread_scheduler_s
{
  //Ordonnanceur FIFO
  mythread_list_t ready_list; 
  //Ordonnanceur Perso
  mythread_list_t ready_ultra_priority;
  mythread_list_t ready_high_priority;
  mythread_list_t ready_medium_priority; 
  mythread_list_t ready_low_priority;
  
  int state;      //On exécute les listes ci-dessus tour à tour
                  //state indique la liste dont c'est le tour:
  //state=0 : high
  //state=1 : ultra
  //state=2 : medium
  //state=3 : ultra
  //state=4 : high
  //state=5 : medium
  //state=6 : ultra
  //state=7 : high
  //state=8 : ultra
  //state=9 : low
  int empty_state;
  mythread_list_t* current_list;
   
} mythread_scheduler_t;

// Ordonnanceur
//--------------------------------------------------------------------------
void mythread_scheduler_init(mythread_scheduler_t *s);
void mythread_scheduler_push(mythread_scheduler_t *s,mythread_t t);
mythread_t mythread_scheduler_pop(mythread_scheduler_t *s);
void mythread_scheduler_next_priority();
void mythread_scheduler_destroy(mythread_scheduler_t *s);
//--------------------------------------------------------------------------








//******************** !! LIBRAIRIE DE THREADS !! ********************//

/* Listes globales */
mythread_scheduler_t scheduler;
mythread_t current_thread       = NULL;

//Fonction principale
int 
main (int argc, char**argv)
{
  //on crée un thread principal avec mythread_main
  mythread_init((void *)&mythread_main, argc, argv);
  return 0;
}



/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                  Fonctions auxiliaires pour les threads                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
//Pour passer argc et argv en un seul argument
//-----------------------------------------------------------------------------
void
mythread_encode_args (mythread_args_t *args, int *argc, char ***argv)
{
  args->argc=*argc;
  args->argv=*argv;
}
//-----------------------------------------------------------------------------
//Pour récupérer argc et argv de la structure mythread_args_t
//-----------------------------------------------------------------------------
void
mythread_decode_args (mythread_args_t *args, int *argc, char ***argv)
{
  *argc=args->argc;
  *argv=args->argv;
}
//-----------------------------------------------------------------------------
//Initialise les listes et crée le thread principal
//-----------------------------------------------------------------------------
void
mythread_init (void *(*func)(void *), int argc, char **argv)
{
  mythread_t main_thread;

  current_thread = NULL;

  //on initialise l'ordonnanceur
  mythread_scheduler_init(&scheduler);

  //on encode argc et argv
  mythread_args_t args;
  mythread_encode_args(&args,&argc,&argv);
  //on défini la priorité
  mythread_attr_t attr;
  attr.priority=high;
  mythread_create(&main_thread, &attr, func,(void *)&args);

  if(FONCTIONNEMENT)
  {
    //Initialise le timer pour le
    //fonctionnement préemptif
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler=&mythread_catchsignal;
    if(!VIRTUAL_TIMER)
      sigaction(SIGALRM,&sa,NULL);
    else
      sigaction(SIGVTALRM,&sa,NULL);
    //On initialise le timer
    mythread_settimer (PREEMPT_INTERVAL);
  }
  //On lance le thread principal
  mythread_swap();  
}
//-----------------------------------------------------------------------------
//Libère le thread courant
//-----------------------------------------------------------------------------
void
mythread_free (void)
{
  mythread_self()->state=terminated;
  //free(mythread_self()->context.uc_stack.ss_sp);
  //mythread_delete(mythread_self());
}
//-----------------------------------------------------------------------------
//Exécute le prochain thread
//-----------------------------------------------------------------------------
void
mythread_swap (void)
{
  //on récupère le prochain thread à exécuter
  mythread_t next = mythread_scheduler_pop(&scheduler);
  if(next != NULL)
  {    

    //on sauve le thread courant
    mythread_t old  = mythread_self();
    //on actualise le thread courant
    current_thread = next; 
    if(old != NULL) //old != NULL sauf pour le thread principal
      swapcontext(&old->context, &next->context);
    else
      setcontext(&next->context);
  }
}
//-----------------------------------------------------------------------------
//Fonction appelée par swapcontext (utile pour le join)
//-----------------------------------------------------------------------------
void
mythread_handler (void *(*func)(void *), void *args)
{
  //On exécute le thread suivit de mythread_exit
  mythread_exit((*func)(args));
}



/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                            Librairie de thread                            */
/*                                                                           */
/*---------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
//Crée un thread                                                                
//-----------------------------------------------------------------------------
int
mythread_create (mythread_t *thread, const mythread_attr_t *attr, void *(*func)(void *), void *args)
{
  *thread = mythread_new();

  if (*thread == NULL)
    return 1;

  if(attr==NULL)
  {
    (*thread)->attr.priority=medium;
  }
  else
  {
    (*thread)->attr=*attr;
  }

  getcontext(&(*thread)->context);               
  (*thread)->context.uc_stack.ss_size = 64*1024;  
  (*thread)->context.uc_stack.ss_sp = malloc((*thread)->context.uc_stack.ss_size);
  (*thread)->context.uc_stack.ss_flags = 0;       
  (*thread)->context.uc_link = NULL; 

  makecontext(&(*thread)->context, (void *)&mythread_handler, 2, func, args);

  (*thread)->state = ready;
  (*thread)->join = NULL;   
  
  mythread_scheduler_push(&scheduler,*thread);
  
  return 0;
}
//-----------------------------------------------------------------------------
//Renvoit le thread courant
//-----------------------------------------------------------------------------
mythread_t
mythread_self (void)
{
  return current_thread;
}
//-----------------------------------------------------------------------------
//Passe la main au prochain thread
//-----------------------------------------------------------------------------
void
mythread_yield ()
{
  mythread_scheduler_push(&scheduler,mythread_self());
  mythread_swap();
}
//-----------------------------------------------------------------------------
//Mise en attente sur un thread
//-----------------------------------------------------------------------------
int
mythread_join (mythread_t thread, void **value_ptr)
{
  if(thread->state!=terminated)
  {
    LOCK();
    mythread_self()->state = join_wait;
    thread->join  = mythread_self();
    thread->value_ptr=value_ptr;
    thread->depending_threads=thread->depending_threads|depend_of_join;
    UNLOCK();
    mythread_swap();
  }
  return 0;
} 
//-----------------------------------------------------------------------------
//Termine le thread courant
//-----------------------------------------------------------------------------
void 
mythread_exit (void* value_ptr)
{
  //On termine le thread
  //On sauve la valeur de retour
  if(mythread_self()->value_ptr!=NULL)
    *mythread_self()->value_ptr=value_ptr;

  //S'il y a un join sur le thread, on remet le thread qui nous attend sur ready
  if (mythread_self()->join && mythread_self()->join->state == join_wait)
  {
    mythread_self()->depending_threads=mythread_self()->depending_threads&(~depend_of_join);
    //En cas de join on rend la main au thread en attente
    mythread_self()->join->state=ready;
    mythread_scheduler_push(&scheduler,mythread_self()->join);
  }
  //On supprime le thread qui termine
  mythread_free();
  //On lance le prochain thread
  mythread_swap();
}





/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                            Gestion des mutex                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// Crée un mutex
//-----------------------------------------------------------------------------
int
mythread_mutex_init (mythread_mutex_t *mutex, const mythread_mutexattr_t *attr)
{
  if (mutex == NULL)
    return EINVAL;
  
  mutex->request = mythread_list_new();
  if (mutex->request == NULL)
    return ENOMEM;

  mutex->locked  = 0;
  mutex->owner = NULL;
  return 0;
}
//-----------------------------------------------------------------------------
// Supprime un mutex
//-----------------------------------------------------------------------------
int
mythread_mutex_destroy (mythread_mutex_t *mutex)
{
  if (mutex == NULL)
    return EINVAL;

  if (mutex->locked)
    return EBUSY;

  mythread_list_delete(mutex->request);
 
  return 0;
}
//-----------------------------------------------------------------------------
// Vérouiller un mutex
//-----------------------------------------------------------------------------
int 
mythread_mutex_lock (mythread_mutex_t *mutex)
{
  if (mutex == NULL)
    return EINVAL;

  while(mutex->locked)
  {
    mythread_list_push_back(mutex->request, mythread_self());
    mythread_swap();
  }
  LOCK();  
  mutex->locked = 1;
  mutex->owner = mythread_self();
  mythread_self()->depending_threads=mythread_self()->depending_threads|depend_of_mutex;
  UNLOCK();
  return 0;
}
//-----------------------------------------------------------------------------
// Dévérouiller un mutex
//-----------------------------------------------------------------------------
int 
mythread_mutex_unlock (mythread_mutex_t *mutex)
{
  if (mutex == NULL)
    return EINVAL;

  if (!mutex->locked)
    return EPERM;

  if(mutex->owner != mythread_self())
    return EPERM;

  LOCK();
  while(!mythread_list_empty(mutex->request))
    mythread_scheduler_push(&scheduler,mythread_list_pop_front(mutex->request));
  mythread_list_delete(mutex->request);  
  mutex->request = mythread_list_new();

  mutex->locked = 0;
  mutex->owner = NULL;
  mythread_self()->depending_threads=mythread_self()->depending_threads&(~depend_of_mutex);
  UNLOCK();
  return 0;
}



/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                          Gestion des conditions                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// Crée une nouvelle condition
//-----------------------------------------------------------------------------
int
mythread_cond_init (mythread_cond_t *cond, const mythread_condattr_t *a)
{
  if (cond == NULL)
    return ENOMEM;

  cond->sleep = mythread_list_new();
  if (cond->sleep == NULL)
    return ENOMEM;  

  return 0;
}
//-----------------------------------------------------------------------------
// Supprime la condition
//-----------------------------------------------------------------------------
int 
mythread_cond_destroy (mythread_cond_t *c)
{
  if (c == NULL)
    return EINVAL;

  if (!mythread_list_empty(c->sleep))
    return EBUSY;

  mythread_list_delete(c->sleep);

  return 0;
}


//-----------------------------------------------------------------------------
// Bloque l'exécution jusqu'au prochain signal sur la condition c
//-----------------------------------------------------------------------------
int 
mythread_cond_wait (mythread_cond_t *c, mythread_mutex_t *m)
{
  if (c == NULL)
    return EINVAL;

  int err = mythread_mutex_unlock(m);
  if(!err) return err;

  LOCK();
  mythread_list_push_back(c->sleep, mythread_self());  
  mythread_self()->state = cond_wait;
  UNLOCK();
  while(mythread_self()->state == cond_wait)
    mythread_swap();
  mythread_mutex_lock(m);

  return 0;
}
//-----------------------------------------------------------------------------
// Envoie un signal sur la condition c
//-----------------------------------------------------------------------------
int 
mythread_cond_signal  (mythread_cond_t *c) 
{
  mythread_t th;
  
  if (c == NULL)
    return EINVAL;

  if (mythread_list_empty(c->sleep))
    return 0;

  LOCK();
  th = mythread_list_pop_front(c->sleep);
  th->state = ready;
  mythread_scheduler_push(&scheduler,th);
  UNLOCK();
  return 0;
}
//-----------------------------------------------------------------------------
// Envoie un signal à tous les threads sur la condition c
//-----------------------------------------------------------------------------
int
mythread_cond_broadcast (mythread_cond_t *c)
{
  mythread_t th;
  
  if (c == NULL)
    return EINVAL;

  if (mythread_list_empty(c->sleep))
    return 0;

  LOCK();
  while (!mythread_list_empty(c->sleep))
  {
    th = mythread_list_pop_front(c->sleep);
    th->state = ready;
  }  
  mythread_scheduler_push(&scheduler,th);
  UNLOCK();
  return 0;
}



/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Ordonnanceur                                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// Initialise l'ordonnanceur
//-----------------------------------------------------------------------------
void mythread_scheduler_init(mythread_scheduler_t *s)
{
  if(!ORDONNANCEMENT)
    s->ready_list     = mythread_list_new();
  else
  {
    s->ready_ultra_priority  = mythread_list_new();
    s->ready_high_priority   = mythread_list_new();
    s->ready_medium_priority = mythread_list_new(); 
    s->ready_low_priority    = mythread_list_new();
    
    s->empty_state=-1;
    s->state=0;
    s->current_list=&s->ready_high_priority;
  }
}
//-----------------------------------------------------------------------------
// Supprime l'ordonnanceur
//-----------------------------------------------------------------------------
void mythread_scheduler_destroy(mythread_scheduler_t *s)
{
  if(!ORDONNANCEMENT)
    mythread_list_delete(s->ready_list);
  else
  {
    mythread_list_delete(s->ready_ultra_priority );
    mythread_list_delete(s->ready_high_priority );
    mythread_list_delete(s->ready_medium_priority );
    mythread_list_delete(s->ready_low_priority );
  }
}
//-----------------------------------------------------------------------------
// Ajoute un thread à la liste des thread prêts
//-----------------------------------------------------------------------------
void mythread_scheduler_push(mythread_scheduler_t *s,mythread_t t)
{
  if(!ORDONNANCEMENT)
    mythread_list_push_back(s->ready_list, t);
  else
  {
    //Priorité de base
    int priority=t->attr.priority;
    //On modifie la priorité si le thread est attendu
    if(t->depending_threads&depend_of_join)  priority+=2;
    if(t->depending_threads&depend_of_mutex) priority+=1;
    
    //on récupère la priorité finale
    if(priority>3) priority=3;
    if(priority<0) priority=0;

    switch(priority)
    {
      case 3: mythread_list_push_back(s->ready_ultra_priority,t);  break;
      case 2: mythread_list_push_back(s->ready_high_priority,t);   break;
      case 1: mythread_list_push_back(s->ready_medium_priority,t); break;
      case 0: mythread_list_push_back(s->ready_low_priority,t);    break;
    }
  }
}
//-----------------------------------------------------------------------------
// Renvoit le prochain thread à exécuter
//-----------------------------------------------------------------------------
mythread_t mythread_scheduler_pop(mythread_scheduler_t *s)
{
  if(!ORDONNANCEMENT)
  {
    if(!mythread_list_empty(s->ready_list))
      return mythread_list_pop_front(s->ready_list);
    else
      return NULL;
  }
  else
  {  
    if(mythread_list_empty(*(s->current_list)))
    {
      if(s->empty_state==-1)
        s->empty_state=s->state;
      else if(s->empty_state==s->state)
        return NULL;
      mythread_scheduler_next_priority(s);    
      return mythread_scheduler_pop(s);
    }
    else
    {
      s->empty_state=-1;
      mythread_t t=mythread_list_pop_front(*(s->current_list));
      mythread_scheduler_next_priority(s);
      return t;
    }  
  }
}
//-----------------------------------------------------------------------------
// Change la liste des priorité en préemptif
//-----------------------------------------------------------------------------
void mythread_scheduler_next_priority(mythread_scheduler_t *s)
{
  s->state=(s->state+1)%10;
  switch(s->state)
  {
    case 0: s->current_list=&s->ready_high_priority;   break;
    case 1: s->current_list=&s->ready_ultra_priority;  break;
    case 2: s->current_list=&s->ready_medium_priority; break;
    case 3: s->current_list=&s->ready_ultra_priority;  break;
    case 4: s->current_list=&s->ready_high_priority;   break;
    case 5: s->current_list=&s->ready_medium_priority; break;
    case 6: s->current_list=&s->ready_ultra_priority;  break;
    case 7: s->current_list=&s->ready_high_priority;   break;
    case 8: s->current_list=&s->ready_ultra_priority;  break;
    case 9: s->current_list=&s->ready_low_priority;    break;
  }
}

