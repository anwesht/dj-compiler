package fall.compiler.assignment1;

/**
 * Created by atuladhar on 8/28/16.
 */
public class MyList {
  public int size;
  private Node head;
  private Node searchNode;

  public void add(int v) {
    if(size == 0){
      this.head = new Node();
      this.head.val = v;
      this.head.next = null;
      resetSearchNode();
    } else {
      Node temp = new Node();
      //Make a copy of current this.head in temp.
      temp.val = this.head.val;
      temp.next = this.head.next;
      //Update this.head to new value and point next to temp.
      this.head.val = v;
      this.head.next = temp;
    }
    //increment size of list.
    size += 1;
  }

  private void resetSearchNode() {
    this.searchNode = this.head;
  }

  private int find(int v) {
    int foundCurrent = 0;

    // foundCurrent == 0 and this.hasNext() == 1
//    for ( ; !(foundCurrent != 0 || this.hasNext() != 1); ) {
    for ( ; !(foundCurrent > 0 || this.hasNext() == 0); ) {
      System.out.println(v + "  Searching this val :" + this.searchNode.val);
      if (this.next() == v) {
        System.out.println("FOUND v: " + v);
        foundCurrent = 1;
      }
    }
    return foundCurrent;
  }

  private int next() {
    int v = this.searchNode.val;
    this.searchNode = this.searchNode.next;
    return v;
  }

  private int hasNext() {
    if(this.searchNode == null) {
      return 0;
    } else {
      return 1;
    }
  }

  public int checkSubSequence(MyList other) {
    int isSubSeq;

    if(other.size > this.size) {
      isSubSeq = 0;
    } else {
      isSubSeq = this.find(other.next());

      // isSubSeq == 1 and other.hasNext() == 1
//      for (; !(isSubSeq != 1 || other.hasNext() != 1); ) {
      for (; !(isSubSeq == 0 || other.hasNext() == 0); ) {
        isSubSeq = this.find(other.next());
      }
      this.resetSearchNode();
      other.resetSearchNode();
    }
    return isSubSeq;
  }

  public int checkSubString(MyList other) {
    int isSubString;

    if(other.size > this.size){
      isSubString = 0;
    } else {
      isSubString = this.find(other.next());
      // isSubString == 1 and other.hasNext() == 1;
//      for (; !( isSubString != 1 || other.hasNext() != 1 ); ) {
      for (; !( isSubString == 0 || other.hasNext() == 0 ); ) {
        // this.hasNext() == 1) and this.next() == other.next()
//        if (!(this.hasNext() != 1 || this.next() != other.next())) {
        if (!(this.hasNext() == 0 || !(this.next() == other.next()))) {
          isSubString = 1;
        } else {
          isSubString = 0;
        }
      }

      this.resetSearchNode();
      other.resetSearchNode();
    }
    return isSubString;
  }

  private class Node {
    int val;
    Node next;
  }
}
