#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>

#define MAX_NAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_ROOM_TYPE_LEN 50
#define DATA_FILE "reservations.dat"

typedef struct Room {
    int roomNumber;
    char roomType[MAX_ROOM_TYPE_LEN];
    int isBooked;
} Room;

typedef struct Reservation {
    char username[MAX_NAME_LEN];
    int roomNumber;
    char checkInDate[11];
    char checkInTime[6];
    char checkOutDate[11];
    char checkOutTime[6];
    struct Reservation* next;
} Reservation;

typedef struct User {
    char username[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int isAdmin;
    struct User* next;
} User;

User * userList = NULL;
Reservation* reservationList = NULL;
Room* rooms = NULL;
int totalRooms = 0;
int maxRooms = 0;

void addUser (char username[], char password[], int isAdmin);
User * authenticateUser (char username[], char password[]);
void displayRooms();
void makeReservation(char username[]);
void viewReservations(char username[]);
void addReservation(char username[], int roomNumber, char checkInDate[], char checkInTime[], char checkOutDate[], char checkOutTime[]);
void removeReservation(char username[]);
void showAdminMenu();
void showUserMenu(char username[]);
void cleanup();
void registerUser ();
void initializeRooms();
void createRoom();
void viewReservationsByRoom();
void resizeRooms();
void saveData();
void loadData();
void checkExpiredReservations();
int isValidDate(char date[]);
int isValidTime(char time[]);
int isLeapYear(int year);
void changePassword(char username[]);
void modifyReservation(char username[]);
void deleteUser ();
void viewAllReservations();
void viewStatistics();
void searchAvailableRooms();

int isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int isValidDate(char date[]) {
    int year, month, day;
    year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + (date[3] - '0');
    month = (date[5] - '0') * 10 + (date[6] - '0');
    day = (date[8] - '0') * 10 + (date[9] - '0');
    
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;
    if (month == 2) {
        if (isLeapYear(year)) {
            if (day > 29) return 0;
        } else {
            if (day > 28) return 0;
        }
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        if (day > 30) return 0;
    }
    return 1;
}

int isValidTime(char time[]) {
    int hour, minute;
    hour = (time[0] - '0') * 10 + (time[1] - '0');
    minute = (time[3] - '0') * 10 + (time[4] - '0');
    
    if (hour < 0 || hour > 23) return 0;
    if (minute < 0 || minute > 59) return 0;
    return 1;
}

void initializeRooms() {
    maxRooms = 10;
    rooms = (Room*)malloc(maxRooms * sizeof(Room));
    int i;
    for (i = 0; i < maxRooms; i++) {
        rooms[i].roomNumber = i + 1;
        strcpy(rooms[i].roomType, "Standard Room");
        rooms[i].isBooked = 0;
    }
    totalRooms = maxRooms;
}

void addUser (char username[], char password[], int isAdmin) {
    User* current = userList;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            return; // User already exists
        }
        current = current->next;
    }
    User* newUser  = (User *)malloc(sizeof(User));
    strcpy(newUser ->username, username);
    strcpy(newUser ->password, password);
    newUser ->isAdmin = isAdmin;
    newUser ->next = userList;
    userList = newUser ;
}

User * authenticateUser (char username[], char password[]) {
    User* current = userList;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && strcmp(current->password, password) == 0) {
            return current; // Authentication successful
        }
        current = current->next;
    }
    return NULL; // Authentication failed
}

void displayRooms() {
    printf("Available Rooms:\n");
    int i;
    for (i = 0; i < totalRooms; i++) {
        if (rooms[i].isBooked == 0) {
            printf("Room %d: %s (Available)\n", rooms[i].roomNumber, rooms[i].roomType);
        }
    }
}

void makeReservation(char username[]) {
    int roomNumber;
    char checkInDate[11], checkInTime[6], checkOutDate[11], checkOutTime[6];
    displayRooms();
    printf("Enter room number to reserve: ");
    scanf("%d", &roomNumber);
    if (roomNumber < 1 || roomNumber > totalRooms || rooms[roomNumber - 1].isBooked == 1) {
        printf("Invalid room number or room is already booked.\n");
        return;
    }
    while (1) {
        printf("Enter check-in date (YYYY-MM-DD): ");
        scanf("%10s", checkInDate);
        if (isValidDate(checkInDate)) {
            break;
        }
        printf("Invalid date format. Please try again.\n");
    }
    while (1) {
        printf("Enter check-in time (HH:MM): ");
        scanf("%5s", checkInTime);
        if (isValidTime(checkInTime)) {
            break;
        }
        printf("Invalid time format. Please try again.\n");
    }
    while (1) {
        printf("Enter check-out date (YYYY-MM-DD): ");
        scanf("%10s", checkOutDate);
        if (isValidDate(checkOutDate)) {
            break;
        }
        printf("Invalid date format. Please try again.\n");
    }
    while (1) {
        printf("Enter check-out time (HH:MM): ");
        scanf("%5s", checkOutTime);
        if (isValidTime(checkOutTime)) {
            break;
        }
        printf("Invalid time format. Please try again.\n");
    }
    if (compareDates(checkInDate, checkOutDate) > 0 || 
        (compareDates(checkInDate, checkOutDate) == 0 && strcmp(checkInTime, checkOutTime) >= 0)) {
        printf("Check-in date/time must be before check-out date/time.\n");
        return;
    }
    addReservation(username, roomNumber, checkInDate, checkInTime, checkOutDate, checkOutTime);
    rooms[roomNumber - 1].isBooked = 1;
    printf("Reservation made successfully for Room %d\n", roomNumber);
}

