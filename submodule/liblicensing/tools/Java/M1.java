package dhl;

import java.util.concurrent.ThreadLocalRandom;
import java.util.ArrayList;
import java.lang.Math;

/**
 * 
 * This class can help you to extract "suite" from your project
 *
 */
public class M1 {
	/**
	 * @param random number of int parameters
	 * @return suite 
	 */
	public long G(int... args) {
		ArrayList<Integer> a = new ArrayList<Integer>();
		ArrayList<Integer> b = new ArrayList<Integer>();
		for (int arg : args) 
			a.add(0xA1 & arg << ThreadLocalRandom.current().nextInt(0, (int) (arg / 2) + 1));
		for (int arg : args) 
			b.add(ThreadLocalRandom.current().nextInt(0, (int) (Math.pow(arg, 2)) + 1)
					^ arg >> ThreadLocalRandom.current().nextInt(0, arg + 1));
		for (int i = 0; i< b.size(); i++) 
			a.set(i, (a.get(i)) % (b.get(i)));
		int r = ThreadLocalRandom.current().nextInt(0, 4);
		int s[] = new int[a.size()];
		for (int i = 0; i< a.size() - 1 ; i++)
			s[i] = a.get(i) ^ a.get(i + 1);
		long sm = 0;
		for (int i: s)
			sm += i;
		return 0xF3 | r ^ sm & 0x77;
	}
}
