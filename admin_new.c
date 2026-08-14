#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "admin.h"

static AdminAccount admins[MAX_ADMIN_ACCOUNTS];

static void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void readLine(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

static void loadAdmins(void) {
    FILE *fp = fopen(ADMIN_FILE, "r");
    if (fp == NULL) {
        for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
            admins[i].active = 0;
        }
        return;
    }
    char line[200];
    int index = 0;
    while (fgets(line, sizeof(line), fp) && index < MAX_ADMIN_ACCOUNTS) {
        if (sscanf(line, "%49[^,],%49[^,],%19[^\\n]",
                   admins[index].username,
                   admins[index].password,
                   admins[index].phone) == 3) {
            admins[index].active = 1;
            index++;
        }
    }
    fclose(fp);
}

static void saveAdmins(void) {
    FILE *fp = fopen(ADMIN_FILE, "w");
    if (fp == NULL) {
        printf("Unable to save admin accounts.\n");
        return;
    }
    for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
        if (admins[i].active) {
            fprintf(fp, "%s,%s,%s\n",
                    admins[i].username,
                    admins[i].password,
                    admins[i].phone);
        }
    }
    fclose(fp);
}

static int findAdminIndex(const char *username) {
    for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
        if (admins[i].active && strcmp(admins[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

static const char* getSlotStatusLabel(int status) {
    switch (status) {
        case 0:
            return "Available";
        case 1:
            return "Unavailable";
        case 2:
            return "Disabled";
        default:
            return "Unknown";
    }
}

static const char* getVehicleTypeLabel(int vehicleType) {
    switch (vehicleType) {
        case 1:
            return "Car";
        case 2:
            return "Bike";
        case 3:
            return "Truck";
        case 4:
            return "Bus";
        case 5:
            return "SUV";
        case 6:
            return "Van";
        case 7:
            return "Microbus";
        default:
            return "Unknown";
    }
}

static void printSlotStatus(const char *filename, const char *label) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("%s not found.\n", label);
        return;
    }
    printf("\n--- %s ---\n", label);
    printf("%-8s %-12s %-20s %-12s %-15s\n", 
           "Slot", "Plate", "Owner", "Vehicle Type", "Status");
    printf("%-8s %-12s %-20s %-12s %-15s\n", 
           "----", "-----", "-----", "------------", "------");
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        int slotNum, vehicleType, accessType, status;
        char plate[20], owner[50];
        long entryTime;
        
        if (sscanf(buffer, "%d,%19[^,],%49[^,],%d,%d,%d,%ld",
                   &slotNum, plate, owner, &vehicleType, &accessType, &status, &entryTime) >= 7) {
            const char *statusLabel = getSlotStatusLabel(status);
            const char *vehicleLabel = getVehicleTypeLabel(vehicleType);
            printf("%-8d %-12s %-20s %-12s %-15s\n", 
                   slotNum, plate, owner, vehicleLabel, statusLabel);
        }
    }
    fclose(fp);
    printf("\n");
}

static void printCsv(const char *filename, const char *label) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("%s not found.\n", label);
        return;
    }
    printf("\n--- %s ---\n", label);
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        printf("%s", buffer);
    }
    fclose(fp);
    printf("\n");
}

static void changeSlotStatus(int disabled) {
    while (1) {
        FILE *fp = fopen(SLOT_FILE, "r");
        if (fp == NULL) {
            printf("Parking slots file missing.\n");
            return;
        }
        char lines[150][256];
        int count = 0;
        while (fgets(lines[count], sizeof(lines[count]), fp) && count < 150) {
            count++;
        }
        fclose(fp);
        int slot = 0;
        printf("Enter slot number (0 to go back): ");
        if (scanf("%d", &slot) != 1) {
            clearInputBuffer();
            printf("Invalid slot number.\n");
            return;
        }
        clearInputBuffer();
        if (slot == 0) {
            printf("Going back to admin dashboard.\n");
            return;
        }
        if (slot < 1 || slot > count) {
            printf("Slot number out of range.\n");
            continue;
        }
        int number, vehicleType, accessType, status;
        long entryTime;
        char plate[20], owner[50];
        if (sscanf(lines[slot - 1], "%d,%19[^,],%49[^,],%d,%d,%d,%ld",
                   &number, plate, owner, &vehicleType, &accessType, &status, &entryTime) >= 1) {
            status = disabled ? 2 : 0;
            snprintf(lines[slot - 1], sizeof(lines[slot - 1]), "%d,%s,%s,%d,%d,%d,%ld\n",
                     number, plate, owner, vehicleType, accessType, status, entryTime);
            fp = fopen(SLOT_FILE, "w");
            if (fp == NULL) {
                printf("Cannot update slot file.\n");
                return;
            }
            for (int i = 0; i < count; i++) {
                fputs(lines[i], fp);
            }
            fclose(fp);
            printf("Slot %d %s.\n", slot, disabled ? "disabled" : "enabled");
            
            printf("\n1. Enable/Disable another slot\n");
            printf("0. Back to admin dashboard\n");
            printf("Enter choice: ");
            int choice;
            if (scanf("%d", &choice) != 1) {
                clearInputBuffer();
                printf("Invalid input.\n");
                return;
            }
            clearInputBuffer();
            if (choice == 0) {
                return;
            }
        }
    }
}

static int sendOtp(void) {
    int otp = 100000 + rand() % 900000;
    printf("Simulated OTP: %06d\n", otp);
    return otp;
}

