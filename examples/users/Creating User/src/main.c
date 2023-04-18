/*
 *--------------------------------------
 * Program Name: Demo
 * Author: Alvajoy 'Alvajoy123' Asante
 * Description: Hydra Demo
 *--------------------------------------
 */

#include <tice.h>
#include <graphx.h>

#include "hydra/hydra.h"

int main(void)
{
    /*
     * Always begin usage of file system with this code,
     * What this code does apply all the files in TI-OS into Hydra
     * filesystem.
     */
    hydra_Begin();

    // Put all you code for messing with the file system in here

    struct hydra_user_t *my_user = hydra_CreateUser("WILL", NULL, HYDRA_USER_TYPE);

    /*
     * Check if the user was created there can be countless amount of 
     * reason why a user was not created.
     */
    if (my_user != NULL)
    {
        /* user was created */
    }else{
        
    }

    /*
     * Always end usage of file system with this code
     * This piece of code basically saves the files
     * and user System and free the data from heap.
     */
    hydra_End();

    return 0;
}