void addReservation(char username[], int roomNumber, char checkInDate[], char checkInTime[], char checkOutDate[], char checkOutTime[]) {
    Reservation* newReservation = (Reservation*)malloc(sizeof(Reservation));
    strcpy(newReservation->username, username);
    newReservation->roomNumber = roomNumber;
    strcpy(newReservation->checkInDate, checkInDate);
    strcpy(newReservation->checkInTime, checkInTime);
    strcpy(newReservation->checkOutDate, checkOutDate);
    strcpy(newReservation->checkOutTime, checkOutTime);
    newReservation->next = reservationList;
    reservationList = newReservation;
}

void removeReservation(char username[]) {
    char inputUsername[MAX_NAME_LEN];
    int roomNumber;
    printf("Enter username to remove reservation: ");
    scanf("%s", inputUsername);
    Reservation* current = reservationList;
    Reservation* prev = NULL;
    while (current != NULL) {
        if (strcmp(current->username, inputUsername) == 0) {
            printf("Enter room number to remove reservation: ");
            scanf("%d", &roomNumber);
            if (roomNumber < 1 || roomNumber > totalRooms) {
                printf("Invalid room number.\n");
                return;
            }
            if (prev == NULL) {
                reservationList = current->next;
            } else {
                prev->next = current->next;
            }
            rooms[roomNumber - 1].isBooked = 0;
            free(current);
            printf("Reservation for Room %d by %s has been removed successfully.\n", roomNumber, inputUsername);
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("No reservation found for user %s.\n", inputUsername);
}

void viewReservations(char username[]) {
    Reservation* current = reservationList;
    int found = 0;
    printf("Reservations for %s:\n", username);
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            printf("Room %d: Check-in: %s %s, Check-out: %s %s\n", 
                   current->roomNumber, 
                   current->checkInDate, 
                   current->checkInTime, 
                   current->checkOutDate, 
                   current->checkOutTime);
            found = 1;
        }
        current = current->next;
    }
    if (!found) {
        printf("No reservations found for %s.\n", username);
    }
}

void createRoom() {
    resizeRooms();
    int roomNumber = totalRooms + 1;
    char roomType[MAX_ROOM_TYPE_LEN];
    printf("Enter room type for Room %d: ", roomNumber);
    scanf("%s", roomType);
    rooms[totalRooms].roomNumber = roomNumber;
    strcpy(rooms[totalRooms].roomType, roomType);
    rooms[totalRooms].isBooked = 0;
    totalRooms++;
    printf("Room %d created successfully!\n", roomNumber);
}

void resizeRooms() {
    if (totalRooms >= maxRooms) {
        maxRooms *= 2;
        rooms = (Room*)realloc(rooms, maxRooms * sizeof(Room));
        int i;
        for ( i = totalRooms; i < maxRooms; i++) {
            rooms[i].roomNumber = i + 1;
            strcpy(rooms[i].roomType, "Standard Room");
            rooms[i].isBooked = 0;
        }
    }
}

void viewReservationsByRoom() {
    int roomNumber;
    printf("Enter room number to view reservations: ");
    scanf("%d", &roomNumber);
    if (roomNumber < 1 || roomNumber > totalRooms) {
        printf("Invalid room number.\n");
        return;
    }
    Reservation* current = reservationList;
    int found = 0;
    printf("Reservations for Room %d:\n", roomNumber);
    while (current != NULL) {
        if (current->roomNumber == roomNumber) {
            printf("Username: %s, Check-in: %s %s, Check-out: %s %s\n", 
                   current->username, 
                   current->checkInDate, 
                   current->checkInTime, 
                   current->checkOutDate, 
                   current->checkOutTime);
            found = 1;
        }
        current = current->next;
    }
    if (!found) {
        printf("No reservations found for Room %d.\n", roomNumber);
    }
}

