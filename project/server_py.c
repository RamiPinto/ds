#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>

#include "list.h"
#include "storage.h"

/******************************************************************************
 *                              GLOBAL VARIABLES                              *
 ******************************************************************************/

// Information needed to handle the requests.
typedef struct {
    int fd;
    struct in_addr cli_addr;
} arg_t;

node_t *usr_list = NULL; // List of users.

const uint8_t PORT_SIZE = 8; // Maximum length of a port number.

int _port;
int _socket;
int _cli_size;

char *rpc_address;

struct sockaddr_in srv_addr;

pthread_mutex_t user_list_lck;
pthread_cond_t user_list_ulck;
uint32_t user_list_worker = 0;


/******************************************************************************
 *                              FUNCTION HEADERS                              *
 ******************************************************************************/

void session_handler(void *args);
void rcv_message(int socket);
void flush_msg_list(char *name);
uint8_t check_port(char *port);
ssize_t read_line(int fd, void *buffer, int n);
ssize_t write_line(int fd, void *buffer, size_t n);
void u_list_lck();
void u_list_ulck();


/******************************************************************************
 *                          FUNCTION IMPLEMENTATION                           *
 ******************************************************************************/

int main(int argc, char **argv) {
    setbuf(stdout, NULL);

    // Check args.
    if (argc != 5 || strcmp(argv[1], "-p") || !check_port(argv[2]) || strcmp(argv[3], "-s")) {
        printf("Usage: ./server -p <port> -s <RPC srv>\n");
        exit(-1);
    }

    rpc_address = malloc(strlen(argv[4]));
    strcpy(rpc_address, argv[4]);

    _port = atoi(argv[2]);
    _socket = socket(AF_INET, SOCK_STREAM, 0); // AF_INET = IPv4
                                               // SOCK_STREAM = TCP
    if (_socket < 0) { // Comprobar el descriptor de fichero.
        printf("[ERR] Socket Open.\n");
        exit(-1);
    }

    // Create the socket.
    bzero((char *) &srv_addr, sizeof(srv_addr)); // Clean the structure.
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(_port);
    bind(_socket, (const struct sockaddr *) &srv_addr, sizeof(srv_addr));

    listen(_socket, SOMAXCONN); // Start listening to new connections.
    _cli_size = sizeof(struct sockaddr_in);


    // Initialize concurrency variables.
    if (pthread_mutex_init(&user_list_lck, NULL) != 0) {
        perror("User list mutex");
        exit(-1);
    }
    if (pthread_cond_init(&user_list_ulck, NULL) != 0) {
        perror("User list cond");
        exit(-1);
    }

    // Initialize rpc service.
    CLIENT *rpc;
    rpc = clnt_create(rpc_address, MSG_STORE, MSG_STORE_V, "TCP");
    if (rpc == NULL) {
        clnt_pcreateerror("Error rpc binding");
        exit(-1);
    }

    int res;
    init_1(&res, rpc);
    if (res == -1) {
        perror("Init rpc");
        exit(-1);
    }

    clnt_destroy(rpc);

    printf("s> init server %s:%d\ns> ", inet_ntoa(srv_addr.sin_addr), _port);

    while (1) {
        int client_socket;
        struct sockaddr_in cli_addr;
        arg_t args;

        // New client connection.
        client_socket = accept(_socket, (struct sockaddr *) &cli_addr,
            (socklen_t *) &_cli_size);

        // Setting arguments struct.
        struct sockaddr_in *s = (struct sockaddr_in *)&cli_addr;
        args.fd = client_socket;
        args.cli_addr = s->sin_addr;

        // Creating the thread that is going to handle the session.
        pthread_t thread;

        if(pthread_create(&thread, NULL, (void *) session_handler,
                &args) == -1) {
            printf("Error creating the thread.\n");
            close(client_socket);
        }
        pthread_detach(thread);
    }

    return 0;
}

/*
 * @Brief: handle a session in the server.
 *
 * @param args: poniter to a void representation of arg_t structure.
 */
