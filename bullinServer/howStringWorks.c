//
//  howStringWorks.c
//  bullinServer
//
//  Created by Ernie on 2020-06-07.
//  Copyright © 2020 Ernie. All rights reserved.
//



// there is no type as String in C
// if you want to declare string, you have to use char*
// which means:

void howStringWorks(void) {
    // https://stackoverflow.com/questions/20944784/why-is-conversion-from-string-constant-to-char-valid-in-c-but-invalid-in-c
    
    
    char const* thisIsAStringInC = "thisIsAStringInC";  //char* is actually a pointer to a character. In C, strings are passed-by-reference by passing the pointer to the first character of the string. The end of the string is determined by setting the byte after the last character of the string to NULL (0).
    
    // char*  is char arrays, char* is a pointer which point to the beginning of that array
    
    char thisIsACharArray[4] = {'a', 'b', 'v', 0}; // 🌟notice single quote means char, double quotes it becomes a char pointer like the previous variable, Ça va dire le string dans le C langue , in c++ there is a template class called string
    
    //signed char, which gives you at least the -127 to 127 range. (-128 to 127 is common)
    // unsigned char, which gives you at least the 0 to 255 range.
    
    printf("howStringWorks: %s----thisIsACharArray:---%s\n", thisIsAStringInC, thisIsACharArray);
    
    
    // how to use char* a[]; char **a;
    
    const char* thisIsCharPointerArray[] = {"aa", "bb", "cc"};
   // char **ThisIsPointerPointsToCharPointer // usually as a funciton parameter accept  the variables like above
    
    char* thisIsCharPointerArray1[] = {"aa", "bb", "cc"};
    char **ThisIsPointerPointsToCharPointer ;
    ThisIsPointerPointsToCharPointer = thisIsCharPointerArray1;
    
    printf("howStringWorks: ----%s----ThisIsPointerPointsToCharPointer:---\n", ThisIsPointerPointsToCharPointer[1]);
    // https://stackoverflow.com/questions/23859238/passing-char-parameter-char-array-c/23861111#23861111
    
};



// comment utiliser char** et  char* var[];: https://stackoverflow.com/questions/23859238/passing-char-parameter-char-array-c/23861111#23861111
void fn(int argc, char ** argv) {
    printf("argc : %d\n", argc);
    for (int i = 0; i < argc; ++i) {
        printf("%s\n", argv[i]);
    }
    
   
    
}
/* 🌟calling
char * var3[3] = { "arg1", "argument2", "arg3" };
char * var4[4] = { "One", "Two", "Three", "Four" };

fn(3, var3);
fn(4, var4);
*/

void fn1(int argc, char argv[][10]) {
    printf("argc : %d\n", argc);
    for (int i = 0; i < argc; ++i) {
        printf("%s\n", argv[i]);
    }
}

/*
 
 char var3[3][10] = { "arg1", "argument2", "arg3" };
 char var4[4][10] = { "arg1", "argument2", "arg3", "arg4" };

 fn(3, var3);
 fn(4, var4);
 */
