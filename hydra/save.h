/**
 * @file save.h
 * @author Alvajoy 'Alvajoy123' Asante
 * @brief Saving / Loading system for files and users.
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 *
 */

#ifndef HYDRA_SAVE_H
#define HYDRA_SAVE_H

#define HYDRA_APPVAR_NAME "HYDRA"
#define HYDRA_VERSION_NUM 0

#define hydra_Begin() \
	hydra_Load();

#define hydra_End() \
	hydra_Save();

#define hydra_FreeFiles() \
	free(hydra_folder);   \
	free(hydra_file);

#define hydra_FreeUsers() \
	free(hydra_user);

#include <tice.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	enum system_type_t
	{
		HYDRA_FILES_TYPE,
		HYDRA_USERS_TYPE,
	};

	/**
	 * @brief Global int used to compare version numbers.
	 */
	static int HYDRA_VERSION_HOLDER = HYDRA_VERSION_NUM;

	/**
	 * @brief Loads all systems of hydra from appvar
	 * 
	 * @return true system was loaded
	 * @return false system was not loaded
	 */
	bool hydra_LoadAll(void);

	/**
	 * @brief Save all systems to hydra appvar.
	 */
	void hydra_SaveAll(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __hydra_SAVE_H__ */