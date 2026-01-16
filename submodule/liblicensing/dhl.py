#!/usr/bin/env python3.6
import ctypes
import os
import struct

""" 
   You must first call load_library function to load underlying dynamic library and call f_1, f_2, f_3, f_4, f_5 
   consecutively to complete the whole process.     
"""


class Param(object):
    TEST_BUFFER_SIZE = 0x00
    EXPIRY_DATE = 0x01  # get when the license is expired 
    MAX_CODE_SIZE = 0x02  # get max code size allowed for analysis
    SUITE = 0x03  # get the suite from license
    UID = 0x04  # get user id from license
    ORGANIZATION = 0x05  # get organization from license
    AGENCY = 0x06  # get agency name from license
    REGION = 0x07  # get region from license (e.g. China), the region indicates where Pinpoint is allowed for usage
    CREATION_DATE = 0x08  # get creation date of the license file
    CPU = 0x09  # get cpu info
    MEMORY = 0x0A  # get memory info
    DISK = 0x0B  # get disk info
    MAC_ADDRESS = 0x0C  # get mac address
    MOTHERBOARD = 0x0D  # get motherboard info


class Ret(object):
    CALL_OK = 0x0
    CALL_OVERFLOW = 0x1
    CALL_ERROR = 0x2
    CALL_IGNORE = 0x3


LIB = None


def load_library(lib_dirs: list = None) -> bool:
    global LIB
    name = _find_lib(lib_dirs)
    if name is not None:
        LIB = ctypes.cdll.LoadLibrary(name=name)
        return True
    return False


def unload_library():
    """
       Basically you do not need to call this function
    """
    global LIB
    if LIB is not None:
        try:
            ctypes.cdll.LoadLibrary('libdl.so.2').dlclose(ctypes.c_void_p(LIB._handle))
        except Exception as e:
            return
        LIB = None


def _search_file():
    """
    :return a path if license is found, otherwise None
    Note:
        Please catch any exception when calling this function
    """
    msg = [88, 72, 87, 48, 82, 77, 54, 74, 53, 53, 84, 83, 78, 80, 78, 66]
    fn = [43, 42, 37, 85, 62, 33, 87, 100, 89, 92, 55]
    fn = ''.join([chr(fn[i] ^ msg[i]) for i in range(0, len(fn))])
    p_dir = [118, 56, 62, 94, 34, 34, 95, 36, 65]
    p_dir = ''.join([chr(p_dir[i] ^ msg[i]) for i in range(0, len(p_dir))])
    path_list = [os.path.join(os.path.expanduser('~'), p_dir, fn)]
    for path in path_list:
        if os.path.isfile(path):
            return path
    return None


def _find_lib(lib_dirs: list = None):
    """
    :param lib_dirs: libdhl.so searching path list, default is the directory where this script resides
    :return an abs path if libdhl.so is found, otherwise None
    Note:
        Please catch any exception when calling this function
    """
    msg = [78, 72, 78, 54, 66, 55, 83, 66, 51, 65, 80, 49, 57, 84, 73, 80]
    lib_fn = [34, 33, 44, 82, 42, 91, 125, 49, 92]
    lib_fn = ''.join([chr(lib_fn[i] ^ msg[i]) for i in range(0, len(lib_fn))])
    if lib_dirs is None:
        lib_dirs = [os.path.join(os.path.abspath(os.path.dirname(__file__)), 'lib' + str(8 * struct.calcsize("P")))]
    for dir in lib_dirs:
        if os.path.isdir(dir):
            files = os.listdir(dir)
            for filename in files:
                if filename == lib_fn:
                    path = os.path.join(dir, filename)
                    if os.path.isfile(path):
                        return path
    return None


def get_max_buf_size(lib_dirs: list = None, file_path: str = ""):
    """
    :param lib_dirs: this arg is useless now, will be removed in the future
    :param file_path: license file path
    :return: the max buffer size, unit is byte, None if some errors occur
    """
    global LIB
    if LIB is None:
        return None

    try:
        result = ctypes.c_uint16(1)
        result_p = ctypes.c_void_p(ctypes.addressof(result))
        ret = LIB.get_attribute(ctypes.create_string_buffer(file_path.encode('ascii')), Param.TEST_BUFFER_SIZE,
                                result_p, 2)
    except Exception as e:
        return None
    if ret == Ret.CALL_OK:
        return result.value
    else:
        return None


def get_lic_attr(lib_dirs: list = None, file_path=None,
                 _type: Param.MAX_CODE_SIZE = Param.MAX_CODE_SIZE):
    """
    :param lib_dirs:    this arg is useless now, will be removed in the future
    :param file_path:   the path of license file path
    :param _type:   must be a member of Param class
    :return:_type=Param.MAX_CODE_SIZE: an int value indicating the max number of code lines allowed for analysis
            _type=Param.EXPIRY_DATE: a date representing when license is expired, the date format is YY-MM-DD
            None if some errors occur
    Note:
        Please catch any exception when calling this function
    """
    global LIB
    if LIB is None:
        return None

    try:

        if file_path is None:
            file_path = _search_file()
            if file_path is None:
                return None

        if _type == Param.MAX_CODE_SIZE:
            result = ctypes.c_uint32(1)
            result_p = ctypes.c_void_p(ctypes.addressof(result))
            ret = LIB.get_attribute(ctypes.c_char_p(file_path.encode('ascii')), _type, result_p, 4)
            if ret == Ret.CALL_OK:
                return result.value
            else:
                return None
        else:
            result = ctypes.create_string_buffer(1024)
            ret = LIB.get_attribute(ctypes.c_char_p(file_path.encode('ascii')), _type, result,
                                    len(result))
            if ret == Ret.CALL_OK:
                return result.value.decode()
            else:
                return None

    except Exception as e:
        return None


def check(func):
    def wrapper(file_path: str, s: int):
        global LIB
        if LIB is None:
            return False
        try:
            f = LIB.__getattr__(func.__name__)
            result = ctypes.c_ulonglong(f(file_path.encode('ascii'), ctypes.c_long(s)))
        except Exception as e:
            return False
        return result.value

    return wrapper


"""
    Now we will use the following functions for verification interface 
    You must call the following functions in a correct order and check the return code (True or False) of each function.
    Usage: f_1(file_path="/path/to/license_file", suite) 
            -> f_2(file_path="/path/to/license_file", suite) 
            -> f_3(file_path="/path/to/license_file", suite)
            -> f_4(file_path="/path/to/license_file", suite)
            -> f_5(file_path="/path/to/license_file", suite)
            -> f_6(file_path="/path/to/license_file", suite)
"""


@check
def f_1():
    return


@check
def f_2():
    return


@check
def f_3():
    return


@check
def f_4():
    return


@check
def f_5():
    return


@check
def f_6():
    return


class State(object):
    """
        This class shows the verification result returned by f_* functions
        PLEASE DO NOT USE VARIABLE NAME IN YOUR PYTHON SCRIPTS, USE HEX VALUE INSTAEAD 
        FOR SECURITY REASON!
    """
    success = 0x00989cc3  # success
    unknown_error = 0x00000000  # unknown error
    invalid = 0x0098c0a5  # invalid
    expired = 0x05f5e0bb  # expired
    err_suite = 0x0099981b  # suite doesn't match
