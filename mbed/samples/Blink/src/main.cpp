/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

DigitalOut myled(LED2);

int main() 
{
    while(1) 
    {
        myled = 1;
        wait(0.1);
        myled = 0;
        wait(0.1);
    }
}
