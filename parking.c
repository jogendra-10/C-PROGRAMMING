/*
 * SMART PARKING SLOT ALLOCATION AND BILLING SYSTEM
 * ------------------------------------------------
 * A simple menu-driven C program.
 *
 * Module 1 : Slot Management        (arrays, loops, conditionals)
 * Module 2 : Vehicle Entry          (structures, functions)
 * Module 3 : Vehicle Exit & Billing (functions, conditionals, pointers)
 * Module 4 : History & Reports      (file handling, sorting)
 */

#include <stdio.h>
#include <string.h>

#define TOTAL_SLOTS 10
#define MAX_RECORDS 100
#define HISTORY_FILE "history.txt"

/* ---------- Structures ---------- */

struct Vehicle {
    char number[15];
    int  type;        /* 1 = Bike, 2 = Car, 3 = Heavy */
    int  entryHour;   /* 0 to 23 */
    int  occupied;    /* 0 = free, 1 = occupied */
};

struct Record {
    char  number[15];
    int   type;
    int   slotNo;
    int   entryHour;
    int   exitHour;
    float fee;
};

/* The parking lot: one array of structures */
struct Vehicle slots[TOTAL_SLOTS];

/* ---------- Function declarations ---------- */

/* Module 1 */
void initSlots();
int  findFreeSlot();
void showAvailability();

/* Module 2 */
void parkVehicle();
int  findVehicle(char num[]);

/* Module 3 */
void  exitVehicle();
float calcFee(int in, int out, int type);

/* Module 4 */
void saveRecord(struct Record *r);
int  loadRecords(struct Record r[]);
void showHistory();
void revenueReport();

/* =====================================================
   MODULE 1 : SLOT MANAGEMENT
   ===================================================== */

/* Make every slot free when the program starts */
void initSlots() {
    int i;
    for (i = 0; i < TOTAL_SLOTS; i++) {
        slots[i].occupied = 0;
    }
}

/* Return the index of the first free slot, or -1 if the lot is full */
int findFreeSlot() {
    int i;
    for (i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].occupied == 0)
            return i;
    }
    return -1;   /* lot full */
}

/* Print every slot and count how many are free */
void showAvailability() {
    int i;
    int freeCount = 0;

    printf("\n--- Slot Availability ---\n");
    for (i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].occupied == 0) {
            printf("Slot %2d : Free\n", i + 1);
            freeCount++;
        } else {
            printf("Slot %2d : Occupied by %s\n", i + 1, slots[i].number);
        }
    }
    printf("Free slots: %d out of %d\n", freeCount, TOTAL_SLOTS);
}

/* =====================================================
   MODULE 2 : VEHICLE ENTRY AND REGISTRATION
   ===================================================== */

/* Look for a parked vehicle by number. Returns slot index or -1 */
int findVehicle(char num[]) {
    int i;
    for (i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].occupied == 1 && strcmp(slots[i].number, num) == 0)
            return i;
    }
    return -1;
}

void parkVehicle() {
    struct Vehicle v;
    int slot;

    /* Step 1: read the details */
    printf("\n--- Vehicle Entry ---\n");
    printf("Enter vehicle number (no spaces): ");
    scanf("%14s", v.number);

    printf("Enter vehicle type (1 = Bike, 2 = Car, 3 = Heavy): ");
    scanf("%d", &v.type);

    printf("Enter entry hour (0-23): ");
    scanf("%d", &v.entryHour);

    /* Step 2: check the type */
    if (v.type < 1 || v.type > 3) {
        printf("Invalid vehicle type! Use 1, 2 or 3.\n");
        return;
    }

    /* Extra check: hour must be valid */
    if (v.entryHour < 0 || v.entryHour > 23) {
        printf("Invalid hour! Use a number from 0 to 23.\n");
        return;
    }

    /* Step 3: check the vehicle is not already parked */
    if (findVehicle(v.number) != -1) {
        printf("This vehicle is already parked!\n");
        return;
    }

    /* Step 4: find a free slot */
    slot = findFreeSlot();

    /* Step 5: lot full? */
    if (slot == -1) {
        printf("Parking is full!\n");
        return;
    }

    /* Step 6: copy the whole vehicle into the slot */
    v.occupied = 1;
    slots[slot] = v;

    printf("Slot %d allotted to %s\n", slot + 1, v.number);
}

/* =====================================================
   MODULE 3 : VEHICLE EXIT AND BILLING
   ===================================================== */

/* Fee = hours x rate. Less than 1 hour is charged as 1 hour */
float calcFee(int in, int out, int type) {
    int hours = out - in;
    float rate;

    if (type == 1) rate = 10;        /* Bike  */
    else if (type == 2) rate = 20;   /* Car   */
    else rate = 40;                  /* Heavy */

    if (hours < 1) hours = 1;

    return hours * rate;
}

