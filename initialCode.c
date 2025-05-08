#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#include <windows.h>

#define MAX_NAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_ROOM_TYPE_LEN 50
#define DATA_FILE "reservations.dat"

// Key codes
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_ENTER 13
#define KEY_ESC 27

// Colors
#ifndef COLOR_NORMAL
#define COLOR_NORMAL 7     // White text on black background
#endif

#ifndef COLOR_HIGHLIGHT
#define COLOR_HIGHLIGHT 240 // Black text on white background
#endif

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
void updateRoomPrices(void);
int isRoomAvailableForDates(int roomNumber, char checkInDate[], char checkOutDate[]);
char* formatDateTime(char date[], char time[]);
int getMenuChoice(char* menuItems[], int itemCount);
void gotoxy(int x, int y);
void setTextColor(int color);
void displayMessage(const char* message);

// Function to position cursor at specific coordinates
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Function to set text color
void setTextColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
// Function to get menu choice using highlighting
int getMenuChoice(char* menuItems[], int itemCount) {
    int selected = 0;
    int key = 0;
    int i;
    
    // Position for menu items
    int menuX = 5; // Changed from 45 to 28 to align with the rest of the interface
    int menuY = 7;  // Vertical position (starting line below the header)
    
    // Display menu items
    for (i = 0; i < itemCount; i++) {
        gotoxy(menuX, menuY + i); // Use menuY for vertical positioning
        
        if (i == selected) {
            setTextColor(COLOR_HIGHLIGHT);
            printf("%s", menuItems[i]);
            setTextColor(COLOR_NORMAL);
        } else {
            printf("%s", menuItems[i]);
        }
    }
    
    // Handle key presses
    while (1) {
        key = getch();
        
        // If arrow key is pressed, getch() returns 0 or 0xE0, then the actual key code
        if (key == 0 || key == 0xE0) {
            key = getch();
            
            if (key == KEY_UP && selected > 0) {
                // Unhighlight current selection
                gotoxy(menuX, menuY + selected);
                setTextColor(COLOR_NORMAL);
                printf("%s", menuItems[selected]);
                
                // Move selection up
                selected--;
                
                // Highlight new selection
                gotoxy(menuX, menuY + selected);
                setTextColor(COLOR_HIGHLIGHT);
                printf("%s", menuItems[selected]);
                setTextColor(COLOR_NORMAL);
            } 
            else if (key == KEY_DOWN && selected < itemCount - 1) {
                // Unhighlight current selection
                gotoxy(menuX, menuY + selected);
                setTextColor(COLOR_NORMAL);
                printf("%s", menuItems[selected]);
                
                // Move selection down
                selected++;
                
                // Highlight new selection
                gotoxy(menuX, menuY + selected);
                setTextColor(COLOR_HIGHLIGHT);
                printf("%s", menuItems[selected]);
                setTextColor(COLOR_NORMAL);
            }
        } 
        else if (key == KEY_ENTER) {
            // Return the selected index + 1 (to match the original menu numbering)
            return selected + 1;
        } 
        else if (key == KEY_ESC) {
            // ESC key pressed, return the last option (usually exit/back)
            return itemCount;
        }
    }
}

void displayHeader(const char* title) {
    system("cls");
    setTextColor(COLOR_NORMAL);
    printf("\n");
    printf("  +--------------------------------------+\n");
    printf("  ¦           %-28s ¦\n", title);
    printf("  +--------------------------------------+\n\n");
}

void displayMessage(const char* message) {
    printf("\n  %s\n", message);
    printf("\n  Press any key to continue...");
    getch();
}

int isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
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
            
            if (!(compareDates(checkOutDate, current->checkInDate) < 0 || 
                  compareDates(checkInDate, current->checkOutDate) > 0)) {
                return 0; 
            }
        }
        current = current->next;
    }
    
    return 1; 
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
            return; 
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
            return current; 
        }
        current = current->next;
    }
    return NULL; 
}

