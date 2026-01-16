/* Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 * Author: John Hu
 * File Description: Interface of obtaining license information
 * Creation Date: 2017-12-21
 * Modification History:
 */

#ifndef PINPOINT_LICENSE_H_
#define PINPOINT_LICENSE_H_
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/// The possible value of parameter size_t type:

/// Get the value of maximum buffer size, in this case buffer is an address
/// pointed to a uint16_t variable and param size is sizeof(uint16_t).
#define PARAM_TEST_MAX_BUFFER_SIZE 0x00

/// Get the value of expiry date of license file to buffer,
/// in this case buffer is a char array, you should set param size larger than the date string size.
/// The date format is YY-mm-dd
#define PARAM_LICENSE_EXPIRY_DATE 0x01

/// Get the maximum number of lines allowed for analysis to buffer,
/// in this case the buffer points to a uint32_t variable , param size is sizeof(uint32_t).
#define PARAM_LICENSE_MAX_CODE_SIZE 0x02

/// Get suite from license. "suite" can be "enterprise", "community", "academic",
/// in this case buffer is a char array, you should set param size larger than string size of suite.
#define PARAM_LICENSE_SUITE 0x03

/// Get uid from license. "uid" is the username of a client,
/// in this case buffer is a char array, you should set param size larger than string size of uid.
#define PARAM_LICENSE_UID 0x04

/// Get organization name from license, such as "Google"
/// in this case buffer is a char array, you should set param size larger than string size of organization.
#define PARAM_LICENSE_ORGANIZATION 0x05

/// Get agency name from license
/// in this case buffer is a char array, you should set param size larger than string size of agency.
#define PARAM_LICENSE_AGENCY 0x06

/// Get region from license, e.g. China, the region indicates where Pinpoint is allowed for usage
/// in this case buffer is a char array, you should set param size larger than string size of region.
#define PARAM_LICENSE_REGION 0x07

/// Get creation date of the license file, the format is like "Jan 01 00:00:00 CST 2018".
#define PARAM_LICENSE_CREATION_DATE 0x08

/// Get cpu info.
#define PARAM_LICENSE_CPU 0x09

/// Get memory info
#define PARAM_LICENSE_MEMORY 0x0A

/// Get disk info
#define PARAM_LICENSE_DISK 0x0B

/// Get mac address
#define PARAM_LICENSE_MAC_ADDRESS 0x0C

/// Get motherboard info
#define PARAM_LICENSE_MOTHERBOARD 0x0D

/// "get_attribute" function will return CALL_OK if success.
#define CALL_OK 0x0

/// "get_attribute" function will return CALL_OVERFLOW if the value of buffer size passed to this function is too small
#define CALL_OVERFLOW 0x1

/// "get_attribute" function will return CALL_ERROR if it is failed.
#define CALL_ERROR 0x2

/// "get_attribute" function will return CALL_IGNORE if it ignores the call.
#define CALL_IGNORE 0x3

/// @param	license_path	/path/to/licensefile
/// @param	type		PARAM_*
/// @param	buffer
/// @param	size		The buffer size
/// @return	CALL_*
size_t get_attribute(const char* license_path, size_t type,
		unsigned char* buffer, size_t size);

/// @param	license_content		License encrypted content as string
/// @param	content_size		The size of content of the license file
/// @param	type				PARAM_*
/// @param	buffer				Where result resides
/// @param	buffer_size			The buffer size
/// @return	CALL_*
size_t get_attribute_s(unsigned char license_content[], size_t content_size,
		size_t type, unsigned char* buffer, size_t buffer_size);

/// @param	license_path	/path/to/licensefile
/// @return	true if the license is valid
bool is_valid(const char* license_path);

/// @param	license_content		License encrypted content as string
/// @param 	content_size		Size of license
/// @return	true if the license is valid
bool is_valid_s(unsigned char license_content[], size_t content_size);

/// @param	license_path	/path/to/licensefile
/// @return true if the license is not expired
bool check_timestamp(const char* license_path);

/// The following functions must be called in order in a process to finish the whole license verification procedures.
/// If any of them return "false", the verification is failed.
/// If these functions are called in a wrong order, the license file will be corrupted.
/// @param	suite	(xxx1x101)2 means enterprise, (x111xx11)2 means community, (xxx0x1x1)2 means academic, "x" means random bits
/// @param 	license_path	license file path
/// @return	STATE_*
uint32_t f_1(const char* license_path, size_t suite);

uint32_t f_2(const char* license_path, size_t suite);

uint32_t f_3(const char* license_path, size_t suite);

uint32_t f_4(const char* license_path, size_t suite);

uint32_t f_5(const char* license_path, size_t suite);

uint32_t f_5(const char* license_path, size_t suite);

uint32_t f_6(const char* license_path, size_t suite);

/// The following constants indicate the state returned by f_* function
/// Success
#define STATE_SUCCESS		0x00989cc3 // 4 bytes long
/// Unknown error
#define STATE_UNKNOWN_ERROR	0x00000000 // 4 bytes long
/// Invalid license
#define STATE_INVALID 		0x0098c0a5 // 4 bytes long
/// License is expired
#define STATE_EXPIRED 		0x05f5e0bb // 4 bytes long
/// Suite doesn't match
#define STATE_ERROR_SUITE	0x0099981b // 4 bytes long

/// The following functions always return false, you can use them for ONCE to perform a
/// special check after ALL f_* functions are called
bool x(const char* license_path, size_t suite);

#ifdef __cplusplus
}
#endif
#endif
