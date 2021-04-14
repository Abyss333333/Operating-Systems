#include "common.h"


int factorial (int num){
    //printf("%d, %d \n",num, sum);
    
    if (num == 0){
        return 1;
    }
    else {
        return (num*factorial(num-1));
    }
    
}

int
main(int argc, char **argv)
{
	if (argc == 1){
        return (printf("Huh?\n"));
    }
    else {
        
        char *ptr;
        long x = strtol (argv[1], &ptr, 10);
        if (x < 1 || ptr[0]=='.'){
           return (printf("Huh?\n"));
           
        }
        else if ( x > 12) {
            return (printf("Overflow\n"));
        }
        int sum = factorial(x);
        printf("%d\n", sum);
    }
	return 0;
}
