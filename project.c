#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SLOTS 50
#define MAX_SUBSCRIPTIONS 100
#define FILE_NAME "parking_slots.csv"
#define HISTORY_FILE "parking_history.csv"
#define SUBSCRIPTION_FILE "subscriptions.csv"

// Rate configuration (FR5)
#define CAR_RATE 50.0
#define BIKE_RATE 20.0
#define TRUCK_RATE 100.0

typedef struct {
    int slotNumber;
    char plateNumber[20];
    int vehicleType; // 1=Car, 2=Bike, 3=Truck
    time_t entryTime;
    int isOccupied;  // 0=Free, 1=Occupied, 2=Maintenance
} Slot;

typedef struct {
    char plateNumber[20];
    char ownerName[50];
    int isRegistered;
    int isSubscribed;
    int paymentType; // 1=Mobile Banking, 2=Credit Card
    char paymentMethod[30];
} SubscriptionEntry;

Slot parkingLot[MAX_SLOTS];
SubscriptionEntry subscriptions[MAX_SUBSCRIPTIONS];

// Function Prototypes
void initializeSystem();
void saveToFile();
void loadFromFile();
void checkIn();
void checkOut();
void displayAvailableSlots();
void adminMenu();
void generateBill(Slot *s, double duration, double total);
void appendToHistory(Slot *s, double duration, double total);
void initializeSubscriptions();
void saveSubscriptionsToFile();
void loadSubscriptionsFromFile();
void registerVehicle();
void subscribeVehicle();
void simulateIrSensorEntry();
void clearInputBuffer();
int findSubscriptionIndex(const char *plateNumber);
int canAccessParking(const char *plateNumber);
void printPaymentOptions(int paymentType);

int main() {
    int choice;
    initializeSystem();
    loadFromFile();
    loadSubscriptionsFromFile();

    while (1) {
        printf("\n=========================================\n");
        printf("    PARKING LOT MANAGEMENT SYSTEM\n");
        printf("=========================================\n");
        printf("1. Check-In Vehicle\n");
        printf("2. Check-Out & Generate Bill\n");
        printf("3. View Available Slots\n");
        printf("4. Admin / Manager Menu\n");
        printf("5. Register Vehicle & Subscribe\n");
        printf("6. Simulate IR Sensor Entry\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clearInputBuffer();

        switch (choice) {
            case 1: checkIn(); break;
            case 2: checkOut(); break;
            case 3: displayAvailableSlots(); break;
            case 4: adminMenu(); break;
            case 5: registerVehicle(); break;
            case 6: simulateIrSensorEntry(); break;
            case 7:
                saveToFile();
                saveSubscriptionsToFile();
                printf("Exiting system. Data saved successfully.\n");
                exit(0);
            default: printf("Invalid choice! Please try again.\n");
        }
    }
    return 0;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void initializeSystem() {
    for (int i = 0; i < MAX_SLOTS; i++) {
        parkingLot[i].slotNumber = i + 1;
        parkingLot[i].isOccupied = 0;
        strcpy(parkingLot[i].plateNumber, "");
        parkingLot[i].vehicleType = 0;
        parkingLot[i].entryTime = 0;
    }
    initializeSubscriptions();
}

void initializeSubscriptions() {
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        subscriptions[i].plateNumber[0] = '\0';
        subscriptions[i].ownerName[0] = '\0';
        subscriptions[i].isRegistered = 0;
        subscriptions[i].isSubscribed = 0;
        subscriptions[i].paymentType = 0;
        subscriptions[i].paymentMethod[0] = '\0';
    }
}

void saveToFile() {
    FILE *fp = fopen(FILE_NAME, "w");
    if (fp == NULL) return;

    for (int i = 0; i < MAX_SLOTS; i++) {
        fprintf(fp, "%d,%s,%d,%ld,%d\n",
            parkingLot[i].slotNumber,
            strlen(parkingLot[i].plateNumber) > 0 ? parkingLot[i].plateNumber : "NONE",
            parkingLot[i].vehicleType,
            (long)parkingLot[i].entryTime,
            parkingLot[i].isOccupied);
    }
    fclose(fp);
}

void loadFromFile() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) return;

    for (int i = 0; i < MAX_SLOTS; i++) {
        long entryTimeTemp;
        fscanf(fp, "%d,%[^,],%d,%ld,%d\n",
            &parkingLot[i].slotNumber,
            parkingLot[i].plateNumber,
            &parkingLot[i].vehicleType,
            &entryTimeTemp,
            &parkingLot[i].isOccupied);

        parkingLot[i].entryTime = (time_t)entryTimeTemp;
        if (strcmp(parkingLot[i].plateNumber, "NONE") == 0) {
            strcpy(parkingLot[i].plateNumber, "");
        }
    }
    fclose(fp);
}

