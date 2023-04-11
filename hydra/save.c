#include "save.h"
#include "files.h"
#include "users.h"

#include <tice.h>
#include <fileioc.h>
#include <debug.h>
#include <string.h>

// Setup the variables for all library
static void hydra_InitSystem(uint8_t type)
{
	switch (type)
	{
	case HYDRA_FILES_TYPE:
		hydra_InitFilesSystem();
		hydra_DetectAllFiles();
		break;

	case HYDRA_USERS_TYPE:
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

		if (ti_Read(&hydra_file_system, sizeof(struct hydra_file_system_t), 1, slot))
		{
			// Allocate space for the user files system
			hydra_folder = malloc(HYDRA_NUM_FOLDERS * sizeof(struct hydra_folders_t));
			hydra_file = malloc(HYDRA_NUM_FILES * sizeof(struct hydra_files_t));

			// Read data from Appvar
			ti_Read(hydra_folder, HYDRA_NUM_FOLDERS * sizeof(struct hydra_folders_t), 1, slot);
			ti_Read(hydra_file, HYDRA_NUM_FILES * sizeof(struct hydra_files_t), 1, slot);

			hydra_CheckFileSystem();

			dbg_sprintf(dbgout, "Oxygen: Loaded file system...\n");
		}
		else
		{
			// User system not found ... what should we do ... init the file system?
			hydra_InitSystem(HYDRA_FILES_TYPE);
			dbg_sprintf(dbgout, "Oxygen: initiated files system...\n");
		}

		if (ti_Read(&hydra_user_system, sizeof(struct hydra_user_system_t), 1, slot))
		{
			hydra_user = malloc(HYDRA_USER_AMOUNT * sizeof(struct hydra_user_t));
			ti_Read(hydra_user, HYDRA_USER_AMOUNT * sizeof(struct hydra_user_t), 1, slot);

			dbg_sprintf(dbgout, "Oxygen: Loaded user system...\n");
		}
		else
		{
			// User system not found ... what should we do ... init the user system?
			hydra_InitSystem(HYDRA_USERS_TYPE);

			dbg_sprintf(dbgout, "Oxygen: initiated user system...\n");
		}

		ti_Close(slot);

		dbg_sprintf(dbgout, "Oxygen: Load Complete...\n");

		return true;
	}
	else
	{
		// Hydra appvar not found
		return false;
	}
}

void hydra_Save(void)
{
	ti_var_t slot;

	// Opens the appvar called hydra
	slot = ti_Open(HYDRA_APPVAR_NAME, "w");

	// Store the current version
	ti_Write(&HYDRA_VERSION_HOLDER, sizeof(int), 1, slot);

	// Stores the files System
	ti_Write(&hydra_file_system, sizeof(struct hydra_file_system_t), 1, slot);
	ti_Write(hydra_folder, hydra_file_system.numfolders * sizeof(struct hydra_folders_t), 1, slot);
	ti_Write(hydra_file, hydra_file_system.numfiles * sizeof(struct hydra_files_t), 1, slot);

	// Stores the user system
	ti_Write(&hydra_user_system, sizeof(struct hydra_user_system_t), 1, slot);
	ti_Write(hydra_user, HYDRA_USER_AMOUNT * sizeof(struct hydra_user_t), 1, slot);

	// Archive the Appvar
	ti_SetArchiveStatus(1, slot);

	// Close the Appvar
	ti_Close(slot);
}