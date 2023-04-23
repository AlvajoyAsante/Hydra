#include "files.h"
#include "save.h"
#include "users.h"

#include <tice.h>
#include <fileioc.h>
#include <string.h>
#include <debug.h>

struct hydra_files_t *hydra_file;
struct hydra_folders_t *hydra_folder;
struct hydra_file_system_t hydra_file_system;

/* Setting up the file system */
void hydra_InitFilesSystem(void)
{
	/* Setting File System Information */
	HYDRA_NUM_FOLDERS = HYDRA_NUM_FILES = HYDRA_NUM_PINS = 0;

	/* Setting File System Settings */
	HYDRA_SETTINGS_ICON = HYDRA_SETTINGS_SORT = true;

	// Create a folder called home in the root of the filesystem (This is where all the programs are held)
	struct hydra_folders_t *home_folder = hydra_AddFolder("HOME", NULL); // ROOT

	// Create a folder called appvar in the home folder (This is where all the appvar are stored)
	hydra_AddFolder("APPVARS", home_folder); // INSIDE HOME.
}

struct hydra_files_t *hydra_SearchFile(char *name, struct hydra_folders_t *location)
{
	if (HYDRA_CURR_USER_ID < 0)
		return NULL;

	// look through the file system and check if the name exist if it does then return a true
	for (int i = 0; i < HYDRA_NUM_FILES + 1; i++)
	{
		if (!strcmp(hydra_file[i].name, name) && hydra_file[i].location[HYDRA_CURR_USER_ID] == location)
		{
			// if user_id equals to -1 don't check the owners
			if (hydra_file[i].user_id[HYDRA_CURR_USER_ID] == HYDRA_CURR_USER_ID)
				return &hydra_file[i];
		}
	}

	return NULL;
}

struct hydra_folders_t *hydra_SearchFolder(char *name, struct hydra_folders_t *location)
{
	// if there isn't an user logged return.
	if (HYDRA_CURR_USER_ID < 0)
		return NULL;

	for (int i = 0; i < HYDRA_NUM_FOLDERS; i++)
	{
		if (!strcmp(hydra_folder[i].name, name) && hydra_folder[i].location == location)
		{
			// if user_id equals to -1 don't check the owners
			if (hydra_folder[i].user_id == HYDRA_CURR_USER_ID)
				return &hydra_folder[i];
		}
	}

	return NULL;
}

/**
 * @brief Check for a name in the file system
 *
 * @param name Name of file
 * @return true file was found in the file system
 * @return false file was not found in the file system
 */
static bool _FindFile(char *name)
{
	for (int i = 0; i < hydra_file_system.numfiles; i++)
	{
		if (!strcmp(hydra_file[i].name, name))
			return true;
	}
	return false;
}

