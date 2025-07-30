#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Max string lengths
#define MAX_USERNAME 100
#define MAX_UNIT_NAME 50
#define MAX_BLOCK_TITLE 100
#define MAX_BLOCK_CONTENT 1000
#define MAX_ASSET_NAME 50
#define MAX_ASSET_LOCATION 50
#define ID_LEN 10
#define MAX_GROUP_NAME 50
#define MAX_GROUP_MEMBERS 10

// Enums for ranks and types
typedef enum {
    RANK_RECRUIT, RANK_SOLDIER, RANK_OFFICER, RANK_COMMANDER, RANK_MAX
} Rank;

typedef enum {
    BLOCK_TYPE_PUBLIC, BLOCK_TYPE_PRIVATE, BLOCK_TYPE_UNIT,
    BLOCK_TYPE_CLASSIFIED, BLOCK_TYPE_MISSION_REPORT,
    BLOCK_TYPE_ASSET_TELEMETRY, BLOCK_TYPE_GROUP_MESSAGE, BLOCK_TYPE_MAX
} BlockType;

typedef enum {
    ASSET_TYPE_SPACECRAFT, ASSET_TYPE_SATELLITE, ASSET_TYPE_DRONE, ASSET_TYPE_BASE, ASSET_TYPE_MAX
} AssetType;

typedef enum {
    ASSET_STATUS_OPERATIONAL, ASSET_STATUS_DAMAGED, ASSET_STATUS_MAINTENANCE, ASSET_STATUS_LOST, ASSET_STATUS_MAX
} AssetStatus;

// Structures
typedef struct {
    char username[MAX_USERNAME];
    int password; // Future Improvement: Implement password hashing
    Rank rank;
    char unit[MAX_UNIT_NAME];
} User;

typedef struct {
    char owner[MAX_USERNAME];
    char title[MAX_BLOCK_TITLE];
    char content[MAX_BLOCK_CONTENT];
    int key; // Future Improvement: Consider more robust key management
    BlockType type;
    Rank min_access_rank;
    char linked_asset_id[ID_LEN];
    char group_destination_name[MAX_GROUP_NAME];
} Block;

typedef struct {
    char id[ID_LEN];
    char name[MAX_ASSET_NAME];
    AssetType type;
    AssetStatus status;
    char location[MAX_ASSET_LOCATION];
    char owner_unit[MAX_UNIT_NAME];
} AerospaceAsset;

typedef struct {
    char sender[MAX_USERNAME];
    char receiver[MAX_USERNAME];
    char message[MAX_BLOCK_CONTENT];
    int key; // Future Improvement: Consider more robust key management
} DirectMessage;

typedef struct {
    char name[MAX_GROUP_NAME];
    char creator[MAX_USERNAME];
    char members[MAX_GROUP_MEMBERS][MAX_USERNAME];
    int num_members;
} Grupo;

// Global asset counter (persisted to file)
static int g_asset_counter = 0;

