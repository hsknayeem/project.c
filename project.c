#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SLOTS 100
#define MAX_SUBSCRIPTIONS 100
#define FILE_NAME "parking_slots.csv"
#define HISTORY_FILE "parking_history.csv"
#define SUBSCRIPTION_FILE "subscriptions.csv"
#define PAYMENT_FILE "payment_transactions.csv"
#define CASH_RECEIPT_FILE "cash_receipts.csv"

#define CAR_RATE 50.0
#define BIKE_RATE 20.0
#define TRUCK_RATE 100.0
#define MONTHLY_PACKAGE_FEE 500.0

typedef struct {
    int slotNumber;
    char plateNumber[20];
    int vehicleType;
    time_t entryTime;
    int isOccupied;
    int accessType;
    int paymentType;
    char paymentMethod[30];
} Slot;

typedef struct {
    char plateNumber[20];
    char ownerName[50];
    char phoneNumber[20];
    char pin[10];
    int isRegistered;
    int isSubscribed;
    int otpVerified;
    int paymentType;
    char paymentMethod[30];
    time_t subscriptionStart;
    time_t subscriptionEnd;
} SubscriptionEntry;

Slot parkingLot[MAX_SLOTS];
SubscriptionEntry subscriptions[MAX_SUBSCRIPTIONS];

void initializeSystem();
void saveToFile();
void loadFromFile();
void initializeSubscriptions();
void saveSubscriptionsToFile();
void loadSubscriptionsFromFile();
void saveTransactionToFile(const char *plateNumber, const char *ownerName,
                           const char *phoneNumber, int paymentType,
                           const char *paymentMethod, int status,
                           const char *details, double amount);
void saveCashReceiptToFile(const Slot *s, double duration, double total,
                           const char *paymentMethod, const char *provider);
void clearInputBuffer();
void printPaymentOptions(int paymentType);
void printMainMenu();
void registerVehicle();
void subscribeVehicle();
void checkIn();
void checkOut();
void displayAvailableSlots();
void adminMenu();
void generateBill(const Slot *s, double duration, double total, int accessType);
void appendToHistory(const Slot *s, double duration, double total, const char *eventType,
                     const char *details);
void simulateIrSensorEntry();
void simulateIrSensorExit();
int findSubscriptionIndex(const char *plateNumber);
int canAccessParking(const char *plateNumber);
int findSlotIndexByPlate(const char *plateNumber);
int getAvailableSlotForAccess(int accessType);
void logSensorEvent(const char *eventType, const Slot *s);

