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
    double pricePerNight;
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

User* userList = NULL;
Reservation* reservationList = NULL;
Room* rooms = NULL;
int totalRooms = 0;
int maxRooms = 0;

// Function prototypes
void addUser(char username[], char password[], int isAdmin);
User* authenticateUser(char username[], char password[]);
void displayRooms();
void makeReservation(char username[]);
void viewReservations(char username[]);
void addReservation(char username[], int roomNumber, char checkInDate[], char checkInTime[], char checkOutDate[], char checkOutTime[]);
void removeReservation(char username[]);
void showAdminMenu();
void showUserMenu(char username[]);
void cleanup();
void registerUser();
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
void deleteUser();
void viewAllReservations();
void viewStatistics();
void searchAvailableRooms();
int compareDates(char date1[], char date2[]);
void displayHeader(const char* title);
int isRoomBooked(int roomNumber);
int isRoomAvailableForDates(int roomNumber, char checkInDate[], char checkOutDate[]);
char* formatDateTime(char date[], char time[]);

int isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void displayHeader(const char* title) {
    printf("\n=== %s ===\n", title);
}

int isValidDate(char date[]) {
    // Check format YYYY-MM-DD
    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return 0;
    }
    
    int year, month, day;
    sscanf(date, "%d-%d-%d", &year, &month, &day);
    
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;
    
    // Check days in month
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
    // Check format HH:MM
    if (strlen(time) != 5 || time[2] != ':') {
        return 0;
    }
    
    int hour, minute;
    sscanf(time, "%d:%d", &hour, &minute);
    
    if (hour < 0 || hour > 23) return 0;
    if (minute < 0 || minute > 59) return 0;
    
    return 1;
}

// Check if a room is currently booked for any date
int isRoomBooked(int roomNumber) {
    Reservation* current = reservationList;
    while (current != NULL) {
        if (current->roomNumber == roomNumber) {
            return 1; // Room is booked
        }
        current = current->next;
    }
    return 0; // Room is not booked
}

// Check if a room is available for specific dates
int isRoomAvailableForDates(int roomNumber, char checkInDate[], char checkOutDate[]) {
    Reservation* current = reservationList;
    
    while (current != NULL) {
        if (current->roomNumber == roomNumber) {
            // Check if there's an overlap in reservation dates
            if (!(compareDates(checkOutDate, current->checkInDate) < 0 || 
                  compareDates(checkInDate, current->checkOutDate) > 0)) {
                return 0; // Room is not available for these dates
            }
        }
        current = current->next;
    }
    
    return 1; // Room is available for these dates
}

void initializeRooms() {
    maxRooms = 10;
    rooms = (Room*)malloc(maxRooms * sizeof(Room));
    int i;
    for (i = 0; i < maxRooms; i++) {
        rooms[i].roomNumber = i + 1;
        
        // Assign different room types and prices
        if (i < 4) {
            strcpy(rooms[i].roomType, "Standard");
            rooms[i].pricePerNight = 1500.0;
        } else if (i < 7) {
            strcpy(rooms[i].roomType, "Deluxe");
            rooms[i].pricePerNight = 2000.0;
        } else {
            strcpy(rooms[i].roomType, "Suite");
            rooms[i].pricePerNight = 3000.0;
        }
    }
    totalRooms = maxRooms;
}

void addUser(char username[], char password[], int isAdmin) {
    User* current = userList;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            return; // User already exists
        }
        current = current->next;
    }
    User* newUser = (User*)malloc(sizeof(User));
    strcpy(newUser->username, username);
    strcpy(newUser->password, password);
    newUser->isAdmin = isAdmin;
    newUser->next = userList;
    userList = newUser;
}

User* authenticateUser(char username[], char password[]) {
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
    system("cls");
    displayHeader("AVAILABLE ROOMS");
    
    printf("\n%-8s %-20s %-12s %-15s\n", "Room #", "Room Type", "Status", "Price/Night(P)");
    printf("----------------------------------------------------------\n");
    
    int i;
    for (i = 0; i < totalRooms; i++) {
        printf("%-8d %-20s %-12s P%-14.0f\n", 
               rooms[i].roomNumber, 
               rooms[i].roomType, 
               "Available", // Always show as available
               rooms[i].pricePerNight);
    }
    
    printf("----------------------------------------------------------\n");
}

