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
	HYDRA_NUM_FOLDERS = HYDRA_NUM_FILES = HYDRA_NUM_PINS = 0;

	// Create a folder called home in the root of the filesystem (This is where all the programs are held)
	struct hydra_folders_t *home_folder = hydra_AddFolder("HOME", NULL); // ROOT

	// Create a folder called appvar in the home folder (This is where all the appvar are stored)
	hydra_AddFolder("APPVARS", home_folder); // INSIDE HOME.
}

struct hydra_files_t *SearchFile(char *name, struct hydra_folders_t *location)
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

struct hydra_folders_t *SearchFolder(char *name, struct hydra_folders_t *location)
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

static bool FindFile(char *name)
{
	for (int i = 0; i < hydra_file_system.numfiles; i++)
	{
		if (!strcmp(hydra_file[i].name, name))
			return 1;
	}
	return 0;
}

void hydra_CheckFileSystem(void)
{

	void *search_pos = NULL;
	char *file_name;
	uint8_t type;
	ti_var_t slot;
	int count = 0;
	struct hydra_user_t *curr_user;

	// Check if there user system is setup
	if (HYDRA_CURR_USER_ID < 0)
		return;

	// Assign user in position
	curr_user = &hydra_user[HYDRA_CURR_USER_ID];

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

	// There was a file add on the calculator.
	if (count > curr_user->file_system->numfiles)
	{
		hydra_RescanFileSystem();
		dbg_sprintf(dbgout, "Check: New files added.\n");
		return;
	}

	// There was a file deleted off the calculator.
	if (count < curr_user->file_system->numfiles)
	{
		for (int i = 0; i < hydra_file_system.numfiles; i++)
		{

			if (!(slot = ti_OpenVar(hydra_file[i].name, "r", type)))
			{
				hydra_DeleteFile(&hydra_file[i]);
				ti_Close(slot);
				dbg_sprintf(dbgout, "Check: Deleted File.\n");
			}
		}
	}

	// Get icons for programs
	hydra_GetAsmIcons();
	hydra_GetBasicIcons();

	dbg_sprintf(dbgout, "Check: Finished Checking.\n");
}

void hydra_RescanFileSystem(void)
{
	void *search_pos = NULL;
	char *file_name;
	uint8_t type;

	ti_var_t slot;
	struct hydra_files_t *curr_file;

	while ((file_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL)
	{
		if (type == OS_TYPE_PRGM || type == OS_TYPE_PROT_PRGM || type == OS_TYPE_APPVAR)
		{
			if ((*file_name != '!') && (*file_name != '#'))
			{
				// Check if the file exist in file system
				if (!FindFile(file_name))
				{
					dbg_sprintf(dbgout, "Rescan: Added File.\n");
					if ((slot = ti_OpenVar(file_name, "r", type)))
					{
						dbg_sprintf(dbgout, "Rescan: Opened File!, Name: %s\n", file_name);

						// Check if there is space for a file
						if (hydra_AddFile(file_name, NULL) == NULL)
						{
							ti_Close(slot);
							return;
						}

						dbg_sprintf(dbgout, "Rescan: Space Added File.\n");

						curr_file = &hydra_file[hydra_file_system.numfiles - 1];

						switch (type)
						{
						case (OS_TYPE_PRGM):
							curr_file->type = HYDRA_BASIC_TYPE;
							curr_file->location[HYDRA_CURR_USER_ID] = &hydra_folder[0];
							break;

						case (OS_TYPE_PROT_PRGM):
							curr_file->type = HYDRA_PBASIC_TYPE;
							curr_file->location[HYDRA_CURR_USER_ID] = &hydra_folder[0];
							break;

						case (OS_TYPE_APPVAR):
							curr_file->type = HYDRA_APPVAR_TYPE;
							curr_file->location[HYDRA_CURR_USER_ID] = &hydra_folder[1];
							break;

						default:
							curr_file->type = HYDRA_ERROR_TYPE; // error
							curr_file->location[HYDRA_CURR_USER_ID] = &hydra_folder[0];
							break;
						}

						curr_file->locked = false;

						curr_file->size = ti_GetSize(slot);
						curr_file->archived = ti_IsArchived(slot);

						curr_file->icon = NULL;
						curr_file->description = NULL;

						dbg_sprintf(dbgout, "Rescan: File Done!.\n");

						ti_Close(slot);
					}
				}
			}
		}
	}

	hydra_SortFiles();
	hydra_GetAsmIcons();
	hydra_GetBasicIcons();

	dbg_sprintf(dbgout, "Rescan: Rescan Finished!\n");
}

void hydra_DetectAllFiles(void)
{
	void *search_pos = NULL;
	char *file_name;
	uint8_t type;

	ti_var_t slot;
	struct hydra_files_t *curr_file;

	dbg_sprintf(dbgout, "Oxygen: File System Search Began! \n");

	while ((file_name = ti_DetectAny(&search_pos, NULL, &type)) != NULL)
	{
		if (type == OS_TYPE_PRGM || type == OS_TYPE_PROT_PRGM || type == OS_TYPE_APPVAR)
		{
			if ((*file_name != '!') && (*file_name != '#'))
			{
				if ((slot = ti_OpenVar(file_name, "r", type)))
				{
					dbg_sprintf(dbgout, "Oxygen: Opened File!, Name: %s\n", file_name);

					// Check if there is space for a file
					if (hydra_AddFile(file_name, NULL) == NULL)
					{
						ti_Close(slot);
						return;
					}

					dbg_sprintf(dbgout, "Search: Allocated File.\n");

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

					dbg_sprintf(dbgout, "Search: File Done!.\n");

					ti_Close(slot);
				}
			}
		}
	}

	dbg_sprintf(dbgout, "Search: Finished search.\n");

	hydra_SortFiles();
	hydra_GetAsmIcons();
	hydra_GetBasicIcons();
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

	if (HYDRA_CURR_USER_ID < 0)
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
	if (SearchFolder(name, location) != NULL)
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

static void _ResetFileOwners(struct hydra_files_t *curr_file)
{
	for (int i = 0; i < HYDRA_MAX_USERS; i++)
	{
		curr_file->user_id[i] = -1;
	}
}

static void _ResetFileLocations(struct hydra_files_t *curr_file)
{
	for (int i = 0; i < HYDRA_MAX_USERS; i++)
	{
		curr_file->location[i] = NULL;
	}
}

static void _ResetFilePinned(struct hydra_files_t *curr_file)
{
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

		// Check if the program is owened by current user, don't waste computation on this file
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

	if (editor_name == NULL || file == NULL)
		return -1;

	progname = file->name;

	if ((slot = ti_OpenVar(editor_name, "r", OS_TYPE_PRGM)))
	{ // Search for Editor

		if ((slot = ti_OpenVar(progname, "r", OS_TYPE_PRGM)))
		{ // Search for Program
			ti_Close(slot);
			if (!ti_SetVar(OS_TYPE_STR, OS_VAR_ANS, progname))
			{
				if (!os_RunPrgm(editor_name, NULL, 0, callback))
					return 0;
			}
		}
		else
			return -3;
	}
	else
		return -2;

	return 0;
}

int hydra_RunProgram(struct hydra_files_t *file, os_runprgm_callback_t callback)
{
	char *progname;
	int value;

	if (file == NULL)
		return -1;

	progname = file->name;

	if ((value = os_RunPrgm(progname, NULL, 0, callback)) != 0)
	{
		return value;
	}

	return 0;
}