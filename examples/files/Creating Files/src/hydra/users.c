#include "users.h"

#include <tice.h>
#include <graphx.h>
#include <fileioc.h>
#include <string.h>

struct hydra_user_t *hydra_user;
struct hydra_user_system_t hydra_user_system;

void hydra_InitUserSystem(void)
{
	/* Setting User System Information */
	HYDRA_USER_AMOUNT = 0;
	HYDRA_CURR_USER_ID = -1;

	/* Setting User System Settings */
	HYDRA_SETTINGS_CAN_RUN = HYDRA_SETTINGS_CAN_PIN = HYDRA_SETTINGS_CAN_EDIT = true;
}

struct hydra_user_t *hydra_CreateUser(char *name, char *password, enum hydra_user_permission_t permission_type)
{
	struct hydra_user_t *curr_user;

	if (HYDRA_USER_AMOUNT > 8)
		return NULL;

	if ((hydra_user = realloc(hydra_user, ++hydra_user_system.amount * sizeof(struct hydra_user_t))) != NULL)
	{
		curr_user = &hydra_user[hydra_user_system.amount - 1];

		/* Setting the user id */
		curr_user->user_id = hydra_user_system.amount - 1;

		/* User Icon */
		curr_user->icon = NULL;

		/* Sets Username */
		if (name != NULL)
		{
			hydra_SetUserName(curr_user, name);
		}
		else
		{
			curr_user->name[0] = '\0';
		}

		/* Sets Password */
		if (password != NULL)
		{
			hydra_SetUserPassword(curr_user, password);
		}
		else
		{
			curr_user->password[0] = '\0';
		}

		/* initialise type and login data */
		curr_user->permission_type = permission_type;
		curr_user->login_time[0] = -1;	  // -1 means there is currently no login time.
		curr_user->description[0] = '\0'; // No description

		/* File system information */
		curr_user->file_system = malloc(sizeof(struct hydra_file_system_t)); // Allocate space fore data
		curr_user->file_system->numfiles = curr_user->file_system->numfolders = curr_user->file_system->numpins = 0;

		return curr_user;
	}

	return NULL;
}

struct hydra_user_t *hydra_SearchUser(char *name, enum hydra_user_permission_t permission_type)
{
	struct hydra_user_t *curr_user;

	for (int i = 0; i < HYDRA_USER_AMOUNT; i++)
	{
		curr_user = &hydra_user[i];
		if (!strcmp(curr_user->name, name) && curr_user->permission_type == permission_type)
		{
			return curr_user;
		}
	}

	return NULL;
}

bool hydra_SetUserDescription(struct hydra_user_t *user, char *description)
{
	if (user == NULL)
		return false;

	strncpy(user->description, description, sizeof(user->description));

	return true;
}

bool hydra_SetUserPassword(struct hydra_user_t *user, char *password)
{

	if (user == NULL)
		return false;

	strncpy(user->password, password, sizeof(user->password));

	return true;
}

bool hydra_SetUserName(struct hydra_user_t *user, char *name)
{
	if (user == NULL)
		return false;

	strncpy(user->name, name, sizeof(user->name));

	return true;
}

bool hydra_UserSetIcon(struct hydra_user_t *user, gfx_sprite_t *icon)
{
	if (user == NULL)
		return false;

	user->icon = icon;

	return true;
}

bool hydra_SetUserID(struct hydra_user_t *user)
{
	//  if there isn't a user in given user
	if (user == NULL)
	{
		HYDRA_CURR_USER_ID = -1;
		return false;
	}

	// if the user_id is lees than the amount of the total users and the total amount of users does not equal to zero/
	if (HYDRA_USER_AMOUNT < user->user_id && HYDRA_USER_AMOUNT != 0)
	{
		// Set the global user id to there set user id
		HYDRA_CURR_USER_ID = user->user_id;
		return true;
	}

	HYDRA_CURR_USER_ID = -1;
	return false;
}

bool hydra_DeleteUser(struct hydra_user_t *user)
{
	if (user == NULL)
	{
		return false;
	}

	if (HYDRA_USER_AMOUNT > 1)
	{
		user = &hydra_user[HYDRA_USER_AMOUNT - 1];
		hydra_user = realloc(hydra_user, --HYDRA_USER_AMOUNT * sizeof(struct hydra_user_t));
		return true;
	}

	return false;
}

static void hydra_UserSetLoginTime(struct hydra_user_t *user)
{
	/**
	 * information about the users login date and time.
	 * Date: login_time[0] ... month / login_time[1] ... day / login_time[2] ... year
	 * Time: login_time[3] ... hour / login_time[4] ... minute
	 */

	uint8_t day, month;
	uint8_t min, hur;
	uint16_t year;

	boot_GetTime(NULL, &min, &hur);
	boot_GetDate(&day, &month, &year);

	user->login_time[0] = (int)month;
	user->login_time[1] = (int)day;
	user->login_time[2] = (int)year;
	user->login_time[3] = (int)hur;
	user->login_time[4] = (int)min;
}

bool hydra_UserCheckPassword(struct hydra_user_t *user, char *input)
{
	if (user == NULL)
		return false;

	if (!strcmp(user->password, input))
	{
		hydra_UserSetLoginTime(user);
		hydra_SetUserID(user);
		return true;
	}

	return false;
}