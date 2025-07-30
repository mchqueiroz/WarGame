-----

# Wargame
This is a terminal-based system written in C designed to manage users, classified blocks/reports, aerospace assets, direct messages, and operational groups within a simulated aerospace organization. **Wargame** provides a secure environment for operators to store, retrieve, and share critical information, simulating a strategic command and control interface.

-----



https://github.com/user-attachments/assets/c00112c3-c818-4a78-b2e9-d2b8dd3e16ab




## Features

  * **Operator Management:**

      * Create new operators with distinct usernames, passwords (numeric only), ranks (Recruit, Soldier, Officer, Commander), and assigned units/squadrons.
      * Secure login process for authorized access.
      * List all registered operators with their details, accessible from a central control.
      * Operators can **change their own passwords** for enhanced security.

  * **Block/Report Management:**

      * Create various types of blocks (Public, Private, Unit, Classified, Mission Report, Asset Telemetry, Group Message) with titles, content, and encryption keys.
      * **Access Control:** Blocks have specific access rules based on their type and the operator's rank or unit.
          * **Private:** Only accessible by the owner.
          * **Unit:** Accessible by operators from the same unit as the owner.
          * **Classified:** Requires a minimum access rank.
          * **Asset Telemetry:** Can be linked to a specific aerospace asset.
          * **Group Message:** Accessible by members of a specific group.
      * List all blocks accessible to the logged-in operator.
      * Open and decrypt block content using the correct key.
      * **Edit existing blocks** (title, content, key) for the owner.
      * **Delete blocks** (requires owner and correct key for confirmation).

  * **Aerospace Asset Management:**

      * Create new aerospace assets (Spacecraft, Satellite, Drone, Base) with unique IDs, names, types, statuses (Operational, Damaged, In Maintenance, Lost), and locations.
      * Assets are assigned to specific units.
      * List all aerospace assets belonging to the logged-in operator's unit.

  * **Direct Messaging (DM):**

      * Send **encrypted direct messages** to other registered operators.
      * List and read received direct messages (requires sender's username and message key).

  * **Operational Group Management:**

      * Create private operational groups with a unique name and a creator.
      * Add new members to groups (only by the group creator).
      * List all groups the logged-in operator is a member of, including their members.
      * **Group messages** can be shared via blocks with the `BLOCK_TYPE_GROUP_MESSAGE` type, visible to all group members.

-----

## Getting Started

### Prerequisites

To compile and run **Wargame**, you'll need a C compiler (like GCC).

### Compilation

Navigate to the project directory in your terminal and compile the source code:

```bash
gcc -o wargame main.c -Wall
```

### Running the Application

After successful compilation, you can run the executable:

```bash
./wargame
```

-----

## How to Use

When you run **Wargame**, you'll be presented with the **Main Access Terminal** menu:

```
=================================================================
        WARGAME: AEROSPACE OPERATIONS MANAGEMENT SYSTEM
=================================================================

-----------------------------------------------------------------
                  :: MAIN ACCESS TERMINAL ::
-----------------------------------------------------------------
  [1] Create New Operator
  [2] Initiate Login
  [3] List All Operators (Control Center)
  [0] Shut Down System
-----------------------------------------------------------------
Enter your option >
```

### Initial Setup

1.  **Create a New Operator:** Select option `1` to create your first operator. You'll need to provide a username, a numeric password, confirm the password, choose a rank, and enter a unit/squadron name.
2.  **Log In:** Once an operator is created, select option `2` to log in using their username and password.

### Operator Menu

After a successful login, you'll enter the **Operations Menu**, tailored to the logged-in operator's credentials:

```
=================================================================
  Welcome, Operator [Username] (Rank: [Rank] - Unit: [Unit Name])
=================================================================
                 :: OPERATIONS MENU ::
-----------------------------------------------------------------
  [1] Create New Block/Report
  [2] List Accessible Blocks/Reports
  [3] Open and View Block/Report Content
  [4] Edit an Existing Block/Report
  [5] Delete a Block/Report
-----------------------------------------------------------------
  [6] Create Aerospace Asset (Spacecraft, Satellite, etc.)
  [7] List Unit Aerospace Assets
-----------------------------------------------------------------
  [8] Send Direct Message
  [9] List My Direct Messages
-----------------------------------------------------------------
 [10] Create Operations Group
 [11] Add Member to Group
 [12] List My Groups
-----------------------------------------------------------------
 [13] Change My Password
  [0] Log Out (Exit)
-----------------------------------------------------------------
Enter your option >
```

Follow the on-screen prompts to interact with the system. Pay attention to the `>>>` prefix in messages; these indicate system feedback (success, error, or informational messages).

-----

## File Storage

**Wargame** persists data using binary files in the same directory as the executable:

  * `users.dat`: Stores operator information.
  * `blocks.dat`: Stores all created blocks/reports.
  * `assets.dat`: Stores aerospace asset details.
  * `messages.dat`: Stores direct messages.
  * `groups.dat`: Stores operational group information.
  * `asset_counter.dat`: Stores the counter for generating unique asset IDs.

**Note:** Deleting these `.dat` files will reset the respective data within the system.

-----

## Contributing

This project is a foundational implementation of a terminal-based system. Feel free to fork the repository, open issues, or submit pull requests with improvements.

### Potential Enhancements:

  * **Robust Password Hashing:** Implement a more secure password hashing mechanism instead of plain integer storage.
  * **User Deletion/Modification:** Add functionality to delete or modify operator accounts by designated administrators.
  * **Search Functionality:** Implement search features for blocks, assets, and messages to quickly locate information.
  * **Date/Time Stamps:** Include creation/modification timestamps for blocks and messages for better tracking.
  * **Enhanced Error Handling:** More comprehensive error handling, especially for file operations, to ensure data integrity.
  * **Network Capabilities:** Extend the system to allow communication and data sharing between different terminals.
  * **GUI Interface:** Develop a graphical user interface for a more modern and intuitive user experience.

-----

## License

This project is open-source and available under the [MIT License](https://www.google.com/search?q=LICENSE).

-----
