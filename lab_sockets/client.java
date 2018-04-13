import java.lang.*;
import java.io.*;
import java.net.*;
import java.util.*;

public class client{
  public static void main (String [] args){
    int res;
    int num[] = new int[2];

    if(args.length != 1){
      System.out.println("Use: client <host>");
      System.exit(0);
    }
    try{
      //create connection
      String host = args[0];
      Socket sc = new Socket(host,2500);
      OutputStream ostream = sc.getOutputStream();
      ObjectOutput s = new ObjectOutputStream(ostream);
      DataInputStream istream = new DataInputStream(sc.getInputStream());
      num[0] = 5;
      num[1] = 2;
      s.writeObject(num);
      s.flush();
      res = istream.readInt();
      sc.close();
      System.out.println("Result = " + res);
    }
    catch (Exception e){
      System.err.println("Exception "+ e.toString());
      e.printStackTrace();
    }
  }
}
