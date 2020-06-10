//
//  hoeStructWorks.c
//  bullinServer
//
//  Created by Ernie on 2020-06-09.
//  Copyright © 2020 Ernie. All rights reserved.
//

#include <stdio.h>




// Pointers can be used to refer to a struct by its address. This is useful for passing structs to a function. The pointer can be dereferenced using the * operator. The -> operator dereferences the pointer to struct (left operand) and then accesses the value of a member of the struct (right operand).
struct point {
   int x;
   int y;
};
struct point my_point = { 3, 7 };
struct point *p = &my_point;  // p is a pointer to my_point

void sFunc() {
    (*p).x = 8;                   /* set the first member of the struct */
    p->x = 8;
    // scanf("%d", &p -> x);
}

                 // set the first member of the struct
//p->x = 8;



/* c++   Pointers to Structure
 #include <iostream>
 using namespace std;

 struct Distance
 {
     int feet;
     float inch;
 };

 int main()
 {
     Distance *ptr, d;

     ptr = &d;
     
     cout << "Enter feet: ";
     cin >> (*ptr).feet;
     cout << "Enter inch: ";
     cin >> (*ptr).inch;
  
     cout << "Displaying information." << endl;
     cout << "Distance = " << (*ptr).feet << " feet " << (*ptr).inch << " inches";

     return 0;
 }
 */