void saveSubscriptionsToFile() {
    FILE *fp = fopen(SUBSCRIPTION_FILE, "w");
    if (fp == NULL) return;

    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (subscriptions[i].isRegistered) {
            fprintf(fp, "%s,%s,%d,%d,%d,%s\n",
                subscriptions[i].plateNumber,
                subscriptions[i].ownerName,
                subscriptions[i].isRegistered,
                subscriptions[i].isSubscribed,
                subscriptions[i].paymentType,
                subscriptions[i].paymentMethod);
        }
    }
    fclose(fp);
}

void loadSubscriptionsFromFile() {
    FILE *fp = fopen(SUBSCRIPTION_FILE, "r");
    if (fp == NULL) return;

    int i = 0;
    while (i < MAX_SUBSCRIPTIONS && fscanf(fp, "%19[^,],%49[^,],%d,%d,%d,%29[^\n]\n",
            subscriptions[i].plateNumber,
            subscriptions[i].ownerName,
            &subscriptions[i].isRegistered,
            &subscriptions[i].isSubscribed,
            &subscriptions[i].paymentType,
            subscriptions[i].paymentMethod) == 6) {
        i++;
    }
    fclose(fp);
}

void displayAvailableSlots() {
    int freeCount = 0;
    printf("\n--- Available Slots ---\n");
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (parkingLot[i].isOccupied == 0) {
            printf("Slot %d ", parkingLot[i].slotNumber);
            freeCount++;
            if (freeCount % 10 == 0) printf("\n");
        }
    }
    printf("\nTotal Available: %d / %d\n", freeCount, MAX_SLOTS);
}

int findSubscriptionIndex(const char *plateNumber) {
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (subscriptions[i].isRegistered && strcmp(subscriptions[i].plateNumber, plateNumber) == 0) {
            return i;
        }
    }
    return -1;
}

int canAccessParking(const char *plateNumber) {
    int index = findSubscriptionIndex(plateNumber);
    return index != -1 && subscriptions[index].isSubscribed == 1;
}

void printPaymentOptions(int paymentType) {
    if (paymentType == 1) {
        printf("Select Mobile Banking Provider:\n");
        printf("1. BKash\n2. Nagad\n3. Upay\n4. Rocket\n");
    } else if (paymentType == 2) {
        printf("Select Credit Card Provider:\n");
        printf("1. Visa\n2. Master Card\n3. Unipay\n4. American Express\n");
    }
}

void subscribeVehicle() {
    char plateNumber[20];
    int index;
    int paymentType;
    int option;

    printf("\n--- Subscribe Vehicle ---\n");
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);

    index = findSubscriptionIndex(plateNumber);
    if (index == -1) {
        printf("Vehicle is not registered yet. Please register first.\n");
        return;
    }

    printf("Choose Payment Type:\n1. Mobile Banking\n2. Credit Card\n");
    printf("Enter choice: ");
    scanf("%d", &paymentType);

    if (paymentType != 1 && paymentType != 2) {
        printf("Invalid payment type.\n");
        return;
    }

    printPaymentOptions(paymentType);
    printf("Enter option: ");
    scanf("%d", &option);

    if (paymentType == 1) {
        const char *providers[] = {"BKash", "Nagad", "Upay", "Rocket"};
        if (option >= 1 && option <= 4) {
            strcpy(subscriptions[index].paymentMethod, providers[option - 1]);
        } else {
            printf("Invalid mobile banking option.\n");
            return;
        }
    } else {
        const char *providers[] = {"Visa", "MasterCard", "UnionPay", "American Express"};
        if (option >= 1 && option <= 4) {
            strcpy(subscriptions[index].paymentMethod, providers[option - 1]);
        } else {
            printf("Invalid credit card option.\n");
            return;
        }
    }

    subscriptions[index].paymentType = paymentType;
    subscriptions[index].isSubscribed = 1;
    saveSubscriptionsToFile();
    printf("Subscription successful using %s.\n", subscriptions[index].paymentMethod);
}

