#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "admin.h"

static AdminAccount admins[MAX_ADMIN_ACCOUNTS];

static void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void readInput(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

static int findAdmin(const char *username) {
    for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
        if (admins[i].created && strcmp(admins[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

void initializeAdminPanel() {
    for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
        admins[i].created = 0;
        admins[i].username[0] = '\0';
        admins[i].password[0] = '\0';
        admins[i].phone[0] = '\0';
    }

    FILE *fp = fopen(ADMIN_FILE, "r");
    if (fp == NULL) {
        return;
    }

    char line[200];
    while (fgets(line, sizeof(line), fp)) {
        char username[50], phone[20], password[50];
        if (sscanf(line, "%49[^,],%19[^,],%49[^
]", username, phone, password) == 3) {
            int index = findAdmin("dummy");
            for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
                if (!admins[i].created) {
                    index = i;
                    break;
                }
            }
            if (index >= 0 && index < MAX_ADMIN_ACCOUNTS) {
                strcpy(admins[index].username, username);
                strcpy(admins[index].phone, phone);
                strcpy(admins[index].password, password);
                admins[index].created = 1;
            }
        }
    }
    fclose(fp);
}

static void saveAdminAccounts() {
    FILE *fp = fopen(ADMIN_FILE, "w");
    if (fp == NULL) {
        printf("Unable to save admin accounts.\n");
        return;
    }

    for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
        if (admins[i].created) {
            fprintf(fp, "%s,%s,%s\n", admins[i].username, admins[i].phone, admins[i].password);
        }
    }
    fclose(fp);
}

static void appendLineToFile(const char *filename, const char *line) {
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) {
        return;
    }
    fprintf(fp, "%s\n", line);
    fclose(fp);
}

static void printCsvFile(const char *filename, const char *title) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("%s file not found.\n", filename);
        return;
    }
    printf("\n--- %s ---\n", title);
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    fclose(fp);
    printf("\n");
}

static void alterSlotStatus(int slotNumber, int newStatus) {
    FILE *fp = fopen(SLOT_FILE, "r");
    if (fp == NULL) {
        printf("Slot data not found.\n");
        return;
    }
    Slot slots[MAX_SLOTS];
    int count = 0;
    char line[200];
    while (fgets(line, sizeof(line), fp) && count < MAX_SLOTS) {
        if (sscanf(line, "%d,%19[^,],%d,%d,%d,%ld,%49[^
]",
                   &slots[count].slotNumber,
                   slots[count].plate,
                   &slots[count].vehicleType,
                   &slots[count].accessType,
                   &slots[count].status,
                   &slots[count].entryTime,
                   slots[count].username) >= 6) {
            count++;
        }
    }
    fclose(fp);

    if (slotNumber < 1 || slotNumber > count) {
        printf("Invalid slot number.\n");
        return;
    }
    slots[slotNumber - 1].status = newStatus;

    fp = fopen(SLOT_FILE, "w");
    if (fp == NULL) {
        printf("Unable to update slot file.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%s,%d,%d,%d,%ld,%s\n",
                slots[i].slotNumber,
                slots[i].plate,
                slots[i].vehicleType,
                slots[i].accessType,
                slots[i].status,
                (long)slots[i].entryTime,
                slots[i].username);
    }
    fclose(fp);
    printf("Slot %d %s successfully.\n", slotNumber, newStatus == 0 ? "enabled" : "disabled");
}

static void printSlotStatus() {
    FILE *fp = fopen(SLOT_FILE, "r");
    if (fp == NULL) {
        printf("Slot data not found.\n");
        return;
    }
    printf("\n--- Slot Status ---\n");
    printf("Slot | Status    | Access      | Vehicle | Plate         | User\n");
    printf("-------------------------------------------------------------\n");
    Slot slot;
    while (fgets((char[]){0}, 1, fp) != EOF) {}
    fseek(fp, 0, SEEK_SET);
    char line[200];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d,%19[^,],%d,%d,%d,%ld,%49[^
]",
                   &slot.slotNumber,
                   slot.plate,
                   &slot.vehicleType,
                   &slot.accessType,
                   &slot.status,
                   &slot.entryTime,
                   slot.username) >= 6) {
            printf("%4d | %s | %s | %7s | %s | %s\n",
                   slot.slotNumber,
                   slot.status == 0 ? "Free    " : slot.status == 1 ? "Occupied" : "Disabled",
                   slot.accessType == 1 ? "Subscriber" : slot.accessType == 2 ? "Cash" : "None",
                   slot.vehicleType == 1 ? "Car" : slot.vehicleType == 2 ? "Bike" : slot.vehicleType == 3 ? "Truck" : "Other",
                   slot.plate[0] ? slot.plate : "-",
                   slot.username[0] ? slot.username : "-");
        }
    }
    fclose(fp);
}

static void viewCsvWithTitle(const char *filename, const char *title) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("No data found for %s.\n", title);
        return;
    }
    printf("\n--- %s ---\n", title);
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    fclose(fp);
    printf("\n");
}

static void exportTransactions() {
    FILE *source = fopen(TRANSACTION_FILE, "r");
    if (source == NULL) {
        printf("No transaction file to export.\n");
        return;
    }
    FILE *dest = fopen("transaction_history.csv", "w");
    if (dest == NULL) {
        printf("Unable to write transaction history export.\n");
        fclose(source);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), source)) {
        fputs(line, dest);
    }
    fclose(source);
    fclose(dest);
    printf("Transaction history exported to transaction_history.csv\n");
}