int main() {
    int choice;
    srand((unsigned int)time(NULL));

    initializeSystem();
    loadFromFile();
    loadSubscriptionsFromFile();

    while (1) {
        printMainMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clearInputBuffer();

        switch (choice) {
            case 1: registerVehicle(); break;
            case 2: subscribeVehicle(); break;
            case 3: checkIn(); break;
            case 4: checkOut(); break;
            case 5: displayAvailableSlots(); break;
            case 6: adminMenu(); break;
            case 7: simulateIrSensorEntry(); break;
            case 8: simulateIrSensorExit(); break;
            case 9:
                saveToFile();
                saveSubscriptionsToFile();
                printf("Exiting system. Data saved successfully.\n");
                exit(0);
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }

    return 0;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void printMainMenu() {
    printf("\n=========================================\n");
    printf("    PARKING LOT MANAGEMENT SYSTEM\n");
    printf("=========================================\n");
    printf("1. Register Vehicle\n");
    printf("2. Subscribe to Monthly Package\n");
    printf("3. Check-In Vehicle\n");
    printf("4. Check-Out & Generate Bill\n");
    printf("5. View Available Slots\n");
    printf("6. Admin / Manager Menu\n");
    printf("7. Simulate IR Sensor Entry\n");
    printf("8. Simulate IR Sensor Exit\n");
    printf("9. Exit\n");
}

void initializeSystem() {
    for (int i = 0; i < MAX_SLOTS; i++) {
        parkingLot[i].slotNumber = i + 1;
        parkingLot[i].isOccupied = 0;
        parkingLot[i].accessType = 0;
        parkingLot[i].paymentType = 0;
        parkingLot[i].entryTime = 0;
        parkingLot[i].vehicleType = 0;
        strcpy(parkingLot[i].plateNumber, "");
        parkingLot[i].paymentMethod[0] = '\0';
    }
    initializeSubscriptions();
}

void initializeSubscriptions() {
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        subscriptions[i].plateNumber[0] = '\0';
        subscriptions[i].ownerName[0] = '\0';
        subscriptions[i].phoneNumber[0] = '\0';
        subscriptions[i].pin[0] = '\0';
        subscriptions[i].isRegistered = 0;
        subscriptions[i].isSubscribed = 0;
        subscriptions[i].otpVerified = 0;
        subscriptions[i].paymentType = 0;
        subscriptions[i].paymentMethod[0] = '\0';
        subscriptions[i].subscriptionStart = 0;
        subscriptions[i].subscriptionEnd = 0;
    }
}

void saveToFile() {
    FILE *fp = fopen(FILE_NAME, "w");
    if (fp == NULL) return;

    for (int i = 0; i < MAX_SLOTS; i++) {
        fprintf(fp, "%d,%s,%d,%ld,%d,%d,%d,%s\n",
                parkingLot[i].slotNumber,
                strlen(parkingLot[i].plateNumber) > 0 ? parkingLot[i].plateNumber : "NONE",
                parkingLot[i].vehicleType,
                (long)parkingLot[i].entryTime,
                parkingLot[i].isOccupied,
                parkingLot[i].accessType,
                parkingLot[i].paymentType,
                parkingLot[i].paymentMethod[0] != '\0' ? parkingLot[i].paymentMethod : "NONE");
    }
    fclose(fp);
}

void loadFromFile() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) return;

    char line[256];
    int i = 0;

    while (i < MAX_SLOTS && fgets(line, sizeof(line), fp) != NULL) {
        long entryTimeTemp;
        char plateBuffer[20];
        char paymentBuffer[30];

        if (sscanf(line, "%d,%19[^,],%d,%ld,%d,%d,%d,%29[^\n]\n",
                   &parkingLot[i].slotNumber,
                   plateBuffer,
                   &parkingLot[i].vehicleType,
                   &entryTimeTemp,
                   &parkingLot[i].isOccupied,
                   &parkingLot[i].accessType,
                   &parkingLot[i].paymentType,
                   paymentBuffer) >= 5) {
            parkingLot[i].entryTime = (time_t)entryTimeTemp;
            if (strcmp(plateBuffer, "NONE") == 0) {
                strcpy(parkingLot[i].plateNumber, "");
            } else {
                strcpy(parkingLot[i].plateNumber, plateBuffer);
            }
            if (strcmp(paymentBuffer, "NONE") == 0) {
                parkingLot[i].paymentMethod[0] = '\0';
            } else {
                strcpy(parkingLot[i].paymentMethod, paymentBuffer);
            }
            i++;
        }
    }

    fclose(fp);
}

void saveSubscriptionsToFile() {
    FILE *fp = fopen(SUBSCRIPTION_FILE, "w");
    if (fp == NULL) return;

    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (subscriptions[i].isRegistered) {
            fprintf(fp, "%s,%s,%s,%s,%d,%d,%d,%d,%s,%ld,%ld\n",
                    subscriptions[i].plateNumber,
                    subscriptions[i].ownerName,
                    subscriptions[i].phoneNumber,
                    subscriptions[i].pin,
                    subscriptions[i].isRegistered,
                    subscriptions[i].isSubscribed,
                    subscriptions[i].otpVerified,
                    subscriptions[i].paymentType,
                    subscriptions[i].paymentMethod,
                    (long)subscriptions[i].subscriptionStart,
                    (long)subscriptions[i].subscriptionEnd);
        }
    }
    fclose(fp);
}

