import java.io.*;
import gnu.getopt.Getopt;
import java.net.*;
import java.util.*;

class client {

	/********************* TYPES **********************/

	/**
	 * @brief Return codes for the protocol methods
	 */
	private static enum RC {
		OK,
		ERROR,
		USER_ERROR
	};

	/******************* ATTRIBUTES *******************/

	private static String _server   = null;
	private static int _port = -1;


	/********************* METHODS ********************/

	/**
	 * @param user - User name to register in the system
	 *
	 * @return OK if successful
	 * @return USER_ERROR if the user is already registered
	 * @return ERROR if another error occurred
	 */
	static RC register(String user)
	{
		try{
			// 5.1.1 Connects to the server, according to the IP and port passed in the command line to the program.
      Socket sc = new Socket(_server,_port);
			// 5.1.2 The string ”REGISTER” is sent indicating the operation
      OutputStream ostream = sc.getOutputStream();
      ObjectOutput s = new ObjectOutputStream(ostream);
			String message = "REGISTER"; // TO DO: Add EOS?
			s.writeObject(message);
			s.flush();
			// 5.1.3 A string of characters is sent with the name of the user to be registered.
			s.writeObject(user); // TO DO: Add EOS?
			s.flush();
			// 5.1.4 It receives from the server a byte that encodes the result of the operation:
			// 0 in case of success, 1 if the user is previously registered, 2 in any other case.
      DataInputStream istream = new DataInputStream(sc.getInputStream());
			int result = istream.readInt();
			// 5.1.5 Close the connection
      sc.close();
			// 5.1.4 Return:
			switch (result){
				case 0: // 0: Register succesful
					System.out.print("REGISTER OK OK \n");
					return RC.OK;
				case 1:  // 1: The user was previously registered
					System.out.print("USERNAME IN USE \n");
					return RC.ERROR;
				case 2:  // 2: There was some other error
					System.out.print("REGISTER FAIL \n");
					return RC.ERROR;
				default: // If the server returns any other thing
					System.out.print("[ERROR] UNKNOWN SERVER MESSAGE \n");
					return RC.ERROR;
			}
    }
    catch (Exception e){
      System.err.println("[ERROR] Unable to stablish connection with the specified host");
      e.printStackTrace();
    }
		return RC.ERROR;
	}

	/**
	 * @param user - User name to unregister from the system
	 *
	 * @return OK if successful
	 * @return USER_ERROR if the user does not exist
	 * @return ERROR if another error occurred
	 */
	static RC unregister(String user)
	{
		try{
			// 5.2.1 Connects to the server, according to the IP and port passed in the command line to the program.
			Socket sc = new Socket(_server,_port);
			// 5.2.2 The string ”UNREGISTER” is sent indicating the operation
			OutputStream ostream = sc.getOutputStream();
			ObjectOutput s = new ObjectOutputStream(ostream);
			String message = "UNREGISTER"; // TO DO: Add EOS?
			s.writeObject(message);
			s.flush();
			// 5.2.3 A string of characters is sent with the name of the user to be registered.
			s.writeObject(user); // TO DO: Add EOS?
			s.flush();
			// 5.2.4 It receives from the server a byte that encodes the result of the operation:
			// 0 in case of success, 1 if the user is previously registered, 2 in any other case.
			DataInputStream istream = new DataInputStream(sc.getInputStream());
			int result = istream.readInt();;
			// 5.2.5 Close the connection
			sc.close();
			// 5.2.4 Return:
			switch (result){
				case 0: // 0: Unregistration succesful
					System.out.print("UNREGISTER OK \n");
					return RC.OK;
				case 1:  // 1: The user wasn't previously registered
					System.out.print("USER DOES NOT EXIST \n");
					return RC.ERROR;
				case 2:  // 2: There was some other error
					System.out.print("UNREGISTER FAIL \n");
					return RC.ERROR;
				default: // If the server returns any other thing
					System.out.print("[ERROR] UNKNOWN SERVER MESSAGE \n");
					return RC.ERROR;
			}
		}
		catch (Exception e){
			System.err.println("[ERROR] Unable to stablish connection with the specified host");
			e.printStackTrace();
		}
		return RC.ERROR;
	}