void exitVehicle() {
    char num[15];
    int slot, exitHour;
    struct Record rec;

    printf("\n--- Vehicle Exit ---\n");
    printf("Enter vehicle number: ");
    scanf("%14s", num);

    /* Step 1: find the vehicle */
    slot = findVehicle(num);
    if (slot == -1) {
        printf("Vehicle not found in the parking lot!\n");
        return;
    }

    /* Step 2: read the exit hour */
    printf("Enter exit hour (0-23): ");
    scanf("%d", &exitHour);

    if (exitHour < 0 || exitHour > 23) {
        printf("Invalid hour! Use a number from 0 to 23.\n");
        return;
    }
    if (exitHour < slots[slot].entryHour) {
        printf("Exit hour cannot be before entry hour (%d)!\n",
               slots[slot].entryHour);
        return;
    }

    /* Step 3: calculate the fee */
    strcpy(rec.number, slots[slot].number);
    rec.type      = slots[slot].type;
    rec.slotNo    = slot + 1;
    rec.entryHour = slots[slot].entryHour;
    rec.exitHour  = exitHour;
    rec.fee       = calcFee(rec.entryHour, rec.exitHour, rec.type);

    /* Step 4: save the visit to the file (pass a pointer) */
    saveRecord(&rec);

    /* Step 5: free the slot */
    slots[slot].occupied = 0;

    /* Step 6: print the fee */
    printf("Slot %d is free now. Fee for %s = Rs. %.2f\n",
           rec.slotNo, rec.number, rec.fee);
}

/* =====================================================
   MODULE 4 : HISTORY AND REVENUE REPORTS
   ===================================================== */

/* Add one line to history.txt */
void saveRecord(struct Record *r) {
    FILE *fp = fopen(HISTORY_FILE, "a");   /* "a" = add to the end */

    if (fp == NULL) {
        printf("Error: could not open %s\n", HISTORY_FILE);
        return;
    }

    fprintf(fp, "%s %d %d %d %d %.2f\n",
            r->number, r->type, r->slotNo,
            r->entryHour, r->exitHour,
            r->fee);
    fclose(fp);
}

/* Read all records from the file. Returns how many were read */
int loadRecords(struct Record r[]) {
    FILE *fp = fopen(HISTORY_FILE, "r");
    int n = 0;

    if (fp == NULL)
        return 0;   /* no file yet = no history */

    while (n < MAX_RECORDS &&
           fscanf(fp, "%14s %d %d %d %d %f",
                  r[n].number, &r[n].type, &r[n].slotNo,
                  &r[n].entryHour, &r[n].exitHour, &r[n].fee) == 6) {
        n++;
    }

    fclose(fp);
    return n;
}

void showHistory() {
    struct Record r[MAX_RECORDS];
    int n, i;

    n = loadRecords(r);

    printf("\n--- Parking History ---\n");
    if (n == 0) {
        printf("No records yet.\n");
        return;
    }

    printf("%-12s %5s %4s %4s %7s\n", "Vehicle", "Slot", "In", "Out", "Fee");
    for (i = 0; i < n; i++) {
        printf("%-12s %5d %4d %4d %7.2f\n",
               r[i].number, r[i].slotNo,
               r[i].entryHour, r[i].exitHour, r[i].fee);
    }
}

void revenueReport() {
    struct Record r[MAX_RECORDS];
    struct Record temp;
    int n, i, j;
    float total = 0;

    n = loadRecords(r);

    printf("\n--- Revenue Report (highest fee first) ---\n");
    if (n == 0) {
        printf("No records yet.\n");
        return;
    }

    /* Bubble sort: highest fee goes first */
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (r[j].fee < r[j + 1].fee) {
                temp = r[j];
                r[j] = r[j + 1];
                r[j + 1] = temp;
            }
        }
    }

    /* Print the sorted list and add up the total */
    for (i = 0; i < n; i++) {
        printf("  %-12s Rs. %8.2f\n", r[i].number, r[i].fee);
        total = total + r[i].fee;
    }

    printf("Vehicles served: %d   Total revenue: Rs. %.2f\n", n, total);
}

/* =====================================================
   MAIN : MENU
   ===================================================== */

int main() {
    int choice;

    initSlots();

    do {
        printf("\n===== SMART PARKING SYSTEM =====\n");
        printf("1. Park a vehicle (entry)\n");
        printf("2. Remove a vehicle (exit and bill)\n");
        printf("3. Show slot availability\n");
        printf("4. Show parking history\n");
        printf("5. Revenue report\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");

        /* If the user types a letter, stop the program safely */
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Closing program.\n");
            break;
        }

        switch (choice) {
            case 1: parkVehicle();      break;
            case 2: exitVehicle();      break;
            case 3: showAvailability(); break;
            case 4: showHistory();      break;
            case 5: revenueReport();    break;
            case 0: printf("Thank you! Goodbye.\n"); break;
            default: printf("Invalid choice! Try again.\n");
        }
    } while (choice != 0);

    return 0;
}
