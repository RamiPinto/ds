struct query_msg {
    string msg<255>;
    string md5<255>;
};

program MESSAGE_SERVER{
	version MESSAGE_SERVER_VERS{
		
		int INIT() = 1;
		int REGISTER(string user<255>) = 2;
		int UNREGISTER(string user<255>) = 3;
		int STORE(string sender<255>, string receiver<255>, int id, string text<255>, string md5<255>, string path<255>) = 4;
		int MSG_COUNT(string user<255>) = 5;
		query_msg QUERY(string user<255>, string <255>) = 6;

	} = 1;

	} = 0x23456788;