void adminMenu(void) {
    loadAdmins();
    while (1) {
        printf("\n=== ADMIN PANEL ===\n");
        printf("1. Create Admin Account\n");
        printf("2. Admin Login\n");
        printf("3. Forget Password\n");
        printf("0. Back\n");
        printf("Enter choice: ");
        int choice;
        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("Invalid input.\n");
            continue;
        }
        clearInputBuffer();
        if (choice == 0) {
            return;
        }
        if (choice == 1) {
            char username[50], password[50], phone[20];
            readLine("Enter admin username: ", username, sizeof(username));
            if (findAdminIndex(username) >= 0) {
                printf("Admin already exists.\n");
                continue;
            }
            readLine("Enter phone number: ", phone, sizeof(phone));
            readLine("Enter password: ", password, sizeof(password));
            for (int i = 0; i < MAX_ADMIN_ACCOUNTS; i++) {
                if (!admins[i].active) {
                    admins[i].active = 1;
                    strcpy(admins[i].username, username);
                    strcpy(admins[i].password, password);
                    strcpy(admins[i].phone, phone);
                    saveAdmins();
                    printf("Admin account created.\n");
                    break;
                }
            }
            continue;
        }
        if (choice == 2) {
            char username[50], password[50];
            readLine("Enter admin username: ", username, sizeof(username));
            int index = findAdminIndex(username);
            if (index < 0) {
                printf("Admin not found.\n");
                continue;
            }
            readLine("Enter password: ", password, sizeof(password));
            if (strcmp(admins[index].password, password) != 0) {
                printf("Incorrect password.\n");
                printf("Forgot password? (1=Yes, 0=No): ");
                int forgot;
                if (scanf("%d", &forgot) != 1) {
                    clearInputBuffer();
                    continue;
                }
                clearInputBuffer();
                if (forgot == 1) {
                    char phone[20];
                    readLine("Enter registered phone number: ", phone, sizeof(phone));
                    if (strcmp(phone, admins[index].phone) != 0) {
                        printf("Phone number mismatch.\n");
                        continue;
                    }
                    int otp = sendOtp();
                    int entered;
                    printf("Enter OTP: ");
                    if (scanf("%d", &entered) != 1) {
                        clearInputBuffer();
                        continue;
                    }
                    clearInputBuffer();
                    if (entered != otp) {
                        printf("OTP failed.\n");
                        continue;
                    }
                    char newPass[50], confirm[50];
                    readLine("Enter new password: ", newPass, sizeof(newPass));
                    readLine("Retype new password: ", confirm, sizeof(confirm));
                    if (strcmp(newPass, confirm) != 0) {
                        printf("Passwords do not match.\n");
                        continue;
                    }
                    strcpy(admins[index].password, newPass);
                    saveAdmins();
                    printf("Password reset successfully.\n");
                }
                continue;
            }
            while (1) {
                printf("\n--- ADMIN DASHBOARD ---\n");
                printf("1. View Full Slot Status\n");
                printf("2. Disable Slot\n");
                printf("3. Enable Slot\n");
                printf("4. View Vehicle Register History\n");
                printf("5. View Payment History\n");
                printf("6. View Subscriber History\n");
                printf("7. View Non-Subscriber History\n");
                printf("8. View Booking History\n");
                printf("9. View Cash Transaction History\n");
                printf("0. Logout\n");
                printf("Enter choice: ");
                int adminChoice;
                if (scanf("%d", &adminChoice) != 1) {
                    clearInputBuffer();
                    printf("Invalid input.\n");
                    continue;
                }
                clearInputBuffer();
                if (adminChoice == 0) {
                    break;
                }
                switch (adminChoice) {
                    case 1:
                        printSlotStatus(SLOT_FILE, "Slot Status");
                        break;
                    case 2:
                        changeSlotStatus(1);
                        break;
                    case 3:
                        changeSlotStatus(0);
                        break;
                    case 4:
                        printCsv(VEHICLE_FILE, "Vehicle Register History");
                        break;
                    case 5:
                        printCsv(TRANSACTION_FILE, "Payment History");
                        break;
                    case 6:
                        printCsv(SUBSCRIPTION_FILE, "Subscriber History");
                        break;
                    case 7:
                        printCsv(BOOKING_FILE, "Non-Subscriber History");
                        break;
                    case 8:
                        printCsv(BOOKING_FILE, "Booking History");
                        break;
                    case 9:
                        printCsv(CASH_RECEIPTS_FILE, "Cash Transaction History");
                        break;
                    default:
                        printf("Invalid selection.\n");
                        break;
                }
            }
            continue;
        }
        if (choice == 3) {
            char username[50], phone[20];
            readLine("Enter admin username: ", username, sizeof(username));
            int index = findAdminIndex(username);
            if (index < 0) {
                printf("Admin not found.\n");
                continue;
            }
            readLine("Enter registered phone number: ", phone, sizeof(phone));
            if (strcmp(phone, admins[index].phone) != 0) {
                printf("Phone number mismatch.\n");
                continue;
            }
            int otp = sendOtp();
            int entered;
            printf("Enter OTP: ");
            if (scanf("%d", &entered) != 1) {
                clearInputBuffer();
                printf("OTP verification failed.\n");
                continue;
            }
            clearInputBuffer();
            if (entered != otp) {
                printf("OTP failed.\n");
                continue;
            }
            char newPass[50], confirm[50];
            readLine("Enter new password: ", newPass, sizeof(newPass));
            readLine("Retype new password: ", confirm, sizeof(confirm));
            if (strcmp(newPass, confirm) != 0) {
                printf("Passwords do not match.\n");
                continue;
            }
            strcpy(admins[index].password, newPass);
            saveAdmins();
            printf("Password reset successfully.\n");
            continue;
        }
    }
}
