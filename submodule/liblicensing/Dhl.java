package dhl;
import java.io.*;

/**
 * Pinpoint license Java interface 
 */
public class Dhl {

	/**
	 * Demo code to verify the license
	 */ 
	public static void main(String[] args) {
		String file_path = "xml_license.dat";
			
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
		long suite = 0x1; // means community, you need to get this variable from your project build parameter
		// Call the verification functions in a particular order (You must do it!)
		if (f_1(file_path, suite) == STATE_SUCCESS && f_2(file_path, suite) == STATE_SUCCESS && 
				f_3(file_path, suite) == STATE_SUCCESS && f_4(file_path, suite) == STATE_SUCCESS && 
					f_5(file_path, suite) == STATE_SUCCESS && f_6(file_path, suite) == STATE_SUCCESS)
			System.out.println("Verification success!");
		else
		    System.out.println("Verification failed!");
	}

	static {
		try {
			System.loadLibrary("dhl");
		}
		catch (Exception e) {
			
		} 
	}

	/** 
	 * License attribute you want to retrieve as well as possible value for param type
	 */
	/**
	 * Get max buffer size (to assure the size of all possible results)
	 */
	public static final int TEST_MAX_BUFFER_SIZE = 0x00;
	
	/**
	 * Get expiry date of the license file
	 */
	public static final int EXPIRY_DATE = 0x01;
	
	/**
	 * Get max code lines allowed to be analyzed from the license file
	 */
	public static final int MAX_CODE_SIZE = 0x02;
	
	/**
	 * Get suite from the license file
	 */
	public static final int SUITE = 0x03;
	
	/**
	 * Get uid from the license file
	 */
	public static final int UID = 0x04;
	
	/**
	 * Get organization from the license file
	 */
	public static final int ORGANIZATION = 0x05;
	
	/**
	 * Get agency from the license file
	 */
	public static final int AGENCY = 0x06;
	
	/**
	 * Get region from the license file
	 */
	public static final int REGION = 0x07;
	
	/**
	 * Get creation date of the license
	 */
	public static final int CREATION_DATE = 0x08;
	
	/** 
	 * The following parameters can help you to extract hardware information from the license file
	 */
	public static final int CPU = 0x09;
	public static final int MEMORY = 0x0A;
	public static final int DISK = 0x0B;
	public static final int MAC_ADDRESS = 0x0C;
	public static final int MOTHERBOARD = 0x0D;

	/**
	 * Possible value for the return code of function "get_lic_attr"
	 */
	public static final int CALL_OK = 0x0;
	public static final int CALL_OVERFLOW = 0x1;
	public static final int CALL_ERROR = 0x2;
	public static final int CALL_IGNORE = 0x3;
	
	/** 
	 * Get particular attribute from a given license file
	 * @param 		filePath	License file path
	 * @param 		type		Specify which attribute you want to retrieve
	 * @param 		buffer		The attribute result will be stored here
	 * @param 		size		The buffer size
	 * @return		CALL_*	
	 * @throws		java.lang.UnsatisfiedLinkError	If fails to load dynamic library
	 */
	public static native int get_attribute(String filePath, int type, byte buffer[], int size);

	/**
	 * Check if the license is valid or not
	 * @param 	filePath 	license file path
	 * @return	true if the license is valid
	 * @throws	java.lang.UnsatisfiedLinkError	If fails to load dynamic library
	 */
	public static native boolean is_valid(String filePath);
	
	/**
	 * Check if the license is expired or not
	 * @param	filePath	license file path
	 * @param	true if the license is not expired
	 * @throws	java.lang.UnsatisfiedLinkError	If fails to load dynamic library
	 */	
	public static native boolean check_timestamp(String filePath);
	
	/**
	 * You should call the following verfication functions in a particular order (You must do it!)
	 * The order is f_1 -> f_2 -> f_3 -> f_4 -> f_5 -> f_6
	 */
	public static native long f_1(String filePath, long suite);
	public static native long f_2(String filePath, long suite);
	public static native long f_3(String filePath, long suite);
	public static native long f_4(String filePath, long suite);
	public static native long f_5(String filePath, long suite);
	public static native long f_6(String filePath, long suite);

	/**
	 * Possible value for the return code of f_* function, showing the state of verification
	 */
	public static final long STATE_SUCCESS = 0x00989cc3;
	public static final long STATE_UNKNOWN_ERROR = 0x00000000;
	public static final long STATE_INVALID = 0x0098c0a5;
	public static final long STATE_EXPIRED = 0x05f5e0bb;
	public static final long STATE_ERROR_SUITE = 0x0099981b;
	
	protected static int bytesToInt(byte input[]){
		int result = 0;
		int num = 1;
		for (int j = 0; j < input.length; j++) {
			int t = input[j];
			if (t < 0)
				t += 256;
			result += (t) * num;
			num *= 256;
		}
		return result;
	}
}