void loadSubscriptionsFromFile() {
    FILE *fp = fopen(SUBSCRIPTION_FILE, "r");
    if (fp == NULL) return;

    char line[400];
    int i = 0;

    while (i < MAX_SUBSCRIPTIONS && fgets(line, sizeof(line), fp) != NULL) {
        long startTemp, endTemp;
        if (sscanf(line, "%19[^,],%49[^,],%19[^,],%9[^,],%d,%d,%d,%d,%29[^,],%ld,%ld\n",
                   subscriptions[i].plateNumber,
                   subscriptions[i].ownerName,
                   subscriptions[i].phoneNumber,
                   subscriptions[i].pin,
                   &subscriptions[i].isRegistered,
                   &subscriptions[i].isSubscribed,
                   &subscriptions[i].otpVerified,
                   &subscriptions[i].paymentType,
                   subscriptions[i].paymentMethod,
                   &startTemp,
                   &endTemp) >= 8) {
            subscriptions[i].subscriptionStart = (time_t)startTemp;
            subscriptions[i].subscriptionEnd = (time_t)endTemp;
            i++;
        }
    }

    fclose(fp);
}

void saveTransactionToFile(const char *plateNumber, const char *ownerName,
                           const char *phoneNumber, int paymentType,
                           const char *paymentMethod, int status,
                           const char *details, double amount) {
    FILE *fp = fopen(PAYMENT_FILE, "a");
    if (fp == NULL) return;

    fprintf(fp, "%s,%s,%s,%d,%s,%d,%s,%.2f,%ld\n",
            plateNumber,
            ownerName,
            phoneNumber,
            paymentType,
            paymentMethod,
            status,
            details,
            amount,
            (long)time(NULL));
    fclose(fp);
}

void saveCashReceiptToFile(const Slot *s, double duration, double total,
                           const char *paymentMethod, const char *provider) {
    FILE *fp = fopen(CASH_RECEIPT_FILE, "a");
    if (fp == NULL) return;

    fprintf(fp, "%s,%d,%d,%.2f,%.2f,%s,%s,%ld\n",
            s->plateNumber,
            s->slotNumber,
            s->vehicleType,
            duration,
            total,
            paymentMethod,
            provider,
            (long)time(NULL));
    fclose(fp);
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
    return index != -1 && subscriptions[index].isSubscribed == 1 && subscriptions[index].otpVerified == 1;
}

int getAvailableSlotForAccess(int accessType) {
    int start = 0;
    int end = MAX_SLOTS;

    if (accessType == 1) {
        end = MAX_SLOTS / 2;
    } else if (accessType == 2) {
        start = MAX_SLOTS / 2;
    }

    for (int i = start; i < end; i++) {
        if (parkingLot[i].isOccupied == 0) {
            return i;
        }
    }
    return -1;
}

int findSlotIndexByPlate(const char *plateNumber) {
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (parkingLot[i].isOccupied == 1 && strcmp(parkingLot[i].plateNumber, plateNumber) == 0) {
            return i;
        }
    }
    return -1;
}

void printPaymentOptions(int paymentType) {
    if (paymentType == 1) {
        printf("Select Digital Banking / Wallet Provider:\n");
        printf("1. BKash\n2. Nagad\n3. Rocket\n4. Upay\n5. Cellfin\n");
    } else if (paymentType == 2) {
        printf("Select Credit Card Provider:\n");
        printf("1. Visa\n2. MasterCard\n3. UnionPay\n4. American Express\n");
    }
}

void logSensorEvent(const char *eventType, const Slot *s) {
    FILE *fp = fopen(HISTORY_FILE, "a");
    if (fp != NULL) {
        fprintf(fp, "IR|%s|%s|%d|%ld\n",
                eventType,
                s->plateNumber,
                s->slotNumber,
                (long)time(NULL));
        fclose(fp);
    }
}

void registerVehicle() {
    char ownerName[50];
    char plateNumber[20];
    int choice;
    int index = -1;

    printf("\n--- Register Vehicle ---\n");
    printf("Enter Owner Name: ");
    scanf("%s", ownerName);
    clearInputBuffer();
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);
    clearInputBuffer();

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
    subscriptions[index].otpVerified = 0;
    subscriptions[index].paymentType = 0;
    subscriptions[index].paymentMethod[0] = '\0';
    subscriptions[index].phoneNumber[0] = '\0';
    subscriptions[index].pin[0] = '\0';
    subscriptions[index].subscriptionStart = 0;
    subscriptions[index].subscriptionEnd = 0;

    printf("Do you want to subscribe now? (1=Yes, 0=No): ");
    scanf("%d", &choice);
    clearInputBuffer();
    if (choice == 1) {
        subscribeVehicle();
    } else {
        saveSubscriptionsToFile();
        printf("Vehicle registered successfully but not subscribed yet.\n");
    }
}

