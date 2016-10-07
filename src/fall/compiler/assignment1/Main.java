package fall.compiler.assignment1;

import java.util.Scanner;

public class Main {

  public static void main(String[] args) {
    // write your code here
    Scanner sc = new Scanner(System.in);
    int i;
    int res;
    MyList firstWord = new MyList();

    for(i = sc.nextInt(); i != 0; i = sc.nextInt() ) {
      firstWord.add(i);
    }
    System.out.println("End of first word.");

    if(firstWord.size > 0) {
      for(i = sc.nextInt(); i != 0; i = sc.nextInt()  ) {
        MyList secondWord = new MyList();

        for(; i != 0; i = sc.nextInt()) {
          secondWord.add(i);
        }

        // Check sub sequence
        res = firstWord.checkSubSequence(secondWord);
        System.out.println("Is sub Seq : " + res + "\n\n\n");

        //Check sub string
        res = firstWord.checkSubString(secondWord);
        System.out.println("Is sub String : " + res);
      }
    } else {
      System.out.println("The End");
    }
  }

  /*public static void checkSecondWord(MyList firstWord, Scanner sc){
    MyList secondWord = new MyList();
    int i = sc.nextInt();

    if (i != 0) {
      while (i != 0) {
        secondWord.add(i);
        i = sc.nextInt();
      }

      // Check sub sequence
      int res = firstWord.checkSubSequence(secondWord);
      System.out.println("Is sub Seq : " + res + "\n\n\n");

      //Check sub string
      res = firstWord.checkSubString(secondWord);
      System.out.println("Is sub String : " + res);

      checkSecondWord(firstWord, sc);
    } else {
      System.out.println("End of checking.");
    }
  }*/
}
