/**
 * @file users.h
 * @author Alvajoy 'Alvajoy123' Asante
 * @brief User system
 * @version 0.1
 *
 * @copyright Copyright (c) 2023
 * 
 * BSD 3-Clause License
 * Copyright (c) 2023, Alvajoy 'Alvajoy123' Asante.
 * All rights reserved.
 *
 * 	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions are met:
 *
 *	Redistributions of source code must retain the above copyright notice, this
 *	list of conditions and the following disclaimer.
 *
 * 	Redistributions in binary form must reproduce the above copyright notice,
 *	this list of conditions and the following disclaimer in the documentation
 *	and/or other materials provided with the distribution.
 *
 * 	Neither the name of the copyright holder nor the names of its
 * 	contributors may be used to endorse or promote products derived from
 * 	this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HYDRA_USERS_H
#define HYDRA_USERS_H

#define HYDRA_CURR_USER_ID hydra_user_system.curr_id
#define HYDRA_USER_AMOUNT hydra_user_system.amount
#define HYDRA_MAX_USERS 8

#include <tice.h>
#include <graphx.h>

#include "files.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	/**
	 * @brief Enum of user types and permissions
	 *
	 */
	enum hydra_user_permission_t
	{
		HYDRA_ADMIN_TYPE,
		HYDRA_USER_TYPE,
		HYDRA_GUEST_TYPE,
	};

	/**
	 * @brief Users
	 */
	struct hydra_user_t
	{
		uint8_t user_id;
		gfx_sprite_t *icon;

		char name[10];
		char password[16];

		/* user permission (limitation) */
		uint8_t type;

		/* User login in management */
		int login_time[5];
		char description[16];

		struct hydra_file_system_t *file_system;
	};
	extern struct hydra_user_t *hydra_user;

	/**
	 * @brief stores amount of users
	 */
	struct hydra_user_system_t
	{
		uint8_t amount; // amount of users
		int curr_id;	// current logged user
	};
	extern struct hydra_user_system_t hydra_user_system;

	/**
	 * @brief Create a user
	 *
	 * @param name Name of User
	 * @param password Password for User
	 * @param type "hydra_user_permission_t" constant
	 * @return struct hydra_user_t* pointer to the position of the user
	 * @return NULL	User could not be created many various reasons
	 */
	struct hydra_user_t *hydra_CreateUser(char *name, char *password, uint8_t type);

	/**
	 * @brief Search for user based on name and type
	 *
	 * @param name Name of User you searching for
	 * @param type "hydra_user_permission_t" constant
	 * @return struct hydra_user_t* pointer to the user file
	 * @return NULL	The User wasn't found
	 */
	struct hydra_user_t *hydra_SearchUser(char *name, uint8_t type);

	/**
	 * @brief Sets the description of a given user
	 *
	 * @param user pointer to the user
	 * @param description description of user
	 * @return true description was set
	 * @return false description wasn't set because user was NULL
	 */
	bool hydra_SetUserDescription(struct hydra_user_t *user, char *description);

	/**
	 * @brief Sets the password of a user
	 *
	 * @param user pointer to the user
	 * @param password password
	 */
	bool hydra_SetUserPassword(struct hydra_user_t *user, char *password);

	/**
	 * @brief Sets the username of a user
	 *
	 * @param user pointer to the user
	 * @param name username
	 */
	bool hydra_SetUserName(struct hydra_user_t *user, char *name);

	/**
	 * @brief Sets the icon of a user
	 *
	 * @param user pointer to the user
	 * @param icon toolchain sprite
	 */
	bool hydra_UserSetIcon(struct hydra_user_t *user, gfx_sprite_t *icon);

	/**
	 * @brief Sets the overall system user id to given user
	 *
	 * @param user pointer to the user
	 */
	bool hydra_SetUserID(struct hydra_user_t *user);

	/**
	 * @brief Deletes given user
	 *
	 * @param user pointer to the user
	 */
	bool hydra_DeleteUser(struct hydra_user_t *user);

	/**
	 * @brief This function is used to check if input is equal to the password in user index.
	 * if the function returns true then input equals to index password and if false etc.
	 *
	 * @param user pointer to the user
	 * @param input password you want to test
	 * @return true password is correct sets the system user id
	 * @return false password is incorrect / couldn't check because user was NULL pointer
	 */
	bool hydra_CheckUserPassword(struct hydra_user_t *user, char *input);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __hydra_USERS_H__ */