void registerVehicle() {
    char ownerName[50];
    char plateNumber[20];
    int choice;
    int index = -1;

    printf("\n--- Register Vehicle ---\n");
    printf("Enter Owner Name: ");
    scanf("%s", ownerName);
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);

    index = findSubscriptionIndex(plateNumber);
    if (index == -1) {
        for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
            if (!subscriptions[i].isRegistered) {
                index = i;
                break;
            }
        }
    }

    if (index == -1) {
        printf("Subscription list is full.\n");
        return;
    }

    strcpy(subscriptions[index].plateNumber, plateNumber);
    strcpy(subscriptions[index].ownerName, ownerName);
    subscriptions[index].isRegistered = 1;
    subscriptions[index].isSubscribed = 0;
    subscriptions[index].paymentType = 0;
    strcpy(subscriptions[index].paymentMethod, "Not subscribed");

    printf("Do you want to subscribe now? (1=Yes, 0=No): ");
    scanf("%d", &choice);
    if (choice == 1) {
        subscribeVehicle();
    } else {
        saveSubscriptionsToFile();
        printf("Vehicle registered successfully but not subscribed yet.\n");
    }
}

void simulateIrSensorEntry() {
    char plateNumber[20];

    printf("\n--- IR Sensor Check ---\n");
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);

    if (canAccessParking(plateNumber)) {
        printf("IR sensor detected a subscribed vehicle.\n");
        printf("Entrance barrier opened automatically.\n");
    } else {
        printf("IR sensor detected an unsubscribed vehicle.\n");
        printf("Entrance barrier remained closed.\n");
    }
}

void checkIn() {
    int slotIndex = -1;
    char plateNumber[20];
    int vehicleType;

    for (int i = 0; i < MAX_SLOTS; i++) {
        if (parkingLot[i].isOccupied == 0) {
            slotIndex = i;
            break;
        }
    }

    if (slotIndex == -1) {
        printf("\nSorry, the parking lot is currently full!\n");
        return;
    }

    printf("\n--- Vehicle Check-In ---\n");
    printf("Allocated Slot: %d\n", parkingLot[slotIndex].slotNumber);

    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);
    printf("Select Vehicle Type (1-Car, 2-Bike, 3-Truck): ");
    scanf("%d", &vehicleType);

    if (!canAccessParking(plateNumber)) {
        printf("IR sensor detected an unsubscribed vehicle.\n");
        printf("Entrance barrier remained closed.\n");
        return;
    }

    strcpy(parkingLot[slotIndex].plateNumber, plateNumber);
    parkingLot[slotIndex].vehicleType = vehicleType;
    parkingLot[slotIndex].entryTime = time(NULL);
    parkingLot[slotIndex].isOccupied = 1;

    saveToFile();
    printf("IR sensor detected a subscribed vehicle.\n");
    printf("Entrance barrier opened automatically.\n");
    printf("Check-in successful! Auto-logged entry time.\n");
}

void checkOut() {
    char searchPlate[20];
    int found = -1;

    printf("\n--- Vehicle Check-Out ---\n");
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", searchPlate);

    for (int i = 0; i < MAX_SLOTS; i++) {
        if (parkingLot[i].isOccupied == 1 && strcmp(parkingLot[i].plateNumber, searchPlate) == 0) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        printf("Vehicle not found in the active parking lot!\n");
        return;
    }

    time_t exitTime = time(NULL);
    double durationSeconds = difftime(exitTime, parkingLot[found].entryTime);
    double durationHours = (durationSeconds / 3600.0) + 1;

    double rate = 0;
    if (parkingLot[found].vehicleType == 1) rate = CAR_RATE;
    else if (parkingLot[found].vehicleType == 2) rate = BIKE_RATE;
    else if (parkingLot[found].vehicleType == 3) rate = TRUCK_RATE;

    double totalCharge = durationHours * rate;

    generateBill(&parkingLot[found], durationHours, totalCharge);
    appendToHistory(&parkingLot[found], durationHours, totalCharge);

    parkingLot[found].isOccupied = 0;
    strcpy(parkingLot[found].plateNumber, "");
    parkingLot[found].vehicleType = 0;
    parkingLot[found].entryTime = 0;

    saveToFile();
}

