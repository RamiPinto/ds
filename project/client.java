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
		// 5.1.1 Connects to the server, according to the IP and port passed in the command line to the program.
		try{
			// 5.1.1 Connects to the server, according to the IP and port passed in the command line to the program.
      Socket sc = new Socket(_server,_port);
			// 5.1.2 The string ”REGISTER” is sent indicating the operation
      OutputStream ostream = sc.getOutputStream();
      ObjectOutput s = new ObjectOutputStream(ostream);
			String message = "REGISTER";
			s.writeObject(message);
			s.flush();
			// 5.1.3 A string of characters is sent with the name of the user to be registered.
			s.writeObject(user);
			s.flush();
			// 5.1.4 It receives from the server a byte that encodes the result of the operation:
			// 0 in case of success, 1 if the user is previously registered, 2 in any other case.
      DataInputStream istream = new DataInputStream(sc.getInputStream());
			int result;
      result = istream.readInt();
			// 5.1.5 Close the connection
      sc.close();
			// 5.1.4 Return:
			switch (result){
				case 1:  // 1: The user was previously registered
					System.out.print("USERNAME IN USE \n");
					return RC.ERROR;
				case 2:  // 2: There was some other error
					System.out.print("REGISTER FAIL \n");
					return RC.ERROR;
				default: // 0: Registration succesful
				System.out.print("REGISTER OK \n");
				return RC.OK;
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
		// Write your code here
		return RC.ERROR;
	}

    /**
	 * @param user - User name to connect to the system
	 *
	 * @return OK if successful
	 * @return USER_ERROR if the user does not exist or if it is already connected
	 * @return ERROR if another error occurred
	 */
	static RC connect(String user)
	{
		// Write your code here
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
