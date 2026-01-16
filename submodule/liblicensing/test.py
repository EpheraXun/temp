# for testing
from dhl import get_max_buf_size
from dhl import get_lic_attr
from dhl import Param
from dhl import load_library, unload_library
from dhl import f_1, f_2, f_3, f_4, f_5, f_6
from dhl import State
import os
import sys


def test_attribute(file_path, _type: Param.EXPIRY_DATE, attribute_name=""):
    result = get_lic_attr(file_path=file_path, _type=_type)
    if result is None:
        print('Can not get %s' % attribute_name)
    else:
        print('{} is {}'.format(attribute_name, result))


def test(file_path, valid=True):
    if not load_library():
        print('Fail to load library!')
        return
    size = get_max_buf_size(file_path=file_path)
    if size is None:
        print('Can not get the max buffer size!')
    else:
        print("Max buffer size is %d" % size)

    print("Testing %s .." % file_path)

    test_attribute(attribute_name='expiry date', file_path=file_path, _type=Param.EXPIRY_DATE)
    test_attribute(attribute_name='max code size', file_path=file_path, _type=Param.MAX_CODE_SIZE)
    test_attribute(attribute_name='suite', file_path=file_path, _type=Param.SUITE)
    test_attribute(attribute_name='uid', file_path=file_path, _type=Param.UID)
    test_attribute(attribute_name='organization', file_path=file_path, _type=Param.ORGANIZATION)
    test_attribute(attribute_name='agency', file_path=file_path, _type=Param.AGENCY)
    test_attribute(attribute_name='region', file_path=file_path, _type=Param.REGION)
    test_attribute(attribute_name='creation date', file_path=file_path, _type=Param.CREATION_DATE)
    test_attribute(attribute_name='cpu', file_path=file_path, _type=Param.CPU)
    test_attribute(attribute_name='memory', file_path=file_path, _type=Param.MEMORY)
    test_attribute(attribute_name='disk', file_path=file_path, _type=Param.DISK)
    test_attribute(attribute_name='mac address', file_path=file_path, _type=Param.MAC_ADDRESS)
    test_attribute(attribute_name='motherboard', file_path=file_path, _type=Param.MOTHERBOARD)

    suite = 0b11110011  # community
    #suite = 0b11110101 # enterprise
    # suite = 0b11000101  # academic
    if valid:
        #assert (f_1(file_path, suite) ^ f_2(file_path, suite) ^ f_3(file_path, suite) ^
         #           f_4(file_path, suite) ^ f_5(file_path, suite) ^ f_6(file_path, suite) == 0)
        try:
            ret = f_1(file_path, suite)
            assert (ret == State.success)
            print('f_1 pass')
            ret = f_2(file_path, suite)
            assert (ret == State.success)
            print('f_2 pass')
            ret = f_3(file_path, suite)
            assert (ret == State.success)
            print('f_3 pass')
            ret = f_4(file_path, suite)
            assert (ret == State.success)
            print('f_4 pass')
            ret = f_5(file_path, suite)
            assert (ret == State.success)
            print('f_5 pass')
            ret = f_6(file_path, suite)
            assert (ret == State.success)
            print('f_6 pass')
        except:
            print("error ret is {}".format(hex(ret)))
            sys.exit(1)
    else:
        assert (f_1(file_path, suite) ^ f_2(file_path, suite) ^ f_3(file_path, suite) ^
                f_4(file_path, suite) ^ f_5(file_path, suite) ^ f_6(file_path, suite) != 0)
    # assert (result ^ valid == 0)
    unload_library()
    print()


def main():
    file_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'legal-license.dat'))
    test(file_path)
    file_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'illegal-license1.dat'))
    test(file_path, valid=False)
    file_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'illegal-license2.dat'))
    test(file_path, valid=False)
    file_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'no-license.dat'))
    test(file_path, valid=False)


if __name__ == '__main__':
    main()