void session_handler(void *args) {
    /* Needed vars. */
    arg_t *client_data = (arg_t *) args; // Casting of arguments.
    int socket = client_data->fd; // Socket file descriptor.
    struct in_addr ip = client_data->cli_addr; // Client IP address.

    char op[BUFFER_SIZE], sender[BUFFER_SIZE], port[PORT_SIZE]; // Needed buffers.
    uint8_t response; // Response byte

    node_t *user; // User struct, in case it's needed to be modified.

    /* Clean buffers */
    bzero(op, BUFFER_SIZE);
    bzero(sender, BUFFER_SIZE);
    bzero(port, PORT_SIZE);

    if (read_line(socket, op, BUFFER_SIZE) == -1) { // Requested operation.
        response = 2; // Error.
        bzero(op, BUFFER_SIZE); // Clean buffer.
    }

    // We handle the request.
    if (!strcmp(op, "REGISTER")) {
        u_list_lck();
        if (read_line(socket, sender, BUFFER_SIZE) == -1) {
            response = 2;
        } else {
            if (usr_list == NULL) {
                usr_list = (node_t *) malloc(sizeof(node_t)); // Allocate memory.

                strcpy(usr_list->usr, sender); // Copy user name.
                usr_list->status = OFFLINE; // Not connected, by default.
                usr_list->seq = 1; // Sequence number to 1.
                usr_list->port = 0; // Empty port.
                usr_list->ip.s_addr = 0; // Empty IP address.
                usr_list->msg_list = NULL; // Empty message list.

                usr_list->next = NULL;
                usr_list->prev = NULL;

                usr_list->busy = 0;
                if (pthread_mutex_init(&usr_list->lock, NULL) != 0 ||
                            pthread_cond_init(&usr_list->unlock, NULL) != 0) {
                    free(usr_list);
                    response = 2;
                } else response = 0;
            } else response = add_usr(usr_list, sender); // Append new user to list.
        }
        u_list_ulck();
    } else if (!strcmp(op, "UNREGISTER")) {
        u_list_lck();
        if (read_line(socket, sender, BUFFER_SIZE) == -1) {
            response = 2;
        } else {
            if (usr_list == NULL) response = 1; // No user in the system.
            else response = rm_usr(&usr_list, sender);
        }
        u_list_ulck();
    } else if (!strcmp(op, "CONNECT")) {
        u_list_lck();
        // Checking possible errors.
        if (read_line(socket, sender, BUFFER_SIZE) == -1) {
            response = 2;
        } else if (read_line(socket, port, PORT_SIZE) == -1) {
            response = 2;
        } else if (!check_port(port)) {
            response = 2;
        } else { // No errors found so far.
            if (usr_list == NULL) response = 1; // No user in the system.
            else {
                user = get_user(usr_list, sender);
                if (user == NULL) response = 1;
                else if (user->status == ONLINE) response = 2;
                else { // Setting the data.
                    user->status = ONLINE;
                    user->port = atoi(port);
                    user->ip = ip;
                    response = 0;
                }
            }
        }
        u_list_ulck();
    } else if (!strcmp(op, "DISCONNECT")) {
        u_list_lck();
        if (read_line(socket, sender, BUFFER_SIZE) == -1) {
            response = 2;
        } else {
            if (usr_list == NULL) response = 1; // No user in the system.
            else {
                user = get_user(usr_list, sender);
                if (user == NULL) response = 1;
                else if (user->status == OFFLINE) response = 2;
                else if (user->ip.s_addr != ip.s_addr) response = 2;
                else { // Setting the data.
                    user->status = OFFLINE;
                    user->port = 0;
                    user->ip.s_addr = 0;
                    response = 0;
                }
            }
        }
        u_list_ulck();
    } else if (!strcmp(op, "SEND")) {
        rcv_message(socket);
    } else response = 2;

    // Print returns.
    if (strcmp(op, "SEND")) {
        if (response == 0) printf("%s %s OK.\ns> ", op, sender);
        else printf("%s %s FAIL.\ns> ", op, sender);
    }

    write_line(socket, &response, 1);

    close(socket);

    // debug(usr_list);

    // If new connection, flush msg list.
    if (response == 0 && !strcmp(op, "CONNECT")) flush_msg_list(sender);
}

/*
 * @Brief: handle the first part of message forwarding.
 *
 * @Param socket: file descriptor of the socket where the connection is created.
 */
