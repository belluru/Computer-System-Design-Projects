public class Decryption {
	public static String decrypt(String keyText, String cipherText) {
		String plainText = "";
			for(int i=0,j=0; i<cipherText.length();j++,i++){
				Character ch = cipherText.charAt(i);
				Character keych = keyText.charAt(j);
				Character originalch = Character.toUpperCase(ch);
				Character originalkeych = Character.toUpperCase(keych);
				int num = (originalch - 'A') - (originalkeych - 'A') % 26;
				//System.out.println("num is "+num+" for char "+ ch+" using key "+keych);
				if(num < 0){
					num = num + 26;
				}
				//System.out.println((char) (num+'A'));
				plainText = plainText + (char) (num+'A');
				if(j == (keyText.length() - 1)){
					j=-1;
				}
			}
		//System.out.println("PlainText in decryption is  "+ plainText);
		return plainText;
	}
}
