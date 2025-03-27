#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_NAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_ROOM_TYPE_LEN 50
#define MAX_GUESTS 5

#define USER_FILE "users.txt"
#define RESERVATION_FILE "reservations.txt"

typedef struct Room {
    int roomNumber;
    char roomType[MAX_ROOM_TYPE_LEN];
    int isBooked;
} Room;

typedef struct Reservation {
    char username[MAX_NAME_LEN];
    int roomNumber;
    char checkInDate[11];
    char checkOutDate[11];
    struct Reservation* next;
} Reservation;

typedef struct User {
    char username[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int isAdmin;
    struct User* next;
} User;

User* userList = NULL;
Reservation* reservationList = NULL;
Room* rooms = NULL;
int totalRooms = 0;
int maxRooms = 0;

void addUser(const char* username, const char* password, int isAdmin);
User* authenticateUser(const char* username, const char* password);
void displayRooms();
void makeReservation(const char* username);
void viewReservations(const char* username);
void addReservation(const char* username, int roomNumber, const char* checkInDate, const char* checkOutDate);
void showAdminMenu();
void showUserMenu(const char* username);
void cleanup();
void registerUser();
void initializeRooms();
void createRoom();
void viewReservationsByRoom();
void resizeRooms();
void saveUsers();
void loadUsers();
void saveReservations();
void loadReservations();
void checkOutRoom(const char* username, int roomNumber);

void initializeRooms() {
    maxRooms = 10;
    rooms = (Room*)malloc(maxRooms * sizeof(Room));
    if (rooms == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < maxRooms; i++) {
        rooms[i].roomNumber = i + 1;
        snprintf(rooms[i].roomType, MAX_ROOM_TYPE_LEN, "Standard Room %d", i + 1);
        rooms[i].isBooked = 0;
    }
    totalRooms = maxRooms;
}

void addUser(const char* username, const char* password, int isAdmin) {
    User* newUser = (User*)malloc(sizeof(User));
    if (newUser == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    strcpy(newUser->username, username);
    strcpy(newUser->password, password);
    newUser->isAdmin = isAdmin;
    newUser->next = userList;
    userList = newUser;
}

User* authenticateUser(const char* username, const char* password) {
    User* current = userList;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && strcmp(current->password, password) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void displayRooms() {
    printf("Available Rooms:\n");
    for (int i = 0; i < totalRooms; i++) {
        if (rooms[i].isBooked == 0) {
            printf("Room %d: %s (Available)\n", rooms[i].roomNumber, rooms[i].roomType);
        }
    }
}

void cleanup() {
    User* currentUser = userList;
    while (currentUser != NULL) {
        User* temp = currentUser;
        currentUser = currentUser->next;
        free(temp);
    }
    Reservation* currentReservation = reservationList;
    while (currentReservation != NULL) {
        Reservation* temp = currentReservation;
        currentReservation = currentReservation->next;
        free(temp);
    }
    free(rooms);
}

int main() {
    int option;
    char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
    User* loggedInUser = NULL;

    initializeRooms();
    loadUsers();
    loadReservations();

    if (userList == NULL) {
        addUser("admin", "admin123", 1);
        addUser("user1", "password1", 0);
    }

    do {
        printf("\nHotel Reservation System\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &option);

        switch (option) {
            case 1:
                registerUser();
                saveUsers();
                break;
            case 2:
                printf("Enter username: ");
                scanf("%s", username);
                printf("Enter password: ");
                scanf("%s", password);
                loggedInUser = authenticateUser(username, password);
                if (loggedInUser == NULL) {
                    printf("\nInvalid username or password.\n");
                } else {
                    printf("\nLogin successful. Welcome %s!\n", username);
                    if (loggedInUser->isAdmin) {
                        showAdminMenu();
                    } else {
                        showUserMenu(username);
                    }
                }
                break;
            case 3:
                printf("Exiting...\n");
                cleanup();
                break;
            default:
                printf("Invalid choice!\n");
                break;
        }
    } while (option != 3);

    return 0;
}