void displayRooms() {
    displayHeader("AVAILABLE ROOMS");
    
    printf("  %-8s %-20s %-12s %-15s\n", "Room #", "Room Type", "Status", "Price/Night(P)");
    printf("  ----------------------------------------------------------\n");
    
    int i;
    for (i = 0; i < totalRooms; i++) {
        printf("  %-8d %-20s %-12s P%-14.0f\n", 
               rooms[i].roomNumber, 
               rooms[i].roomType, 
               "Available", // Always show as available
               rooms[i].pricePerNight);
    }
    
    printf("  ----------------------------------------------------------\n");
}

void makeReservation(char username[]) {
    displayHeader("MAKE A RESERVATION");
    
    int roomNumber;
    char checkInDate[11], checkInTime[6], checkOutDate[11], checkOutTime[6];
    
    displayRooms();
    
    printf("\n  Enter room number to reserve: ");
    scanf("%d", &roomNumber);
    
    if (roomNumber < 1 || roomNumber > totalRooms) {
        displayMessage("Error: Invalid room number.");
        return;
    }
    
    printf("  Enter check-in date (YYYY-MM-DD): ");
    scanf("%10s", checkInDate);
    
    if (!isValidDate(checkInDate)) {
        displayMessage("Error: Invalid date format.");
        return;
    }
    
    printf("  Enter check-in time (HH:MM): ");
    scanf("%5s", checkInTime);
    
    if (!isValidTime(checkInTime)) {
        displayMessage("Error: Invalid time format.");
        return;
    }
    
    printf("  Enter check-out date (YYYY-MM-DD): ");
    scanf("%10s", checkOutDate);
    
    if (!isValidDate(checkOutDate)) {
        displayMessage("Error: Invalid date format.");
        return;
    }
    
    printf("  Enter check-out time (HH:MM): ");
    scanf("%5s", checkOutTime);
    
    if (!isValidTime(checkOutTime)) {
        displayMessage("Error: Invalid time format.");
        return;
    }
    
    // Validate check-in before check-out
    if (compareDates(checkInDate, checkOutDate) > 0 || 
        (compareDates(checkInDate, checkOutDate) == 0 && strcmp(checkInTime, checkOutTime) >= 0)) {
        displayMessage("Error: Check-in date/time must be before check-out date/time.");
        return;
    }
    
    // Check if room is available for the selected dates
    if (!isRoomAvailableForDates(roomNumber, checkInDate, checkOutDate)) {
        char message[100];
        sprintf(message, "Error: Room %d is not available for the selected dates.\nPlease select different dates or a different room.", roomNumber);
        displayMessage(message);
        return;
    }
    
    addReservation(username, roomNumber, checkInDate, checkInTime, checkOutDate, checkOutTime);
    
    // Save data immediately after making a reservation
    saveData();
    
    char message[100];
    sprintf(message, "Reservation Successful\nRoom %d has been reserved for you.", roomNumber);
    displayHeader("RESERVATION SUCCESSFUL");
    displayMessage(message);
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
    displayHeader("REMOVE RESERVATION");
    
    char inputUsername[MAX_NAME_LEN];
    int roomNumber;
    
    printf("  Enter username to remove reservation: ");
    scanf("%s", inputUsername);
    
    Reservation* current = reservationList;
    Reservation* prev = NULL;
    int found = 0;
    
    // Display user's reservations
    printf("\n  Reservations for %s:\n", inputUsername);
    printf("  %-8s %-25s %-25s\n", "Room #", "Check-in", "Check-out");
    printf("  ------------------------------------------------------------------\n");
    
    while (current != NULL) {
        if (strcmp(current->username, inputUsername) == 0) {
            printf("  %-8d %-25s %-25s\n", 
                   current->roomNumber, 
                   formatDateTime(current->checkInDate, current->checkInTime),
                   formatDateTime(current->checkOutDate, current->checkOutTime));
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        char message[100];
        sprintf(message, "No reservations found for user %s.", inputUsername);
        displayMessage(message);
        return;
    }
    
    printf("\n  Enter room number to remove reservation: ");
    scanf("%d", &roomNumber);
    
    if (roomNumber < 1 || roomNumber > totalRooms) {
        displayMessage("Error: Invalid room number.");
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
            
            // Save data immediately after removing a reservation
            saveData();
            
            char message[100];
            sprintf(message, "Reservation for Room %d by %s has been removed successfully.", roomNumber, inputUsername);
            displayHeader("RESERVATION REMOVED");
            displayMessage(message);
            return;
        }
        prev = current;
        current = current->next;
    }
    
    if (!found) {
        displayMessage("No reservation found for this room and user.");
    }
}

