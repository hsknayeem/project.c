#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "admin.h"

#define MAX_USERS 100
#define MAX_VEHICLES 200
#define MAX_SUBSCRIPTIONS 200
#define MAX_BOOKINGS 200
#define MAX_SLOTS 100
#define MAX_DIVISIONS 8
#define MAX_CATEGORIES 7
#define MONTHLY_FEE 1000.0
#define CAR_RATE 50.0
#define BIKE_RATE 20.0
#define TRUCK_RATE 100.0
#define BUS_RATE 80.0
#define SUV_RATE 65.0
#define VAN_RATE 60.0
#define MICROBUS_RATE 70.0
#define CASH_DISCOUNT 0.05
#define BOOKING_FINE 1000.0

#define USERS_FILE "users.csv"
#define VEHICLES_FILE "vehicles.csv"
#define SUBSCRIPTIONS_FILE "subscriptions.csv"
#define BOOKINGS_FILE "bookings.csv"
#define SLOTS_FILE "parking_slots.csv"
#define TRANSACTIONS_FILE "payment_transactions.csv"
#define CASH_RECEIPTS_FILE "cash_receipts.csv"

const char *divisions[MAX_DIVISIONS] = {
    "Dhaka", "Chittagong", "Rajshahi", "Khulna", "Barisal", "Sylhet", "Rangpur", "Mymensingh"};
const char *categories[MAX_CATEGORIES] = {
    "Car", "Bike", "Truck", "Bus", "SUV", "Van", "Microbus"};
const char *internetProviders[4] = {"bKash", "Nagad", "Rocket", "Upay"};
const char *creditProviders[4] = {"Visa", "Master Card", "UnionPay", "American Express"};

typedef struct
{
    char username[50];
    char password[50];
    char phone[20];
    int active;
} UserAccount;

typedef struct
{
    char nid[20];
    char plate[20];
    char phone[20];
    char owner[50];
    int registered;
} Vehicle;

typedef struct
{
    char plate[20];
    char owner[50];
    int active;
    char method[30];
    char provider[30];
    time_t start;
    time_t end;
} Subscription;

typedef struct
{
    char plate[20];
    char owner[50];
    char division[30];
    char category[30];
    time_t start;
    time_t end;
    int active;
} Booking;

typedef struct
{
    int slotNumber;
    char plate[20];
    char owner[50];
    int vehicleType;
    int accessType;
    int status;
    time_t entryTime;
} ParkingSlot;

static UserAccount users[MAX_USERS];
static Vehicle vehicles[MAX_VEHICLES];
static Subscription subscriptions[MAX_SUBSCRIPTIONS];
static Booking bookings[MAX_BOOKINGS];
static ParkingSlot parkingSlots[MAX_SLOTS];
static int refreshToMainMenu = 0;

static void clearInputBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

static void readLine(const char *prompt, char *buffer, int size)
{
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL)
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }
    }
}

static int sendOtp(void)
{
    int otp = 100000 + rand() % 900000;
    printf("Simulated OTP: %06d\n", otp);
    return otp;
}

static void trimWhitespace(char *s)
{
    if (s == NULL)
        return;
    // trim leading
    char *start = s;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
    // trim trailing
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
    {
        s[len - 1] = '\0';
        len--;
    }
}