	/**
 * @return Opened socket if succesful
 * @return Throw exception if no free port was found
 */
	static ServerSocket findSocket() throws Exception{
		for (int port = 1025; port < 65535 ; port++) { //Port must be in the range 1024 < port < 65535
			 try {
					 return new ServerSocket(port);
			 }
			 catch (Exception ex) {
					 continue; // try next port
			 }
	 	}
		throw new IOException("[ERROR] Unable to find a free port");
	}

	/**
 * Parallel thread: listens.
 */

	public class listener implements Runnable{
		ServerSocket serverAddr;

		listener(ServerSocket serverAddr) {
			this.serverAddr = serverAddr;
			System.out.println(serverAddr.getLocalPort());
		}

		@Override
		public void run (){
			Socket sc = null;
			String message;
			while(true){
				try{
					// Waiting for connection
					sc = serverAddr.accept();
					InputStream istream = sc.getInputStream();
					ObjectInput in 			= new ObjectInputStream(istream);
					// TO DO: MESSAGE RECEPTION
				}
				catch(Exception e){
				}
			}
		}
	}


    /**
	 * @param user - User name to connect to the system
	 *
	 * @return OK if successful
	 * @return USER_ERROR if the user does not exist or if it is already connected
	 * @return ERROR if another error occurred
	 */
	static RC connect(String user){

		// Internally the client will search for a valid free port
			// 2 Options

			// Option 1: We don't mind the port used

		/*try{
			ServerSocket s = new ServerSocket(0);
		}
		catch (Exception e){
			System.err.println("[ERROR] Error while creating the socket");
		}*/


		try{
			// Option 2: We want the port to be within a range
			ServerSocket client_socket = findSocket();
			// Create a thread that will be in charge of listening (on the IP and port selected) and attend to the messages sent by other users from the server.
			Runnable client_thread = new listener(client_socket);
			new Thread(client_thread).start();

			// 5.3.1 Connects to the server, according to the IP and port passed in the command line to the program.
			Socket sc = new Socket(_server,_port);
			// 5.3.2 The string ”CONNECT” is sent indicating the operation.
			OutputStream ostream = sc.getOutputStream();
			ObjectOutput s = new ObjectOutputStream(ostream);
			String message = "CONNECT"; // TO DO: Add EOS?
			s.writeObject(message);
			s.flush();

			// 5.3.3 A string is sent with the name of the user.
			s.writeObject(user); // TO DO: Add EOS?
			s.flush();

			// 5.3.4 We send a string of characters encoding the client’s listening port number.
			String port = String.valueOf(client_socket.getLocalPort());
			s.writeObject(port); // TO DO: Add EOS?
			s.flush();

			// 5.3.5 It receives a byte from the server that encodes the result of the operation.
			DataInputStream istream = new DataInputStream(sc.getInputStream());
			int result = istream.readInt();

		  // 5.3.6 Close the connection.
			sc.close();

			// 5.3.5 Return
			switch (result){
				case 0: // 0: Connection succesful
					System.out.print("CONNECT OK \n");
					return RC.OK;
				case 1:  // 1: The user wasn't previously registered
					System.out.print("CONNECT FAIL, USER DOES NOT EXIST \n");
					return RC.ERROR;
				case 2:  // 2: The client is already connected
					System.out.print("USER ALREADY CONNECTED \n");
					return RC.ERROR;
				case 3:  // 3: There was some other error
					System.out.print("CONNECT FAIL \n");
					return RC.ERROR;
				default: // If the server returns any other thing
					System.out.print("[ERROR] UNKNOWN SERVER MESSAGE \n");
					return RC.ERROR;
			}
		}
		catch(Exception e){
			System.err.println("[ERROR] No available ports");
		}
		return RC.ERROR;
	}