void subscribeVehicle() {
    char plateNumber[20];
    char phoneNumber[20];
    char pin[10];
    int index;
    int paymentType;
    int option;
    int otp;
    int enteredOtp;
    const char *provider = "None";

    printf("\n--- Monthly Package Subscription ---\n");
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);
    clearInputBuffer();

    index = findSubscriptionIndex(plateNumber);
    if (index == -1) {
        printf("Vehicle is not registered yet. Please register first.\n");
        return;
    }

    printf("Choose Payment System:\n1. Digital Banking / Wallet\n2. Credit Card\n");
    printf("Enter choice: ");
    scanf("%d", &paymentType);
    clearInputBuffer();

    if (paymentType != 1 && paymentType != 2) {
        printf("Invalid payment type.\n");
        return;
    }

    printPaymentOptions(paymentType);
    printf("Enter provider option: ");
    scanf("%d", &option);
    clearInputBuffer();

    if (paymentType == 1) {
        const char *providers[] = {"BKash", "Nagad", "Rocket", "Upay", "Cellfin"};
        if (option >= 1 && option <= 5) {
            provider = providers[option - 1];
        } else {
            printf("Invalid provider selection.\n");
            return;
        }
    } else {
        const char *providers[] = {"Visa", "MasterCard", "UnionPay", "American Express"};
        if (option >= 1 && option <= 4) {
            provider = providers[option - 1];
        } else {
            printf("Invalid provider selection.\n");
            return;
        }
    }

    printf("Enter Username / Phone Number: ");
    scanf("%s", phoneNumber);
    clearInputBuffer();
    printf("Enter PIN: ");
    scanf("%s", pin);
    clearInputBuffer();

    otp = 100000 + rand() % 900000;
    printf("Verification message sent to %s.\n", phoneNumber);
    printf("Simulated OTP: %06d\n", otp);
    printf("Enter OTP: ");
    scanf("%d", &enteredOtp);
    clearInputBuffer();

    if (enteredOtp != otp) {
        printf("OTP verification failed. Subscription cancelled.\n");
        saveTransactionToFile(plateNumber,
                               subscriptions[index].ownerName,
                               phoneNumber,
                               paymentType,
                               provider,
                               0,
                               "OTP failed",
                               0.0);
        return;
    }

    strcpy(subscriptions[index].phoneNumber, phoneNumber);
    strcpy(subscriptions[index].pin, pin);
    subscriptions[index].paymentType = paymentType;
    strcpy(subscriptions[index].paymentMethod, provider);
    subscriptions[index].isSubscribed = 1;
    subscriptions[index].otpVerified = 1;
    subscriptions[index].subscriptionStart = time(NULL);
    subscriptions[index].subscriptionEnd = subscriptions[index].subscriptionStart + 30 * 24 * 60 * 60;

    saveSubscriptionsToFile();
    saveTransactionToFile(plateNumber,
                           subscriptions[index].ownerName,
                           phoneNumber,
                           paymentType,
                           provider,
                           1,
                           "Monthly package activated",
                           MONTHLY_PACKAGE_FEE);

    printf("Subscription completed successfully.\n");
    printf("Monthly parking package is active until %s.", ctime(&subscriptions[index].subscriptionEnd));
}