void viewReservations(char username[]) {
    displayHeader("YOUR RESERVATIONS");
    
    Reservation* current = reservationList;
    int found = 0;
    
    printf("  %-8s %-25s %-25s\n", "Room #", "Check-in", "Check-out");
    printf("  ------------------------------------------------------------------\n");
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            printf("  %-8d %-25s %-25s\n", 
                   current->roomNumber, 
                   formatDateTime(current->checkInDate, current->checkInTime),
                   formatDateTime(current->checkOutDate, current->checkOutTime));
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        char message[100];
        sprintf(message, "No reservations found for %s.", username);
        printf("  %s\n", message);
    }
    
    displayMessage("");
}

void createRoom() {
    displayHeader("CREATE NEW ROOM");
    
    resizeRooms();
    int roomNumber = totalRooms + 1;
    char roomType[MAX_ROOM_TYPE_LEN];
    double pricePerNight;
    
    printf("  Enter room type for Room %d: ", roomNumber);
    scanf("%s", roomType);
    
    printf("  Enter price per night for Room %d: $", roomNumber);
    scanf("%lf", &pricePerNight);
    
    rooms[totalRooms].roomNumber = roomNumber;
    strcpy(rooms[totalRooms].roomType, roomType);
    rooms[totalRooms].pricePerNight = pricePerNight;
    totalRooms++;
    
    // Save data immediately after creating a room
    saveData();
    
    char message[100];
    sprintf(message, "Room %d created successfully!", roomNumber);
    displayHeader("ROOM CREATED");
    displayMessage(message);
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
    displayHeader("ROOM RESERVATIONS");
    
    int roomNumber;
    
    printf("  Enter room number to view reservations: ");
    scanf("%d", &roomNumber);
    
    if (roomNumber < 1 || roomNumber > totalRooms) {
        displayMessage("Error: Invalid room number.");
        return;
    }
    
    Reservation* current = reservationList;
    int found = 0;
    
    printf("\n  Reservations for Room %d:\n", roomNumber);
    printf("  %-10s %-25s %-25s\n", "Username", "Check-in", "Check-out");
    printf("  ------------------------------------------------------------------\n");
    
    while (current != NULL) {
        if (current->roomNumber == roomNumber) {
            printf("  %-10s %-25s %-25s\n", 
                   current->username, 
                   formatDateTime(current->checkInDate, current->checkInTime),
                   formatDateTime(current->checkOutDate, current->checkOutTime));
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        char message[100];
        sprintf(message, "No reservations found for Room %d.", roomNumber);
        printf("  %s\n", message);
    }
    
    displayMessage("");
}

void changePassword(char username[]) {
    displayHeader("CHANGE PASSWORD");
    
    char oldPassword[MAX_PASSWORD_LEN], newPassword[MAX_PASSWORD_LEN];
    
    printf("  Enter your current password: ");
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
        displayMessage("Error: Current password is incorrect.");
        return;
    }
    
    printf("\n  Enter new password: ");
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
    
    // Save data immediately after changing password
    saveData();
    
    displayHeader("PASSWORD CHANGED");
    displayMessage("Your password has been changed successfully!");
}
void modifyReservation(char username[]) {
    displayHeader("MODIFY RESERVATION");
    
    // Display user's reservations first
    Reservation* current = reservationList;
    int found = 0;
    
    printf("  Your Current Reservations:\n");
    printf("  %-8s %-25s %-25s\n", "Room #", "Check-in", "Check-out");
    printf("  ------------------------------------------------------------------\n");
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            printf("  %-8d %-25s %-25s\n", 
                   current->roomNumber, 
                   formatDateTime(current->checkInDate, current->checkInTime),
                   formatDateTime(current->checkOutDate, current->checkOutTime));
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        displayMessage("You have no reservations to modify.");
        return;
    }
    
    int roomNumber;
    printf("\n  Enter room number to modify reservation: ");
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
        char message[100];
        sprintf(message, "Error: No reservation found for Room %d.", roomNumber);
        displayMessage(message);
        return;
    }
    
    // Display current reservation details
    printf("\n  Current Check-in Date: %s, Check-in Time: %s\n", current->checkInDate, current->checkInTime);
    printf("  Current Check-out Date: %s, Check-out Time: %s\n", current->checkOutDate, current->checkOutTime);
    
    // Instead of using getMenuChoice, directly display the options and get input
    printf("\n  What would you like to modify?\n");
    printf("  1. Check-in date and time\n");
    printf("  2. Check-out date and time\n");
    printf("  3. Cancel and go back\n\n");
    
    printf("  Enter your choice (1-3): ");
    int choice;
    scanf("%d", &choice);
    
    char newDate[11], newTime[6];
    
    switch (choice) {
        case 1: // Modify check-in
            printf("\n  Enter new check-in date (YYYY-MM-DD): ");
            scanf("%s", newDate);
            
            if (!isValidDate(newDate)) {
                displayMessage("Error: Invalid date format.");
                return;
            }
            
            printf("  Enter new check-in time (HH:MM): ");
            scanf("%s", newTime);
            
            if (!isValidTime(newTime)) {
                displayMessage("Error: Invalid time format.");
                return;
            }
            
            // Validate that new check-in date/time is before check-out date/time
            if (compareDates(newDate, current->checkOutDate) > 0 || 
                (compareDates(newDate, current->checkOutDate) == 0 && 
                 strcmp(newTime, current->checkOutTime) >= 0)) {
                displayMessage("Error: Check-in date/time must be before check-out date/time.");
                return;
            }
            
            // If validation passes, update the reservation
            strcpy(current->checkInDate, newDate);
            strcpy(current->checkInTime, newTime);
            
            // Save data immediately after modifying a reservation
            saveData();
            
            char message1[100];
            sprintf(message1, "Your check-in has been updated to %s at %s", newDate, newTime);
            displayHeader("RESERVATION MODIFIED");
            displayMessage(message1);
            break;
            
        case 2: // Modify check-out
            printf("\n  Enter new check-out date (YYYY-MM-DD): ");
            scanf("%s", newDate);
            
            if (!isValidDate(newDate)) {
                displayMessage("Error: Invalid date format.");
                return;
            }
            
            printf("  Enter new check-out time (HH:MM): ");
            scanf("%s", newTime);
            
            if (!isValidTime(newTime)) {
                displayMessage("Error: Invalid time format.");
                return;
            }
            
            // Validate that check-in date/time is before new check-out date/time
            if (compareDates(current->checkInDate, newDate) > 0 || 
                (compareDates(current->checkInDate, newDate) == 0 && 
                 strcmp(current->checkInTime, newTime) >= 0)) {
                displayMessage("Error: Check-out date/time must be after check-in date/time.");
                return;
            }
            
            // If validation passes, update the reservation
            strcpy(current->checkOutDate, newDate);
            strcpy(current->checkOutTime, newTime);
            
            // Save data immediately after modifying a reservation
            saveData();
            
            char message2[100];
            sprintf(message2, "Your check-out has been updated to %s at %s", newDate, newTime);
            displayHeader("RESERVATION MODIFIED");
            displayMessage(message2);
            break;
            
        case 3: // Cancel
            displayMessage("Modification cancelled.");
            break;
            
        default:
            displayMessage("Error: Invalid choice.");
            break;
    }
}