static int findUser(const char *username)
{
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].active && strcmp(users[i].username, username) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int findVehicle(const char *plate)
{
    for (int i = 0; i < MAX_VEHICLES; i++)
    {
        if (vehicles[i].registered && strcmp(vehicles[i].plate, plate) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int findSubscription(const char *plate)
{
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++)
    {
        if (subscriptions[i].active && strcmp(subscriptions[i].plate, plate) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int findBooking(const char *plate)
{
    for (int i = 0; i < MAX_BOOKINGS; i++)
    {
        if (bookings[i].active && strcmp(bookings[i].plate, plate) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int findParkedSlot(const char *plate)
{
    for (int i = 0; i < MAX_SLOTS; i++)
    {
        if (parkingSlots[i].status == 1 && strcmp(parkingSlots[i].plate, plate) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int findFreeSlot(int accessType)
{
    int start = (accessType == 1) ? 0 : MAX_SLOTS / 2;
    int end = (accessType == 1) ? MAX_SLOTS / 2 : MAX_SLOTS;
    for (int i = start; i < end; i++)
    {
        if (parkingSlots[i].status == 0)
        {
            return i;
        }
    }
    return -1;
}

static void initializeParkingSlots(void)
{
    FILE *fp = fopen(SLOTS_FILE, "r");
    if (fp == NULL)
    {
        fp = fopen(SLOTS_FILE, "w");
        if (fp == NULL)
        {
            return;
        }
        for (int i = 0; i < MAX_SLOTS; i++)
        {
            parkingSlots[i].slotNumber = i + 1;
            parkingSlots[i].plate[0] = '\0';
            parkingSlots[i].owner[0] = '\0';
            parkingSlots[i].vehicleType = 0;
            parkingSlots[i].accessType = 0;
            parkingSlots[i].status = 0;
            parkingSlots[i].entryTime = 0;
            fprintf(fp, "%d,,,0,0,0,0\n", parkingSlots[i].slotNumber);
        }
        fclose(fp);
        return;
    }
    char line[256];
    int index = 0;
    while (fgets(line, sizeof(line), fp) && index < MAX_SLOTS)
    {
        int number, vehicleType, accessType, status;
        long entryTime;
        if (sscanf(line, "%d,%19[^,],%49[^,],%d,%d,%d,%ld",
                   &number,
                   parkingSlots[index].plate,
                   parkingSlots[index].owner,
                   &vehicleType,
                   &accessType,
                   &status,
                   &entryTime) >= 1)
        {
            parkingSlots[index].slotNumber = number;
            parkingSlots[index].vehicleType = vehicleType;
            parkingSlots[index].accessType = accessType;
            parkingSlots[index].status = status;
            parkingSlots[index].entryTime = (time_t)entryTime;
            index++;
        }
    }
    fclose(fp);
    for (int i = index; i < MAX_SLOTS; i++)
    {
        parkingSlots[i].slotNumber = i + 1;
        parkingSlots[i].plate[0] = '\0';
        parkingSlots[i].owner[0] = '\0';
        parkingSlots[i].vehicleType = 0;
        parkingSlots[i].accessType = 0;
        parkingSlots[i].status = 0;
        parkingSlots[i].entryTime = 0;
    }
}

static void saveParkingSlots(void)
{
    FILE *fp = fopen(SLOTS_FILE, "w");
    if (fp == NULL)
    {
        return;
    }
    for (int i = 0; i < MAX_SLOTS; i++)
    {
        fprintf(fp, "%d,%s,%s,%d,%d,%d,%ld\n",
                parkingSlots[i].slotNumber,
                parkingSlots[i].plate,
                parkingSlots[i].owner,
                parkingSlots[i].vehicleType,
                parkingSlots[i].accessType,
                parkingSlots[i].status,
                (long)parkingSlots[i].entryTime);
    }
    fclose(fp);
}

static void loadUsers(void)
{
    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL)
    {
        for (int i = 0; i < MAX_USERS; i++)
        {
            users[i].active = 0;
        }
        return;
    }
    char line[200];
    int index = 0;
    while (fgets(line, sizeof(line), fp) && index < MAX_USERS)
    {
        if (sscanf(line, "%49[^,],%49[^,],%19[^\\n]",
                   users[index].username,
                   users[index].password,
                   users[index].phone) == 3)
        {
            users[index].active = 1;
            trimWhitespace(users[index].phone);
            index++;
        }
    }
    fclose(fp);
}

static void saveUsers(void)
{
    FILE *fp = fopen(USERS_FILE, "w");
    if (fp == NULL)
    {
        return;
    }
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].active)
        {
            fprintf(fp, "%s,%s,%s\n", users[i].username, users[i].password, users[i].phone);
        }
    }
    fclose(fp);
}

static void loadVehicles(void)
{
    FILE *fp = fopen(VEHICLES_FILE, "r");
    if (fp == NULL)
    {
        for (int i = 0; i < MAX_VEHICLES; i++)
        {
            vehicles[i].registered = 0;
        }
        return;
    }
    char line[300];
    int index = 0;
    while (fgets(line, sizeof(line), fp) && index < MAX_VEHICLES)
    {
        if (sscanf(line, "%19[^,],%19[^,],%19[^,],%49[^,],%d",
                   vehicles[index].nid,
                   vehicles[index].plate,
                   vehicles[index].phone,
                   vehicles[index].owner,
                   &vehicles[index].registered) == 5)
        {
            index++;
        }
    }
    fclose(fp);
}

static void saveVehicles(void)
{
    FILE *fp = fopen(VEHICLES_FILE, "w");
    if (fp == NULL)
    {
        return;
    }
    for (int i = 0; i < MAX_VEHICLES; i++)
    {
        if (vehicles[i].registered)
        {
            fprintf(fp, "%s,%s,%s,%s,%d\n",
                    vehicles[i].nid,
                    vehicles[i].plate,
                    vehicles[i].phone,
                    vehicles[i].owner,
                    vehicles[i].registered);
        }
    }
    fclose(fp);
}

static void loadSubscriptions(void)
{
    FILE *fp = fopen(SUBSCRIPTIONS_FILE, "r");
    if (fp == NULL)
    {
        for (int i = 0; i < MAX_SUBSCRIPTIONS; i++)
        {
            subscriptions[i].active = 0;
        }
        return;
    }
    char line[300];
    int index = 0;
    while (fgets(line, sizeof(line), fp) && index < MAX_SUBSCRIPTIONS)
    {
        if (sscanf(line, "%19[^,],%49[^,],%d,%29[^,],%29[^,],%ld,%ld",
                   subscriptions[index].plate,
                   subscriptions[index].owner,
                   &subscriptions[index].active,
                   subscriptions[index].method,
                   subscriptions[index].provider,
                   (long *)&subscriptions[index].start,
                   (long *)&subscriptions[index].end) == 7)
        {
            index++;
        }
    }
    fclose(fp);
}

static void saveSubscriptions(void)
{
    FILE *fp = fopen(SUBSCRIPTIONS_FILE, "w");
    if (fp == NULL)
    {
        return;
    }
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++)
    {
        if (subscriptions[i].active)
        {
            fprintf(fp, "%s,%s,%d,%s,%s,%ld,%ld\n",
                    subscriptions[i].plate,
                    subscriptions[i].owner,
                    subscriptions[i].active,
                    subscriptions[i].method,
                    subscriptions[i].provider,
                    (long)subscriptions[i].start,
                    (long)subscriptions[i].end);
        }
    }
    fclose(fp);
}

static void loadBookings(void)
{
    FILE *fp = fopen(BOOKINGS_FILE, "r");
    if (fp == NULL)
    {
        for (int i = 0; i < MAX_BOOKINGS; i++)
        {
            bookings[i].active = 0;
        }
        return;
    }
    char line[400];
    int index = 0;
    while (fgets(line, sizeof(line), fp) && index < MAX_BOOKINGS)
    {
        if (sscanf(line, "%19[^,],%49[^,],%29[^,],%29[^,],%ld,%ld,%d",
                   bookings[index].plate,
                   bookings[index].owner,
                   bookings[index].division,
                   bookings[index].category,
                   (long *)&bookings[index].start,
                   (long *)&bookings[index].end,
                   &bookings[index].active) == 7)
        {
            index++;
        }
    }
    fclose(fp);
}

static void saveBookings(void)
{
    FILE *fp = fopen(BOOKINGS_FILE, "w");
    if (fp == NULL)
    {
        return;
    }
    for (int i = 0; i < MAX_BOOKINGS; i++)
    {
        if (bookings[i].active)
        {
            fprintf(fp, "%s,%s,%s,%s,%ld,%ld,%d\n",
                    bookings[i].plate,
                    bookings[i].owner,
                    bookings[i].division,
                    bookings[i].category,
                    (long)bookings[i].start,
                    (long)bookings[i].end,
                    bookings[i].active);
        }
    }
    fclose(fp);
}

static void appendTransaction(const char *plate, const char *owner, int vehicleType,
                              int accessType, double amount, const char *detail)
{
    FILE *fp = fopen(TRANSACTIONS_FILE, "a");
    if (fp == NULL)
    {
        return;
    }
    fprintf(fp, "%s,%s,%d,%d,%.2f,%s,%ld\n",
            plate,
            owner,
            vehicleType,
            accessType,
            amount,
            detail,
            (long)time(NULL));
    fclose(fp);
}

static void appendCashReceipt(const ParkingSlot *slot, double duration, double total)
{
    FILE *fp = fopen(CASH_RECEIPTS_FILE, "a");
    if (fp == NULL)
    {
        return;
    }
    fprintf(fp, "%s,%d,%d,%.2f,%.2f,%s,%ld\n",
            slot->plate,
            slot->slotNumber,
            slot->vehicleType,
            duration,
            total,
            slot->accessType == 1 ? "Subscriber" : "Cash",
            (long)time(NULL));
    fclose(fp);
}

static void appendParkingHistory(const char *event, const char *plate,
                                 int slotNumber, const char *detail)
{
    FILE *fp = fopen("parking_history.csv", "a");
    if (fp == NULL)
    {
        return;
    }
    fprintf(fp, "%s,%s,%d,%s,%ld\n", event, plate, slotNumber, detail, (long)time(NULL));
    fclose(fp);
}

static int hasActiveSubscription(const char *plate)
{
    int index = findSubscription(plate);
    if (index < 0)
    {
        return 0;
    }
    return subscriptions[index].active && time(NULL) <= subscriptions[index].end;
}

static void createAccount(void)
{
    char username[50];
    char password[50];
    char phone[20];
    readLine("Enter username: ", username, sizeof(username));
    if (findUser(username) >= 0)
    {
        printf("Username already exists.\n");
        return;
    }
    readLine("Enter password: ", password, sizeof(password));
    readLine("Enter phone number: ", phone, sizeof(phone));
    int otp = sendOtp();
    int entered;
    printf("Enter OTP: ");
    if (scanf("%d", &entered) != 1)
    {
        clearInputBuffer();
        printf("Invalid OTP.\n");
        return;
    }
    clearInputBuffer();
    if (entered != otp)
    {
        printf("OTP verification failed.\n");
        return;
    }
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (!users[i].active)
        {
            users[i].active = 1;
            strcpy(users[i].username, username);
            strcpy(users[i].password, password);
            strcpy(users[i].phone, phone);
            saveUsers();
            printf("Account created successfully.\n");
            return;
        }
    }
    printf("User capacity reached.\n");
}

static void forgetPassword(void)
{
    char username[50];
    char phone[20];
    readLine("Enter username: ", username, sizeof(username));
    int index = findUser(username);
    if (index < 0)
    {
        printf("Username not found.\n");
        return;
    }
    readLine("Enter registered phone number: ", phone, sizeof(phone));
    trimWhitespace(phone);
    trimWhitespace(users[index].phone);
    if (strcmp(phone, users[index].phone) != 0)
    {
        printf("Phone mismatch.\n");
        return;
    }
    int otp = sendOtp();
    int entered;
    printf("Enter OTP: ");
    if (scanf("%d", &entered) != 1)
    {
        clearInputBuffer();
        printf("Invalid OTP.\n");
        return;
    }
    clearInputBuffer();
    if (entered != otp)
    {
        printf("OTP failed.\n");
        return;
    }
    char password[50];
    char confirm[50];
    readLine("Enter new password: ", password, sizeof(password));
    readLine("Retype new password: ", confirm, sizeof(confirm));
    if (strcmp(password, confirm) != 0)
    {
        printf("Passwords do not match.\n");
        return;
    }
    strcpy(users[index].password, password);
    saveUsers();
    printf("Password updated successfully.\n");
}

static void registerVehicle(const char *username)
{
    char nid[20];
    char plate[20];
    char phone[20];
    readLine("Enter NID number: ", nid, sizeof(nid));
    readLine("Enter vehicle plate number: ", plate, sizeof(plate));
    if (findVehicle(plate) >= 0)
    {
        printf("Vehicle already registered.\n");
        return;
    }
    readLine("Enter phone number: ", phone, sizeof(phone));
    for (int i = 0; i < MAX_VEHICLES; i++)
    {
        if (!vehicles[i].registered)
        {
            vehicles[i].registered = 1;
            strcpy(vehicles[i].nid, nid);
            strcpy(vehicles[i].plate, plate);
            strcpy(vehicles[i].phone, phone);
            strcpy(vehicles[i].owner, username);
            saveVehicles();
            printf("Vehicle registered successfully.\n");
            return;
        }
    }
    printf("Vehicle storage full.\n");
}

static void subscribeVehicle(const char *username)
{
    while (1)
    {
        printf("\n=== SUBSCRIPTION MENU ===\n");
        printf("1. Subscribe to Monthly Package\n");
        printf("2. Cancel Subscription\n");
        printf("0. Back\n");
        printf("Enter choice: ");
        int menuChoice;
        if (scanf("%d", &menuChoice) != 1)
        {
            clearInputBuffer();
            printf("Invalid selection.\n");
            continue;
        }
        clearInputBuffer();

        if (menuChoice == 0)
        {
            return;
        }

        if (menuChoice == 2)
        {
            char plate[20];
            readLine("Enter vehicle plate number: ", plate, sizeof(plate));
            int vehicleIndex = findVehicle(plate);
            if (vehicleIndex < 0 || strcmp(vehicles[vehicleIndex].owner, username) != 0)
            {
                printf("Vehicle not registered under your username.\n");
                continue;
            }
            int subscriptionIndex = findSubscription(plate);
            if (subscriptionIndex < 0 || !subscriptions[subscriptionIndex].active || time(NULL) > subscriptions[subscriptionIndex].end)
            {
                printf("No active subscription found for this vehicle.\n");
                continue;
            }
            printf("Active subscription until %s", ctime(&subscriptions[subscriptionIndex].end));
            printf("Cancel subscription?\n");
            printf("1. Yes\n");
            printf("0. No\n");
            printf("Enter choice: ");
            int cancelChoice;
            if (scanf("%d", &cancelChoice) != 1)
            {
                clearInputBuffer();
                printf("Invalid selection.\n");
                continue;
            }
            clearInputBuffer();
            if (cancelChoice == 1)
            {
                subscriptions[subscriptionIndex].active = 0;
                saveSubscriptions();
                printf("Subscription canceled. No refund.\n");
            }
            else
            {
                printf("Cancellation aborted. Returning to subscription menu.\n");
            }
            continue;
        }

        if (menuChoice == 1)
        {
            char plate[20];
            readLine("Enter vehicle plate number: ", plate, sizeof(plate));
            int vehicleIndex = findVehicle(plate);
            if (vehicleIndex < 0 || strcmp(vehicles[vehicleIndex].owner, username) != 0)
            {
                printf("Vehicle not registered under your username.\n");
                continue;
            }
            int subscriptionIndex = findSubscription(plate);
            if (subscriptionIndex >= 0 && subscriptions[subscriptionIndex].active && time(NULL) <= subscriptions[subscriptionIndex].end)
            {
                printf("Active subscription until %s", ctime(&subscriptions[subscriptionIndex].end));
                printf("You already have an active subscription. Choose cancel if you want to stop it.\n");
                continue;
            }

            int system;
            const char *provider = NULL;
            while (1)
            {
                printf("Choose payment system:\n");
                printf("0. Back\n");
                printf("1. Internet Banking / Wallet\n");
                printf("2. Credit Card\n");
                printf("Enter choice: ");
                if (scanf("%d", &system) != 1)
                {
                    clearInputBuffer();
                    printf("Invalid input.\n");
                    break;
                }
                clearInputBuffer();

                if (system == 0)
                {
                    printf("Returning to subscription menu.\n");
                    break;
                }
                if (system != 1 && system != 2)
                {
                    printf("Invalid payment system.\n");
                    continue;
                }

                int providerChoice;
                while (1)
                {
                    printf("Choose provider:\n");
                    printf("0. Back\n");
                    if (system == 1)
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            printf("%d. %s\n", i + 1, internetProviders[i]);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            printf("%d. %s\n", i + 1, creditProviders[i]);
                        }
                    }
                    printf("Selection: ");
                    if (scanf("%d", &providerChoice) != 1)
                    {
                        clearInputBuffer();
                        printf("Invalid input.\n");
                        break;
                    }
                    clearInputBuffer();

                    if (providerChoice == 0)
                    {
                        printf("Going back to payment system selection.\n");
                        break;
                    }
                    if (providerChoice < 1 || providerChoice > 4)
                    {
                        printf("Invalid provider. Please try again.\n");
                        continue;
                    }
                    provider = system == 1 ? internetProviders[providerChoice - 1] : creditProviders[providerChoice - 1];
                    break;
                }

                if (provider == NULL)
                {
                    continue;
                }

                char account[30];
                char pin[20];
                if (system == 1)
                {
                    readLine("Enter mobile number: ", account, sizeof(account));
                }
                else
                {
                    readLine("Enter card number: ", account, sizeof(account));
                }
                readLine("Enter PIN: ", pin, sizeof(pin));
                double amount = MONTHLY_FEE;
                printf("Subscription amount is fixed at %.2f. Proceeding with payment...\n", amount);
                int otp = sendOtp();
                int entered;
                printf("Enter OTP: ");
                if (scanf("%d", &entered) != 1)
                {
                    clearInputBuffer();
                    printf("Invalid OTP.\n");
                    break;
                }
                clearInputBuffer();
                if (entered != otp)
                {
                    printf("OTP verification failed.\n");
                    break;
                }
                int saveIndex = subscriptionIndex;
                if (saveIndex < 0)
                {
                    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++)
                    {
                        if (!subscriptions[i].active)
                        {
                            saveIndex = i;
                            break;
                        }
                    }
                }
                if (saveIndex < 0)
                {
                    printf("Subscription limit reached.\n");
                    break;
                }
                strcpy(subscriptions[saveIndex].plate, plate);
                strcpy(subscriptions[saveIndex].owner, username);
                subscriptions[saveIndex].active = 1;
                strcpy(subscriptions[saveIndex].method, system == 1 ? "Internet Banking" : "Credit Card");
                strcpy(subscriptions[saveIndex].provider, provider);
                subscriptions[saveIndex].start = time(NULL);
                subscriptions[saveIndex].end = subscriptions[saveIndex].start + 30LL * 24 * 60 * 60;
                saveSubscriptions();
                appendTransaction(plate, username, 0, 1, amount, "Subscription payment");
                printf("Subscription completed. 10%% discount active.\n");
                printf("Valid until %s", ctime(&subscriptions[saveIndex].end));
                return;
            }
            continue;
        }

        printf("Invalid selection.\n");
    }
}

static void makeBooking(const char *username)
{
    char plate[20];
    readLine("Enter vehicle plate number: ", plate, sizeof(plate));
    int vehicleIndex = findVehicle(plate);
    if (vehicleIndex < 0 || strcmp(vehicles[vehicleIndex].owner, username) != 0)
    {
        printf("Vehicle not registered under your username.\n");
        return;
    }
    printf("Select division:\n");
    printf("0. Back\n");
    for (int i = 0; i < MAX_DIVISIONS; i++)
    {
        printf("%d. %s\n", i + 1, divisions[i]);
    }
    int division;
    printf("Enter choice: ");
    if (scanf("%d", &division) != 1)
    {
        clearInputBuffer();
        printf("Invalid selection.\n");
        return;
    }
    clearInputBuffer();
    if (division == 0)
    {
        printf("Returning to booking menu.\n");
        return;
    }
    if (division < 1 || division > MAX_DIVISIONS)
    {
        printf("Invalid division.\n");
        return;
    }
    printf("Select category:\n");
    for (int i = 0; i < MAX_CATEGORIES; i++)
    {
        printf("%d. %s\n", i + 1, categories[i]);
    }
    int category;
    printf("Enter choice: ");
    if (scanf("%d", &category) != 1)
    {
        clearInputBuffer();
        printf("Invalid selection.\n");
        return;
    }
    clearInputBuffer();
    if (category < 1 || category > MAX_CATEGORIES)
    {
        printf("Invalid category.\n");
        return;
    }
    for (int i = 0; i < MAX_BOOKINGS; i++)
    {
        if (!bookings[i].active)
        {
            strcpy(bookings[i].plate, plate);
            strcpy(bookings[i].owner, username);
            strcpy(bookings[i].division, divisions[division - 1]);
            strcpy(bookings[i].category, categories[category - 1]);
            bookings[i].start = time(NULL);
            bookings[i].end = bookings[i].start + 30LL * 24 * 60 * 60;
            bookings[i].active = 1;
            saveBookings();
            printf("Booking created for %s in %s (%s).\n", plate, bookings[i].division, bookings[i].category);
            printf("Collect vehicle before %s", ctime(&bookings[i].end));
            return;
        }
    }
    printf("Booking capacity reached.\n");
}

static void cancelBooking(const char *username)
{
    char plate[20];
    readLine("Enter vehicle plate number: ", plate, sizeof(plate));
    int bookingIndex = findBooking(plate);
    if (bookingIndex < 0 || !bookings[bookingIndex].active || strcmp(bookings[bookingIndex].owner, username) != 0)
    {
        printf("No active booking found for this vehicle under your account.\n");
        return;
    }
    printf("Booking found for %s in %s (%s).\n", bookings[bookingIndex].plate, bookings[bookingIndex].division, bookings[bookingIndex].category);
    printf("Cancel booking?\n");
    printf("1. Yes\n");
    printf("0. No\n");
    printf("Enter choice: ");
    int choice;
    if (scanf("%d", &choice) != 1)
    {
        clearInputBuffer();
        printf("Invalid selection.\n");
        return;
    }
    clearInputBuffer();
    if (choice == 1)
    {
        bookings[bookingIndex].active = 0;
        saveBookings();
        printf("Booking canceled successfully.\n");
    }
    else
    {
        printf("Cancellation aborted. Returning to booking menu.\n");
    }
}

static void manageBooking(const char *username)
{
    while (1)
    {
        printf("\n=== BOOKING MENU ===\n");
        printf("1. Create Booking\n");
        printf("2. Cancel Booking\n");
        printf("0. Back\n");
        printf("Enter choice: ");
        int choice;
        if (scanf("%d", &choice) != 1)
        {
            clearInputBuffer();
            printf("Invalid selection.\n");
            continue;
        }
        clearInputBuffer();
        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            makeBooking(username);
        }
        else if (choice == 2)
        {
            cancelBooking(username);
        }
        else
        {
            printf("Invalid selection.\n");
        }
    }
}

static double getRate(int type)
{
    switch (type)
    {
    case 1:
        return CAR_RATE;
    case 2:
        return BIKE_RATE;
    case 3:
        return TRUCK_RATE;
    case 4:
        return BUS_RATE;
    case 5:
        return SUV_RATE;
    case 6:
        return VAN_RATE;
    case 7:
        return MICROBUS_RATE;
    default:
        return 0.0;
    }
}

static void checkInVehicle(const char *username)
{
    char plate[20];
    int accessType;
    printf("1. Subscriber\n2. Cash\n0. Back\nEnter choice: ");
    if (scanf("%d", &accessType) != 1)
    {
        clearInputBuffer();
        printf("Invalid input.\n");
        return;
    }
    clearInputBuffer();
    if (accessType == 0)
    {
        printf("Returning to user dashboard.\n");
        return;
    }
    if (accessType != 1 && accessType != 2)
    {
        printf("Invalid check-in type.\n");
        return;
    }
    readLine("Enter vehicle plate number: ", plate, sizeof(plate));
    int vehicleIndex = findVehicle(plate);
    if (vehicleIndex < 0 || strcmp(vehicles[vehicleIndex].owner, username) != 0)
    {
        printf("Vehicle not registered under your username.\n");
        return;
    }
    if (accessType == 1 && !hasActiveSubscription(plate))
    {
        printf("No active subscription found.\n");
        return;
    }
    int vehicleType;
    printf("Select vehicle type:\n");
    printf("0. Back\n");
    printf("1. Car\n");
    printf("2. Bike\n");
    printf("3. Truck\n");
    printf("4. Bus\n");
    printf("5. SUV\n");
    printf("6. Van\n");
    printf("7. Microbus\n");
    printf("Enter choice: ");
    if (scanf("%d", &vehicleType) != 1)
    {
        clearInputBuffer();
        printf("Invalid input.\n");
        return;
    }
    clearInputBuffer();
    if (vehicleType == 0)
    {
        printf("Returning to check-in type selection.\n");
        return;
    }
    if (vehicleType < 1 || vehicleType > 7)
    {
        printf("Invalid vehicle type.\n");
        return;
    }
    int slotIndex = findFreeSlot(accessType);
    if (slotIndex < 0)
    {
        printf("No available slot in this zone.\n");
        return;
    }
    strcpy(parkingSlots[slotIndex].plate, plate);
    strcpy(parkingSlots[slotIndex].owner, username);
    parkingSlots[slotIndex].vehicleType = vehicleType;
    parkingSlots[slotIndex].accessType = accessType;
    parkingSlots[slotIndex].status = 1;
    parkingSlots[slotIndex].entryTime = time(NULL);
    saveParkingSlots();
    appendParkingHistory("ENTRY", plate, parkingSlots[slotIndex].slotNumber,
                         accessType == 1 ? "Subscriber check-in" : "Cash check-in");
    printf("IR sensor detected vehicle and entrance barrier opened.\n");
    printf("Check-in successful, slot %d assigned.\n", parkingSlots[slotIndex].slotNumber);
}

static void checkOutVehicle(const char *username)
{
    char plate[20];
    readLine("Enter vehicle plate number: ", plate, sizeof(plate));
    int slotIndex = findParkedSlot(plate);
    if (slotIndex < 0)
    {
        printf("Vehicle not found in parked slots.\n");
        return;
    }
    if (strcmp(parkingSlots[slotIndex].owner, username) != 0)
    {
        printf("Vehicle not registered under your account.\n");
        return;
    }
    time_t now = time(NULL);
    double hours = difftime(now, parkingSlots[slotIndex].entryTime) / 3600.0;
    if (hours < 1.0)
    {
        hours = 1.0;
    }
    double rate = getRate(parkingSlots[slotIndex].vehicleType);
    double charge = 0.0;
    if (parkingSlots[slotIndex].accessType == 2)
    {
        double discount = hasActiveSubscription(plate) ? 0.10 : CASH_DISCOUNT;
        charge = hours * rate * (1.0 - discount);
    }
    int bookingIndex = findBooking(plate);
    double fine = 0.0;
    if (bookingIndex >= 0 && now > bookings[bookingIndex].end)
    {
        fine = BOOKING_FINE;
        bookings[bookingIndex].active = 0;
        saveBookings();
        printf("Booking expired. Fine %.2f imposed.\n", fine);
    }
    double total = charge + fine;
    printf("\n--- Checkout Receipt ---\n");
    printf("Plate: %s\n", plate);
    printf("Slot: %d\n", parkingSlots[slotIndex].slotNumber);
    printf("Duration: %.2f hours\n", hours);
    printf("Parking fee: %.2f\n", charge);
    if (fine > 0)
    {
        printf("Fine: %.2f\n", fine);
    }
    printf("Total due: %.2f\n", total);
    appendTransaction(plate, username, parkingSlots[slotIndex].vehicleType,
                      parkingSlots[slotIndex].accessType, total,
                      parkingSlots[slotIndex].accessType == 2 ? "Cash checkout" : "Subscriber checkout");
    if (parkingSlots[slotIndex].accessType == 2)
    {
        appendCashReceipt(&parkingSlots[slotIndex], hours, charge);
    }
    appendParkingHistory("EXIT", plate, parkingSlots[slotIndex].slotNumber,
                         parkingSlots[slotIndex].accessType == 2 ? "Cash checkout" : "Subscriber checkout");
    parkingSlots[slotIndex].status = 0;
    parkingSlots[slotIndex].plate[0] = '\0';
    parkingSlots[slotIndex].owner[0] = '\0';
    parkingSlots[slotIndex].vehicleType = 0;
    parkingSlots[slotIndex].accessType = 0;
    parkingSlots[slotIndex].entryTime = 0;
    saveParkingSlots();
    printf("IR sensor detected exit and barrier opened.\n");
}

static void userDashboard(const char *username)
{
    while (1)
    {
        printf("\n=== USER DASHBOARD ===\n");
        printf("1. Register Vehicle\n");
        printf("2. Subscribe / Cancel Subscription\n");
        printf("3. Booking / Cancel Booking\n");
        printf("4. Check-In\n");
        printf("5. Check-Out\n");
        printf("6. Refresh\n");
        printf("7. Logout\n");
        printf("0. Back\n");
        printf("Enter choice: ");
        int choice;
        if (scanf("%d", &choice) != 1)
        {
            clearInputBuffer();
            printf("Invalid selection.\n");
            continue;
        }
        clearInputBuffer();
        if (choice == 0)
        {
            printf("Returning to login menu.\n");
            return;
        }
        switch (choice)
        {
        case 1:
            registerVehicle(username);
            break;
        case 2:
            subscribeVehicle(username);
            break;
        case 3:
            manageBooking(username);
            break;
        case 4:
            checkInVehicle(username);
            break;
        case 5:
            checkOutVehicle(username);
            break;
        case 6:
            printf("Refreshing... returning to main menu.\n");
            refreshToMainMenu = 1;
            return;
        case 7:
            printf("Logging out. Thank you for using Parking Management System.\n");
            return;
        default:
            printf("Invalid selection.\n");
            break;
        }
    }
}

static void loginMenu(void)
{
    while (1)
    {
        printf("\n=== LOGIN MENU ===\n");
        printf("1. Login\n");
        printf("2. Forgot Password\n");
        printf("0. Back\n");
        printf("Enter choice: ");
        int choice;
        if (scanf("%d", &choice) != 1)
        {
            clearInputBuffer();
            printf("Invalid selection.\n");
            continue;
        }
        clearInputBuffer();
        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            char username[50];
            char password[50];
            readLine("Enter username: ", username, sizeof(username));
            readLine("Enter password: ", password, sizeof(password));
            int index = findUser(username);
            if (index < 0 || strcmp(users[index].password, password) != 0)
            {
                printf("Login failed.\n");
                printf("Forgot password? (1=Yes, 0=No): ");
                int forgot;
                if (scanf("%d", &forgot) != 1)
                {
                    clearInputBuffer();
                    continue;
                }
                clearInputBuffer();
                if (forgot == 1)
                {
                    forgetPassword();
                }
            }
            else
            {
                printf("Login successful.\n");
                refreshToMainMenu = 0;
                userDashboard(username);
                if (refreshToMainMenu)
                {
                    return;
                }
            }
        }
        else if (choice == 2)
        {
            forgetPassword();
        }
        else
        {
            printf("Invalid selection.\n");
        }
    }
}

int main(void)
{
    srand((unsigned int)time(NULL));
    loadUsers();
    loadVehicles();
    loadSubscriptions();
    loadBookings();
    initializeParkingSlots();
    while (1)
    {
        printf("\n=== PARKING LOT MANAGEMENT ===\n");
        printf("1. Create Account\n");
        printf("2. Login\n");
        printf("3. Admin Panel\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        int choice;
        if (scanf("%d", &choice) != 1)
        {
            clearInputBuffer();
            printf("Invalid selection.\n");
            continue;
        }
        clearInputBuffer();
        if (choice == 0)
        {
            printf("Exiting.\n");
            return 0;
        }
        if (choice == 1)
        {
            createAccount();
        }
        else if (choice == 2)
        {
            loginMenu();
        }
        else if (choice == 3)
        {
            adminMenu();
        }
        else
        {
            printf("Invalid selection.\n");
        }
    }
}
