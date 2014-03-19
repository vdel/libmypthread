/** @file
 * @brief Listes doublement chain�es automatiques
 */


/** @remarks list how-to
 * *********************************************************
 * LIST_TYPE(FOO, contenu);
 *  - d�clare les types suivants
 *      + pour les cellules : FOO_t
 *      + pour les listes : FOO_list_t
 *      + pour les it�rateurs : FOO_itor_t
 *  - d�clare les accesseurs suivants :
 *     * cr�ation d'une cellule 
 *   FOO_t      FOO_new(void);  
 *     * suppression d'une cellule
 *   void       FOO_delete(FOO_t); 
 *     * cr�ation d'une liste (vide)
 *   FOO_list_t FOO_list_new(void);
 *     * suppression d'une liste
 *   void       FOO_list_delete(FOO_list_t);
 *     * teste si une liste est vide
 *   int        FOO_list_empty(FOO_list_t);
 *     * retire un �l�ment de la liste
 *   void       FOO_list_erase(FOO_list_t, FOO_t);
 *     * ajoute une �l�ment en queue de liste
 *   void       FOO_list_push_back(FOO_list_t, FOO_t);
 *     * ajoute un �l�ment en t�te de list
 *   void       FOO_list_push_front(FOO_list_t, FOO_t);
 *     * retire l'�l�ment en queue de liste
 *   FOO_t      FOO_list_pop_back(FOO_list_t);
 *     * retire l'�lement en t�te de liste
 *   FOO_t      FOO_list_pop_front(FOO_list_t);
 *     * retourne l'�l�ment en queue de liste
 *   FOO_t      FOO_list_back(FOO_list_t);
 *     * retourne l'�lement en t�te de liste
 *   FOO_t      FOO_list_front(FOO_list_t);
 * *********************************************************
 * Exemples d'utilisation :
 *  - au d�part, on a :
 *    struct ma_structure_s
 *    {
 *      int a;
 *      int b;
 *    };
 *  - on veut en faire une liste. On remplace la d�claration par :
 *    LIST_TYPE(ma_structure,
 *      int a;
 *      int b;
 *    );
 *    qui cr�e les types ma_structure_t et ma_structure_list_t.
 *  - allocation d'une liste vide :
 *  ma_structure_list_t l = ma_structure_list_new();
 *  - ajouter un �l�ment 'e' en t�te de la liste 'l' :
 *  ma_structure_t e = ma_structure_new();
 *  e->a = 0;
 *  e->b = 1;
 *  ma_structure_list_push_front(l, e);
 *  - it�rateur de liste :
 *  ma_structure_itor_t i;
 *  for(i  = ma_structure_list_begin(l);
 *      i != ma_structure_list_end(l);
 *      i  = ma_structure_list_next(i))
 *  {
 *    printf("a=%d; b=%d\n", i->a, i->b);
 *  }
 * *********************************************************
 */



/**@hideinitializer
 * Generates a new type for list of elements */
#define LIST_TYPE(ENAME, DECL) \
  LIST_DECLARE_TYPE(ENAME) \
  LIST_CREATE_TYPE(ENAME, DECL)

/**@hideinitializer
 * Forward type declaration for lists */
#define LIST_DECLARE_TYPE(ENAME) \
  /** automatic type: ENAME##_list_t is a list of ENAME##_t */ \
  typedef struct ENAME##_list_s* ENAME##_list_t; \
  /** automatic type: defines ENAME##_t */ \
  typedef struct ENAME##_s* ENAME##_t; \
  /** automatic type: ENAME##_itor_t is an iterator on lists of ENAME##_t */ \
  typedef ENAME##_t ENAME##_itor_t;

/**@hideinitializer
 * The effective type declaration for lists */
#define LIST_CREATE_TYPE(ENAME, DECL) \
  /** from automatic type: ENAME##_t */ \
  struct ENAME##_s \
  { \
    struct ENAME##_s*_prev; /**< @internal previous cell */ \
    struct ENAME##_s*_next; /**< @internal next cell */ \
    DECL \
  }; \
  /** @internal */ \
  struct ENAME##_list_s \
  { \
    struct ENAME##_s* _head; /**< @internal head of the list */ \
    struct ENAME##_s* _tail; /**< @internal tail of the list */ \
  }; \
  /** @internal */static inline ENAME##_t ENAME##_new(void) \
    { ENAME##_t e = (ENAME##_t)malloc(sizeof(struct ENAME##_s)); \
      e->_next = NULL; e->_prev = NULL; return e; } \
  /** @internal */static inline void ENAME##_delete(ENAME##_t e) \
    { free(e); } \
  /** @internal */static inline void ENAME##_list_push_front(ENAME##_list_t l, ENAME##_t e) \
    { if(l->_tail == NULL) l->_tail = e; else l->_head->_prev = e; \
      e->_prev = NULL; e->_next = l->_head; l->_head = e; } \
  /** @internal */static inline void ENAME##_list_push_back(ENAME##_list_t l, ENAME##_t e) \
    {  if(l->_head == NULL) l->_head = e; else l->_tail->_next = e; \
      e->_next = NULL; e->_prev = l->_tail; l->_tail = e; } \
  /** @internal */static inline ENAME##_t ENAME##_list_front(ENAME##_list_t l) \
    { return l->_head; } \
  /** @internal */static inline ENAME##_t ENAME##_list_back(ENAME##_list_t l) \
    { return l->_tail; } \
  /** @internal */static inline ENAME##_list_t ENAME##_list_new(void) \
    { ENAME##_list_t l; l=(ENAME##_list_t)malloc(sizeof(struct ENAME##_list_s)); \
      l->_head=NULL; l->_tail=l->_head; return l; } \
  /** @internal */static inline int ENAME##_list_empty(ENAME##_list_t l) \
    { return (l->_head == NULL); } \
  /** @internal */static inline void ENAME##_list_delete(ENAME##_list_t l) \
    { free(l); } \
  /** @internal */static inline void ENAME##_list_erase(ENAME##_list_t l, ENAME##_t c) \
    { ENAME##_t p = c->_prev; if(p) p->_next = c->_next; else l->_head = c->_next; \
      if(c->_next) c->_next->_prev = p; else l->_tail = p; } \
  /** @internal */static inline ENAME##_t ENAME##_list_pop_front(ENAME##_list_t l) \
    { ENAME##_t e = ENAME##_list_front(l); \
      ENAME##_list_erase(l, e); return e; } \
  /** @internal */static inline ENAME##_t ENAME##_list_pop_back(ENAME##_list_t l) \
    { ENAME##_t e = ENAME##_list_back(l); \
      ENAME##_list_erase(l, e); return e; } \
  /** @internal */static inline ENAME##_itor_t ENAME##_list_begin(ENAME##_list_t l) \
    { return l->_head; } \
  /** @internal */static inline ENAME##_itor_t ENAME##_list_end(ENAME##_list_t l) \
    { return NULL; } \
  /** @internal */static inline ENAME##_itor_t ENAME##_list_next(ENAME##_itor_t i) \
    { return i->_next; } \
  /** @internal */static inline int ENAME##_list_size(ENAME##_list_t l) \
    { ENAME##_itor_t i=l->_head; int k=0; while(i!=NULL){k++;i=i->_next;} return k; }


