package dhl;

import java.util.concurrent.ThreadLocalRandom;
import java.util.ArrayList;
import java.lang.Math;

/**
 * 
 * This class can help you to extract "suite" from your project
 *
 */
public class M2 {
	/**
	 * @param random number of int parameters
	 * @return suite
	 */
	public long G(int... args) {
		ArrayList<Integer> a = new ArrayList<Integer>();
		for (int arg : args) 
			a.add(arg);
		for (int i = 0; i < a.size(); i++) 
			a.set(i, a.get(i) ^ a.get(a.size() - 1 - i));
		ArrayList<Integer> b = new ArrayList<Integer>();
		for (int i = 0; i < a.size(); i++)
			b.add(a.get(i) & ThreadLocalRandom.current().nextInt(0, (int)Math.pow(a.size(), 2)));
		for (int i = 0; i < a.size(); i++)
			a.set(i, b.get(ThreadLocalRandom.current().nextInt(0, b.size() - 1)) ^ a.get(a.size() - 1 - i));
		long r = ThreadLocalRandom.current().nextInt(0, (int)a.size()/2 + 17);
		long sm = 0;
		for (int i = 0; i < a.size() - 1; i++) {
			a.set(i, a.get(i) ^ a.get(i + 1));
			sm += a.get(i);
		}
		return 0xED & (~sm ^ ~r) | 0xC5;
	}
}