void deleteUser() {
    displayHeader("DELETE USER");
    
    // Display all users first
    User* currentUser = userList;
    
    printf("  All Users:\n");
    printf("  %-10s %-8s\n", "Username", "Role");
    printf("  -------------------\n");
    
    while (currentUser != NULL) {
        printf("  %-10s %-8s\n", 
               currentUser->username, 
               currentUser->isAdmin ? "Admin" : "User");
        currentUser = currentUser->next;
    }
    
    char username[MAX_NAME_LEN];
    printf("\n  Enter username to delete: ");
    scanf("%s", username);
    
    // Don't allow deleting the admin account
    if (strcmp(username, "admin") == 0) {
        displayMessage("Error: Cannot delete the main admin account.");
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
            
            // Save data immediately after deleting a user
            saveData();
            
            char message[100];
            sprintf(message, "User %s and all their reservations have been deleted.", username);
            displayHeader("USER DELETED");
            displayMessage(message);
            return;
        }
        prev = current;
        current = current->next;
    }
    
    if (!found) {
        displayMessage("Error: User not found.");
    }
}

void viewAllReservations() {
    displayHeader("ALL RESERVATIONS");
    
    Reservation* current = reservationList;
    
    if (current == NULL) {
        displayMessage("No reservations found.");
        return;
    }
    
    printf("  %-10s %-8s %-25s %-25s\n", "Username", "Room #", "Check-in", "Check-out");
    printf("  ----------------------------------------------------------------------\n");
    
    while (current != NULL) {
        printf("  %-10s %-8d %-25s %-25s\n", 
               current->username, 
               current->roomNumber, 
               // Format check-in date and time together
               formatDateTime(current->checkInDate, current->checkInTime),
               // Format check-out date and time together
               formatDateTime(current->checkOutDate, current->checkOutTime));
        current = current->next;
    }
    
    displayMessage("");
}

