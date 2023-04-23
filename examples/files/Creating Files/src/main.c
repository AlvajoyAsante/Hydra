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
    /* Clear the homescreen */
    os_ClrHome();

    /*
     * Always begin usage of file system with this code,
     * What this code does apply all the files in TI-OS into Hydra
     * filesystem.
     */
    hydra_Begin();

    // Put all you code for messing with the file system in here

    /* This piece of code create a temp file in the hydra file system (File will be deleted on `hydra_Begin` because the file isn't in TI-OS) */
    struct hydra_files_t *TEST_FILE = hydra_CreateFile("TEST", HYDRA_HOME_FOLDER);

    /* We want to check if the file named demo is in the hydra file system ... if the variable is NULL stop */
    if (TEST_FILE != NULL)
    {
        /* The file was found! ... the file was created*/
        os_SetCursorPos(0, 0);
        os_PutStrFull("FILE CREATED!");
    }
    else
    {
        /* The file was not found! .. the file was not created this can because of many issues */
        os_SetCursorPos(0, 0);
        os_PutStrFull("FILE NOT CREATED!");
    }

    /*
     * Always end usage of file system with this code
     * This piece of code basically saves the files
     * and user System and free the data from heap.
     */
    hydra_End();

    return 0;
}