void checkIn() {
    char plateNumber[20];
    int vehicleType;
    int accessType;
    int slotIndex;
    int subIndex;

    printf("\n--- Vehicle Check-In ---\n");
    printf("Select check-in type:\n1. Subscribed Monthly Package\n2. Cash / Walk-In\n");
    printf("Enter choice: ");
    scanf("%d", &accessType);
    clearInputBuffer();

    if (accessType != 1 && accessType != 2) {
        printf("Invalid check-in type.\n");
        return;
    }

    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);
    clearInputBuffer();
    printf("Select Vehicle Type (1-Car, 2-Bike, 3-Truck): ");
    scanf("%d", &vehicleType);
    clearInputBuffer();

    if (accessType == 1) {
        subIndex = findSubscriptionIndex(plateNumber);
        if (subIndex == -1 || !canAccessParking(plateNumber)) {
            printf("Subscribed access denied. Please complete registration and payment first.\n");
            return;
        }
    }

    slotIndex = getAvailableSlotForAccess(accessType);
    if (slotIndex == -1) {
        if (accessType == 1) {
            printf("No subscribed parking slot is available right now.\n");
        } else {
            printf("No cash parking slot is available right now.\n");
        }
        return;
    }

    strcpy(parkingLot[slotIndex].plateNumber, plateNumber);
    parkingLot[slotIndex].vehicleType = vehicleType;
    parkingLot[slotIndex].entryTime = time(NULL);
    parkingLot[slotIndex].isOccupied = 1;
    parkingLot[slotIndex].accessType = accessType;
    parkingLot[slotIndex].paymentType = accessType == 2 ? 3 : 1;
    strcpy(parkingLot[slotIndex].paymentMethod, accessType == 2 ? "Cash" : "Monthly Package");

    logSensorEvent("ENTRY", &parkingLot[slotIndex]);
    saveToFile();
    appendToHistory(&parkingLot[slotIndex], 0.0, 0.0, "ENTRY", accessType == 1 ? "Subscribed entry" : "Cash entry");

    printf("IR sensor detected the vehicle.\n");
    printf("Entrance barrier opened automatically.\n");
    printf("Check-in successful. Slot %d assigned.\n", parkingLot[slotIndex].slotNumber);
}

void checkOut() {
    char searchPlate[20];
    int found;

    printf("\n--- Vehicle Check-Out ---\n");
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", searchPlate);
    clearInputBuffer();

    found = findSlotIndexByPlate(searchPlate);
    if (found == -1) {
        printf("Vehicle not found in the active parking lot.\n");
        return;
    }

    time_t exitTime = time(NULL);
    double durationSeconds = difftime(exitTime, parkingLot[found].entryTime);
    double durationHours = (durationSeconds / 3600.0) + 1.0;
    double rate = 0.0;
    double totalCharge = 0.0;

    if (parkingLot[found].vehicleType == 1) {
        rate = CAR_RATE;
    } else if (parkingLot[found].vehicleType == 2) {
        rate = BIKE_RATE;
    } else if (parkingLot[found].vehicleType == 3) {
        rate = TRUCK_RATE;
    }

    if (parkingLot[found].accessType == 2) {
        totalCharge = durationHours * rate;
    }

    generateBill(&parkingLot[found], durationHours, totalCharge, parkingLot[found].accessType);
    appendToHistory(&parkingLot[found], durationHours, totalCharge,
                    "EXIT", parkingLot[found].accessType == 2 ? "Cash checkout" : "Subscription checkout");
    logSensorEvent("EXIT", &parkingLot[found]);

    parkingLot[found].isOccupied = 0;
    parkingLot[found].accessType = 0;
    parkingLot[found].paymentType = 0;
    parkingLot[found].paymentMethod[0] = '\0';
    parkingLot[found].entryTime = 0;
    parkingLot[found].vehicleType = 0;
    strcpy(parkingLot[found].plateNumber, "");

    saveToFile();
}

void generateBill(const Slot *s, double duration, double total, int accessType) {
    printf("\n=========================================\n");
    printf("             PARKING RECEIPT\n");
    printf("=========================================\n");
    printf("Slot Number   : %d\n", s->slotNumber);
    printf("Plate Number  : %s\n", s->plateNumber);
    printf("Vehicle Type  : %s\n", s->vehicleType == 1 ? "Car" : (s->vehicleType == 2 ? "Bike" : "Truck"));
    printf("Duration (hrs): %.2f\n", duration);

    if (accessType == 2) {
        printf("Payment Mode  : Cash / Walk-In\n");
        printf("Total Charge  : $%.2f\n", total);
        saveCashReceiptToFile(s, duration, total, "Cash", "Walk-In");
    } else {
        printf("Payment Mode  : Monthly Subscription\n");
        printf("Total Charge  : $0.00\n");
    }

    printf("=========================================\n");
}

