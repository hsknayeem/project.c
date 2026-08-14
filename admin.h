#ifndef ADMIN_H
#define ADMIN_H

#include <time.h>

#define MAX_ADMIN_ACCOUNTS 10

#define SLOT_FILE "parking_slots.csv"
#define VEHICLE_FILE "vehicles.csv"
#define TRANSACTION_FILE "payment_transactions.csv"
#define BOOKING_FILE "bookings.csv"
#define SUBSCRIPTION_FILE "subscriptions.csv"
#define ADMIN_FILE "admin_accounts.csv"
#define HISTORY_FILE "parking_history.csv"
#define CASH_RECEIPTS_FILE "cash_receipts.csv"

typedef struct {
    int slotNumber;
    char plate[20];
    int vehicleType;
    int accessType;
    int status; // 0 = free, 1 = occupied, 2 = disabled
    time_t entryTime;
    char username[50];
} Slot;

typedef struct {
    char username[50];
    char password[50];
    char phone[20];
    int active;
} AdminAccount;

void initializeAdminPanel();
void adminMenu();

#endif // ADMIN_H