void viewStatistics() {
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
    
    printf("  Total Rooms: %d\n", totalRooms);
    printf("  Booked Rooms: %d\n", bookedRooms);
    printf("  Available Rooms: %d\n", totalRooms - bookedRooms);
    printf("  Total Reservations: %d\n", totalReservations);
    printf("  Occupancy Rate: %d%%\n", totalRooms > 0 ? (bookedRooms * 100) / totalRooms : 0);
    
    displayMessage("");
}

void searchAvailableRooms() {
    displayHeader("SEARCH ROOMS");
    
    printf("  Available Room Types:\n");
    printf("  1. Standard - Basic comfortable room\n");
    printf("  2. Deluxe - Spacious room with better amenities\n");
    printf("  3. Suite - Luxury accommodation\n");
    
    char roomType[MAX_ROOM_TYPE_LEN];
    printf("\n  Enter room type to search (or 'all' for all types): ");
    scanf("%s", roomType);
    
    printf("\n  %s Rooms:\n", strcmp(roomType, "all") == 0 ? "All" : roomType);
    printf("\n  %-8s %-20s %-12s %-15s\n", "Room #", "Room Type", "Status", "Price/Night(P)");
    printf("  ----------------------------------------------------------\n");
    
    int i, found = 0;
    for (i = 0; i < totalRooms; i++) {
        if (strcmp(roomType, "all") == 0 || strcmp(rooms[i].roomType, roomType) == 0) {
            printf("  %-8d %-20s %-12s P%-14.0f\n", 
                   rooms[i].roomNumber, 
                   rooms[i].roomType,
                   "Available", // Always show as available
                   rooms[i].pricePerNight);
            found = 1;
        }
    }
    
    if (!found) {
        char message[100];
        sprintf(message, "No rooms of type %s found.", roomType);
        printf("  %s\n", message);
    }
    
    displayMessage("");
}

void showAdminMenu() {
    int choice;
    
    do {
        displayHeader("ADMIN MENU");
        
        char* adminOptions[] = {
            "View all reservations",
            "View all users",
            "Create a new room",
            "View room reservations",
            "Remove a reservation",
            "Delete a user",
            "View statistics",
            "Search available rooms",
            "Log out"
        };
        
        printf("  Use UP/DOWN keys to navigate and ENTER to select:\n\n");
        choice = getMenuChoice(adminOptions, 9);
        
        switch (choice) {
            case 1:
                viewAllReservations();
                break;
            case 2:
                displayHeader("ALL USERS");
                
                printf("  %-10s %-8s\n", "Username", "Role");
                printf("  -------------------\n");
                
                if (userList == NULL) {
                    printf("  No users found.\n");
                } else {
                    User* currentUser = userList;
                    while (currentUser != NULL) {
                        printf("  %-10s %-8s\n", 
                               currentUser->username, 
                               currentUser->isAdmin ? "Admin" : "User");
                        currentUser = currentUser->next;
                    }
                }
                
                displayMessage("");
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
    displayHeader("LOGGING OUT");
    printf("  Saving data and logging out...\n");
    
    // Save data before logout
    saveData();
    
    // Don't do cleanup or reload here
    break;
        }
    } while (choice != 9);
}

