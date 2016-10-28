#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {

        //testing for malloc

        //test to see is we malloc next object in superblock
        void *a = malloc(5);
        printf("Malloc a(5) %p\n", a);
        void *a1 = malloc(5);
        printf("Malloc a1(5) %p\n", a1);

        //test to see if this mallocs in another superblock of different level
        void *b = malloc(500);
        printf("Malloc b(50) %p\n", b);

        //test for when malloc size is negative
        void *d = malloc(-1);
        printf("Malloc d(-1) %p\n", d);

        //test for when malloc size is 0
        void *e = malloc(0);
        printf("Malloc e(0) %p\n", e);

        //test for what happens when we reach the end of a super block
        void *f = malloc(2048);
        printf("Malloc f(2048) %p\n", f);
        void *g = malloc(2048);
        printf("Malloc g(2048) %p\n", g);


        //testing for free

        //are we freeing something we malloced?
        free(a);
        printf("Free a %p\n", a);
        free(a1);
        printf("Free a1 %p\n", a1);

        //what happens when we free something twice
        printf("trying to free an object twice, if free code is not commented out, this will cause a trace/breakpoint error\n");
//      free(a1);

        //are we putting the correct objects on free_list
        void *h = malloc(5);
        printf("Malloc h(5) %p\n", h); //should be same address as a1

        //passing in null pointer for free
        printf("passing free a nullpointer\n");
//      void *nullPoint = NULL;
//      free(nullPoint);

        //freeing something that hasnt been malloced
//      printf("freeing something that hasnt been malloced");
//      void *z;
//      free(z);
//      printf("Should cause an issue");

        //poisoning test

        void *w = malloc(5);
        void* current;
        //for loop should return all poisoned bytes for the object as cd
        for(current = w; current < w + 32; current+=sizeof(int)){
                printf("current %x\n",*((int*) current));
        }
        printf("\n");
        *((int*)w) = 5;  //first 4 bytes should be 5, the rest is still poisoned
        for(current = w; current < w + 32; current+=sizeof(int)){
                printf("current %x\n",*((int*) current));
        }


        printf("\n");
        printf("Free \n");
        free(w);
        //for loop should return the first 8 bytes as a the free list then the rest poisoned with ab
        for(current = w; current < w + 64; current+=sizeof(int)){
                printf("current %x\n",*((int*) current));
        }


        //munmap test
        printf("\n");
        printf("Malloc i(2048), j(2048)....");
        void *i = malloc(2048);
        void *j = malloc(2048);
        void *k = malloc(2048);
        void *l = malloc(2048);
        void *m = malloc(2048);
        printf("Malloced 5 objects of size 2048\n");

        printf("\n");
        printf("Free i,k,m\n");
        free(i);
        free(k);
        free(m);
        return (errno);
}