void generateBill(Slot *s, double duration, double total) {
    printf("\n=========================================\n");
    printf("             PARKING RECEIPT\n");
    printf("=========================================\n");
    printf("Slot Number   : %d\n", s->slotNumber);
    printf("Plate Number  : %s\n", s->plateNumber);
    printf("Vehicle Type  : %s\n", s->vehicleType == 1 ? "Car" : (s->vehicleType == 2 ? "Bike" : "Truck"));
    printf("Duration (hrs): %.2f\n", duration);
    printf("Total Charge  : $%.2f\n", total);
    printf("=========================================\n");
}

void appendToHistory(Slot *s, double duration, double total) {
    FILE *fp = fopen(HISTORY_FILE, "a");
    if (fp != NULL) {
        fprintf(fp, "Plate:%s, Slot:%d, Type:%d, Duration:%.2f, Paid:%.2f\n",
                s->plateNumber, s->slotNumber, s->vehicleType, duration, total);
        fclose(fp);
    }
}

void adminMenu() {
    int adminChoice, slotToManage;
    printf("\n--- Admin Management ---\n");
    printf("1. View Full Slot Status\n");
    printf("2. Disable Slot (Maintenance)\n");
    printf("3. Enable Slot\n");
    printf("4. View History Log\n");
    printf("5. View Subscription Records\n");
    printf("Enter choice: ");
    scanf("%d", &adminChoice);

    if (adminChoice == 1) {
        for (int i = 0; i < MAX_SLOTS; i++) {
            printf("Slot %d: ", parkingLot[i].slotNumber);
            if (parkingLot[i].isOccupied == 1) printf("Occupied (%s)\n", parkingLot[i].plateNumber);
            else if (parkingLot[i].isOccupied == 2) printf("MAINTENANCE\n");
            else printf("Free\n");
        }
    } else if (adminChoice == 2) {
        printf("Enter slot to disable: ");
        scanf("%d", &slotToManage);
        if (slotToManage >= 1 && slotToManage <= MAX_SLOTS) {
            parkingLot[slotToManage - 1].isOccupied = 2;
            saveToFile();
            printf("Slot %d disabled.\n", slotToManage);
        }
    } else if (adminChoice == 3) {
        printf("Enter slot to enable: ");
        scanf("%d", &slotToManage);
        if (slotToManage >= 1 && slotToManage <= MAX_SLOTS) {
            parkingLot[slotToManage - 1].isOccupied = 0;
            saveToFile();
            printf("Slot %d enabled.\n", slotToManage);
        }
    } else if (adminChoice == 4) {
        FILE *fp = fopen(HISTORY_FILE, "r");
        char buffer[255];
        if (fp == NULL) {
            printf("No history found.\n");
        } else {
            printf("\n--- History Log ---\n");
            while (fgets(buffer, sizeof(buffer), fp)) {
                printf("%s", buffer);
            }
            fclose(fp);
        }
    } else if (adminChoice == 5) {
        printf("\n--- Subscription Records ---\n");
        for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
            if (subscriptions[i].isRegistered) {
                printf("Plate: %s | Owner: %s | Registered: %s | Subscribed: %s | Payment: %s\n",
                    subscriptions[i].plateNumber,
                    subscriptions[i].ownerName,
                    subscriptions[i].isRegistered ? "Yes" : "No",
                    subscriptions[i].isSubscribed ? "Yes" : "No",
                    subscriptions[i].paymentMethod[0] != '\0' ? subscriptions[i].paymentMethod : "None");
            }
        }
    }
}