void rcv_message(int socket) {
    char sender[BUFFER_SIZE], receiver[BUFFER_SIZE], msg[BUFFER_SIZE], md5[BUFFER_SIZE];
    uint8_t response;
    node_t *user_s, *user_r;

    // Wipe buffers.
    bzero(sender, BUFFER_SIZE);
    bzero(receiver, BUFFER_SIZE);
    bzero(msg, BUFFER_SIZE);
    bzero(md5, BUFFER_SIZE);

    // Receive all the data.
    if (read_line(socket, sender, BUFFER_SIZE) == -1) response = 2; // Read the username.
    else if (read_line(socket, receiver, BUFFER_SIZE) == -1) response = 2; // Read the username.
    else if (read_line(socket, msg, BUFFER_SIZE) == -1) response = 2; // Read the message.
    else if (read_line(socket, md5, BUFFER_SIZE) == -1) response = 2; // Read the message.
    else {
        // Get the users if exist.
        u_list_lck();
        user_s = get_user(usr_list, sender);
        user_r = get_user(usr_list, receiver);
        u_list_ulck();

        if (user_s == NULL || user_r == NULL) response = 1;
        else if (user_s->status == OFFLINE) response = 2;
        else response = 0;

        if (response != 0) { // Receiver not in the system.
            write_line(socket, &response, 1);
            return;
        } else { // Enqueue the message to receiver msg_list.
            char seq[BUFFER_SIZE];

            if (user_s->seq == 0) { // Something went wrong.
                response = 2;
                write_line(socket, &response, 1);
                return;
            }
            sprintf(seq, "%d", user_s->seq);

            pthread_mutex_lock(&user_r->lock);
            while (user_r->busy) pthread_cond_wait(&user_r->unlock, &user_r->lock);
            ++user_r->busy;

            add_msg(usr_list, sender, receiver, msg, md5, user_s->seq++);

            --user_r->busy;
            pthread_cond_broadcast(&user_r->unlock);
            pthread_mutex_unlock(&user_r->lock);

            write_line(socket, &response, 1);
            write_line(socket, seq, strlen(seq));

            // Call rpc to store the message.
            CLIENT *rpc;
            rpc = clnt_create(rpc_address, MSG_STORE, MSG_STORE_V, "TCP");
            if (rpc == NULL) {
                printf("ERROR , SERVICE  NOT  AVAILABLE\n");
            } else {
                int res;
                insert_1(user_s->usr, user_r->usr, seq, msg, md5, &res, rpc);
                if (res == -1) {
                    perror("Init rpc");
                }

                clnt_destroy(rpc);
            }
        }

        if (user_r->status == ONLINE) flush_msg_list(receiver); // If user connected send msg.
        else {
            int s = user_s->seq;
            printf("MESSAGE %d FROM %s TO %s STORED\ns> ", --s, sender, receiver);
        }
        return;
    }

    write_line(socket, &response, 1);
}

/*
 * @Brief: send all the messages to a user.
 *
 * @Param name: name of the user who is going to receive the messages.
 */
