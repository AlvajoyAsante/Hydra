/**
 * @file save.h
 * @author Alvajoy 'Alvajoy123' Asante
 * @brief Saving / Loading system for files and users.
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