void showUserMenu(char username[]) {
    int choice;
    
    do {
        displayHeader("USER MENU");
        
        char* userOptions[] = {
            "Make a reservation",
            "View my reservations",
            "Modify my reservation",
            "Change my password",
            "Search available rooms",
            "Log out"
        };
        
        printf("  Use UP/DOWN keys to navigate and ENTER to select:\n\n");
        choice = getMenuChoice(userOptions, 6);
        
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
    displayHeader("LOGGING OUT");
    printf("  Saving data and logging out...\n");
    
    // IMPORTANT: Save data before logout
    saveData();
    
    // Don't clear the lists here - we'll do it in main() after returning
    break;
            default:
                displayMessage("Error: Invalid choice!");
                break;
        }
    } while (choice != 6);
}

void registerUser() {
    displayHeader("USER REGISTRATION");
    
    char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
    
    printf("  Enter a username: ");
    scanf("%s", username);
    
    // Check if username already exists
    User* current = userList;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            displayMessage("Error: Username already exists.\nPlease choose a different username.");
            return;
        }
        current = current->next;
    }
    
    printf("  Enter a password: ");
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
    
    // Save data immediately after registering a user
    saveData();
    
    displayHeader("REGISTRATION SUCCESSFUL");
    displayMessage("You can now log in with your credentials.");
}

void cleanup() {
    User* currentUser = userList;
    while (currentUser != NULL) {
        User* temp = currentUser;
        currentUser = currentUser->next;
        free(temp);
    }
    userList = NULL;
    
    Reservation* currentReservation = reservationList;
    while (currentReservation != NULL) {
        Reservation* temp = currentReservation;
        currentReservation = currentReservation->next;
        free(temp);
    }
    reservationList = NULL;
    
    free(rooms);
    rooms = NULL;
}

void saveData() {
    FILE* file = fopen(DATA_FILE, "w");
    if (file == NULL) {
        displayMessage("Error: Could not open file for writing.");
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
        // File doesn't exist yet, not an error
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

void updateRoomPrices(void) {
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
    char* mainOptions[] = {
        "Register",
        "Login",
        "Exit"
    };
    
    int option;
    char username[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
    User* loggedInUser = NULL;
    
    // Initialize and load data only once at program start
    initializeRooms();
    loadData();
    updateRoomPrices();
    checkExpiredReservations();
    
    // Add default users if they don't exist
    if (userList == NULL) {
        addUser("admin", "admin123", 1);
        addUser("user1", "password1", 0);
    }
    
    do {
        displayHeader("HOTEL RESERVATION SYSTEM");
        
        printf("  Use UP/DOWN keys to navigate and ENTER to select:\n\n");
        option = getMenuChoice(mainOptions, 3);
        
        switch (option) {
            case 1:
                registerUser();
                break;
            case 2:
                displayHeader("USER LOGIN");
                
                printf("  Enter username: ");
                scanf("%s", username);
                
                printf("  Enter password: ");
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
                    printf("\n\n  Error: Invalid username or password.\n");
                    printf("  Please register first if you haven't.\n");
                    printf("\n  Press any key to continue...");
                    getch();
                } else {
                    displayHeader("LOGIN SUCCESSFUL");
                    printf("  Welcome %s!\n", username);
                    printf("\n  Press any key to continue...");
                    getch();
                    
                    if (loggedInUser->isAdmin) {
                        showAdminMenu();
                    } else {
                        showUserMenu(username);
                    }
                    
                    // No cleanup or reload needed here
                }
                break;
            case 3:
                displayHeader("EXITING");
                printf("  Saving data and exiting...\n");
                saveData();
                break;
            default:
                displayMessage("Error: Invalid choice!");
                break;
        }
    } while (option != 3);
    
    cleanup(); // Only clean up at program exit
    return 0;
}
