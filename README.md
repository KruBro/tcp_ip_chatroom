# TCP/IP Concurrent Chatroom

This project is a multi-process, concurrent client-server chat application written in C. It uses POSIX sockets for network communication, `fork()` for concurrency, and anonymous pipes for Inter-Process Communication (IPC). It relies on a custom binary protocol and a flat-file database to handle user authentication, single chats, group chats, and online user tracking.

---

## 1. System Architecture & Pipelining

The server uses a multi-process architecture to handle many clients at once without slowing down. It does this by dividing the work between a "Parent" process and many "Child" processes.

### The Pipelining Flow (IPC)

Because processes cannot easily share memory, they communicate using pipes.

1. **The Parent Process (The Router):**
The server starts and waits for new network connections. When a client connects, the server accepts the connection and immediately calls `fork()` to create a new child process. The parent then goes back to waiting for more connections or internal messages.
2. **The Child Process (The Worker):**
The child process takes full ownership of the connected client. It waits for the client to send network messages.
3. **Uplink (Child to Parent):**
When a child receives a chat message from its client, it writes that message into a shared pipe called the **Uplink**. The parent process is constantly monitoring this Uplink using the `select()` function.
4. **Downlink (Parent to Child):**
When the parent reads a message from the Uplink, it looks at the target username. It looks up which child process belongs to that target user. The parent then sends the message down a dedicated **Downlink** pipe specific to that child. The child reads it from the Downlink and sends it over the TCP socket to the actual client.

---

## 2. How to Build and Run

### Building the Project

The project includes a `makefile` to automate compilation. Run the following command in the root directory:

```bash
make

```

This will compile the source code and generate two executable files: `server_app` and `client_app`.

### Running the Server

You must start the server before any clients can connect.

```bash
./server_app

```

The server will print logs to the terminal showing that it has created a socket, bound to the local network, and is listening for connections.

### Running the Client

Open a new terminal window and run:

```bash
./client_app

```

You will be greeted with the starting menu. You must either Register a new account or Login to an existing one. Once logged in, you can send messages to other logged-in users.

---

## 3. Directory Structure

* **`client/`**: Contains all the code for the user-facing application.
* **`server/`**: Contains the core logic for routing messages, handling processes, and managing the database.
* **`common/`**: Contains code shared by both the client and the server, such as network wrappers, validation rules, and the protocol rules.
* **`db/`**: Stores the `users.db` text file where user credentials and statuses are saved.

---

## 4. Function Breakdown

Here is a detailed explanation of every major function in the code, written simply.

### Server Functions (`server/`)

**`server.c`**

* **`main()`**: The starting point of the server. It sets up the main TCP socket, binds it to an IP and Port, and sets it to listen. It creates the main Uplink pipe. It then enters a continuous `while(1)` loop, using `select()` to watch for new client connections or incoming messages on the Uplink pipe. When a new client connects, it calls `fork()`.
* **`signal_handler(int signum)`**: A safety net. When a child process disconnects and dies, this function cleans up its leftover resources so the server does not get clogged with "zombie" processes.

**`server_auth.c`**

* **`handle_auth_request()`**: When a new client connects, this is the first function the child process calls. It looks at the client's request to see if they want to Register or Login. It checks the password in the database and returns a success or failure reply.

**`server_chat.c`**

* **`run_chat_session()`**: The main loop for a child process after a successful login. It uses `select()` to wait for two things:
1. A network message from the user (which it then forwards to the parent via the Uplink).
2. An internal message from the parent (which it then forwards to the user over the network).



**`server_ipc.c`**

* **`add_client()`**: Saves a new child process's Process ID (PID) and its dedicated Downlink pipe into a tracking array so the parent knows how to reach it later.
* **`remove_client()`**: Deletes a child process from the tracking array when a user logs out or disconnects.
* **`mark_client_logged_in()`**: Links a specific username to a specific child process in the tracking array.
* **`find_downlink_fd()`**: A search function used by the parent. It takes a username, searches the tracking array, and returns the correct Downlink pipe so the parent can route a private message to them.
* **`get_online_downlink_fds()`**: Used for group chats. It returns a list of every active Downlink pipe except the sender's, so the parent can broadcast a message to everyone.

**`server_db.c`**

* **`db_register()`**: Opens the `users.db` file. It locks the file to prevent corruption, checks if the username already exists, and if not, adds a new line with the username and password.
* **`db_login()`**: Opens `users.db`, searches for the username, and checks if the provided password matches the saved password.
* **`db_set_user_status()`**: Changes a user's state to "ONLINE" or "OFFLINE" in the database. It does this safely by writing everything to a temporary file (`users.db.tmp`), updating the target user's line, and then renaming the temporary file to overwrite the old one.
* **`db_get_online_users()`**: Opens `users.db`, scans line by line for any user with an "ONLINE" tag, and formats their names into a single text string to send back to the user.

### Client Functions (`client/`)

**`client.c`**

* **`main()`**: The starting point of the client. It prompts the user to log in. Once logged in, it enters a `while(1)` loop. It uses `select()` to monitor both the keyboard (Standard Input) and the network socket. This allows the client to receive incoming messages instantly even while the user is sitting idle at the menu.
* **`send_auth_request()`**: Packages the user's typed username and password into a struct, connects to the server's IP address, sends the package, and waits for a Yes/No reply.

**`client_ui.c`**

* **`prompt_login_credentials()`**: Prints the initial "Register/Login" menu and safely reads the user's keyboard input, filtering out bad characters.
* **`display_chat_menu()`**: Prints the options: Single Chat, Group Chat, List Online Users, Logout.
* **`prompt_chat_command()`**: Asks the user who they want to message and what they want to say. It packages this into a `Msg` struct ready to be sent over the wire.
* **`display_incoming_message()`**: Prints incoming network messages to the screen neatly.

### Common Utility Functions (`common/`)

**`netutils.c`**

* **`readn()`**: A wrapper for the standard `read()` function. Network data sometimes arrives in chunks. This function uses a loop to guarantee that exactly `n` bytes are read before moving on, preventing broken or half-read messages.
* **`writen()`**: A wrapper for the standard `write()` function. It guarantees that exactly `n` bytes are pushed onto the network wire, even if the operating system tries to pause the transmission.

**`validation.c`**

* **`is_valid_field()`**: A security check. It ensures usernames and passwords only contain letters, numbers, and underscores to prevent crashes or database injection errors.

---

## 5. The Wire Protocol

Because C uses raw bytes over a socket, both the client and server must agree on the exact size and shape of the data being sent. This is defined in `common/protocol.h`.

* **`SignInType`**: Sent from Client to Server. Contains the Username, Password, and whether the user is trying to Register or Login.
* **`Reply`**: Sent from Server to Client. Contains a single number (enum code) that translates to things like `REPLY_LOGIN_SUCCESS` or `REPLY_PASSWORD_INCORRECT`.
* **`Msg`**: The core chat packet. Contains the Message Text, the Sender's Name, the Receiver's Name, and the type of chat (Single, Group, List Users).
* **`IpcUplinkMsg`**: A special packet only used internally by the server's pipes. It wraps a standard `Msg` but adds the Process ID (PID) so the parent knows which child sent it.