static int _FilesInOS(void)
{
	void *search_pos = NULL;
	char *file_name;
	uint8_t type;

	int count = 0;

	// Count all files on the calculator
	while ((file_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL)
	{
		if (type == OS_TYPE_PRGM || type == OS_TYPE_PROT_PRGM || type == OS_TYPE_APPVAR)
		{
			if ((*file_name != '!') && (*file_name != '#'))
			{
				count++;
			}
		}
	}

	return count;
}

bool hydra_Detect(enum hydra_search_type_t search_type)
{
	void *search_pos = NULL;
	char *file_name;
	uint8_t type = '\0';
	ti_var_t slot;
	struct hydra_user_t *curr_user;
	struct hydra_files_t *curr_file;
	int count = 0;

	/* Check if the user is logged in the system */
	if (HYDRA_CURR_USER_ID < 0)
		return false;

	curr_user = &hydra_user[HYDRA_CURR_USER_ID];

	switch (search_type)
	{
	case HYDRA_HARD_SEARCH:
		count = _FilesInOS();

		if (count > curr_user->file_system->numfiles)
		{
			hydra_Detect(HYDRA_RESCAN_SEARCH);
			dbg_sprintf(dbgout, "Check: New files added.\n");
			return true;
		}

		if (count < curr_user->file_system->numfiles)
		{
			for (int i = 0; i < hydra_file_system.numfiles; i++)
			{

				if (!(slot = ti_OpenVar(hydra_file[i].name, "r", hydra_GetFileType(hydra_file[i].type))))
				{
					hydra_DeleteFile(&hydra_file[i]);
					ti_Close(slot);
					dbg_sprintf(dbgout, "Check: Deleted File.\n");
					return true;
				}
			}
		}
		break;

	default:

		dbg_sprintf(dbgout, "Detect: File System Search Began! \n");

		while ((file_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL)
		{
			if (type == OS_TYPE_PRGM || type == OS_TYPE_PROT_PRGM || type == OS_TYPE_APPVAR)
			{
				/* Make sure were not getting files that aren't crazy <_< */
				if ((*file_name != '!') && (*file_name != '#'))
				{
					/* Checks if the user wants to rescan */
					if (search_type == HYDRA_RESCAN_SEARCH)
					{
						/* Checks if the file exist in the file system */
						if (_FindFile(file_name))
						{
							continue;
						}
					}

					if ((slot = ti_OpenVar(file_name, "r", type)))
					{
						dbg_sprintf(dbgout, "Detect: Opened File!, Name: %s\n", file_name);

						/* Checks if there is space for a file */
						if (hydra_AddFile(file_name, NULL) == NULL)
						{
							ti_Close(slot);
							return false;
						}

						dbg_sprintf(dbgout, "Detect: Allocated File.\n");

						curr_file = &hydra_file[hydra_file_system.numfiles - 1];

						switch (type)
						{
						case OS_TYPE_PRGM:
							curr_file->type = HYDRA_BASIC_TYPE;
							curr_file->location[HYDRA_CURR_USER_ID] = &hydra_folder[0];
							break;

						case OS_TYPE_PROT_PRGM:
							curr_file->type = HYDRA_PBASIC_TYPE;
							curr_file->location[HYDRA_CURR_USER_ID] = &hydra_folder[0];
							break;

						case OS_TYPE_APPVAR:
							curr_file->type = HYDRA_APPVAR_TYPE;
							curr_file->location[HYDRA_CURR_USER_ID] = &hydra_folder[1];
							break;

						default:
							curr_file->type = HYDRA_ERROR_TYPE;
							curr_file->location[HYDRA_CURR_USER_ID] = &hydra_folder[0];
							break;
						}

						curr_file->locked = false;

						curr_file->size = ti_GetSize(slot);
						curr_file->archived = ti_IsArchived(slot);

						curr_file->icon = NULL;
						curr_file->description = NULL;

						/* Check if the file is hidden */
						if (type == OS_TYPE_PRGM || type == OS_TYPE_PROT_PRGM)
						{
							ti_Rewind(slot);
							if (ti_GetC(slot) == ' ')
							{
								curr_file->hidden = true;
							}
							else
							{
								curr_file->hidden = false;
							}
						}

						dbg_sprintf(dbgout, "Detect: File Done!.\n");

						ti_Close(slot);
					}
				}
			}
		}

		dbg_sprintf(dbgout, "Detect: Finished search.\n");
		break;
	}

	/* Check if the hydra settings allows sorting */
	if (HYDRA_SETTINGS_SORT)
	{
		hydra_SortFiles();
		dbg_sprintf(dbgout, "Detect: Sorted files.\n");
	}

	/* Check if the hydra settings allow icon detecting  */
	if (HYDRA_SETTINGS_ICON)
	{
		hydra_GetAsmIcons();
		hydra_GetBasicIcons();
		dbg_sprintf(dbgout, "Detect: Set Icons.\n");
	}

	return true;
}

/* Hiding Files */
int hydra_HideFile(struct hydra_files_t *file)
{
	ti_var_t slot;
	uint8_t type;

	/* Make sure we make space for error checking */
	if (file == NULL || HYDRA_CURR_USER_ID == HYDRA_USER_NOT_SET)
		return -1;

	/* Check if it's a basic or protected program */
	type = hydra_GetFileType(file->type);

	if (type != OS_TYPE_PRGM || type != OS_TYPE_APPVAR)
		return -2;

	/* Check if the file is already hidden */
	if (file->hidden == false)
	{
		/* Hide the file from TI-OS and mark it as hidden */

		if (!(slot = ti_OpenVar(file->name, "r+", type)))
		{
			/* The file was found lets start editing */

			char temp[10] = {0};
			ti_GetName(temp, slot);
			temp[0] ^= 64;			

			/* Go to the beginning of the program */
			ti_Rewind(slot);

			/* Check for colon */
			if (ti_GetC(slot) == ':')
			{
				/* Replace colon with space */
				ti_PutC(' ', slot);
			}
			else
			{
				return -4;
			}

			/* Done editing */
			ti_Close(slot);

			ti_Rename(file->name, temp);
		}
		else
		{
			/* File was not found */
			return -3;
		}

		/* Set Hidden to true */
		file->hidden = true;
	}
	else
	{
		/* Un-Hide the file from TI-OS and un-mark it as hidden */
		if (!(slot = ti_OpenVar(file->name, "r+", type)))
		{
			/* The file was found lets start editing */
			char temp[10] = {0};
			ti_GetName(temp, slot);
			temp[0] ^= 64;


			/* Go to the beginning of the program */
			ti_Rewind(slot);

			/* Check for space */
			if (ti_GetC(slot) == ' ')
			{
				/* Replace space with colon */
				ti_PutC(':', slot);
			}
			else
			{
				return -4;
			}

			/* Done editing */
			ti_Close(slot);

			ti_Rename(file->name, temp);
		}
		else
		{
			/* File was not found */
			return -3;
		}

		file->hidden = false;
	}

	return file->hidden;
}

/* Pinning folders and files */
bool hydra_PinFolder(struct hydra_folders_t *folder)
{
	if (folder == NULL || HYDRA_CURR_USER_ID == HYDRA_USER_NOT_SET)
		return false;

	folder->pinned = true;

	return folder->pinned;
}

bool hydra_PinFile(struct hydra_files_t *file)
{
	if (file == NULL || HYDRA_CURR_USER_ID == HYDRA_USER_NOT_SET)
		return false;

	file->pinned[HYDRA_CURR_USER_ID] = true;

	return file->pinned[HYDRA_CURR_USER_ID];
}

bool hydra_isFolderPinned(struct hydra_folders_t *folder)
{
	if (folder == NULL || HYDRA_CURR_USER_ID == HYDRA_USER_NOT_SET)
		return false;

	return folder->pinned;
}

bool hydra_isFilePinned(struct hydra_files_t *file)
{
	if (file == NULL || HYDRA_CURR_USER_ID == HYDRA_USER_NOT_SET)
		return false;

	return file->pinned[HYDRA_CURR_USER_ID];
}

/* Deleting folders and files */
bool hydra_DeleteFolder(struct hydra_folders_t *folder)
{
	/* Reallocate the folder structure */
	if ((HYDRA_NUM_FOLDERS > 0) && (folder->user_id == HYDRA_CURR_USER_ID))
	{
		folder = &hydra_folder[HYDRA_NUM_FOLDERS - 1];
		hydra_folder = realloc(hydra_folder, --HYDRA_NUM_FOLDERS * sizeof(struct hydra_folders_t));
	}
	else if (folder->user_id != HYDRA_CURR_USER_ID)
	{
		/* If the folder is not set to the current logged in user then return (cannot delete) */
		return false;
	}

	/* Delete all files associated with deleted folder */
	for (int i = 0; i < HYDRA_NUM_FILES; i++)
	{
		// Checks all files in the file system.
		struct hydra_files_t *curr_file = &hydra_file[i];

		// Check if current user owns the file if not continue
		if (curr_file->user_id[HYDRA_CURR_USER_ID] != HYDRA_CURR_USER_ID)
			continue;

		// Check if the location is in the index
		if (curr_file->location[HYDRA_CURR_USER_ID] == folder)
		{
			hydra_DeleteFile(curr_file);
		}
	}

	/* Delete all folders associated with deleted folder*/
	for (int i = 0; i < HYDRA_NUM_FOLDERS; i++)
	{
		struct hydra_folders_t *curr_folder = &hydra_folder[i];

		if (curr_folder->user_id != HYDRA_CURR_USER_ID)
			continue;

		if (curr_folder->location == folder)
		{
			hydra_DeleteFolder(curr_folder);
		}
	}

	return true;
}

/**
 * @brief Checks if a file is contained by any other user
 *
 * @param file pointer to the file
 * @param user_id index of where the
 * @return true file is contained by any other user
 * @return false file is not contained by any other user
 */
static bool _isContainedByOtherUsers(struct hydra_files_t *file, uint8_t user_id)
{

	// Check if the user id is in range
	if (!(user_id >= 0 && user_id <= HYDRA_MAX_USERS))
		return false;

	// Check if file is contained by current user
	if (file->user_id[user_id] != user_id)
		return false;

	// Check if there is a user id in another section
	for (int i = 0; i < HYDRA_MAX_USERS; i++)
	{
		if (i != user_id)
		{
			if (file->user_id[i] == i)
			{
				return true;
			}
		}
	}

	return false;
}

bool hydra_DeleteFile(struct hydra_files_t *file)
{
	struct hydra_user_t *curr_user;

	if (HYDRA_CURR_USER_ID < 0 || file == NULL)
		return false;

	// Check if there is files in the file system and check if the file is owned by the user
	if (hydra_file_system.numfiles > 0 && file->user_id[HYDRA_CURR_USER_ID] == HYDRA_CURR_USER_ID)
	{
		if (!_isContainedByOtherUsers(file, HYDRA_CURR_USER_ID))
		{
			file = &hydra_file[HYDRA_NUM_FILES - 1];
			hydra_file = realloc(hydra_file, --hydra_file_system.numfiles * sizeof(struct hydra_files_t));

			// Remove files amount from each users file system
			for (int i = 0; i < HYDRA_USER_AMOUNT; i++)
			{
				curr_user = &hydra_user[i];
				curr_user->file_system->numfiles--;
			}

			return true;
		}
		else
		{
			// Remove user relation to file.
			file->user_id[HYDRA_CURR_USER_ID] = -1;

			curr_user = &hydra_user[HYDRA_CURR_USER_ID];
			curr_user->file_system->numfiles--;

			return true;
		}
	}

	return false;
}

bool hydra_ForceDeleteFile(struct hydra_files_t *file)
{
	struct hydra_user_t *curr_user;
	uint8_t type = hydra_GetFileType(file->type);

	if (type == 255)
		return false;

	if (ti_DeleteVar(file->name, type))
	{
		file = &hydra_file[HYDRA_NUM_FILES - 1];
		hydra_file = realloc(hydra_file, --hydra_file_system.numfiles * sizeof(struct hydra_files_t));

		// Remove files amount from each users file system
		for (int i = 0; i < HYDRA_USER_AMOUNT; i++)
		{
			curr_user = &hydra_user[i];
			curr_user->file_system->numfiles--;
		}

		return true;
	}

	return false;
}

/* Creating folders and files */
struct hydra_folders_t *hydra_AddFolder(char *name, struct hydra_folders_t *location)
{
	struct hydra_user_t *curr_user;

	// Check if there is a user currently logged in
	if (HYDRA_CURR_USER_ID < 0)
		return NULL;

	// Check if there is a folder in that location with the same name
	if (hydra_SearchFolder(name, location) != NULL)
		return NULL;

	if ((hydra_folder = realloc(hydra_folder, ++HYDRA_NUM_FOLDERS * sizeof(struct hydra_folders_t))) != NULL)
	{

		// Get position of folder
		struct hydra_folders_t *curr_folder = &hydra_folder[HYDRA_NUM_FOLDERS - 1];

		// Set the folders name
		strncpy(curr_folder->name, name, 9);

		// Assign the folder to a user
		curr_folder->user_id = HYDRA_CURR_USER_ID;

		// Assign the folder to a position (Position can be NULL for no position)
		curr_folder->location = location;

		curr_user = &hydra_user[HYDRA_CURR_USER_ID];

		// Increase the amount of folders in the file system.
		curr_user->file_system->numfolders++;

		dbg_sprintf(dbgout, "Added Folder: Reallocated folder, numfolders = %d\n", HYDRA_NUM_FOLDERS);

		return curr_folder;
	}

	return NULL;
}

/**
 * @brief Reset any owners contained within the file
 *
 * @param curr_file pointer to the file
 */
static void _ResetFileOwners(struct hydra_files_t *curr_file)
{
	if (curr_file == NULL)
	{
		return;
	}

	for (int i = 0; i < HYDRA_MAX_USERS; i++)
	{
		curr_file->user_id[i] = -1;
	}
}

static void _ResetFileLocations(struct hydra_files_t *curr_file)
{
	if (curr_file == NULL)
	{
		return;
	}

	for (int i = 0; i < HYDRA_MAX_USERS; i++)
	{
		curr_file->location[i] = NULL;
	}
}

static void _ResetFilePinned(struct hydra_files_t *curr_file)
{
	if (curr_file == NULL)
	{
		return;
	}

	for (int i = 0; i < HYDRA_MAX_USERS; i++)
	{
		curr_file->pinned[i] = false;
	}
}

struct hydra_files_t *hydra_AddFile(char *name, struct hydra_folders_t *location)
{
	struct hydra_user_t *curr_user;

	// Check if there is a user currently logged in
	if (HYDRA_CURR_USER_ID < 0)
		return NULL;

	// Check if there is any space to create a file.
	if ((hydra_file = realloc(hydra_file, ++HYDRA_NUM_FILES * sizeof(struct hydra_files_t))) != NULL)
	{
		struct hydra_files_t *curr_file = &hydra_file[HYDRA_NUM_FILES - 1];

		// Reset the owners of files
		_ResetFileOwners(curr_file);

		// Reset File location for each owner
		_ResetFileLocations(curr_file);

		// Reset File owner pinned
		_ResetFilePinned(curr_file);

		// Assign the current user_id to the file and prepare file for icon and location data.
		curr_file->user_id[HYDRA_CURR_USER_ID] = HYDRA_CURR_USER_ID;
		curr_file->icon = NULL;

		curr_user = &hydra_user[HYDRA_CURR_USER_ID];
		curr_user->file_system->numfiles++;

		// Give the file a name and set the location
		strcpy(curr_file->name, name);
		curr_file->location[HYDRA_CURR_USER_ID] = location;

		dbg_sprintf(dbgout, "Added File: Reallocated file, numfiles = %d\n", HYDRA_NUM_FILES);

		return curr_file;
	}

	return NULL;
}

/* Detecting files icons */
void hydra_GetAsmIcons(void)
{
	uint8_t data_pos;
	ti_var_t slot;

	for (int i = 0; i < hydra_file_system.numfiles; i++)
	{
		struct hydra_files_t *curr_file = &hydra_file[i];

		// Check if the program is owned by current user, don't waste computation on this file
		if (curr_file->user_id[HYDRA_CURR_USER_ID] != HYDRA_CURR_USER_ID)
			continue;

		if ((slot = ti_OpenVar(curr_file->name, "r", hydra_GetFileType(curr_file->type))))
		{
			if (hydra_GetFileType(curr_file->type) == OS_TYPE_PROT_PRGM)
			{
				for (int r = 0; r < 20; r++)
				{
					ti_Read(&data_pos, 1, 1, slot);

					if (r == 0)
					{
						if ((data_pos != 0xEF) && (data_pos + 1) != 0x7B)
						{
							curr_file->type = HYDRA_PBASIC_TYPE;
							curr_file->locked = true;
							break;
						}
					}

					if (r == 2)
					{
						switch (data_pos)
						{
						case 0x00:
							curr_file->type = HYDRA_C_TYPE;
							break;

						case 0x7F:
							curr_file->type = HYRA_ICE_TYPE;
							break;

						default:
							curr_file->type = HYDRA_ASM_TYPE;
							break;
						}

						curr_file->locked = true;
					}

					if (data_pos == 16)
					{
						curr_file->icon = (gfx_sprite_t *)(ti_GetDataPtr(slot) - 2);
						curr_file->description = (char *)(ti_GetDataPtr(slot) + 256);
					}

					data_pos++;
				}
			}
			ti_Close(slot);
		}
	}
}

void hydra_GetBasicIcons(void)
{
	ti_var_t slot;

	// Search Headers
	uint8_t search_dcs[6] = {OS_TOK_COLON, OS_TOK_D, OS_TOK_C, OS_TOK_S, OS_TOK_NEWLINE, OS_TOK_DOUBLE_QUOTE};
	uint8_t search_ice[2] = {OS_TOK_COLON, OS_TOK_IMAGINARY};

	// Palette used for dcs icons
	uint8_t palette[16] = {223, 24, 224, 0, 248, 6, 228, 96, 16, 29, 231, 255, 222, 189, 148, 74};
	uint8_t data_pos = '\0';
	char temp[2] = " ";

	// Searches through all of Hydras ile system
	for (int i = 0; i < hydra_file_system.numfiles; i++)
	{

		struct hydra_files_t *curr_file = &hydra_file[i];

		// Check if the program is owned by current user, don't waste computation on this file
		if (curr_file->user_id[HYDRA_CURR_USER_ID] != HYDRA_CURR_USER_ID)
			continue;
		;

		// Check if the file is basic or protected basic
		if (curr_file->type == HYDRA_PBASIC_TYPE || curr_file->type == HYDRA_BASIC_TYPE)
		{

			// open program for reading
			if ((slot = ti_OpenVar(curr_file->name, "r", hydra_GetFileType(curr_file->type))))
			{
				// Checks if the data in the program has the ICE header
				if (!memcmp(search_ice, ti_GetDataPtr(slot), 2))
				{
					curr_file->type = HYDRA_ICES_TYPE;
					continue;
				}

				// Checks if the data in the program has the "dsc" header
				if (!memcmp(search_dcs, ti_GetDataPtr(slot), 6))
				{

					curr_file->icon = gfx_MallocSprite(16, 16);
					ti_Seek(6, 0, slot);

					for (int r = 0; r < 256; r++)
					{
						ti_Read(temp, 1, 1, slot);
						ti_Read((void *)(data_pos + r), 1, 1, slot);

						if (data_pos != OS_TOK_DOUBLE_QUOTE)
						{
							curr_file->icon->data[r] = palette[strtol(temp, NULL, 16)];
						}
						else
						{
							free(curr_file->icon);
							curr_file->icon = NULL;
							break;
						}

						data_pos++;
					}
				}

				ti_Close(slot);
			}
		}

		if (curr_file->type == HYDRA_BASIC_TYPE)
			curr_file->locked = false;
	}
}

/* Sorting all files */
static int CompareFolderNames(const void *a, const void *b)
{
	return strcmp(((struct hydra_folders_t *)a)->name, ((struct hydra_folders_t *)b)->name);
}

void hydra_SortFolders(void)
{
	qsort(&(hydra_folder[2]), hydra_file_system.numfolders - 2, sizeof(struct hydra_folders_t), CompareFolderNames);
}

static int CompareFileNames(const void *a, const void *b)
{
	return strcmp(((struct hydra_files_t *)a)->name, ((struct hydra_files_t *)b)->name);
}

void hydra_SortFiles(void)
{
	qsort(hydra_file, hydra_file_system.numfiles, sizeof(struct hydra_files_t), CompareFileNames);
}

/* Other */
uint8_t hydra_GetFileType(enum hydra_file_type_t type)
{
	switch (type)
	{
	case HYDRA_BASIC_TYPE:
		return OS_TYPE_PRGM;

	case HYDRA_PBASIC_TYPE:
	case HYDRA_ASM_TYPE:
	case HYDRA_C_TYPE:
	case HYRA_ICE_TYPE:
	case HYDRA_ICES_TYPE:
		return OS_TYPE_PROT_PRGM;

	case HYDRA_APPVAR_TYPE:
		return OS_TYPE_APPVAR;

	default:
		return 255;
	}
}

int hydra_EditProgram(struct hydra_files_t *file, const char *editor_name, os_runprgm_callback_t callback)
{
	ti_var_t slot;
	char *progname;

	/* Check for user */
	if (HYDRA_CURR_USER_ID < 0)
	{
		return -2;
	}

	/* Check if the user is a guest */
	if (hydra_user[HYDRA_CURR_USER_ID].permission_type == HYDRA_GUEST_TYPE)
	{
		return -2;
	}

	/* Checks if there is a valid editor name and file */
	if (editor_name == NULL || file == NULL)
		return -1;

	progname = file->name;

	/* Checks for the editor to make sure it exists */
	if ((slot = ti_OpenVar(editor_name, "r", OS_TYPE_PRGM))) // What if the editor is not basic???
	{
		ti_Close(slot);

		/* Checks for the file to make sure it exists */
		if ((slot = ti_OpenVar(progname, "r", OS_TYPE_PRGM)))
		{
			ti_Close(slot);

			/* Sets the Ans to program developer wants to edit */
			if (!ti_SetVar(OS_TYPE_STR, OS_VAR_ANS, progname))
			{
				/* Runs the program */
				if (os_RunPrgm(editor_name, NULL, 0, callback))
				{
					return -5; // Program ended up running
				}

				return -4; // Program wasn't able to run
			}
		}
		else
			return -3; // Program develop wanted to edit was not found
	}
	else
		return -3; // Editor Program was not found

	return 0;
}

int hydra_RunProgram(struct hydra_files_t *file, os_runprgm_callback_t callback)
{
	char *progname;
	int value;

	/* Check if the file point doesn't exist */
	if (file == NULL)
		return -1;

	progname = file->name;

	/* Run program and check for error */
	if ((value = os_RunPrgm(progname, NULL, 0, callback)) != 0)
	{
		return value; // Return any error code produced by os_RunPrgm
	}

	return 0; // Return zero if program was ran
}