void makeReservation(char username[]) {
    system("cls");
    displayHeader("MAKE A RESERVATION");
    
    int roomNumber;
    char checkInDate[11], checkInTime[6], checkOutDate[11], checkOutTime[6];
    
    displayRooms();
    
    printf("\nEnter room number to reserve: ");
    scanf("%d", &roomNumber);
    
    if (roomNumber < 1 || roomNumber > totalRooms) {
        printf("\nError: Invalid room number.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    printf("Enter check-in date (YYYY-MM-DD): ");
    scanf("%10s", checkInDate);
    
    if (!isValidDate(checkInDate)) {
        printf("\nError: Invalid date format.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    printf("Enter check-in time (HH:MM): ");
    scanf("%5s", checkInTime);
    
    if (!isValidTime(checkInTime)) {
        printf("\nError: Invalid time format.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    printf("Enter check-out date (YYYY-MM-DD): ");
    scanf("%10s", checkOutDate);
    
    if (!isValidDate(checkOutDate)) {
        printf("\nError: Invalid date format.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    printf("Enter check-out time (HH:MM): ");
    scanf("%5s", checkOutTime);
    
    if (!isValidTime(checkOutTime)) {
        printf("\nError: Invalid time format.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    // Validate check-in before check-out
    if (compareDates(checkInDate, checkOutDate) > 0 || 
        (compareDates(checkInDate, checkOutDate) == 0 && strcmp(checkInTime, checkOutTime) >= 0)) {
        printf("\nError: Check-in date/time must be before check-out date/time.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    // Check if room is available for the selected dates
    if (!isRoomAvailableForDates(roomNumber, checkInDate, checkOutDate)) {
        printf("\nError: Room %d is not available for the selected dates.\n", roomNumber);
        printf("Please select different dates or a different room.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    addReservation(username, roomNumber, checkInDate, checkInTime, checkOutDate, checkOutTime);
    
    system("cls");
    printf("\n=== Reservation Successful ===\n");
    printf("Room %d has been reserved for you.\n", roomNumber);
    printf("\nPress any key to continue...");
    getch();
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
    system("cls");
    displayHeader("REMOVE RESERVATION");
    
    char inputUsername[MAX_NAME_LEN];
    int roomNumber;
    
    printf("Enter username to remove reservation: ");
    scanf("%s", inputUsername);
    
    Reservation* current = reservationList;
    Reservation* prev = NULL;
    int found = 0;
    
    // Display user's reservations
    printf("\nReservations for %s:\n", inputUsername);
    printf("%-8s %-25s %-25s\n", "Room #", "Check-in", "Check-out");
    printf("------------------------------------------------------------------\n");
    
    while (current != NULL) {
        if (strcmp(current->username, inputUsername) == 0) {
            printf("%-8d %-25s %-25s\n", 
                   current->roomNumber, 
                   formatDateTime(current->checkInDate, current->checkInTime),
                   formatDateTime(current->checkOutDate, current->checkOutTime));
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("\nNo reservations found for user %s.\n", inputUsername);
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    printf("\nEnter room number to remove reservation: ");
    scanf("%d", &roomNumber);
    
    if (roomNumber < 1 || roomNumber > totalRooms) {
        printf("\nError: Invalid room number.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    current = reservationList;
    prev = NULL;
    found = 0;
    
    while (current != NULL) {
        if (strcmp(current->username, inputUsername) == 0 && current->roomNumber == roomNumber) {
            found = 1;
            
            if (prev == NULL) {
                reservationList = current->next;
            } else {
                prev->next = current->next;
            }
            
            free(current);
            
            system("cls");
            printf("\n=== Reservation Removed ===\n");
            printf("Reservation for Room %d by %s has been removed successfully.\n", roomNumber, inputUsername);
            printf("\nPress any key to continue...");
            getch();
            return;
        }
        prev = current;
        current = current->next;
    }
    
    if (!found) {
        printf("\nNo reservation found for this room and user.\n");
        printf("\nPress any key to continue...");
        getch();
    }
}

void viewReservations(char username[]) {
    system("cls");
    displayHeader("YOUR RESERVATIONS");
    
    Reservation* current = reservationList;
    int found = 0;
    
    printf("\n%-8s %-25s %-25s\n", "Room #", "Check-in", "Check-out");
    printf("------------------------------------------------------------------\n");
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            printf("%-8d %-25s %-25s\n", 
                   current->roomNumber, 
                   formatDateTime(current->checkInDate, current->checkInTime),
                   formatDateTime(current->checkOutDate, current->checkOutTime));
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("No reservations found for %s.\n", username);
    }
    
    printf("\nPress any key to continue...");
    getch();
}

void createRoom() {
    system("cls");
    displayHeader("CREATE NEW ROOM");
    
    resizeRooms();
    int roomNumber = totalRooms + 1;
    char roomType[MAX_ROOM_TYPE_LEN];
    double pricePerNight;
    
    printf("Enter room type for Room %d: ", roomNumber);
    scanf("%s", roomType);
    
    printf("Enter price per night for Room %d: $", roomNumber);
    scanf("%lf", &pricePerNight);
    
    rooms[totalRooms].roomNumber = roomNumber;
    strcpy(rooms[totalRooms].roomType, roomType);
    rooms[totalRooms].pricePerNight = pricePerNight;
    totalRooms++;
    
    system("cls");
    printf("\n=== Room Created ===\n");
    printf("Room %d created successfully!\n", roomNumber);
    printf("\nPress any key to continue...");
    getch();
}

void resizeRooms() {
    if (totalRooms >= maxRooms) {
        maxRooms *= 2;
        rooms = (Room*)realloc(rooms, maxRooms * sizeof(Room));
        int i;
        for (i = totalRooms; i < maxRooms; i++) {
            rooms[i].roomNumber = i + 1;
            strcpy(rooms[i].roomType, "Standard");
            rooms[i].pricePerNight = 1500.0;
        }
    }
}

void viewReservationsByRoom() {
    system("cls");
    displayHeader("ROOM RESERVATIONS");
    
    int roomNumber;
    
    printf("Enter room number to view reservations: ");
    scanf("%d", &roomNumber);
    
    if (roomNumber < 1 || roomNumber > totalRooms) {
        printf("\nError: Invalid room number.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    Reservation* current = reservationList;
    int found = 0;
    
    printf("\nReservations for Room %d:\n", roomNumber);
    printf("%-10s %-25s %-25s\n", "Username", "Check-in", "Check-out");
    printf("------------------------------------------------------------------\n");
    
    while (current != NULL) {
        if (current->roomNumber == roomNumber) {
            printf("%-10s %-25s %-25s\n", 
                   current->username, 
                   formatDateTime(current->checkInDate, current->checkInTime),
                   formatDateTime(current->checkOutDate, current->checkOutTime));
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("No reservations found for Room %d.\n", roomNumber);
    }
    
    printf("\nPress any key to continue...");
    getch();
}

void changePassword(char username[]) {
    system("cls");
    displayHeader("CHANGE PASSWORD");
    
    char oldPassword[MAX_PASSWORD_LEN], newPassword[MAX_PASSWORD_LEN];
    
    printf("Enter your current password: ");
    int i = 0;
    char ch;
    
    while ((ch = getch()) != '\r') {
        if (ch == '\b') {
            if (i > 0) {
                printf("\b \b");
                i--;
            }
        } else {
            oldPassword[i++] = ch;
            printf("*");
        }
    }
    oldPassword[i] = '\0';
    
    User* user = authenticateUser(username, oldPassword);
    if (!user) {
        printf("\n\nError: Current password is incorrect.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    printf("\nEnter new password: ");
    i = 0;
    
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
    
    strcpy(user->password, newPassword);
    
    system("cls");
    printf("\n=== Password Changed ===\n");
    printf("Your password has been changed successfully!\n");
    printf("\nPress any key to continue...");
    getch();
}

void modifyReservation(char username[]) {
    system("cls");
    displayHeader("MODIFY RESERVATION");
    
    // Display user's reservations first
    Reservation* current = reservationList;
    int found = 0;
    
    printf("\nYour Current Reservations:\n");
    printf("%-8s %-25s %-25s\n", "Room #", "Check-in", "Check-out");
    printf("------------------------------------------------------------------\n");
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            printf("%-8d %-25s %-25s\n", 
                   current->roomNumber, 
                   formatDateTime(current->checkInDate, current->checkInTime),
                   formatDateTime(current->checkOutDate, current->checkOutTime));
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("You have no reservations to modify.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    int roomNumber;
    printf("\nEnter room number to modify reservation: ");
    scanf("%d", &roomNumber);
    
    current = reservationList;
    found = 0;
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && current->roomNumber == roomNumber) {
            found = 1;
            break;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("\nError: No reservation found for Room %d.\n", roomNumber);
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    // Display current reservation details
    printf("\nCurrent Check-in Date: %s, Check-in Time: %s\n", current->checkInDate, current->checkInTime);
    printf("Current Check-out Date: %s, Check-out Time: %s\n", current->checkOutDate, current->checkOutTime);
    
    // Menu for what to modify
    printf("\nWhat would you like to modify?\n");
    printf("1. Check-in date and time\n");
    printf("2. Check-out date and time\n");
    printf("3. Cancel and go back\n");
    
    int choice;
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    
    char newDate[11], newTime[6];
    
    switch (choice) {
        case 1: // Modify check-in
            printf("Enter new check-in date (YYYY-MM-DD): ");
            scanf("%s", newDate);
            
            if (!isValidDate(newDate)) {
                printf("\nError: Invalid date format.\n");
                printf("\nPress any key to continue...");
                getch();
                return;
            }
            
            printf("Enter new check-in time (HH:MM): ");
            scanf("%s", newTime);
            
            if (!isValidTime(newTime)) {
                printf("\nError: Invalid time format.\n");
                printf("\nPress any key to continue...");
                getch();
                return;
            }
            
            // Validate that new check-in date/time is before check-out date/time
            if (compareDates(newDate, current->checkOutDate) > 0 || 
                (compareDates(newDate, current->checkOutDate) == 0 && 
                 strcmp(newTime, current->checkOutTime) >= 0)) {
                printf("\nError: Check-in date/time must be before check-out date/time.\n");
                printf("\nPress any key to continue...");
                getch();
                return;
            }
            
            // If validation passes, update the reservation
            strcpy(current->checkInDate, newDate);
            strcpy(current->checkInTime, newTime);
            
            system("cls");
            printf("\n=== Reservation Modified ===\n");
            printf("Your check-in has been updated to %s at %s\n", newDate, newTime);
            break;
            
        case 2: // Modify check-out
            printf("Enter new check-out date (YYYY-MM-DD): ");
            scanf("%s", newDate);
            
            if (!isValidDate(newDate)) {
                printf("\nError: Invalid date format.\n");
                printf("\nPress any key to continue...");
                getch();
                return;
            }
            
            printf("Enter new check-out time (HH:MM): ");
            scanf("%s", newTime);
            
            if (!isValidTime(newTime)) {
                printf("\nError: Invalid time format.\n");
                printf("\nPress any key to continue...");
                getch();
                return;
            }
            
            // Validate that check-in date/time is before new check-out date/time
            if (compareDates(current->checkInDate, newDate) > 0 || 
                (compareDates(current->checkInDate, newDate) == 0 && 
                 strcmp(current->checkInTime, newTime) >= 0)) {
                printf("\nError: Check-out date/time must be after check-in date/time.\n");
                printf("\nPress any key to continue...");
                getch();
                return;
            }
            
            // If validation passes, update the reservation
            strcpy(current->checkOutDate, newDate);
            strcpy(current->checkOutTime, newTime);
            
            system("cls");
            printf("\n=== Reservation Modified ===\n");
            printf("Your check-out has been updated to %s at %s\n", newDate, newTime);
            break;
            
        case 3: // Cancel
            printf("Modification cancelled.\n");
            break;
            
        default:
            printf("\nError: Invalid choice.\n");
            break;
    }
    
    printf("\nPress any key to continue...");
    getch();
}

void deleteUser() {
    system("cls");
    displayHeader("DELETE USER");
    
    // Display all users first
    User* currentUser = userList;
    
    printf("\nAll Users:\n");
    printf("%-10s %-8s\n", "Username", "Role");
    printf("-------------------\n");
    
    while (currentUser != NULL) {
        printf("%-10s %-8s\n", 
               currentUser->username, 
               currentUser->isAdmin ? "Admin" : "User");
        currentUser = currentUser->next;
    }
    
    char username[MAX_NAME_LEN];
    printf("\nEnter username to delete: ");
    scanf("%s", username);
    
    // Don't allow deleting the admin account
    if (strcmp(username, "admin") == 0) {
        printf("\nError: Cannot delete the main admin account.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    User* current = userList;
    User* prev = NULL;
    int found = 0;
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            found = 1;
            
            // Delete all reservations for this user first
            Reservation* currentRes = reservationList;
            Reservation* prevRes = NULL;
            
            while (currentRes != NULL) {
                if (strcmp(currentRes->username, username) == 0) {
                    if (prevRes == NULL) {
                        reservationList = currentRes->next;
                        free(currentRes);
                        currentRes = reservationList;
                    } else {
                        prevRes->next = currentRes->next;
                        free(currentRes);
                        currentRes = prevRes->next;
                    }
                } else {
                    prevRes = currentRes;
                    currentRes = currentRes->next;
                }
            }
            
            // Now delete the user
            if (prev == NULL) {
                userList = current->next;
            } else {
                prev->next = current->next;
            }
            
            free(current);
            
            system("cls");
            printf("\n=== User Deleted ===\n");
            printf("User %s and all their reservations have been deleted.\n", username);
            printf("\nPress any key to continue...");
            getch();
            return;
        }
        prev = current;
        current = current->next;
    }
    
    if (!found) {
        printf("\nError: User not found.\n");
        printf("\nPress any key to continue...");
        getch();
    }
}

void viewAllReservations() {
    system("cls");
    displayHeader("ALL RESERVATIONS");
    
    Reservation* current = reservationList;
    
    if (current == NULL) {
        printf("No reservations found.\n");
        printf("\nPress any key to continue...");
        getch();
        return;
    }
    
    printf("\n%-10s %-8s %-25s %-25s\n", "Username", "Room #", "Check-in", "Check-out");
    printf("----------------------------------------------------------------------\n");
    
    while (current != NULL) {
        printf("%-10s %-8d %-25s %-25s\n", 
               current->username, 
               current->roomNumber, 
               // Format check-in date and time together
               formatDateTime(current->checkInDate, current->checkInTime),
               // Format check-out date and time together
               formatDateTime(current->checkOutDate, current->checkOutTime));
        current = current->next;
    }
    
    printf("\nPress any key to continue...");
    getch();
}

void viewStatistics() {
    system("cls");
    displayHeader("HOTEL STATISTICS");
    
    int totalReservations = 0;
    int bookedRooms = 0;
    int i;
    
    Reservation* current = reservationList;
    while (current != NULL) {
        totalReservations++;
        current = current->next;
    }
    
    // Count unique booked rooms
    for (i = 1; i <= totalRooms; i++) {
        if (isRoomBooked(i)) {
            bookedRooms++;
        }
    }
    
    printf("\nTotal Rooms: %d\n", totalRooms);
    printf("Booked Rooms: %d\n", bookedRooms);
    printf("Available Rooms: %d\n", totalRooms - bookedRooms);
    printf("Total Reservations: %d\n", totalReservations);
    printf("Occupancy Rate: %d%%\n", totalRooms > 0 ? (bookedRooms * 100) / totalRooms : 0);
    
    printf("\nPress any key to continue...");
    getch();
}

void searchAvailableRooms() {
    system("cls");
    displayHeader("SEARCH ROOMS");
    
    printf("\nAvailable Room Types:\n");
    printf("1. Standard - Basic comfortable room\n");
    printf("2. Deluxe - Spacious room with better amenities\n");
    printf("3. Suite - Luxury accommodation\n");
    
    char roomType[MAX_ROOM_TYPE_LEN];
    printf("\nEnter room type to search (or 'all' for all types): ");
    scanf("%s", roomType);
    
    printf("\n%s Rooms:\n", strcmp(roomType, "all") == 0 ? "All" : roomType);
    printf("\n%-8s %-20s %-12s %-15s\n", "Room #", "Room Type", "Status", "Price/Night(P)");
    printf("----------------------------------------------------------\n");
    
    int i, found = 0;
    for (i = 0; i < totalRooms; i++) {
        if (strcmp(roomType, "all") == 0 || strcmp(rooms[i].roomType, roomType) == 0) {
            printf("%-8d %-20s %-12s P%-14.0f\n", 
                   rooms[i].roomNumber, 
                   rooms[i].roomType,
                   "Available", // Always show as available
                   rooms[i].pricePerNight);
            found = 1;
        }
    }
    
    if (!found) {
        printf("No rooms of type %s found.\n", roomType);
    }
    
    printf("\nPress any key to continue...");
    getch();
}

void showAdminMenu() {
    int choice;
    
    do {
        system("cls");
        displayHeader("ADMIN MENU");
        
        printf("\n1. View all reservations\n");
        printf("2. View all users\n");
        printf("3. Create a new room\n");
        printf("4. View room reservations\n");
        printf("5. Remove a reservation\n");
        printf("6. Delete a user\n");
        printf("7. View statistics\n");
        printf("8. Search available rooms\n");
        printf("9. Log out\n");
        
        printf("\nEnter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                viewAllReservations();
                break;
            case 2:
                system("cls");
                displayHeader("ALL USERS");
                
                printf("\n%-10s %-8s\n", "Username", "Role");
                printf("-------------------\n");
                
                if (userList == NULL) {
                    printf("No users found.\n");
                } else {
                    User* currentUser = userList;
                    while (currentUser != NULL) {
                        printf("%-10s %-8s\n", 
                               currentUser->username, 
                               currentUser->isAdmin ? "Admin" : "User");
                        currentUser = currentUser->next;
                    }
                }
                
                printf("\nPress any key to continue...");
                getch();
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
                deleteUser();
                break;
            case 7:
                viewStatistics();
                break;
            case 8:
                searchAvailableRooms();
                break;
            case 9:
                system("cls");
                printf("Logging out...\n");
                break;
            default:
                printf("\nError: Invalid choice! Please try again.\n");
                printf("\nPress any key to continue...");
                getch();
                break;
        }
    } while (choice != 9);
}

void showUserMenu(char username[]) {
    int choice;
    
    do {
        system("cls");
        displayHeader("USER MENU");
        
        printf("\n1. Make a reservation\n");
        printf("2. View my reservations\n");
        printf("3. Modify my reservation\n");
        printf("4. Change my password\n");
        printf("5. Search available rooms\n");
        printf("6. Log out\n");
        
        printf("\nEnter choice: ");
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
                searchAvailableRooms();
                break;
            case 6:
                system("cls");
                printf("Logging out...\n");
                break;
            default:
                printf("\nError: Invalid choice!\n");
                printf("\nPress any key to continue...");
                getch();
                break;
        }
    } while (choice != 6);
}

void registerUser() {
    system("cls");
    displayHeader("USER REGISTRATION");
    
    char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
    
    printf("Enter a username: ");
    scanf("%s", username);
    
    // Check if username already exists
    User* current = userList;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            printf("\nError: Username already exists.\n");
            printf("Please choose a different username.\n");
            printf("\nPress any key to continue...");
            getch();
            return;
        }
        current = current->next;
    }
    
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
    
    addUser(username, password, 0);
    
    system("cls");
    printf("\n=== Registration Successful ===\n");
    printf("You can now log in with your credentials.\n");
    printf("\nPress any key to continue...");
    getch();
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

void saveData() {
    FILE* file = fopen(DATA_FILE, "w");
    if (file == NULL) {
        return;
    }
    
    // Save users
    User* currentUser = userList;
    while (currentUser != NULL) {
        fprintf(file, "USER:%s:%s:%d\n", currentUser->username, currentUser->password, currentUser->isAdmin);
        currentUser = currentUser->next;
    }
    
    // Save reservations
    Reservation* currentReservation = reservationList;
    while (currentReservation != NULL) {
        fprintf(file, "RESERVATION:%s:%d:%s:%s:%s:%s\n", 
                currentReservation->username, 
                currentReservation->roomNumber, 
                currentReservation->checkInDate, 
                currentReservation->checkInTime, 
                currentReservation->checkOutDate, 
                currentReservation->checkOutTime);
        currentReservation = currentReservation->next;
    }
    
    // Save room information
    int i;
    for (i = 0; i < totalRooms; i++) {
        fprintf(file, "ROOM:%d:%s:%.2f\n", 
                rooms[i].roomNumber, 
                rooms[i].roomType, 
                rooms[i].pricePerNight);
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
        while (line[i] != ':' && i < 19) {
            type[i] = line[i];
            i++;
        }
        type[i] = '\0';
        
        if (strcmp(type, "USER") == 0) {
            char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
            int isAdmin;
            sscanf(line, "USER:%[^:]:%[^:]:%d", username, password, &isAdmin);
            addUser(username, password, isAdmin);
        } else if (strcmp(type, "RESERVATION") == 0) {
            char username[MAX_NAME_LEN], checkInDate[11], checkInTime[6], checkOutDate[11], checkOutTime[6];
            int roomNumber;
            sscanf(line, "RESERVATION:%[^:]:%d:%[^:]:%[^:]:%[^:]:%[^\n]", 
                   username, &roomNumber, checkInDate, checkInTime, checkOutDate, checkOutTime);
            addReservation(username, roomNumber, checkInDate, checkInTime, checkOutDate, checkOutTime);
        } else if (strcmp(type, "ROOM") == 0) {
            int roomNumber;
            char roomType[MAX_ROOM_TYPE_LEN];
            double pricePerNight;
            sscanf(line, "ROOM:%d:%[^:]:%lf", &roomNumber, roomType, &pricePerNight);
            
            // If we need to expand the rooms array
            while (roomNumber > totalRooms) {
                resizeRooms();
                totalRooms++;
            }
            
            rooms[roomNumber - 1].roomNumber = roomNumber;
            strcpy(rooms[roomNumber - 1].roomType, roomType);
            rooms[roomNumber - 1].pricePerNight = pricePerNight;
        }
    }
    
    fclose(file);
}

void updateRoomPrices() {
	int i;
    for (i = 0; i < totalRooms; i++) {
        if (strcmp(rooms[i].roomType, "Standard") == 0) {
            rooms[i].pricePerNight = 1500.0;
        } else if (strcmp(rooms[i].roomType, "Deluxe") == 0) {
            rooms[i].pricePerNight = 2000.0;
        } else if (strcmp(rooms[i].roomType, "Suite") == 0) {
            rooms[i].pricePerNight = 3000.0;
        }
    }
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
            // This reservation has expired
            if (prev == NULL) {
                reservationList = current->next;
                Reservation* temp = current;
                current = current->next;
                free(temp);
            } else {
                prev->next = current->next;
                Reservation* temp = current;
                current = current->next;
                free(temp);
            }
        } else {
            prev = current;
            current = current->next;
        }
    }
}

int compareDates(char date1[], char date2[]) {
    // Parse the dates into year, month, day components
    int year1, month1, day1;
    int year2, month2, day2;
    
    sscanf(date1, "%d-%d-%d", &year1, &month1, &day1);
    sscanf(date2, "%d-%d-%d", &year2, &month2, &day2);
    
    // Compare years
    if (year1 != year2) {
        return year1 - year2;
    }
    
    // If years are equal, compare months
    if (month1 != month2) {
        return month1 - month2;
    }
    
    // If months are equal, compare days
    return day1 - day2;
}

char* formatDateTime(char date[], char time[]) {
    static char formatted[30];
    sprintf(formatted, "%s %s", date, time);
    return formatted;
}

int main() {
    int option;
    char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
    User* loggedInUser = NULL;
    
    initializeRooms();
    loadData();
    updateRoomPrices(); // Update prices to ensure they're correct
    checkExpiredReservations();
    
    // Add default users if they don't exist
    if (userList == NULL) {
        addUser("admin", "admin123", 1);
        addUser("user1", "password1", 0);
    }
    
    do {
        system("cls");
        displayHeader("HOTEL RESERVATION SYSTEM");
        
        printf("\n1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        
        printf("\nEnter choice: ");
        scanf("%d", &option);
        
        switch (option) {
            case 1:
                registerUser();
                break;
            case 2:
                system("cls");
                displayHeader("USER LOGIN");
                
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
                
                loggedInUser = authenticateUser(username, password);
                
                if (loggedInUser == NULL) {
                    printf("\n\nError: Invalid username or password.\n");
                    printf("Please register first if you haven't.\n");
                    printf("\nPress any key to continue...");
                    getch();
                } else {
                    system("cls");
                    printf("\n=== Login Successful ===\n");
                    printf("Welcome %s!\n", username);
                    printf("\nPress any key to continue...");
                    getch();
                    
                    if (loggedInUser->isAdmin) {
                        showAdminMenu();
                    } else {
                        showUserMenu(username);
                    }
                }
                break;
            case 3:
                system("cls");
                printf("Saving data and exiting...\n");
                saveData();
                break;
            default:
                printf("\nError: Invalid choice!\n");
                printf("\nPress any key to continue...");
                getch();
                break;
        }
    } while (option != 3);
    
    cleanup();
    return 0;
}
