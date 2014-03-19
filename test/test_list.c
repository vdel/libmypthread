#include <stdio.h>
#include <stdlib.h>
#include "list.h"

LIST_TYPE(ma_structure1,
     int val;
     char* name;
   );
LIST_TYPE(ma_structure2,
     int val;
     char* name;
     ma_structure2_t ma_s;
   );

int test_ma_structure1(){
 printf("*** test_ma_structure1 ****\n");
 ma_structure1_list_t list = ma_structure1_list_new();
 ma_structure1_t a = ma_structure1_new();
 ma_structure1_t b = ma_structure1_new();
 ma_structure1_t c = ma_structure1_new();
 ma_structure1_t d = ma_structure1_new();
 ma_structure1_t e = ma_structure1_new();
 a->val = 0;
 a->name = "toto0";
 b->val = 1;
 b->name = "toto1";
 c->val = 2;
 c->name = "toto3";
 d->val = 3;
 d->name = "toto4";
 e->val = 4;
 e->name = "toto5";
 ma_structure1_list_push_front(list, a);
 ma_structure1_list_push_front(list, b);
 ma_structure1_list_push_back(list, c);
 ma_structure1_list_push_back(list, d);
 ma_structure1_list_push_front(list, e);
 
 printf("size=%d\n", ma_structure1_list_size(list));
 
 ma_structure1_itor_t i;
 for(i  = ma_structure1_list_begin(list);
     i != ma_structure1_list_end(list);
     i  = ma_structure1_list_next(i))
 {
   printf("{val=%d , name=%s}\n", i->val, i->name);
 }
 return 0;
}

int test_ma_structure2(){
 printf("*** test_ma_structure2 ****\n");
 ma_structure2_list_t list = ma_structure2_list_new();
 ma_structure2_t a = ma_structure2_new();
 ma_structure2_t b = ma_structure2_new();
 a->val = 0;
 a->name = "toto0";
 a->ma_s=b;
 b->val = 1;
 b->name = "toto1";
 b->ma_s=a;
 ma_structure2_list_push_front(list, a);
 ma_structure2_list_push_back(list, b);
 
 printf("size=%d\n", ma_structure2_list_size(list));
 
 ma_structure2_itor_t i;
 for(i  = ma_structure2_list_begin(list);
     i != ma_structure2_list_end(list);
     i  = ma_structure2_list_next(i))
 {
   printf("{val=%d , name=%s , ma_s->val=%d , ma_s->name=%s}\n", i->val, i->name,i->ma_s->val,i->ma_s->name);
 }
 return 0;
}

int main(int argc, char ** argv){
	test_ma_structure1();
	test_ma_structure2();
	return 0;
}