void adminMenu() {
    int choice;
    initializeAdminPanel();
    while (1) {
        printf("\n=== ADMIN PANEL ===\n");
        printf("1. Create Admin Account\n");
        printf("2. Admin Login\n");
        printf("0. Back to Main Menu\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("Invalid input.\n");
            continue;
        }
        clearInputBuffer();
        if (choice == 0) {
            break;
        }
        else if (choice == 1) {
            char username[50], phone[20], password[50];
            readInput("Enter new admin username: ", username, sizeof(username));
            readInput("Enter mobile number: ", phone, sizeof(phone));
            readInput("Enter password: ", password, sizeof(password));
            if (findAdmin(username) >= 0) {
                printf("Admin username already exists.\n");
                continue;
            }
            int index = -1;
            for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
                if (!admins[i].created) {
                    index = i;
                    break;
                }
            }
            if (index < 0) {
                printf("Admin account limit reached.\n");
                continue;
            }
            strcpy(admins[index].username, username);
            strcpy(admins[index].phone, phone);
            strcpy(admins[index].password, password);
            admins[index].created = 1;
            saveAdminAccounts();
            printf("Admin account created successfully.\n");
        }
        else if (choice == 2) {
            char username[50], password[50], phone[20];
            readInput("Enter admin username: ", username, sizeof(username));
            int adminIndex = findAdmin(username);
            if (adminIndex < 0) {
                printf("Admin username not found.\n");
                continue;
            }
            readInput("Enter password: ", password, sizeof(password));
            if (strcmp(admins[adminIndex].password, password) != 0) {
                printf("Incorrect password.\n");
                int forget;
                printf("Forgot password? (1=Yes, 0=No): ");
                if (scanf("%d", &forget) != 1) {
                    clearInputBuffer();
                    continue;
                }
                clearInputBuffer();
                if (forget == 1) {
                    readInput("Enter registered mobile number: ", phone, sizeof(phone));
                    if (strcmp(phone, admins[adminIndex].phone) != 0) {
                        printf("Mobile number does not match admin record.\n");
                        continue;
                    }
                    int otp = 100000 + rand() % 900000;
                    printf("Simulated OTP sent: %d\n", otp);
                    int entered;
                    printf("Enter OTP: ");
                    if (scanf("%d", &entered) != 1) {
                        clearInputBuffer();
                        continue;
                    }
                    clearInputBuffer();
                    if (entered != otp) {
                        printf("OTP incorrect.\n");
                        continue;
                    }
                    readInput("Enter new password: ", password, sizeof(password));
                    readInput("Retype new password: ", phone, sizeof(phone));
                    if (strcmp(password, phone) != 0) {
                        printf("Passwords do not match.\n");
                        continue;
                    }
                    strcpy(admins[adminIndex].password, password);
                    saveAdminAccounts();
                    printf("Admin password updated successfully.\n");
                }
                continue;
            }
            int loggedIn = 1;
            while (loggedIn) {
                printf("\n--- ADMIN DASHBOARD ---\n");
                printf("1. View Full Slot Status\n");
                printf("2. Disable Slot\n");
                printf("3. Enable Slot\n");
                printf("4. Vehicle Register History\n");
                printf("5. Payment History\n");
                printf("6. Subscriber History\n");
                printf("7. Non-Subscriber History\n");
                printf("8. Booking History\n");
                printf("9. Export Transaction History\n");
                printf("0. Logout\n");
                printf("Enter choice: ");
                if (scanf("%d", &choice) != 1) {
                    clearInputBuffer();
                    printf("Invalid input.\n");
                    continue;
                }
                clearInputBuffer();
                if (choice == 0) {
                    loggedIn = 0;
                    printf("Admin logged out.\n");
                } else if (choice == 1) {
                    printSlotStatus();
                } else if (choice == 2) {
                    int slot;
                    printf("Enter slot number to disable: ");
                    if (scanf("%d", &slot) != 1) {
                        clearInputBuffer();
                        continue;
                    }
                    clearInputBuffer();
                    alterSlotStatus(slot, 2);
                } else if (choice == 3) {
                    int slot;
                    printf("Enter slot number to enable: ");
                    if (scanf("%d", &slot) != 1) {
                        clearInputBuffer();
                        continue;
                    }
                    clearInputBuffer();
                    alterSlotStatus(slot, 0);
                } else if (choice == 4) {
                    viewCsvWithTitle(VEHICLE_FILE, "Vehicle Register History");
                } else if (choice == 5) {
                    viewCsvWithTitle(TRANSACTION_FILE, "Payment History");
                } else if (choice == 6) {
                    viewCsvWithTitle(SUBSCRIPTION_FILE, "Subscriber History");
                } else if (choice == 7) {
                    viewCsvWithTitle(BOOKING_FILE, "Non-Subscriber / Booking History");
                } else if (choice == 8) {
                    viewCsvWithTitle(BOOKING_FILE, "Booking History");
                } else if (choice == 9) {
                    exportTransactions();
                } else {
                    printf("Invalid selection.\n");
                }
            }
        } else {
            printf("Invalid selection.\n");
        }
    }
}