	 /**
	 * @param user - User name to disconnect from the system
	 *
	 * @return OK if successful
	 * @return USER_ERROR if the user does not exist
	 * @return ERROR if another error occurred
	 */
	static RC disconnect(String user)
	{
		// Write your code here
		return RC.ERROR;
	}

	 /**
	 * @param user    - Receiver user name
	 * @param message - Message to be sent
	 *
	 * @return OK if the server had successfully delivered the message
	 * @return USER_ERROR if the user is not connected (the message is queued for delivery)
	 * @return ERROR the user does not exist or another error occurred
	 */
	static RC send(String user, String message)
	{
		// Write your code here
		return RC.ERROR;
	}

	/**
	 * @brief Command interpreter for the client. It calls the protocol functions.
	 */
	static void shell()
	{
		boolean exit = false;
		String input;
		String [] line;
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

		while (!exit) {
			try {
				System.out.print("c> ");
				input = in.readLine();
				line = input.split("\\s");

				if (line.length > 0) {
					/*********** REGISTER *************/
					if (line[0].equals("REGISTER")) {
						if  (line.length == 2) {
							register(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: REGISTER <userName>");
						}
					}

					/********** UNREGISTER ************/
					else if (line[0].equals("UNREGISTER")) {
						if  (line.length == 2) {
							unregister(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: UNREGISTER <userName>");
						}
                    }

                    /************ CONNECT *************/
                    else if (line[0].equals("CONNECT")) {
						if  (line.length == 2) {
							connect(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: CONNECT <userName>");
                    	}
                    }

                    /********** DISCONNECT ************/
                    else if (line[0].equals("DISCONNECT")) {
						if  (line.length == 2) {
							disconnect(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: DISCONNECT <userName>");
                    	}
                    }

                    /************** SEND **************/
                    else if (line[0].equals("SEND")) {
						if  (line.length >= 3) {
							// Remove first two words
							String message = input.substring(input.indexOf(' ')+1).substring(input.indexOf(' ')+1);
							send(line[1], message); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: SEND <userName> <message>");
                    	}
                    }

                    /************** QUIT **************/
                    else if (line[0].equals("QUIT")){
						if (line.length == 1) {
							exit = true;
						} else {
							System.out.println("Syntax error. Use: QUIT");
						}
					}

					/************* UNKNOWN ************/
					else {
						System.out.println("Error: command '" + line[0] + "' not valid.");
					}
				}
			} catch (java.io.IOException e) {
				System.out.println("Exception: " + e);
				e.printStackTrace();
			}
		}
	}

	/**
	 * @brief Prints program usage
	 */
	static void usage()
	{
		System.out.println("Usage: java -cp . client -s <server> -p <port>");
	}

	/**
	 * @brief Parses program execution arguments
	 */
	static boolean parseArguments(String [] argv)
	{
		Getopt g = new Getopt("client", argv, "ds:p:");

		int c;
		String arg;

		while ((c = g.getopt()) != -1) {
			switch(c) {
				//case 'd':
				//	_debug = true;
				//	break;
				case 's':
					_server = g.getOptarg();
					break;
				case 'p':
					arg = g.getOptarg();
					_port = Integer.parseInt(arg);
					break;
				case '?':
					System.out.print("getopt() returned " + c + "\n");
					break; // getopt() already printed an error
				default:
					System.out.print("getopt() returned " + c + "\n");
			}
		}

		if (_server == null)
			return false;

		if ((_port < 1024) || (_port > 65535)) {
			System.out.println("Error: Port must be in the range 1024 <= port <= 65535");
			return false;
		}

		return true;
	}



	/********************* MAIN **********************/

	public static void main(String[] argv)
	{
		if(!parseArguments(argv)) {
			usage();
			return;
		}

		// Write code here

		shell();
		System.out.println("+++ FINISHED +++");
	}
}
