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
    /* By  Setting both sorting and scanning you can speed up the searching  */
    HYDRA_SETTINGS_SORT = false;
    HYDRA_SETTINGS_ICON = false;

    /*
     * Always begin usage of file system with this code,
     * What this code does apply all the files in TI-OS into Hydra
     * filesystem.
     */
    hydra_Begin();

    // Put all you code for messing with the file system in here

    /*
     * Always end usage of file system with this code
     * This piece of code basically saves the files
     * and user System and free the data from heap.
     */
    hydra_End();

    return 0;
}
