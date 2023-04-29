#include<stdio.h>
#include<string.h>
#include "my_utils.h"

bool try_convert_to_id(const char *string, int *result) 
{
    *result = 0;
    if (*string == '-') {return false;}
    if (*string == '\0') {return false;}      //empty string(without digits)

    while (*string) 
    {
        if ((*string < '0') || (*string > '9')) 
        {
            return false;
        } else 
        {
            *result = *result * 10 + (*string - 48);
            string++;
        }
    }     
    return true;
}