// Utility functions
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Function to safely read an integer from stdin
int getIntegerInput() {
    int value;
    int result = scanf("%d", &value);
    if (result != 1) {
        printf(">>> Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return -1; // Indicate an invalid input
    }
    clearInputBuffer();
    return value;
}

void loadAssetCounter() {
    FILE *f = fopen("asset_counter.dat", "rb");
    if (f) {
        fread(&g_asset_counter, sizeof(int), 1, f);
        fclose(f);
    } else {
        g_asset_counter = 0;
    }
}

void saveAssetCounter() {
    FILE *f = fopen("asset_counter.dat", "wb");
    if (f) {
        fwrite(&g_asset_counter, sizeof(int), 1, f);
        fclose(f);
    } else {
        printf(">>> Error: Could not save asset counter.\n");
    }
}

const char* rankToString(Rank r) {
    switch (r) {
        case RANK_RECRUIT: return "Recruit";
        case RANK_SOLDIER: return "Soldier";
        case RANK_OFFICER: return "Officer";
        case RANK_COMMANDER: return "Commander";
        default: return "Unknown";
    }
}

Rank stringToRank(const char* s) {
    if (strcmp(s, "Recruit") == 0) return RANK_RECRUIT;
    if (strcmp(s, "Soldier") == 0) return RANK_SOLDIER;
    if (strcmp(s, "Officer") == 0) return RANK_OFFICER;
    if (strcmp(s, "Commander") == 0) return RANK_COMMANDER;
    return RANK_RECRUIT; // Default to Recruit if not found
}

const char* blockTypeToString(BlockType bt) {
    switch (bt) {
        case BLOCK_TYPE_PUBLIC: return "Public";
        case BLOCK_TYPE_PRIVATE: return "Private";
        case BLOCK_TYPE_UNIT: return "Unit";
        case BLOCK_TYPE_CLASSIFIED: return "Classified";
        case BLOCK_TYPE_MISSION_REPORT: return "Mission Report";
        case BLOCK_TYPE_ASSET_TELEMETRY: return "Asset Telemetry";
        case BLOCK_TYPE_GROUP_MESSAGE: return "Group Message";
        default: return "Unknown";
    }
}

const char* assetTypeToString(AssetType at) {
    switch (at) {
        case ASSET_TYPE_SPACECRAFT: return "Spacecraft";
        case ASSET_TYPE_SATELLITE: return "Satellite";
        case ASSET_TYPE_DRONE: return "Drone";
        case ASSET_TYPE_BASE: return "Base";
        default: return "Unknown";
    }
}

const char* assetStatusToString(AssetStatus as) {
    switch (as) {
        case ASSET_STATUS_OPERATIONAL: return "Operational";
        case ASSET_STATUS_DAMAGED: return "Damaged";
        case ASSET_STATUS_MAINTENANCE: return "In Maintenance";
        case ASSET_STATUS_LOST: return "Lost";
        default: return "Unknown";
    }
}

void encrypt(char *text, int key) {
    for (int i = 0; text[i] != '\0'; i++) {
        text[i] += key;
    }
}

void decrypt(char *text, int key) {
    for (int i = 0; text[i] != '\0'; i++) {
        text[i] -= key;
    }
}

// User functions
int findUserByUsername(const char *username, User *u_out) {
    FILE *f = fopen("users.dat", "rb");
    if (!f) return 0;
    User u;
    while (fread(&u, sizeof(User), 1, f)) {
        if (strcmp(u.username, username) == 0) {
            *u_out = u;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int userExists(const char *username) {
    User u_dummy;
    return findUserByUsername(username, &u_dummy);
}

int login(char *loggedUsername, Rank *loggedRank, char *loggedUnit) {
    char username_input[MAX_USERNAME];
    int password;

    printf("\n--- Operator Access ---\n");
    printf("Username: ");
    fgets(username_input, MAX_USERNAME, stdin);
    username_input[strcspn(username_input, "\n")] = 0;

    printf("Password (numeric only): ");
    password = getIntegerInput();
    if (password == -1) return 0;

    FILE *f = fopen("users.dat", "rb");
    if (!f) {
        printf(">>> Error: Could not open user file.\n");
        return 0;
    }

    User u;
    while (fread(&u, sizeof(User), 1, f)) {
        if (strcmp(u.username, username_input) == 0 && u.password == password) {
            strncpy(loggedUsername, u.username, MAX_USERNAME - 1);
            loggedUsername[MAX_USERNAME - 1] = '\0';
            *loggedRank = u.rank;
            strncpy(loggedUnit, u.unit, MAX_UNIT_NAME - 1);
            loggedUnit[MAX_UNIT_NAME - 1] = '\0';
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    printf(">>> Login failed. Incorrect username or password.\n");
    return 0;
}

void createUser() {
    User u;
    int passwordConfirmation;
    int rank_option;
    char username_input[MAX_USERNAME];
    char unit_input[MAX_UNIT_NAME];

    printf("\n--- Create New Operator ---\n");
    printf("Username (max. %d characters): ", MAX_USERNAME - 1);
    fgets(username_input, MAX_USERNAME, stdin);
    username_input[strcspn(username_input, "\n")] = 0;

    if (strlen(username_input) >= MAX_USERNAME) {
        printf(">>> Error: Username too long. Max: %d.\n", MAX_USERNAME - 1);
        return;
    }
    strncpy(u.username, username_input, MAX_USERNAME - 1);
    u.username[MAX_USERNAME - 1] = '\0';

    if (userExists(u.username)) {
        printf(">>> Error: User '%s' already exists. Please choose another username.\n", u.username);
        return;
    }

    printf("Password (numeric only): ");
    u.password = getIntegerInput();
    if (u.password == -1) return;

    printf("Confirm password: ");
    passwordConfirmation = getIntegerInput();
    if (passwordConfirmation == -1) return;

    if (u.password != passwordConfirmation) {
        printf(">>> Passwords do not match. Operator not created.\n");
        return;
    }

    printf("\nSelect operator rank:\n");
    for (int i = 0; i < RANK_MAX; i++) {
        printf("  %d - %s\n", i + 1, rankToString((Rank)i));
    }
    printf("Option > ");
    rank_option = getIntegerInput();

    if (rank_option < 1 || rank_option > RANK_MAX) {
        printf(">>> Invalid rank option. Setting to Recruit.\n");
        u.rank = RANK_RECRUIT;
    } else {
        u.rank = (Rank)(rank_option - 1);
    }

    printf("Unit/Squadron (max. %d characters): ", MAX_UNIT_NAME - 1);
    fgets(unit_input, MAX_UNIT_NAME, stdin);
    unit_input[strcspn(unit_input, "\n")] = 0;

    if (strlen(unit_input) >= MAX_UNIT_NAME) {
        printf(">>> Error: Unit name too long. Max: %d. Setting to 'N/A'.\n", MAX_UNIT_NAME - 1);
        strncpy(u.unit, "N/A", MAX_UNIT_NAME - 1);
        u.unit[MAX_UNIT_NAME - 1] = '\0';
    } else {
        strncpy(u.unit, unit_input, MAX_UNIT_NAME - 1);
        u.unit[MAX_UNIT_NAME - 1] = '\0';
    }

    FILE *f = fopen("users.dat", "ab");
    if (!f) {
        printf(">>> Error: Could not create user file.\n");
        return;
    }
    fwrite(&u, sizeof(User), 1, f);
    fclose(f);

    printf(">>> Operator '%s' (%s, %s) created successfully!\n", u.username, rankToString(u.rank), u.unit);
}

void listUsers() {
    FILE *f = fopen("users.dat", "rb");
    if (!f) {
        printf("\n=== Registered Operators ===\n");
        printf("No operators registered yet.\n");
        return;
    }

    User u;
    printf("\n=== List of Registered Operators ===\n");
    printf("----------------------------------------\n");
    printf("Username                Rank           Unit\n");
    printf("----------------------------------------\n");
    int count = 0;
    while (fread(&u, sizeof(User), 1, f)) {
        printf("%-20s%-15s%-20s\n", u.username, rankToString(u.rank), u.unit);
        count++;
    }
    fclose(f);
    printf("----------------------------------------\n");
    if (count == 0) {
        printf("No operators registered yet.\n");
    } else {
        printf("Total operators: %d\n", count);
    }
}

void changeUserPassword(const char *username) {
    int currentPassword, newPassword, confirmNewPassword;

    printf("\n--- Change Operator Password ---\n");
    printf("Enter your current password: ");
    currentPassword = getIntegerInput();
    if (currentPassword == -1) return;

    FILE *f = fopen("users.dat", "r+b");
    if (!f) {
        printf(">>> Error: Could not open user file.\n");
        return;
    }

    User u;
    long pos = -1;
    while (fread(&u, sizeof(User), 1, f)) {
        if (strcmp(u.username, username) == 0 && u.password == currentPassword) {
            pos = ftell(f) - sizeof(User);
            break;
        }
    }

    if (pos == -1) {
        printf(">>> Incorrect current password or operator not found.\n");
        fclose(f);
        return;
    }

    printf("Enter new password (numeric only): ");
    newPassword = getIntegerInput();
    if (newPassword == -1) {
        fclose(f);
        return;
    }

    printf("Confirm new password: ");
    confirmNewPassword = getIntegerInput();
    if (confirmNewPassword == -1) {
        fclose(f);
        return;
    }

    if (newPassword != confirmNewPassword) {
        printf(">>> New password and confirmation do not match. Password not changed.\n");
        fclose(f);
        return;
    }

    u.password = newPassword;
    fseek(f, pos, SEEK_SET);
    fwrite(&u, sizeof(User), 1, f);
    fclose(f);
    printf(">>> Password changed successfully!\n");
}

// Block functions
void createBlock(const char *block_owner_username, Rank owner_rank, const char *owner_unit) {
    Block b;
    strncpy(b.owner, block_owner_username, MAX_USERNAME - 1);
    b.owner[MAX_USERNAME - 1] = '\0';

    char title_input[MAX_BLOCK_TITLE];
    char content_input[MAX_BLOCK_CONTENT];

    printf("\n--- Create New Block/Report ---\n");
    printf("Block title (max. %d characters): ", MAX_BLOCK_TITLE - 1);
    fgets(title_input, MAX_BLOCK_TITLE, stdin);
    title_input[strcspn(title_input, "\n")] = 0;

    if (strlen(title_input) >= MAX_BLOCK_TITLE) {
        printf(">>> Error: Block title too long. Max: %d.\n", MAX_BLOCK_TITLE - 1);
        return;
    }
    strncpy(b.title, title_input, MAX_BLOCK_TITLE - 1);
    b.title[MAX_BLOCK_TITLE - 1] = '\0';

    FILE *check_f = fopen("blocks.dat", "rb");
    if (check_f) {
        Block temp_b;
        while (fread(&temp_b, sizeof(Block), 1, check_f)) {
            if (strcmp(temp_b.owner, block_owner_username) == 0 && strcmp(temp_b.title, b.title) == 0) {
                printf(">>> A block with this title already exists for you. Choose another.\n");
                fclose(check_f);
                return;
            }
        }
        fclose(check_f);
    }

    printf("Content (max. %d characters):\n", MAX_BLOCK_CONTENT - 1);
    printf(">>> ");
    fgets(content_input, MAX_BLOCK_CONTENT, stdin);
    content_input[strcspn(content_input, "\n")] = 0;

    if (strlen(content_input) >= MAX_BLOCK_CONTENT) {
        printf(">>> Error: Block content too long. Max: %d. Please try again.\n", MAX_BLOCK_CONTENT - 1);
        return;
    }
    strncpy(b.content, content_input, MAX_BLOCK_CONTENT - 1);
    b.content[MAX_BLOCK_CONTENT - 1] = '\0';

    printf("Numeric key for encryption: ");
    b.key = getIntegerInput();
    if (b.key == -1) return;

    printf("\nChoose BLOCK TYPE:\n");
    for (int i = 0; i < BLOCK_TYPE_MAX; i++) {
        printf("  %d - %s\n", i + 1, blockTypeToString((BlockType)i));
    }
    printf("Option > ");
    int type_option = getIntegerInput();

    if (type_option < 1 || type_option > BLOCK_TYPE_MAX) {
        printf(">>> Invalid type option. Setting to Public.\n");
        b.type = BLOCK_TYPE_PUBLIC;
    } else {
        b.type = (BlockType)(type_option - 1);
    }

    strncpy(b.linked_asset_id, "N/A", ID_LEN - 1);
    b.linked_asset_id[ID_LEN - 1] = '\0';
    b.min_access_rank = RANK_RECRUIT;
    strncpy(b.group_destination_name, "N/A", MAX_GROUP_NAME - 1);
    b.group_destination_name[MAX_GROUP_NAME - 1] = '\0';

    if (b.type == BLOCK_TYPE_CLASSIFIED) {
        printf("Minimum rank for access (1-Recruit, 2-Soldier, 3-Officer, 4-Commander): ");
        int min_rank_opt = getIntegerInput();
        if (min_rank_opt == -1) return;

        if (min_rank_opt >= 1 && min_rank_opt <= RANK_MAX) {
            b.min_access_rank = (Rank)(min_rank_opt - 1);
        } else {
            printf(">>> Invalid option. Setting minimum access to Officer.\n");
            b.min_access_rank = RANK_OFFICER;
        }
    } else if (b.type == BLOCK_TYPE_ASSET_TELEMETRY) {
        char asset_id_input[ID_LEN];
        printf("Linked Aerospace Asset ID (e.g., ASSET001): ");
        fgets(asset_id_input, ID_LEN, stdin);
        asset_id_input[strcspn(asset_id_input, "\n")] = 0;

        if (strlen(asset_id_input) >= ID_LEN) {
            printf(">>> Asset ID too long. Max: %d. Setting to 'N/A'.\n", ID_LEN - 1);
            strncpy(b.linked_asset_id, "N/A", ID_LEN - 1);
            b.linked_asset_id[ID_LEN - 1] = '\0';
        } else {
            strncpy(b.linked_asset_id, asset_id_input, ID_LEN - 1);
            b.linked_asset_id[ID_LEN - 1] = '\0';
        }
    } else if (b.type == BLOCK_TYPE_GROUP_MESSAGE) {
        char group_name_input[MAX_GROUP_NAME];
        printf("Group Name for the message: ");
        fgets(group_name_input, MAX_GROUP_NAME, stdin);
        group_name_input[strcspn(group_name_input, "\n")] = 0;

        if (strlen(group_name_input) >= MAX_GROUP_NAME) {
            printf(">>> Error: Group destination name too long. Max: %d. Setting to 'N/A'.\n", MAX_GROUP_NAME - 1);
            strncpy(b.group_destination_name, "N/A", MAX_GROUP_NAME - 1);
            b.group_destination_name[MAX_GROUP_NAME - 1] = '\0';
        } else {
            strncpy(b.group_destination_name, group_name_input, MAX_GROUP_NAME - 1);
            b.group_destination_name[MAX_GROUP_NAME - 1] = '\0';
        }
    }

    encrypt(b.content, b.key);

    FILE *f = fopen("blocks.dat", "ab");
    if (!f) {
        printf(">>> Error: Could not create block file.\n");
        return;
    }
    fwrite(&b, sizeof(Block), 1, f);
    fclose(f);

    printf(">>> Block '%s' (%s) created successfully!\n", b.title, blockTypeToString(b.type));
}

// Group auxiliary functions
int groupExists(const char* group_name) {
    FILE *f = fopen("groups.dat", "rb");
    if (!f) return 0;
    Grupo g;
    while (fread(&g, sizeof(Grupo), 1, f)) {
        if (strcmp(g.name, group_name) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int findGroupByName(const char* group_name, Grupo* g_out) {
    FILE *f = fopen("groups.dat", "rb");
    if (!f) return 0;
    Grupo g;
    while (fread(&g, sizeof(Grupo), 1, f)) {
        if (strcmp(g.name, group_name) == 0) {
            *g_out = g;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int isUserInGroup(const char* username, const char* group_name) {
    Grupo g;
    if (!findGroupByName(group_name, &g)) {
        return 0;
    }
    if (strcmp(g.creator, username) == 0) {
        return 1;
    }
    for (int i = 0; i < g.num_members; i++) {
        if (strcmp(g.members[i], username) == 0) {
            return 1;
        }
    }
    return 0;
}

void listBlocks(const char *logged_username, Rank logged_rank, const char *logged_unit) {
    FILE *f = fopen("blocks.dat", "rb");
    if (!f) {
        printf("\n=== Your Blocks and Accessible Blocks ===\n");
        printf("No blocks found.\n");
        return;
    }

    Block b;
    int found_any_block = 0;
    printf("\n=== Your Blocks and Accessible Blocks, %s ===\n", logged_username);
    printf("--------------------------------------------------------------------------------\n");
    printf("Title                    Owner                Type               Min. Access/Asset/Group\n");
    printf("--------------------------------------------------------------------------------\n");
    while (fread(&b, sizeof(Block), 1, f)) {
        int can_view = 0;
        if (strcmp(b.owner, logged_username) == 0) {
            can_view = 1;
        } else if (b.type == BLOCK_TYPE_PUBLIC) {
            can_view = 1;
        } else if (b.type == BLOCK_TYPE_UNIT) {
            User block_owner_user;
            if (findUserByUsername(b.owner, &block_owner_user) && strcmp(block_owner_user.unit, logged_unit) == 0) {
                can_view = 1;
            }
        } else if (b.type == BLOCK_TYPE_CLASSIFIED && logged_rank >= b.min_access_rank) {
            can_view = 1;
        } else if (b.type == BLOCK_TYPE_GROUP_MESSAGE) {
            if (isUserInGroup(logged_username, b.group_destination_name)) {
                can_view = 1;
            }
        }

        if (can_view) {
            char extra_info[MAX_BLOCK_TITLE + MAX_ASSET_NAME + MAX_GROUP_NAME + 50] = "";
            if (b.type == BLOCK_TYPE_CLASSIFIED) {
                snprintf(extra_info, sizeof(extra_info), "Min. Rank: %s", rankToString(b.min_access_rank));
            } else if (b.type == BLOCK_TYPE_ASSET_TELEMETRY && strcmp(b.linked_asset_id, "N/A") != 0) {
                snprintf(extra_info, sizeof(extra_info), "Asset: %s", b.linked_asset_id);
            } else if (b.type == BLOCK_TYPE_GROUP_MESSAGE && strcmp(b.group_destination_name, "N/A") != 0) {
                snprintf(extra_info, sizeof(extra_info), "Group: %s", b.group_destination_name);
            }
            printf("%-25s%-20s%-18s%s\n", b.title, b.owner, blockTypeToString(b.type), extra_info);
            found_any_block = 1;
        }
    }
    fclose(f);
    printf("--------------------------------------------------------------------------------\n");
    if (!found_any_block) {
        printf("No blocks found for you or no accessible blocks.\n");
    }
}

void openBlock(const char *logged_username, Rank logged_rank, const char *logged_unit) {
    char title_input[MAX_BLOCK_TITLE];
    int provided_key;

    printf("\n--- Open Block/Report ---\n");
    printf("Enter the title of the block to open: ");
    fgets(title_input, MAX_BLOCK_TITLE, stdin);
    title_input[strcspn(title_input, "\n")] = 0;

    printf("Enter the decryption key: ");
    provided_key = getIntegerInput();
    if (provided_key == -1) return;

    FILE *f = fopen("blocks.dat", "rb");
    if (!f) {
        printf(">>> Error: Could not open block file.\n");
        return;
    }

    Block b;
    int found = 0;
    while (fread(&b, sizeof(Block), 1, f)) {
        if (strcmp(b.title, title_input) == 0) {
            int can_open = 0;
            if (strcmp(b.owner, logged_username) == 0) {
                can_open = 1;
            } else if (b.type == BLOCK_TYPE_PUBLIC) {
                can_open = 1;
            } else if (b.type == BLOCK_TYPE_UNIT) {
                User block_owner_user;
                if (findUserByUsername(b.owner, &block_owner_user) && strcmp(block_owner_user.unit, logged_unit) == 0) {
                    can_open = 1;
                }
            } else if (b.type == BLOCK_TYPE_CLASSIFIED && logged_rank >= b.min_access_rank) {
                can_open = 1;
            } else if (b.type == BLOCK_TYPE_GROUP_MESSAGE) {
                if (isUserInGroup(logged_username, b.group_destination_name)) {
                    can_open = 1;
                }
            }

            if (can_open) {
                if (b.key == provided_key) {
                    decrypt(b.content, provided_key);
                    printf("\n=== Content of Block '%s' (Owner: %s) ===\n", b.title, b.owner);
                    printf("Type: %s\n", blockTypeToString(b.type));
                    if (b.type == BLOCK_TYPE_CLASSIFIED) {
                        printf("Minimum Access Rank: %s\n", rankToString(b.min_access_rank));
                    }
                    if (b.type == BLOCK_TYPE_ASSET_TELEMETRY && strcmp(b.linked_asset_id, "N/A") != 0) {
                        printf("Linked Asset: %s\n", b.linked_asset_id);
                    }
                    if (b.type == BLOCK_TYPE_GROUP_MESSAGE && strcmp(b.group_destination_name, "N/A") != 0) {
                        printf("Destination Group: %s\n", b.group_destination_name);
                    }
                    printf("-----------------------------------------------------\n");
                    printf("%s\n", b.content);
                    printf("-----------------------------------------------------\n");
                    printf("Encryption Key: %d (Keep it safe!)\n", b.key);
                    found = 1;
                } else {
                    printf(">>> Incorrect decryption key for this block.\n");
                }
            } else {
                printf(">>> You do not have permission to open this block.\n");
            }
            break;
        }
    }
    fclose(f);
    if (!found) {
        printf(">>> Block '%s' not found.\n", title_input);
    }
}

void editBlock(const char *logged_username, Rank logged_rank, const char *logged_unit) {
    char oldTitle_input[MAX_BLOCK_TITLE];
    char newContentTemp[MAX_BLOCK_CONTENT];
    char newTitleTemp[MAX_BLOCK_TITLE];
    int newKey;
    int currentKey;

    printf("\n--- Edit Existing Block/Report ---\n");
    printf("Enter the title of the block you want to edit: ");
    fgets(oldTitle_input, MAX_BLOCK_TITLE, stdin);
    oldTitle_input[strcspn(oldTitle_input, "\n")] = 0;

    printf("Enter the current encryption key for the block: ");
    currentKey = getIntegerInput();
    if (currentKey == -1) return;

    FILE *f = fopen("blocks.dat", "r+b");
    if (!f) {
        printf(">>> Error: Could not open block file.\n");
        return;
    }

    Block b;
    long pos = -1;
    while (fread(&b, sizeof(Block), 1, f)) {
        if (strcmp(b.owner, logged_username) == 0 && strcmp(b.title, oldTitle_input) == 0) {
            if (b.key == currentKey) {
                pos = ftell(f) - sizeof(Block);
                break;
            } else {
                printf(">>> Incorrect encryption key for this block.\n");
                fclose(f);
                return;
            }
        }
    }

    if (pos == -1) {
        printf(">>> Block '%s' not found or does not belong to you.\n", oldTitle_input);
        fclose(f);
        return;
    }

    decrypt(b.content, b.key);

    printf("Enter new title (leave blank to keep '%s', max. %d characters): ", b.title, MAX_BLOCK_TITLE - 1);
    fgets(newTitleTemp, MAX_BLOCK_TITLE, stdin);
    newTitleTemp[strcspn(newTitleTemp, "\n")] = 0;

    if (strlen(newTitleTemp) > 0) {
        if (strlen(newTitleTemp) >= MAX_BLOCK_TITLE) {
            printf(">>> Error: New title too long. Max: %d. Keeping old title.\n", MAX_BLOCK_TITLE - 1);
        } else {
            FILE *check_f = fopen("blocks.dat", "rb");
            if (check_f) {
                Block temp_b;
                int title_exists = 0;
                while (fread(&temp_b, sizeof(Block), 1, check_f)) {
                    if (strcmp(temp_b.owner, logged_username) == 0 && strcmp(temp_b.title, newTitleTemp) == 0 && strcmp(temp_b.title, b.title) != 0) {
                        title_exists = 1;
                        break;
                    }
                }
                fclose(check_f);
                if (title_exists) {
                    printf(">>> A block with the new title already exists for you. Keeping old title.\n");
                } else {
                    strncpy(b.title, newTitleTemp, MAX_BLOCK_TITLE - 1);
                    b.title[MAX_BLOCK_TITLE - 1] = '\0';
                }
            }
        }
    }

    printf("Enter new content (leave blank to keep current, max. %d characters):\n", MAX_BLOCK_CONTENT - 1);
    printf("Current content:\n%s\n", b.content);
    printf("New content > ");
    fgets(newContentTemp, MAX_BLOCK_CONTENT, stdin);
    newContentTemp[strcspn(newContentTemp, "\n")] = 0;

    if (strlen(newContentTemp) > 0) {
        if (strlen(newContentTemp) >= MAX_BLOCK_CONTENT) {
            printf(">>> Error: New content too long. Max: %d. Keeping old content.\n", MAX_BLOCK_CONTENT - 1);
        } else {
            strncpy(b.content, newContentTemp, MAX_BLOCK_CONTENT - 1);
            b.content[MAX_BLOCK_CONTENT - 1] = '\0';
        }
    }

    printf("Enter the new encryption key (enter current key to keep): ");
    newKey = getIntegerInput();
    if (newKey == -1) {
        fclose(f);
        return;
    }

    b.key = newKey;
    encrypt(b.content, b.key);

    fseek(f, pos, SEEK_SET);
    fwrite(&b, sizeof(Block), 1, f);
    fclose(f);

    printf(">>> Block '%s' updated successfully!\n", b.title);
}

void deleteBlock(const char *logged_username) {
    char title_input[MAX_BLOCK_TITLE];
    int key;
    int found = 0;

    printf("\n--- Delete Block/Report ---\n");
    printf("Enter the title of the block you wish to DELETE: ");
    fgets(title_input, MAX_BLOCK_TITLE, stdin);
    title_input[strcspn(title_input, "\n")] = 0;

    printf("Enter the encryption key to confirm deletion: ");
    key = getIntegerInput();
    if (key == -1) return;

    FILE *f_source = fopen("blocks.dat", "rb");
    if (!f_source) {
        printf(">>> No blocks found for deletion.\n");
        return;
    }

    FILE *f_temp = fopen("temp_blocks.dat", "wb");
    if (!f_temp) {
        printf(">>> Error: Could not create temporary file for deletion.\n");
        fclose(f_source);
        return;
    }

    Block b;
    while (fread(&b, sizeof(Block), 1, f_source)) {
        if (strcmp(b.owner, logged_username) == 0 && strcmp(b.title, title_input) == 0) {
            if (b.key == key) {
                found = 1;
                printf(">>> Block '%s' deleted successfully.\n", title_input);
            } else {
                printf(">>> Incorrect key for block '%s'. Block NOT deleted.\n", title_input);
                fwrite(&b, sizeof(Block), 1, f_temp);
            }
        } else {
            fwrite(&b, sizeof(Block), 1, f_temp);
        }
    }

    fclose(f_source);
    fclose(f_temp);

    if (remove("blocks.dat") != 0) {
        printf(">>> Error removing original file. There might be residual data.\n");
    }
    if (rename("temp_blocks.dat", "blocks.dat") != 0) {
        printf(">>> Error renaming temporary file. There might be data issues.\n");
    }

    if (!found) {
        printf(">>> Block '%s' not found or does not belong to you.\n", title_input);
    }
}

// Aerospace Asset functions
void generateAssetID(char *id_buffer) {
    g_asset_counter++;
    snprintf(id_buffer, ID_LEN, "ASSET%03d", g_asset_counter);
    saveAssetCounter();
}

void createAerospaceAsset(const char *owner_unit) {
    AerospaceAsset a;
    strncpy(a.owner_unit, owner_unit, MAX_UNIT_NAME - 1);
    a.owner_unit[MAX_UNIT_NAME - 1] = '\0';
    generateAssetID(a.id);

    char name_input[MAX_ASSET_NAME];
    char location_input[MAX_ASSET_LOCATION];

    printf("\n--- Create New Aerospace Asset ---\n");
    printf("Asset Name (max. %d characters): ", MAX_ASSET_NAME - 1);
    fgets(name_input, MAX_ASSET_NAME, stdin);
    name_input[strcspn(name_input, "\n")] = 0;
    if (strlen(name_input) >= MAX_ASSET_NAME) {
        printf(">>> Error: Asset name too long. Max: %d.\n", MAX_ASSET_NAME - 1);
        return;
    }
    strncpy(a.name, name_input, MAX_ASSET_NAME - 1);
    a.name[MAX_ASSET_NAME - 1] = '\0';

    printf("\nChoose ASSET TYPE:\n");
    for (int i = 0; i < ASSET_TYPE_MAX; i++) {
        printf("  %d - %s\n", i + 1, assetTypeToString((AssetType)i));
    }
    printf("Option > ");
    int asset_type_option = getIntegerInput();
    if (asset_type_option == -1) return;

    if (asset_type_option < 1 || asset_type_option > ASSET_TYPE_MAX) {
        printf(">>> Invalid type option. Setting to Spacecraft.\n");
        a.type = ASSET_TYPE_SPACECRAFT;
    } else {
        a.type = (AssetType)(asset_type_option - 1);
    }

    printf("\nChoose ASSET STATUS:\n");
    for (int i = 0; i < ASSET_STATUS_MAX; i++) {
        printf("  %d - %s\n", i + 1, assetStatusToString((AssetStatus)i));
    }
    printf("Option > ");
    int asset_status_option = getIntegerInput();
    if (asset_status_option == -1) return;

    if (asset_status_option < 1 || asset_status_option > ASSET_STATUS_MAX) {
        printf(">>> Invalid status option. Setting to Operational.\n");
        a.status = ASSET_STATUS_OPERATIONAL;
    } else {
        a.status = (AssetStatus)(asset_status_option - 1);
    }

    printf("Asset Location (max. %d characters): ", MAX_ASSET_LOCATION - 1);
    fgets(location_input, MAX_ASSET_LOCATION, stdin);
    location_input[strcspn(location_input, "\n")] = 0;
    if (strlen(location_input) >= MAX_ASSET_LOCATION) {
        printf(">>> Error: Location too long. Max: %d. Setting to 'Unknown'.\n", MAX_ASSET_LOCATION - 1);
        strncpy(a.location, "Unknown", MAX_ASSET_LOCATION - 1);
        a.location[MAX_ASSET_LOCATION - 1] = '\0';
    } else {
        strncpy(a.location, location_input, MAX_ASSET_LOCATION - 1);
        a.location[MAX_ASSET_LOCATION - 1] = '\0';
    }

    FILE *f = fopen("assets.dat", "ab");
    if (!f) {
        printf(">>> Error: Could not create asset file.\n");
        return;
    }
    fwrite(&a, sizeof(AerospaceAsset), 1, f);
    fclose(f);

    printf(">>> Asset '%s' (ID: %s, Type: %s, Unit: %s) created successfully!\n",
           a.name, a.id, assetTypeToString(a.type), a.owner_unit);
}

void listAerospaceAssets(const char *logged_unit) {
    FILE *f = fopen("assets.dat", "rb");
    if (!f) {
        printf("\n=== Aerospace Assets of Unit %s ===\n", logged_unit);
        printf("No aerospace assets found.\n");
        return;
    }

    AerospaceAsset a;
    int found_any_asset = 0;
    printf("\n=== Aerospace Assets of Unit %s ===\n", logged_unit);
    printf("--------------------------------------------------------------------------------\n");
    printf("ID         Name                   Type               Status             Location\n");
    printf("--------------------------------------------------------------------------------\n");
    while (fread(&a, sizeof(AerospaceAsset), 1, f)) {
        if (strcmp(a.owner_unit, logged_unit) == 0) {
            printf("%-10s%-23s%-19s%-19s%s\n",
                   a.id, a.name, assetTypeToString(a.type), assetStatusToString(a.status), a.location);
            found_any_asset = 1;
        }
    }
    fclose(f);
    printf("--------------------------------------------------------------------------------\n");
    if (!found_any_asset) {
        printf("No assets found for your unit.\n");
    }
}

// Direct Message (DM) functions
void sendDirectMessage(const char *sender_username) {
    DirectMessage dm;
    strncpy(dm.sender, sender_username, MAX_USERNAME - 1);
    dm.sender[MAX_USERNAME - 1] = '\0';

    char receiver_input[MAX_USERNAME];
    char message_input[MAX_BLOCK_CONTENT];

    printf("\n--- Send Direct Message ---\n");
    printf("Enter the recipient's username: ");
    fgets(receiver_input, MAX_USERNAME, stdin);
    receiver_input[strcspn(receiver_input, "\n")] = 0;

    if (!userExists(receiver_input)) {
        printf(">>> Error: Recipient '%s' not found.\n", receiver_input);
        return;
    }
    strncpy(dm.receiver, receiver_input, MAX_USERNAME - 1);
    dm.receiver[MAX_USERNAME - 1] = '\0';

    printf("Enter your message (max. %d characters):\n", MAX_BLOCK_CONTENT - 1);
    printf(">>> ");
    fgets(message_input, MAX_BLOCK_CONTENT, stdin);
    message_input[strcspn(message_input, "\n")] = 0;

    if (strlen(message_input) >= MAX_BLOCK_CONTENT) {
        printf(">>> Error: Message too long. Max: %d. Please try again.\n", MAX_BLOCK_CONTENT - 1);
        return;
    }
    strncpy(dm.message, message_input, MAX_BLOCK_CONTENT - 1);
    dm.message[MAX_BLOCK_CONTENT - 1] = '\0';

    printf("Enter a numeric key to encrypt the message: ");
    dm.key = getIntegerInput();
    if (dm.key == -1) return;

    encrypt(dm.message, dm.key);

    FILE *f = fopen("messages.dat", "ab");
    if (!f) {
        printf(">>> Error: Could not open message file.\n");
        return;
    }
    fwrite(&dm, sizeof(DirectMessage), 1, f);
    fclose(f);

    printf(">>> Message sent to '%s' successfully!\n", dm.receiver);
}

void listDirectMessages(const char *logged_username) {
    FILE *f = fopen("messages.dat", "rb");
    if (!f) {
        printf("\n=== Your Received Messages ===\n");
        printf("No messages received.\n");
        return;
    }

    DirectMessage dm;
    int found_any_message = 0;
    printf("\n=== Your Received Messages, %s ===\n", logged_username);
    printf("--------------------------------------------------\n");
    printf("From                  Subject/Start (Encrypted)\n");
    printf("--------------------------------------------------\n");

    // First pass for preview
    while (fread(&dm, sizeof(DirectMessage), 1, f)) {
        if (strcmp(dm.receiver, logged_username) == 0) {
            char preview_content[21]; // 20 chars + null terminator
            strncpy(preview_content, dm.message, 20);
            preview_content[20] = '\0';
            printf("%-20s%.20s...\n", dm.sender, preview_content);
            found_any_message = 1;
        }
    }
    printf("--------------------------------------------------\n");

    if (!found_any_message) {
        printf("You have no received messages.\n");
        fclose(f);
        return;
    }

    printf("\nTo read a specific message, enter 'y'. To return to the menu, enter 'n'. (y/n): ");
    char choice_char[5];
    fgets(choice_char, sizeof(choice_char), stdin);
    char choice = choice_char[0];

    if (choice == 'y' || choice == 'Y') {
        char sender_msg_input[MAX_USERNAME];
        int msg_key;

        printf("Enter the sender's username for the message you want to read: ");
        fgets(sender_msg_input, MAX_USERNAME, stdin);
        sender_msg_input[strcspn(sender_msg_input, "\n")] = 0;

        printf("Enter the message's encryption key: ");
        msg_key = getIntegerInput();
        if (msg_key == -1) {
            fclose(f);
            return;
        }

        fseek(f, 0, SEEK_SET); // Rewind to the beginning for detailed reading

        int message_decrypted = 0;
        while (fread(&dm, sizeof(DirectMessage), 1, f)) {
            if (strcmp(dm.receiver, logged_username) == 0 && strcmp(dm.sender, sender_msg_input) == 0) {
                if (dm.key == msg_key) {
                    decrypt(dm.message, msg_key);
                    printf("\n--- Message from %s ---\n", dm.sender);
                    printf("---------------------------\n");
                    printf("%s\n", dm.message);
                    printf("---------------------------\n");
                    message_decrypted = 1;
                } else {
                    printf(">>> Incorrect key for message from %s.\n", sender_msg_input);
                }
                break;
            }
        }
        if (!message_decrypted) {
            printf(">>> Message from '%s' not found for you or incorrect key.\n", sender_msg_input);
        }
    }
    fclose(f);
}

// Group functions
void createGroup(const char *creator_username) {
    Grupo g;
    strncpy(g.creator, creator_username, MAX_USERNAME - 1);
    g.creator[MAX_USERNAME - 1] = '\0';
    g.num_members = 0;

    char group_name_input[MAX_GROUP_NAME];

    printf("\n--- Create New Operations Group ---\n");
    printf("Group Name (max. %d characters): ", MAX_GROUP_NAME - 1);
    fgets(group_name_input, MAX_GROUP_NAME, stdin);
    group_name_input[strcspn(group_name_input, "\n")] = 0;

    if (strlen(group_name_input) >= MAX_GROUP_NAME) {
        printf(">>> Error: Group name too long. Max: %d.\n", MAX_GROUP_NAME - 1);
        return;
    }
    strncpy(g.name, group_name_input, MAX_GROUP_NAME - 1);
    g.name[MAX_GROUP_NAME - 1] = '\0';

    if (groupExists(g.name)) {
        printf(">>> Error: A group with this name already exists. Choose another.\n");
        return;
    }

    if (g.num_members < MAX_GROUP_MEMBERS) {
        // Add creator as first member
        strncpy(g.members[g.num_members], creator_username, MAX_USERNAME - 1);
        g.members[g.num_members][MAX_USERNAME - 1] = '\0';
        g.num_members++;
    } else {
        printf(">>> Error: Member limit reached (cannot add creator automatically).\n");
        return;
    }

    FILE *f = fopen("groups.dat", "ab");
    if (!f) {
        printf(">>> Error: Could not create group file.\n");
        return;
    }
    fwrite(&g, sizeof(Grupo), 1, f);
    fclose(f);
    printf(">>> Group '%s' created successfully by %s!\n", g.name, g.creator);
}

void addGroupMember(const char *logged_username) {
    char group_name_input[MAX_GROUP_NAME];
    char new_member_username_input[MAX_USERNAME];

    printf("\n--- Add Member to Group ---\n");
    printf("Group Name: ");
    fgets(group_name_input, MAX_GROUP_NAME, stdin);
    group_name_input[strcspn(group_name_input, "\n")] = 0;

    Grupo g;
    if (!findGroupByName(group_name_input, &g)) {
        printf(">>> Error: Group '%s' not found.\n", group_name_input);
        return;
    }

    if (strcmp(g.creator, logged_username) != 0) {
        printf(">>> Error: Only the creator (%s) can add members to this group.\n", g.creator);
        return;
    }

    printf("New member's username: ");
    fgets(new_member_username_input, MAX_USERNAME, stdin);
    new_member_username_input[strcspn(new_member_username_input, "\n")] = 0;

    if (strlen(new_member_username_input) >= MAX_USERNAME) {
        printf(">>> Error: Member username too long. Max: %d.\n", MAX_USERNAME - 1);
        return;
    }

    if (!userExists(new_member_username_input)) {
        printf(">>> Error: User '%s' does not exist.\n", new_member_username_input);
        return;
    }

    for (int i = 0; i < g.num_members; i++) {
        if (strcmp(g.members[i], new_member_username_input) == 0) {
            printf("'%s' is already a member of group '%s'.\n", new_member_username_input, group_name_input);
            return;
        }
    }

    if (g.num_members >= MAX_GROUP_MEMBERS) {
        printf(">>> Error: Group '%s' has reached its maximum limit of %d members.\n", group_name_input, MAX_GROUP_MEMBERS);
        return;
    }

    strncpy(g.members[g.num_members], new_member_username_input, MAX_USERNAME - 1);
    g.members[g.num_members][MAX_USERNAME - 1] = '\0';
    g.num_members++;

    FILE *f = fopen("groups.dat", "r+b");
    if (!f) {
        printf(">>> Error opening group file for update.\n");
        return;
    }
    long pos = -1;
    Grupo temp_g;
    while(fread(&temp_g, sizeof(Grupo), 1, f)) {
        if (strcmp(temp_g.name, group_name_input) == 0) {
            pos = ftell(f) - sizeof(Grupo);
            break;
        }
    }
    if (pos != -1) {
        fseek(f, pos, SEEK_SET);
        fwrite(&g, sizeof(Grupo), 1, f);
        printf(">>> '%s' added to group '%s' successfully!\n", new_member_username_input, group_name_input);
    } else {
        printf(">>> Error: Group not found during update.\n");
    }
    fclose(f);
}

void listMyGroups(const char *logged_username) {
    FILE *f = fopen("groups.dat", "rb");
    if (!f) {
        printf("\n=== Your Operations Groups ===\n");
        printf("No groups found.\n");
        return;
    }

    Grupo g;
    int found_any_group = 0;
    printf("\n=== Your Operations Groups, %s ===\n", logged_username);
    printf("--------------------------------------------------------------------------------\n");
    printf("Group Name             Creator              Members\n");
    printf("--------------------------------------------------------------------------------\n");
    while (fread(&g, sizeof(Grupo), 1, f)) {
        if (isUserInGroup(logged_username, g.name)) {
            printf("%-23s%-20s", g.name, g.creator);
            for (int i = 0; i < g.num_members; i++) {
                printf("%s%s", g.members[i], (i == g.num_members - 1) ? "" : ", ");
            }
            printf("\n");
            found_any_group = 1;
        }
    }
    fclose(f);
    printf("--------------------------------------------------------------------------------\n");
    if (!found_any_group) {
        printf("You are not a member of any group.\n");
    }
}

// Menus
void loggedInMenu(const char *logged_username, Rank logged_rank, const char *logged_unit) {
    int option;
    do {
        printf("\n=================================================================\n");
        printf("  Welcome, Operator %s (Rank: %s - Unit: %s)\n",
               logged_username, rankToString(logged_rank), logged_unit);
        printf("=================================================================\n");
        printf("                 :: OPERATIONS MENU ::\n");
        printf("-----------------------------------------------------------------\n");
        printf("  [1] Create New Block/Report\n");
        printf("  [2] List Accessible Blocks/Reports\n");
        printf("  [3] Open and View Block/Report Content\n");
        printf("  [4] Edit an Existing Block/Report\n");
        printf("  [5] Delete a Block/Report\n");
        printf("-----------------------------------------------------------------\n");
        printf("  [6] Create Aerospace Asset (Spacecraft, Satellite, etc.)\n");
        printf("  [7] List Unit Aerospace Assets\n");
        printf("-----------------------------------------------------------------\n");
        printf("  [8] Send Direct Message\n");
        printf("  [9] List My Direct Messages\n");
        printf("-----------------------------------------------------------------\n");
        printf(" [10] Create Operations Group\n");
        printf(" [11] Add Member to Group\n");
        printf(" [12] List My Groups\n");
        printf("-----------------------------------------------------------------\n");
        printf(" [13] Change My Password\n");
        printf("  [0] Log Out (Exit)\n");
        printf("-----------------------------------------------------------------\n");
        printf("Enter your option > ");
        option = getIntegerInput();
        if (option == -1) continue;

        switch (option) {
            case 1: createBlock(logged_username, logged_rank, logged_unit); break;
            case 2: listBlocks(logged_username, logged_rank, logged_unit); break;
            case 3: openBlock(logged_username, logged_rank, logged_unit); break;
            case 4: editBlock(logged_username, logged_rank, logged_unit); break;
            case 5: deleteBlock(logged_username); break;
            case 6: createAerospaceAsset(logged_unit); break;
            case 7: listAerospaceAssets(logged_unit); break;
            case 8: sendDirectMessage(logged_username); break;
            case 9: listDirectMessages(logged_username); break;
            case 10: createGroup(logged_username); break;
            case 11: addGroupMember(logged_username); break;
            case 12: listMyGroups(logged_username); break;
            case 13: changeUserPassword(logged_username); break;
            case 0: printf(">>> Disconnecting from '%s' operations network...\n", logged_username); break;
            default: printf(">>> Invalid command. Please try again.\n");
        }
        printf("\n"); // Adds a blank line after each operation for better readability
    } while (option != 0);
}

int main() {
    loadAssetCounter();

    int option;
    char loggedUsername[MAX_USERNAME];
    Rank loggedRank;
    char loggedUnit[MAX_UNIT_NAME];

    // ASCII Art Banner
    printf("█     █░ ▄▄▄       ██▀███   ██▀███   ▒█████   ▒█████   ███▄ ▄███▓\n");
    printf("▓█░ █ ░█░▒████▄    ▓██ ▒ ██▒▓██ ▒ ██▒▒██▒  ██▒▒██▒  ██▒▓██▒▀█▀ ██▒\n");
    printf("▒█░ █ ░█ ▒██  ▀█▄  ▓██ ░▄█ ▒▓██ ░▄█ ▒▒██░  ██▒▒██░  ██▒▓██    ▓██░\n");
    printf("░█░ █ ░█ ░██▄▄▄▄██ ▒██▀▀█▄  ▒██▀▀█▄  ▒██   ██░▒██   ██░▒██    ▒██ \n");
    printf("░░██▒██▓  ▓█   ▓██▒░██▓ ▒██▒░██▓ ▒██▒░ ████▓▒░░ ████▓▒░▒██▒   ░██▒\n");
    printf("░ ▓░▒ ▒   ▒▒   ▓▒█░░ ▒▓ ░▒▓░░ ▒▓ ░▒▓░░ ▒░▒░▒░ ░ ▒░▒░▒░ ░ ▒░   ░  ░\n");
    printf("  ▒ ░ ░    ▒   ▒▒ ░  ░▒ ░ ▒░  ░▒ ░ ▒░  ░ ▒ ▒░   ░ ▒ ▒░ ░  ░      ░\n");
    printf("  ░   ░    ░   ▒     ░░   ░   ░░   ░ ░ ░ ░ ▒  ░ ░ ░ ▒  ░      ░   \n");
    printf("    ░          ░  ░   ░        ░         ░ ░      ░ ░         ░   \n");
    printf("                                                                 \n");
    printf("=================================================================\n");
    printf("        AEROSPACE OPERATIONS MANAGEMENT SYSTEM\n");
    printf("=================================================================\n");

    do {
        printf("\n-----------------------------------------------------------------\n");
        printf("                  :: MAIN ACCESS TERMINAL ::\n");
        printf("-----------------------------------------------------------------\n");
        printf("  [1] Create New Operator\n");
        printf("  [2] Initiate Login\n");
        printf("  [3] List All Operators (Control Center)\n");
        printf("  [0] Shut Down System\n");
        printf("-----------------------------------------------------------------\n");
        printf("Enter your option > ");
        option = getIntegerInput();
        if (option == -1) continue;

        switch (option) {
            case 1: createUser(); break;
            case 2:
                if (login(loggedUsername, &loggedRank, loggedUnit)) {
                    printf(">>> Access authorized! Welcome, Operator %s.\n", loggedUsername);
                    loggedInMenu(loggedUsername, loggedRank, loggedUnit);
                }
                break;
            case 3: listUsers(); break;
            case 0: printf(">>> Shutting down terminal...\n"); break;
            default: printf(">>> Invalid command. Please try again.\n");
        }
        printf("\n"); // Adds a blank line after each operation
    } while (option != 0);

    return 0;
}