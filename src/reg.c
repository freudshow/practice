#include <regex.h>                                                              
#include <locale.h>                                                             
#include <stdio.h>                                                              
#include <stdlib.h>                                                             
                                                                                
int tmain(int argc, char* argv[]) 
{                                                                        
    regex_t    preg;                                                            
    char       *string = "a simple string";                                     
    char       *pattern = ".*(simple).*";                                       
    int        rc;                                                              
    size_t     nmatch = 2;                                                      
    regmatch_t pmatch[2];                                                       
                                                                                
    if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {                    
       printf("regcomp() failed, returning nonzero (%d)\n", rc);                
       exit(1);                                                                 
    }                                                                           
                                                                                
    if ((rc = regexec(&preg, string, nmatch, pmatch, 0)) != 0) {                
       printf("failed to ERE match '%s' with '%s',returning %d.\n",             
       string, pattern, rc);                                                    
    } else {
       printf("match success\n");
    }                                                 
                                                                                
    regfree(&preg);                                                             
    
    return 0;
}

#include "types.h"
#include "mcheck.h"
#include "regex.h"
#include "stdio.h"
#include "stdlib.h"
#include"regex_internal.h"


static const char text[] = "This is a test; this is a test";

int main (void)
{
    regex_t re;
    regmatch_t rm[2];

    int n;
    int i;
    re_token_t *tkn;

    n = regcomp (&re, "a test", (REG_EXTENDED|REG_ICASE));
    if (n != 0)
    {
        char buf[500];
        regerror (n, &re, buf, sizeof (buf));
        printf ("regcomp failed: %s\n", buf);
        exit (1);
    }

    printf("re.buffer=%p\n",re.buffer);
    re_dfa_t *dfa=(re_dfa_t *)re.buffer;
    tkn=dfa->nodes;
    for (n = 0; n < 1; ++n)
    {
        printf("Bin tree addr is %p\n",dfa->str_tree_storage);
        printf("DFA state_table  is %p\n",dfa->state_table);
        printf("\tDFA state_table->(num=%d,alloc=%d,array=%p)\n",dfa->state_table->num,dfa->state_table->alloc,dfa->state_table->array);

        printf("DFA init_state  is %p\n",dfa->init_state);
        printf("\tDFA init_state->(hash=%d,entrance_nodes=%p,nodes.nelem=%d)\n",dfa->init_state->hash,dfa->init_state->entrance_nodes,dfa->init_state->nodes.nelem);
        printf("DFA init_state_begbuf  is %p\n",dfa->init_state_begbuf);
        printf("DFA init_state_nl  is %p\n",dfa->init_state_nl);
        printf("DFA init_state_word  is %p\n",dfa->init_state_word);

        printf("DFA node addr is %p\n",dfa->nodes);
        printf("DFA next addr is %p\n",dfa->nexts);

        printf("DFA nodes_alloc  is %d\n",dfa->nodes_alloc);
        printf("DFA nodes_len  is %d\n",dfa->nodes_len);
        printf("DFA edests  is %p\n",dfa->edests);
        printf("\tDFA edests->(nelem=%d,alloc=%d,elem=%p)\n",dfa->edests->nelem,dfa->edests->alloc,dfa->edests->elems);
        printf("DFA eclosures  is %p\n",dfa->eclosures);
        printf("\tDFA eclosures->(nelem=%d,alloc=%d,elem=%p)\n",dfa->eclosures->nelem,dfa->eclosures->alloc,dfa->eclosures->elems);

        printf("DFA inveclosures  is %p\n",dfa->inveclosures);

        printf("DFA inveclosures  is %p\n",dfa->inveclosures);




        printf("DFA org_indices  is %p\n",dfa->org_indices);
        printf("DFA sb_char  is %p\n",dfa->sb_char);

        for (i = 0; i < dfa->nodes_len; ++i)
        {
            printf("(%d,%c),(C:%d,D:%d)\n",tkn->type,tkn->opr.idx,tkn->constraint,tkn->duplicated);
            tkn+=1;
        }

        regexec (&re, text, 1, rm, 0);
        printf("\nrm[0].eo=%d,so=%d",rm[0].rm_eo,rm[0].rm_so);
        memset((void *)&rm[0],0,sizeof(rm));

    }
    regfree (&re);

  return 0;
}