void appendToHistory(const Slot *s, double duration, double total, const char *eventType,
                     const char *details) {
    FILE *fp = fopen(HISTORY_FILE, "a");
    if (fp != NULL) {
        fprintf(fp, "%s,%s,%d,%d,%.2f,%.2f,%s,%ld\n",
                eventType,
                s->plateNumber,
                s->slotNumber,
                s->vehicleType,
                duration,
                total,
                details,
                (long)time(NULL));
        fclose(fp);
    }
}

void displayAvailableSlots() {
    int freeCount = 0;
    int subscribedFree = 0;
    int cashFree = 0;

    printf("\n--- Available Slots ---\n");
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (parkingLot[i].isOccupied == 0) {
            if (i < MAX_SLOTS / 2) {
                subscribedFree++;
            } else {
                cashFree++;
            }
            freeCount++;
        }
    }

    printf("Subscribed Zone Free: %d / %d\n", subscribedFree, MAX_SLOTS / 2);
    printf("Cash Zone Free: %d / %d\n", cashFree, MAX_SLOTS / 2);
    printf("Total Available: %d / %d\n", freeCount, MAX_SLOTS);
}

void simulateIrSensorEntry() {
    char plateNumber[20];
    int slotIndex;

    printf("\n--- IR Sensor Entry Check ---\n");
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);
    clearInputBuffer();

    slotIndex = findSlotIndexByPlate(plateNumber);
    if (slotIndex != -1) {
        printf("IR sensor detected an active vehicle.\n");
        printf("Entrance barrier opened automatically.\n");
    } else {
        printf("IR sensor did not detect an active vehicle.\n");
        printf("Entrance barrier remained closed.\n");
    }
}

void simulateIrSensorExit() {
    char plateNumber[20];
    int slotIndex;

    printf("\n--- IR Sensor Exit Check ---\n");
    printf("Enter Vehicle Plate Number: ");
    scanf("%s", plateNumber);
    clearInputBuffer();

    slotIndex = findSlotIndexByPlate(plateNumber);
    if (slotIndex != -1) {
        printf("IR sensor detected the vehicle at exit.\n");
        printf("Exit barrier opened automatically.\n");
    } else {
        printf("IR sensor found no active vehicle for this plate number.\n");
        printf("Exit barrier remained closed.\n");
    }
}

void adminMenu() {
    int adminChoice;
    int slotToManage;

    printf("\n--- Admin Management ---\n");
    printf("1. View Full Slot Status\n");
    printf("2. Disable Slot (Maintenance)\n");
    printf("3. Enable Slot\n");
    printf("4. View History Log\n");
    printf("5. View Subscription Records\n");
    printf("6. View Payment Transactions\n");
    printf("Enter choice: ");
    scanf("%d", &adminChoice);
    clearInputBuffer();

    if (adminChoice == 1) {
        for (int i = 0; i < MAX_SLOTS; i++) {
            printf("Slot %d: ", parkingLot[i].slotNumber);
            if (parkingLot[i].isOccupied == 1) {
                printf("Occupied (%s)", parkingLot[i].plateNumber);
                if (parkingLot[i].accessType == 1) {
                    printf(" [Subscribed]\n");
                } else {
                    printf(" [Cash]\n");
                }
            } else if (parkingLot[i].isOccupied == 2) {
                printf("MAINTENANCE\n");
            } else {
                printf("Free\n");
            }
        }
    } else if (adminChoice == 2) {
        printf("Enter slot to disable: ");
        scanf("%d", &slotToManage);
        clearInputBuffer();
        if (slotToManage >= 1 && slotToManage <= MAX_SLOTS) {
            parkingLot[slotToManage - 1].isOccupied = 2;
            saveToFile();
            printf("Slot %d disabled.\n", slotToManage);
        }
    } else if (adminChoice == 3) {
        printf("Enter slot to enable: ");
        scanf("%d", &slotToManage);
        clearInputBuffer();
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
    } else if (adminChoice == 6) {
        FILE *fp = fopen(PAYMENT_FILE, "r");
        char buffer[255];
        if (fp == NULL) {
            printf("No payment transactions found.\n");
        } else {
            printf("\n--- Payment Transactions ---\n");
            while (fgets(buffer, sizeof(buffer), fp)) {
                printf("%s", buffer);
            }
            fclose(fp);
        }
    }
}
