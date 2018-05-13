import java.io.*;
import gnu.getopt.Getopt;
import java.net.*;
import java.util.*;
import java.util.ArrayList;

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
	private static Boolean user_connected = false;
	private static ServerSocket client_socket;
	private static String current_user;

	/********************* METHODS ********************/

	/**
	 * @param user - User name to register in the system
	 *
	 * @return OK if successful
	 * @return USER_ERROR if the user is already registered
	 * @return ERROR if another error occurred
	 */
	static RC register(String user){

		try{
			// 5.1.1 Connects to the server, according to the IP and port passed in the command line to the program.
      Socket sc = new Socket(_server,_port);
			// 5.1.2 The string ”REGISTER” is sent indicating the operation
      OutputStream ostream = sc.getOutputStream();
			ostream.write("REGISTER\0".getBytes());
			ostream.flush();
			// 5.1.3 A string of characters is sent with the name of the user to be registered.
			ostream.write((user + "\0").getBytes());
			ostream.flush();
			// 5.1.4 It receives from the server a byte that encodes the result of the operation:
			// 0 in case of success, 1 if the user is previously registered, 2 in any other case.
      DataInputStream istream = new DataInputStream(sc.getInputStream());
			int result = istream.readInt();
			// 5.1.5 Close the connection
      sc.close();
			// 5.1.4 Return:
			switch (result){
				case 0: // 0: Register succesful
					// Prepare and store class in the ArrayList
					System.out.print("REGISTER OK\n");
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
	static RC unregister(String user){
		try{
			// 5.2.1 Connects to the server, according to the IP and port passed in the command line to the program.
			Socket sc = new Socket(_server,_port);
			// 5.2.2 The string ”UNREGISTER” is sent indicating the operation
			OutputStream ostream = sc.getOutputStream();
			ostream.write("UNREGISTER\0".getBytes());
			ostream.flush();
			// 5.2.3 A string of characters is sent with the name of the user to be registered.
			ostream.write((user + "\0").getBytes());
			ostream.flush();
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
					if(user_connected && user.equals(current_user)){
						user_connected = false;
						client_socket.close();
					}
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
		}
		return RC.ERROR;
	}

	/**
 * Parallel thread: listens.
 */

	public static class Listener extends Thread{
		ServerSocket serverAddr;

		Listener(ServerSocket serverAddr) {
			this.serverAddr = serverAddr;
		}

		@Override
		public void run (){
			Socket sc = null;
			String operation;
			String username;
			String id;
			String [] message = new String[255];
			while(true){
				try{
					// Waiting for connection
					sc = serverAddr.accept();

					//5.6.2  The string ”SEND MESSAGE” or ”SEND MESS ACK” is sent indicating the operation.
					InputStream istream = sc.getInputStream();
					ObjectInput input 	= new ObjectInputStream(istream);
					operation = (String) input.readObject();
					System.out.println(operation);
						if(operation.equals("SEND_MESSAGE")){//Operation SEND_MESSAGE received
						  //5.6.3 A string is sent with the name that identifies the user sending the message
							istream = sc.getInputStream();
							input   = new ObjectInputStream(istream);
							username = (String) input.readObject();

							//5.6.4 A string is sent by encoding the identifier associated with the message
							istream = sc.getInputStream();
							input   = new ObjectInputStream(istream);
							id = (String) input.readObject();

							//5.6.5 A string is sent with the message
							istream = sc.getInputStream();
							input   = new ObjectInputStream(istream);
							message = (String[]) input.readObject();

							//Display received message
							System.out.println("MESSAGE "+id+" FROM "+username+" : \n"+message+"\nEND");

							//5.6.6 Close the connection.
							sc.close();
						}
						else if(operation.equals("SEND_MESS_ACK")){ // Operation SEND_MESS_ACK received
							//5.6.3 A string is sent by encoding the identifier associated with the message
							istream = sc.getInputStream();
							input   = new ObjectInputStream(istream);
							id = (String) input.readObject();

							//Display confirmation messages
							System.out.println("SEND MESSAGE "+id+" OK");

							//5.6.4 Close the connection.
							sc.close();
						}
						else System.out.println("[ERROR] Unknown operation code received");
				}
				catch(Exception e){
					System.out.println("Thread listening on "+serverAddr.getLocalPort()+" has stopped listening");
					return;
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
		 //Only one user can be connected at the same time
		 if(user_connected){
			 System.out.println("[ERROR]An user is already connected, please disconnect first before connecting a new user");
			 return RC.ERROR;
		 }

		Thread client_thread = null;
		ServerSocket local_socket = null;

		try{

			// Internally the client will search for a valid free port
			local_socket = new ServerSocket(0);
			// Create a thread that will be in charge of listening (on the IP and port selected) and attend to the messages sent by other users from the server.
			client_thread = new Listener(local_socket);
			client_thread.start();

			// 5.3.1 Connects to the server, according to the IP and port passed in the command line to the program.
			Socket sc = new Socket(_server,_port);

			// 5.3.2 The string ”CONNECT” is sent indicating the operation.
			OutputStream ostream = sc.getOutputStream();
			ostream.write("CONNECT\0".getBytes());
			ostream.flush();

			// 5.3.3 A string is sent with the name of the user.
			ostream.write((user + "\0").getBytes());
			ostream.flush();

			// 5.3.4 We send a string of characters encoding the client’s listening port number.
			String port = String.valueOf(local_socket.getLocalPort());
			ostream.write((port + "\0").getBytes());
			ostream.flush();

			// 5.3.5 It receives a byte from the server that encodes the result of the operation.
			DataInputStream istream = new DataInputStream(sc.getInputStream());
			int result = istream.readInt();
		  // 5.3.6 Close the connection.
			sc.close();

			// 5.3.5 Return
			switch (result){
				case 0: // 0: Connection succesful
					System.out.print("CONNECT OK \n");
					user_connected = true;
					current_user = user;
					client_socket = local_socket;
					return RC.OK;
				case 1:  // 1: The user wasn't previously registered
				  local_socket.close();
					System.out.print("CONNECT FAIL, USER DOES NOT EXIST \n");
					return RC.ERROR;
				case 2:  // 2: The client is already connected
					local_socket.close();//Close the socket opened
					System.out.print("USER ALREADY CONNECTED \n");
					return RC.ERROR;
				case 3:  // 3: There was some other error
					local_socket.close();//Close the socket opened
					System.out.print("CONNECT FAIL \n");
					return RC.ERROR;
				default: // If the server returns any other thing
					local_socket.close();//Close the socket opened
					System.out.print("[ERROR] UNKNOWN SERVER MESSAGE \n");
					return RC.ERROR;
			}
		}
		catch(Exception e){
			try{
				local_socket.close();
			}
			catch( Exception socket){
				System.err.println("[ERROR] Unable to close the socket");
			}

			System.err.println("[ERROR] Unable to stablish connection with the specified host");
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
	static RC disconnect(String user){
		try{
			// 5.4.1 Connects to the server, according to the IP and port passed in the command line to the program.
      Socket sc = new Socket(_server,_port);
			// 5.4.2 The string ”DISCONNECT” is sent indicating the operation
      OutputStream ostream = sc.getOutputStream();
			ostream.write("DISCONNECT\0".getBytes());
			ostream.flush();
			// 5.4.3 A string of characters is sent with the name of the user to be disconnected
			ostream.write((user + "\0").getBytes());
			ostream.flush();
			// 5.4.4 It receives from the server a byte that encodes the result of the operation:
			// 0 in case of success, 1 if the user does not exist, 2 if the user is not connected and 3 in any other case
      DataInputStream istream = new DataInputStream(sc.getInputStream());
			int result = istream.readInt();
			// 5.4.5 Close the connection
      sc.close();
			// 5.4.4 Return:
			switch (result){
				case 0: // 0: Connection succesful
					System.out.print("DISCONNECT OK \n");
					user_connected = false;
					current_user = null;
					client_socket.close();

					return RC.OK;
				case 1:  // 1: The user wasn't previously registered
					System.out.print("DISCONNECT FAIL / USER DOES NOT EXIST \n");
					return RC.ERROR;
				case 2:  // 2: The client is already disconnected
					System.out.print("DISCONNECT FAIL / USER NOT CONNECTED \n");
					return RC.ERROR;
				case 3:  // 3: There was some other error
					System.out.print("DISCONNECT FAIL \n");
					return RC.ERROR;
				default: // If the server returns any other thing
					System.out.print("[ERROR] UNKNOWN SERVER MESSAGE \n");
					return RC.ERROR;
			}
    }
    catch (Exception e){
      System.err.println("[ERROR] Unable to stablish connection with the specified host");
    }
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
	static RC send(String user, String message)	{
		try{
			// 5.5.1 Connects to the server, according to the IP and port passed in the command line to the program.
			Socket sc = new Socket(_server,_port);

			// 5.5.2 The string ”SEND” is sent indicating the operation
			OutputStream ostream = sc.getOutputStream();
			ostream.write("SEND\0".getBytes());
			ostream.flush();

			// 5.5.3 A string is sent with the name that identifies the user sending the message.
			ostream.write((current_user + "\0").getBytes()); //FROM
			ostream.flush();

			// 5.5.4 A string is sent with the name that identifies the recipient user of the message.
			ostream.write((user + "\0").getBytes()); //TO
			ostream.flush();

			// 5.5.5 A string is sent in which the message to be sent is encoded.
			ostream.write((message + "\0").getBytes());
			ostream.flush();

			// 5.5.6 It receives from the server a byte that encodes the result of the operation:
			// 0 in case of success, 1 if the user is previously registered, 2 in any other case.
			DataInputStream intstream = new DataInputStream(sc.getInputStream());
			int result = intstream.readInt();;

			System.out.println(result);
			// 5.5.6 Return:
			switch (result){
				case 0: // 0: Unregistration succesful
					// 	5.5.6 In this case, you will then receive a character string that will encode the numeric identifier assigned to the message
					InputStream stringstream = sc.getInputStream();
					ObjectInput input 	= new ObjectInputStream(stringstream);
					System.out.println("SEND OK - MESSAGE"+((String) input.readObject()));
					// 5.5.7 Close the connection
					sc.close();
					return RC.OK;
				case 1:  // 1: The user wasn't previously registered
					System.out.print("SEND FAIL / USER DOES NOT EXIST \n");
					// 5.5.7 Close the connection
					sc.close();
					return RC.ERROR;
				case 2:  // 2: There was some other error
					System.out.print("SEND FAIL \n");
					// 5.5.7 Close the connection
					sc.close();
					return RC.ERROR;
				default: // If the server returns any other thing
					System.out.print("[ERROR] UNKNOWN SERVER MESSAGE \n");
					// 5.5.7 Close the connection
					sc.close();
					return RC.ERROR;
			}
		}
		catch (Exception e){
			System.err.println("[ERROR] Unable to stablish connection with the specified host");
			e.printStackTrace(System.out);
		}
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
							if(user_connected){
								user_connected = false;
								client_socket.close();
							}
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