void changePassword(char username[]) {
    char newPassword[MAX_PASSWORD_LEN];
    printf("Enter new password: ");
    int i = 0;
    char ch;
    while ((ch = getch()) != '\r') {
        if (ch == '\b') {
            if (i > 0) {
                printf("\b \b");
                i--;
            }
        } else {
            newPassword[i++] = ch;
            printf("*");
        }
    }
    newPassword[i] = '\0';
    User* user = authenticateUser (username, newPassword);
    if (user) {
        strcpy(user->password, newPassword);
        printf("\nPassword changed successfully!\n");
    } else {
        printf("\nFailed to change password. Please try again.\n");
    }
}

void modifyReservation(char username[]) {
    int roomNumber;
    printf("Enter room number to modify reservation: ");
    scanf("%d", &roomNumber);
    Reservation* current = reservationList;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && current->roomNumber == roomNumber) {
            printf("Current Check-in Date: %s, Check-in Time: %s\n", current->checkInDate, current->checkInTime);
            printf("Enter new check-in date (YYYY-MM-DD): ");
            scanf("%s", current->checkInDate);
            printf("Enter new check-in time (HH:MM): ");
            scanf("%s", current->checkInTime);
            printf("Reservation modified successfully!\n");
            return;
        }
        current = current->next;
    }
    printf("No reservation found for Room %d.\n", roomNumber);
}

