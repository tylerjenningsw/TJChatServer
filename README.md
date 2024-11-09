# TJ Chat Server

A multi-user TCP chat server implementation supporting real-time messaging, user authentication, and direct messaging capabilities.

## Features

- **User Authentication**
  - User registration with username/password
  - Login/logout functionality 
  - Prevention of duplicate logins
  - Session management

- **Messaging Capabilities**
  - Public chat room messaging
  - Direct messaging between users
  - Message history logging
  - Large message support with automatic chunking

- **Command System**
  - Command prefix customization
  - Built-in command parser
  - Comprehensive help system

### Available Commands

| Command | Description | Example |
|---------|-------------|---------|
| `register` | Create a new user account | `/register username password` |
| `login` | Log in to an existing account | `/login username password` |
| `logout` | Log out of current session | `/logout` |
| `getlist` | View currently active users | `/getlist` |
| `getlog` | Retrieve chat history | `/getlog` |
| `send` | Send a direct message to user | `/send username message` |
| `help` | Display available commands | `/help` |

## Technical Details

- Written in C++ for Windows platforms
- Uses Windows Socket API (Winsock2)
- Supports concurrent client connections
- Buffer size: 256 bytes
- Implements custom message framing protocol

### Message Protocol

Messages are framed using a length-prefixed format:
1. Length byte (1 byte) - indicates message length + 1
2. Message content (up to 254 bytes)
3. Null terminator (1 byte)

Large messages are automatically split into chunks of 253 bytes maximum.

## Error Handling

The server implements robust error handling for:
- Invalid commands
- Connection issues
- Authentication failures
- Buffer overflows
- Duplicate logins
- Disconnection events

## Building the Project

### Prerequisites
- Visual Studio 2022 or later
- Windows SDK 10.0.22000.0 or later
- C++17 compiler support

### Build Steps
1. Open the solution in Visual Studio
2. Select the desired configuration (Debug/Release)
3. Build the solution (F7)

## Usage

1. Start the server executable
2. Clients can connect using any TCP client on the configured port
3. Register a new account using the `/register` command
4. Log in using the `/login` command
5. Use `/help` to see available commands

## Security Considerations

- Passwords are currently stored in plain text (not recommended for production)
- No encryption for network traffic
- Basic input validation implemented

## Future Improvements

- [ ] Add SSL/TLS support
- [ ] Implement password hashing
- [ ] Add persistent storage for user accounts
- [ ] Support for chat rooms/channels
- [ ] Add message encryption
- [ ] Implement rate limiting
- [ ] Add administrative commands

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