void flush_msg_list(char *name) {
    node_t *user = get_user(usr_list, name);
    msg_t *msg = pop_msg(usr_list, name);


    pthread_mutex_lock(&user->lock);
    while (user->busy) pthread_cond_wait(&user->unlock, &user->lock);
    ++user->busy;

    while (msg != NULL) {
        int sock;
        struct sockaddr_in serv_addr;
        char sequence[BUFFER_SIZE], sender[BUFFER_SIZE];

        sprintf(sequence, "0%d", msg->seq);
        strcpy(sender, msg->from);


        bzero((char *) &serv_addr, sizeof(serv_addr));

        sock = socket(AF_INET, SOCK_STREAM, 0);
        serv_addr.sin_family = AF_INET;
        memcpy((char *) &user->ip, (char *) &serv_addr.sin_addr, sizeof(struct in_addr));
        serv_addr.sin_port = htons(user->port);

        // Connection error. Disconnect user.
        if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            close(sock);

            // Disconnect user.
            user->status = OFFLINE;
            user->port = 0;
            user->ip.s_addr = 0;

            --user->busy;
            pthread_cond_broadcast(&user->unlock);
            pthread_mutex_unlock(&user->lock);
        }

        if (!strcmp(sender, "SEND_MESS_ACK")) {
            char *init = "SEND_MESS_ACK";
            write_line(sock, init, strlen(init));
            write_line(sock, sequence, strlen(sequence));
        } else {
            char *init = "SEND_MESSAGE";

            write_line(sock, init, strlen(init));
            write_line(sock, sender, strlen(sender));
            write_line(sock, sequence, strlen(sequence));
            write_line(sock, msg->md5, strlen(msg->md5));
            write_line(sock, msg->message, strlen(msg->message));
            printf("SEND MESSAGE %s FROM %s TO %s\ns> ", sequence, sender, name);

            add_msg(usr_list, "SEND_MESS_ACK", sender, "SEND_MESS_ACK", "", msg->seq);

            node_t *user_s = get_user(usr_list, sender);
            if (user_s != NULL && user_s->status == ONLINE) // Send the message ACK.
                    flush_msg_list(sender);
        }
        close(sock);

        msg = pop_msg(usr_list, name);
    }

    --user->busy;
    pthread_cond_broadcast(&user->unlock);
    pthread_mutex_unlock(&user->lock);
}

/*
 * @Brief: write a buffer in a line.
 *
 * @Param fd: file descriptor where we are going to write.
 * @Param *buffer: buffer to store the data we want to write.
 * @Param n: size of the buffer.
 *
 * @Return: 0 on succes, -1 upon error.
 */
ssize_t write_line(int fd, void *buffer, size_t n) {
    int last_write = 0;
    size_t length = n;
    char *buff;
    char nl = '\0';

    if (!buffer) {
        errno = EINVAL;
        return -1;
    }

    buff = (char *) buffer;

    while (length > 0 && last_write >= 0) {
        last_write = write(fd, buff, length);
        length -= (size_t ) last_write;
        buff += last_write;
    }

   if (n > 1) write(fd, &nl, 1);

    if (last_write < 0) return -1; // Error.
    else return 0; // Write done.
}

/*
 * @Brief: read a line from a file. In this case we consider the delimiters of a
 *  line the characters: '\n' and '\0'.
 *
 * @Param fd: file descriptor from which we are going to read.
 * @Param *buffer: buffer to store the read data.
 * @Param n: size of the buffer.
 *
 * @Return: number of bytes read. -1 upon error.
 */
ssize_t read_line(int fd, void *buffer, int n) {
    int last_read;
    int total_read;
    char *buff;
    char c;

    if (n < 0 || !buffer) {
        errno = EINVAL;
        return -1;
    }

    buff = buffer;
    bzero(buff, strlen(buff));

    // We try to fill the buffer until (n - 1) to add in the end '\0'.
    for (total_read = 0; total_read < (n - 1); ++total_read) {
        last_read = read(fd, &c, 1); // Read 1 byte.

        if (last_read == -1) { // Checking read errors.
            if (errno == EINTR) continue; // interrupted -> restart read().
            else return -1;
        } else if (last_read == 0) { // We have reached EOF.
            break;
        } else {
            if (c == '\n' || c == '\0') break;
            *buff++ = c;
        }
    }

    *buff = '\0'; // Set the delimiter.
    return total_read;
}

/*
 * @Brief: check that the introduced port is indeed a positive integer number
 *  in the range (2^16 - 1) because ports are 16 bits but port 0 is reserved.
 *
 * @Return: 0 if it is not a valid port, 1 otherwise.
 */
uint8_t check_port(char *port) {
    if (strlen(port) > PORT_SIZE) return 0;
    while (*port)
        if (!isdigit(*port++)) return 0;
    // Make the < 2^16 thing here.
    return 1;
}

/*
 * @Brief: procedure to lock the list of users.
 */
void u_list_lck() {
    pthread_mutex_lock(&user_list_lck);
    while (user_list_worker)
        pthread_cond_wait(&user_list_ulck, &user_list_lck);
    ++user_list_worker;
}

/*
 * @Brief: procedure to lock the list of users.
 */
void u_list_ulck() {
    --user_list_worker;
    pthread_cond_broadcast(&user_list_ulck);
    pthread_mutex_unlock(&user_list_lck);
}
