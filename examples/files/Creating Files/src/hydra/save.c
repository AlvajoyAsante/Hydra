#include "save.h"
#include "files.h"
#include "users.h"

#include <tice.h>
#include <fileioc.h>
#include <debug.h>
#include <string.h>

// Setup the variables for all library
static void _initialise(uint8_t type)
{
	switch (type)
	{
	case HYDRA_FILES_TYPE:
		/* Initialise File System for use */
		hydra_InitFilesSystem();

		/* Detect Files on calculator system */
		hydra_Detect(HYDRA_SOFT_SEARCH);
		break;

	case HYDRA_USERS_TYPE:
		/* Initialise User System for use */
		hydra_InitUserSystem();

		/* Initialise User for User, File System */
		hydra_CreateUser("ADMIN", NULL, HYDRA_ADMIN_TYPE);
		break;
	}
}

bool hydra_Load(void)
{
	ti_var_t slot;

	if ((slot = ti_Open(HYDRA_APPVAR_NAME, "r")))
	{
		if (ti_Read(&HYDRA_VERSION_HOLDER, sizeof(int), 1, slot))
		{
			if (HYDRA_VERSION_NUM > HYDRA_VERSION_NUM || HYDRA_VERSION_NUM < HYDRA_VERSION_NUM)
			{
				/* Returns false if its a new or old version */
				return false;
			}
		}

		if (ti_Read(&hydra_user_system, sizeof(struct hydra_user_system_t), 1, slot))
		{
			hydra_user = malloc(HYDRA_USER_AMOUNT * sizeof(struct hydra_user_t));
			ti_Read(hydra_user, HYDRA_USER_AMOUNT * sizeof(struct hydra_user_t), 1, slot);

			dbg_sprintf(dbgout, "Hydra: Loaded user system...\n");
		}
		else
		{
			/* User system not found ... what should we do ... initialise the user system? */
			_initialise(HYDRA_USERS_TYPE);
			dbg_sprintf(dbgout, "Hydra: initiated user system...\n");
		}

		if (ti_Read(&hydra_file_system, sizeof(struct hydra_file_system_t), 1, slot))
		{
			/* Allocate space for the user files system */
			hydra_folder = malloc(HYDRA_NUM_FOLDERS * sizeof(struct hydra_folders_t));
			hydra_file = malloc(HYDRA_NUM_FILES * sizeof(struct hydra_files_t));

			/* Read data from Appvar */
			ti_Read(hydra_folder, HYDRA_NUM_FOLDERS * sizeof(struct hydra_folders_t), 1, slot);
			ti_Read(hydra_file, HYDRA_NUM_FILES * sizeof(struct hydra_files_t), 1, slot);

			/* Check for any changes in file system */
			hydra_Detect(HYDRA_HARD_SEARCH);

			dbg_sprintf(dbgout, "Hydra: Loaded file system...\n"); // debugging information
		}
		else
		{
			/* file system not found ... what should we do ... initialise the file system? */
			_initialise(HYDRA_FILES_TYPE); //
			dbg_sprintf(dbgout, "Hydra: initiated files system...\n");
		}

		/* Finished Loading Information */
		ti_Close(slot);

		dbg_sprintf(dbgout, "Hydra: Load Complete...\n");

		return true;
	}

	/* The Hydra appvar was not found */
	_initialise(HYDRA_USERS_TYPE);
	_initialise(HYDRA_FILES_TYPE);
	return false;
}

void hydra_Save(void)
{
	ti_var_t slot;

	slot = ti_Open(HYDRA_APPVAR_NAME, "w");

	/* Store the current version */
	ti_Write(&HYDRA_VERSION_HOLDER, sizeof(int), 1, slot);

	/* Stores the user system */
	ti_Write(&hydra_user_system, sizeof(struct hydra_user_system_t), 1, slot); // Basic information about the user system

	if (hydra_user_system.amount > 0 && hydra_user != NULL)
	{ // Check if there is any users to store and make sure the pointer isn't NULL
		ti_Write(hydra_user, HYDRA_USER_AMOUNT * sizeof(struct hydra_user_t), 1, slot);
	}

	/* Stores the files System */
	ti_Write(&hydra_file_system, sizeof(struct hydra_file_system_t), 1, slot); // Basic information about the file system

	if (hydra_file_system.numfolders > 0 && hydra_folder != NULL)
	{ // Checks if there is any folders to store and make sure the pointer isn't NULL
		ti_Write(hydra_folder, hydra_file_system.numfolders * sizeof(struct hydra_folders_t), 1, slot);
	}

	if (hydra_file_system.numfiles > 0 && hydra_file != NULL)
	{ // Checks if there is any files to store and make sure the pointer isn't NULL
		ti_Write(hydra_file, hydra_file_system.numfiles * sizeof(struct hydra_files_t), 1, slot);
	}

	ti_SetArchiveStatus(1, slot);

	ti_Close(slot);

	hydra_FreeAll(); // Free all data off heap

	dbg_sprintf(dbgout, "Hydra: Save Complete...\n");
}