void deleteUser () {
    char username[MAX_NAME_LEN];
    printf("Enter username to delete: ");
    scanf("%s", username);
    User* current = userList;
    User* prev = NULL;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            if (prev == NULL) {
                userList = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            printf("User  %s deleted successfully.\n", username);
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("User  %s not found.\n", username);
}

void viewAllReservations() {
    Reservation* current = reservationList;
    if (current == NULL) {
        printf("No reservations found.\n");
        return;
    }
    printf("All Reservations:\n");
    while (current != NULL) {
        printf("Username: %s, Room %d, Check-in: %s %s, Check-out: %s %s\n", 
               current->username, 
               current->roomNumber, 
               current->checkInDate, 
               current->checkInTime, 
               current->checkOutDate, 
               current->checkOutTime);
        current = current->next;
    }
}

void viewStatistics() {
    int totalReservations = 0;
    Reservation* current = reservationList;
    while (current != NULL) {
        totalReservations++;
        current = current->next;
    }
    printf("Total Reservations: %d\n", totalReservations);
}

void searchAvailableRooms() {
    char roomType[MAX_ROOM_TYPE_LEN];
    printf("Enter room type to search: ");
    scanf("%s", roomType);
    printf("Available Rooms of type %s:\n", roomType);
    int i;
    for (i = 0; i < totalRooms; i++) {
        if (rooms[i].isBooked == 0) {
            printf("Room %d: %s (Available)\n", rooms[i].roomNumber, rooms[i].roomType);
        }
    }
}

void showAdminMenu() {
    int choice;
    do {
        printf("\nAdmin Menu:\n");
        printf("1. View all reservations\n");
        printf("2. View all users\n");
        printf("3. Create a new room\n");
        printf("4. View reservations by room\n");
        printf("5. Remove a reservation\n");
        printf("6. Delete a user\n");
        printf("7. View statistics\n");
        printf("8. Log out\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                viewAllReservations();
                break;
            case 2:
                printf("All Users:\n");
                if (userList == NULL) {
                    printf("No users found.\n");
                } else {
                    User* currentUser  = userList;
                    while (currentUser  != NULL) {
                        printf("Username: %s, %s\n", 
                               currentUser ->username, 
                               currentUser ->isAdmin ? "Admin" : "User ");
                        currentUser  = currentUser ->next;
                    }
                }
                break;
            case 3:
                createRoom();
                break;
            case 4:
                viewReservationsByRoom();
                break;
            case 5:
                removeReservation("admin");
                break;
            case 6:
                deleteUser ();
                break;
            case 7:
                viewStatistics();
                break;
            case 8:
                printf("Logging out...\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
                break;
        }
    } while (choice != 8);
}

void showUserMenu(char username[]) {
    int choice;
    do {
        printf("\nUser  Menu:\n");
        printf("1. Make a reservation\n");
        printf("2. View my reservations\n");
        printf("3. Modify my reservation\n");
        printf("4. Change my password\n");
        printf("5. Log out\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                makeReservation(username);
                break;
            case 2:
                viewReservations(username);
                break;
            case 3:
                modifyReservation(username);
                break;
            case 4:
                changePassword(username);
                break;
            case 5:
                printf("Logging out...\n");
                break;
            default:
                printf("Invalid choice!\n");
                break;
        }
    } while (choice != 5);
}

void registerUser () {
    char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
    printf("Enter a username: ");
    scanf("%s", username);
    printf("Enter a password: ");
    int i = 0;
    char ch;
    while ((ch = getch()) != '\r') {
        if (ch == '\b') {
            if (i > 0) {
                printf("\b \b");
                i--;
            }
        } else {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    addUser (username, password, 0);
    printf("\nRegistration successful! You can now log in.\n");
}

void cleanup() {
    User* currentUser  = userList;
    while (currentUser  != NULL) {
        User* temp = currentUser ;
        currentUser  = currentUser ->next;
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

void saveData() {
    FILE* file = fopen(DATA_FILE, "w");
    if (file == NULL) {
        return;
    }
    User* currentUser  = userList;
    while (currentUser  != NULL) {
        fprintf(file, "USER:%s:%s:%d\n", currentUser ->username, currentUser ->password, currentUser ->isAdmin);
        currentUser  = currentUser ->next;
    }
    Reservation* currentReservation = reservationList;
    while (currentReservation != NULL) {
        fprintf(file, "RESERVATION:%s:%d:%s %s:%s %s\n", currentReservation->username, currentReservation->roomNumber, currentReservation->checkInDate, currentReservation->checkInTime, currentReservation->checkOutDate, currentReservation->checkOutTime);
        currentReservation = currentReservation->next;
    }
    fclose(file);
}

void loadData() {
    FILE* file = fopen(DATA_FILE, "r");
    if (file == NULL) {
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char type[20];
        int i = 0;
        while (line[i] != ':') {
            type[i] = line[i];
            i++;
        }
        type[i] = '\0';
        if (strcmp(type, "USER") == 0) {
            char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
            int isAdmin;
            sscanf(line, "USER:%[^:]:%[^:]:%d", username, password, &isAdmin);
            addUser (username, password, isAdmin);
        } else if (strcmp(type, "RESERVATION") == 0) {
            char username[MAX_NAME_LEN], checkInDate[11], checkInTime[6], checkOutDate[11], checkOutTime[6];
            int roomNumber;
            sscanf(line, "RESERVATION:%[^:]:%d:%[^ ] %[^:]:%[^ ] %[^:]", username, &roomNumber, checkInDate, checkInTime, checkOutDate, checkOutTime);
            addReservation(username, roomNumber, checkInDate, checkInTime, checkOutDate, checkOutTime);
            rooms[roomNumber - 1].isBooked = 1;
        }
    }
    fclose(file);
}

void checkExpiredReservations() {
    time_t now = time(NULL);
    struct tm* currentTime = localtime(&now);
    char currentDate[11];
    char currentTimeStr[6];
    int year = currentTime->tm_year + 1900;
    int month = currentTime->tm_mon + 1;
    int day = currentTime->tm_mday;
    int hour = currentTime->tm_hour;
    int minute = currentTime->tm_min;

    snprintf(currentDate, sizeof(currentDate), "%04d-%02d-%02d", year, month, day);
    snprintf(currentTimeStr, sizeof(currentTimeStr), "%02d:%02d", hour, minute);

    Reservation* current = reservationList;
    Reservation* prev = NULL;
    while (current != NULL) {
        if (compareDates(current->checkOutDate, currentDate) < 0 || 
            (compareDates(current->checkOutDate, currentDate) == 0 && strcmp(current->checkOutTime, currentTimeStr) < 0)) {
            rooms[current->roomNumber - 1].isBooked = 0;
            if (prev == NULL) {
                reservationList = current->next;
            } else {
                prev->next = current->next;
            }
            Reservation* temp = current;
            current = current->next;
            free(temp);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

int compareDates(char date1[], char date2[]) {
    return strcmp(date1, date2);
}

int main() {
    int option;
    char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
    User* loggedInUser  = NULL;
    initializeRooms();
    loadData();
    checkExpiredReservations();
    addUser ("admin", "admin123", 1);
    addUser ("user1", "password1", 0);
    do {
        printf("\nHotel Reservation System\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &option);
        switch (option) {
            case 1:
                registerUser ();
                break;
            case 2:
                printf("Enter username: ");
                scanf("%s", username);
                printf("Enter password: ");
                int i = 0;
                char ch;
                while ((ch = getch()) != '\r') {
                    if (ch == '\b') {
                        if (i > 0) {
                            printf("\b \b");
                            i--;
                        }
                    } else {
                        password[i++] = ch;
                        printf("*");
                    }
                }
                password[i] = '\0';
                loggedInUser  = authenticateUser (username, password);
                if (loggedInUser  == NULL) {
                    printf("\nInvalid username or password. Please register first if you haven't.\n");
                } else {
                    printf("\nLogin successful. Welcome %s!\n", username);
                    if (loggedInUser ->isAdmin) {
                        showAdminMenu();
                    } else {
                        showUserMenu(username);
                    }
                }
                break;
            case 3:
                printf("Exiting...\n");
                saveData();
                break;
            default:
                printf("Invalid choice!\n");
                break;
        }
    } while (option != 3);
    cleanup();
    return 0;
}
