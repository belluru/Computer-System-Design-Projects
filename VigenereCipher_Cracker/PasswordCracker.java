import java.io.File;
import java.io.FileNotFoundException;
import java.util.HashSet;
import java.util.Scanner;


public class PasswordCracker {
	private static String cipherText = null;
	private static int firstWordLength;
	private static HashSet<String> dict = new HashSet<String>();
	public static void main(String[] args) throws FileNotFoundException {
		try{
			Scanner sc = new Scanner(new File("F:\\java\\VigenerePasswordCracker\\src\\dict.txt"));
			while (sc.hasNextLine()) {
		        dict.add(sc.nextLine());
		    }
		}
		catch(FileNotFoundException e){
			e.printStackTrace();
		}
		
		System.out.println("Enter the cipher text to be decrypted:");
		Scanner r = new Scanner(System.in);
		cipherText = r.next();
		System.out.println("Enter the key length:");
		int keyLength = r.nextInt();
		System.out.println("Enter the first word length:");
		firstWordLength = r.nextInt();
		//System.out.println(cipherText + keyLength + firstWordLength);
		r.close();
		GenerateSequences(keyLength);
	}
	
	public static void GenerateSequences(String s, int length) {
		    if (length==0) { 
		    	//System.out.println(s); 
		    	String plainText = Decryption.decrypt(s, cipherText.substring(0, firstWordLength));
		    	//System.out.println("plaintext is "+plainText);
		    	if(dict.contains(plainText)){
		    		if(s.length() < firstWordLength){
		    			String pre = s.substring(0,(firstWordLength % s.length()));
		    			String post = s.substring((firstWordLength % s.length()));
		    			s = post+pre;
		    		}
		    		plainText = plainText + Decryption.decrypt(s, cipherText.substring(firstWordLength));
		    		System.out.println(plainText + "\n");
		    	}
		    	return;
		    }
		    for(char ch='a'; ch<='z'; ch++) {
		    	GenerateSequences((String)(s+ch), length-1);
		    }
		}
	public static void GenerateSequences(int length) { GenerateSequences((String)(""), length); }

}
