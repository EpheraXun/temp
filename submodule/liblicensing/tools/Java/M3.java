package dhl;

import java.util.concurrent.ThreadLocalRandom;
import java.util.ArrayList;
import java.lang.Math;

/**
 * 
 * This class can help you to extract "suite" from your project
 *
 */
public class M3 {
	/**
	 * @param random
	 *            number of int parameters
	 * @return suite
	 */
	public long G(int... args) {
		ArrayList<Integer> a = new ArrayList<Integer>();
		for (int arg : args)
			a.add(arg);
		int n1 = ThreadLocalRandom.current().nextInt(0, (int) Math.max(1, Math.pow(2, a.size()) - 1));
		int n2 = ThreadLocalRandom.current().nextInt(0, Math.max(1, a.size() - 1))
				^ ThreadLocalRandom.current().nextInt(0, 17);
		int m = Math.max(n1, n2);
		int n = Math.min(n1, n2) + 1;
		int r = m % n;
		while (r != 0) {
			m = n;
			n = r;
			r = m % n;
		}
		return 0xB8 & (long) Math.pow(n, m) | (0xA5 ^ (r & 0x90)) | 0xD0;
	}
}
