# Awale Multiplayer Game
## Description

This project is a multiplayer implementation of the Awale board game, using TCP sockets for communication between clients and a central server.

The server manages player connections, username authentication, player availability, challenges, and communication between two players during a game.

Multiple clients can connect to the same server and interact with each other through a command-based interface.

-------

## Launching the Project

The project uses a Makefile to compile both the server and client.

### 1. Compile the project

Run the following command from the root directory:

```bash
make
```

### Available Make Commands

| Command | Description |
|---|---|
| `make` | Compiles the server and client (default command). |
| `make all` | Compiles both executables. |
| `make debug` | Compiles the server and client with debugging symbols (`-g`), useful for debugging with GDB. |
| `make clean` | Removes compiled executables from `bin/` and `debug/`. |
| `make distclean` | Removes executables and all generated object files from `obj/` and `debug/`. |

**Tips:**
- Use `make debug` when investigating crashes or segmentation faults.
- Use `make clean` before rebuilding if you want to remove previous executables.
- Use `make distclean` for a completely fresh rebuild.
- The compiled executables are stored in `bin/` (normal build) or `debug/` (debug build).


------
## Implemented Features
### 1. Client Connection
   - Clients can connect to the server by specifying its IP address and port.
   - TCP sockets are used for network communication.
   - Each client must choose a username before accessing the server.
### 2. Username Authentication
   - The server checks whether a username is already in use.
   - A client cannot connect using a username that is currently being used by another connected player.
   - Disconnected players can reconnect using their previous username.
   - The server assigns an ID to each registered player.
### 3. Player Management
   - The server maintains a table containing connected players.
   - Each player has an associated username, ID, socket descriptor, and availability status.
   - Clients can request a list of currently available players.
   - The list includes the player's username and corresponding ID.
   - Players who are currently playing a game are marked as unavailable.
### 4. Challenge System
   - A connected player can challenge another player using their ID.
   - The challenged player can accept or refuse the invitation.
   - The requesting player waits for the other player's response.
   - If the challenge is accepted, both players enter a game session.
   - Players participating in a game are temporarily marked as unavailable.
### 5. Awale Game
   - Two players can play against each other through the server.
   - The server forwards moves between the two clients.
   - Each client displays the game board locally.
   - The game logic handles moves, scores, and end-game conditions.
   - A game ends when a player reaches the required score or when a player can no longer play.
### 6. Multithreading
   - The server uses POSIX threads (pthread) to handle multiple client connections.
   - Each newly connected client is handled by a separate communication thread.
   - A mutex protects access to the shared player table.
   - A condition variable is used to synchronize access to player registration. 
   
------
## Client Commands

Once connected and authenticated, the following commands are available:


| Command | Description |
|---|---|
|l|Display the list of available players, including their IDs.|
|c|Check for an incoming challenge.|
|x|Challenge another player by entering their ID.|
|q|Disconnect from the server.|

During a challenge, the challenged player can respond with:

| Input | Description |
|-------|---|
| y     |Accept the challenge.|
| n     |Refuse the challenge.|

------
## Known Bugs and Issues

The following issues are currently known and still need to be investigated or fixed.

### 1. Challenge System
   - Deadlock when checking for challenges:
   - The client can get stuck when attempting to check whether someone has challenged them.
   - Deadlock when a challenge is unanswered:
   - If a player sends a challenge and the targeted player does not respond, the requesting client can remain blocked indefinitely.
   - Potential crash when challenging yourself:
   - Attempting to challenge your own player ID may cause the client or server to crash.
   - Challenge response handling:
   - The communication between the challenger, the challenged player, and the server can become inconsistent in certain situations.
### 2. Game Synchronization
   - Winner notification missing:
     - At the end of an Awale game, the winner is not properly notified.
   - Server/client synchronization issue:
     - After a game finishes, the server can become confused about which client is communicating or which communication thread should handle the socket.
   - Potential communication thread issues:
     - The server transfers responsibility for a client's socket between threads during a challenge. This can lead to synchronization problems.
### 3. Input Handling
   - Multiple characters entered for commands:
     - When entering commands, avoid typing more than one character at a time.
     - Additional characters may remain buffered in the input stream and cause unexpected input to be processed later.
   - Unexpected server messages:
     - Buffered input or unexpected messages can cause the server and client to become desynchronized, resulting in strange responses.


------
## Technical Information
Technologies
Language: C
Networking: TCP sockets
Concurrency: POSIX Threads (pthread)
Synchronization: Mutexes and condition variables
I/O multiplexing: poll()
Architecture

### The project follows a client-server architecture:

#### Server:

Accepts incoming TCP connections.
Authenticates players.
Maintains the player table.
Handles player availability.
Manages challenges.
Forwards game moves between clients.

#### Client:

Connects to the server.
Authenticates using a username.
Displays available players.
Sends and receives challenges.
Runs the local Awale game interface.
Exchanges game moves with the opponent through the server.
Current Development Status

### The project currently supports:

Multiple client connections.
Username authentication and duplicate username prevention.
Player listing.
Challenge requests and responses.
Two-player Awale games.
Server-side game communication.

However, the project is still under development.

----------