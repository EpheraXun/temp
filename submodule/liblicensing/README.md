# libLicensing   
  
Python:

    from dhl import Param    
    from dhl import get_lic_attr
    from dhl import f_1, f_2, f_3, f_4, f_5, f_6
    from dhl import load_library
    from dhl import State
    import sys
    
    """
       Warning: You must call some verfication functions in a particular order.
       The order is: load_library -> f_1 -> f_2 -> f_3 -> f_4 -> f_5 -> f_6
       More important, call these functions in RANDOM places in your program.
    """

    if not load_library(): # You may need to specify library searching path here.
        print('Fail to load libdhl.so!')
        return 


    # Get a particular attribute from license (low level interface)
    # You need to specify the attribute by members of "Param" class 
    num = get_lic_attr(_type=Param.MAX_CODE_LINE, libdirs=['/usr/lib', '/usr/lib64'])
    if num is None:
        print("Error!")
    else:
        print("Number of max code lines is %d" % num)
   
    # Call the following verification functions in this order to complete the whole verification process.
    suite = 0x1 # Before calling the following functions you should get "suite" from your project via m*.py in tools/Python
    file_path='/path/to/license'
    assert (f_1(file_path, suite) == State.success and f_2(file_path, suite) == State.success and 
              f_3(file_path, suite) == State.success and f_4(file_path, suite) == State.success and 
                f_5(file_path, suite) == State.success and f_6(file_path, suite) == State.success)
        

    
C-C++:     
    
    // Warning: You should call some verfication functions in a particular order.
    // The order is: f_1 -> f_2 -> f_3 -> f_4 -> f_5 -> f_6
    // And more important, call these functions in RANDOM places in your program.

    #include "license.h"
    using namespace std;
    // "/home/username" is user's home directory
    char* path = "/home/username/.pinpoint/sbrella.lic"; // Please hard-coding this license file path
    size_t ret;

    // Get a particular attribute from license (low level interface)
    uint16_t max_buf_size;
    ret = get_attribute(path, PARAM_TEST_MAX_BUFFER_SIZE, (unsigned char*)&max_buf_size, sizeof(uint16_t))
    if (ret == CALL_OK){
        cout << "max buffer size is: " << max_buf_size << endl;
    }
    else{
        cout << "Can not get max buffer size." << endl;
        return 1;
    }

    // Get a particular attribute from license (low level interface)
    uint32_t num;
    ret = get_attribute(path, PARAM_LICENSE_MAX_CODE_LINE, (unsigned char*)&num, sizeof(uint32_t));
    if (ret == CALL_OK){
        printf("The number is %d\n", num);
    }
    else{
        printf("Fail to get the number of code lines allowed for analysis. Exit\n");
        return 1;
    }
    
    size_t suite = 0x1; // You should get "suite" from your executable first
    // Call verification functions in this particular order, you need to check the return code of each function
    assert (f_1(file_path, suite) && State.success && f_2(file_path, suite) == State.success &&
              f_3(file_path, suite) == State.success && f_4(file_path, suite) == State.success && 
                f_5(file_path, suite) == State.success && f_6(file_path, suite) == State.success)

Java:

    // Warning: You should call some verfication functions in a particular order.
    // The order is: f_1, f_2, f_3, f_4, f_5, f_6
    // And more important, call these functions in RANDOM places in your program.

    import dhl.Dhl

    public class Test {
	    public static void main(String[] args) {
		    String file_path = "xml_license.dat";
		    // check whether the license file is valid or not

		    if (is_valid(file_path))
			    System.out.println("This license file is valid.");
		    else {
			    System.out.println("This license is not valid! Exit.");
			    return;
		    }

		    int ret;
		    byte buffer[] = new byte[2];
		    // get max buffer size (for testing)
		    int buf_size = 0;
		    ret = get_attribute(file_path, TEST_MAX_BUFFER_SIZE, buffer, buffer.length);
		    if (ret != CALL_OK) {
			    System.out.println("Can not get max buffer size.");
			    return;
		    } else {
			    try {
				    buf_size = Dhl.bytesToInt(buffer);
				    System.out.println("Max buffer size is: " + buf_size);
			    }
			    catch (Exception e) {
				    System.out.println("Can not show max buffer size.");
				    return;
			    }
		    }

		    // get expiry date
		    buffer = new byte[buf_size];
		    ret = get_attribute(file_path, EXPIRY_DATE, buffer, buffer.length);
		    if (ret != CALL_OK)
			    System.out.println("Can not get expiry date of the license.");
		    else {
			    try {
				    String date = new String(buffer, "US-ASCII");
				    System.out.println("Expiry date is: " + date);
			    } catch (Exception e) {
				    System.out.println("Unable to show expiry date.");
			    }
		    }

		    // get max code size
		    int code_size = 0x0;
		    buffer = new byte[4];
		    ret = get_attribute(file_path, MAX_CODE_SIZE, buffer, buffer.length);
		    if (ret != CALL_OK)
			    System.out.println("Can not get max code size of the license.");
		    else {
			    try {
				    code_size = Dhl.bytesToInt(buffer);
				    System.out.println("Max code size is: " + code_size);
			    }
			    catch (Exception e) {
				    System.out.println("Can not show max code size.");
			    }
		    }

	            long suite = 0x1; // you should get "suite" from your project via M*.java in tools/Java
                    // Please call the following functions in seperate places in your process, this is only for demo
                    if (f_1(file_path, suite) == STATE_SUCCESS && f_2(file_path, suite) == STATE_SUCCESS &&
                                f_3(file_path, suite) == STATE_SUCCESS && f_4(file_path, suite) == STATE_SUCCESS &&
                                        f_5(file_path, suite) == STATE_SUCCESS && f_6(file_path, suite) == STATE_SUCCESS)
                        System.out.println("Verification success!");
                    else
                        System.out.println("Verification failed!");

           }
    }
