#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_RECORDS 100
#define DATA_FILE   "credit.dat"
#define EXPORT_FILE "accounts.txt"
#define LOG_FILE    "transaction_history.txt"

struct clientData {
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// Prototypes
void initializeFile(FILE *fPtr);
void logTransaction(const char *msg);
void exportAccounts(FILE *fPtr);    // Choice 1
void updateRecord(FILE *fPtr);      // Choice 2
void newRecord(FILE *fPtr);         // Choice 3
void deleteRecord(FILE *fPtr);      // Choice 4
void accountSummary(FILE *fPtr);    // Choice 6
void viewHistory();                 // Choice 7

int main() {
    FILE *cfPtr;
    int choice;

    if ((cfPtr = fopen(DATA_FILE, "rb+")) == NULL) {
        printf("File not found. Creating new %s file...\n", DATA_FILE);
        cfPtr = fopen(DATA_FILE, "wb+");
        if (!cfPtr) { printf("Error: Cannot create file.\n"); return 1; }
        initializeFile(cfPtr);
        printf("File created successfully.\n");
    }

    const char *menu =
        "\nEnter your choice\n"
        "1 - store a formatted text file of accounts called\n"
        "    \"accounts.txt\" for printing\n"
        "2 - update an account\n"
        "3 - add a new account\n"
        "4 - delete an account\n"
        "5 - end program\n"
        "6 - account summary\n"
        "7 - view transaction history\n"
        "? ";

    while (printf("%s", menu) && scanf("%d", &choice) == 1 && choice != 5) {
        switch (choice) {
            case 1: exportAccounts(cfPtr); break;
            case 2: updateRecord(cfPtr);   break;
            case 3: newRecord(cfPtr);      break;
            case 4: deleteRecord(cfPtr);   break;
            case 6: accountSummary(cfPtr); break;
            case 7: viewHistory();         break;
            default: printf("Invalid choice. Please enter 1-4, 6, or 7.\n");
        }
    }

    printf("\nEnd of run.\n");
    fclose(cfPtr);
    return 0;
}

/* ─────────────────────────────────────────
   HELPER: Initialize 100 blank records
   ───────────────────────────────────────── */
void initializeFile(FILE *fPtr) {
    struct clientData blank = {0, "", "", 0.0};
    for (int i = 0; i < MAX_RECORDS; i++)
        fwrite(&blank, sizeof(struct clientData), 1, fPtr);
    logTransaction("SYSTEM  - File initialized (credit.dat created)");
}

/* ─────────────────────────────────────────
   HELPER: Append timestamped log entry
   ───────────────────────────────────────── */
void logTransaction(const char *msg) {
    FILE *lPtr = fopen(LOG_FILE, "a");
    if (!lPtr) return;
    time_t now = time(NULL);
    char *ts = ctime(&now);
    ts[strlen(ts) - 1] = '\0';           // strip trailing newline
    fprintf(lPtr, "[%s]  %s\n", ts, msg);
    fclose(lPtr);
}

/* ─────────────────────────────────────────
   CHOICE 1: Export all accounts to accounts.txt
   ───────────────────────────────────────── */
void exportAccounts(FILE *fPtr) {
    struct clientData c;
    FILE *outPtr = fopen(EXPORT_FILE, "w");
    if (!outPtr) {
        printf("Error: Could not open %s for writing.\n", EXPORT_FILE);
        return;
    }

    /* Header */
    fprintf(outPtr, "%-10s %-16s %-11s %10s\n",
            "Account", "Last Name", "First Name", "Balance");
    fprintf(outPtr, "------------------------------------------------\n");

    rewind(fPtr);
    int count = 0;
    while (fread(&c, sizeof(struct clientData), 1, fPtr)) {
        if (c.acctNum != 0) {
            fprintf(outPtr, "%-10u %-16s %-11s %10.2f\n",
                    c.acctNum, c.lastName, c.firstName, c.balance);
            count++;
        }
    }

    fprintf(outPtr, "------------------------------------------------\n");
    fprintf(outPtr, "Total accounts: %d\n", count);
    fclose(outPtr);

    if (count == 0)
        printf("No active accounts to export.\n");
    else
        printf("%d account(s) written to \"%s\".\n", count, EXPORT_FILE);

    logTransaction("EXPORT  - Accounts exported to accounts.txt");
}

/* ─────────────────────────────────────────
   CHOICE 2: Update (adjust balance of) account
   ───────────────────────────────────────── */
void updateRecord(FILE *fPtr) {
    struct clientData c;
    unsigned int acc;
    double adjustment;

    printf("Enter account number to update: ");
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX_RECORDS) {
        printf("Account number must be 1-%d.\n", MAX_RECORDS);
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&c, sizeof(struct clientData), 1, fPtr);

    if (c.acctNum == 0) {
        printf("Account #%u is empty.\n", acc);
        return;
    }

    printf("\n  Account  : %u\n", c.acctNum);
    printf("  Name     : %s %s\n", c.firstName, c.lastName);
    printf("  Balance  : %.2f\n", c.balance);
    printf("Enter adjustment (+credit / -debit): ");
    scanf("%lf", &adjustment);

    c.balance += adjustment;

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&c, sizeof(struct clientData), 1, fPtr);

    printf("Updated balance for account #%u: %.2f\n", acc, c.balance);

    char buf[120];
    sprintf(buf, "UPDATE  - Acct #%-3u  %-10s %-10s  Adjustment: %+.2f  New Balance: %.2f",
            acc, c.lastName, c.firstName, adjustment, c.balance);
    logTransaction(buf);
}

/* ─────────────────────────────────────────
   CHOICE 3: Add a new account
   ───────────────────────────────────────── */
void newRecord(FILE *fPtr) {
    struct clientData c = {0, "", "", 0.0};
    unsigned int acc;

    printf("Enter new account number (1-%d): ", MAX_RECORDS);
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX_RECORDS) {
        printf("Account number must be 1-%d.\n", MAX_RECORDS);
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&c, sizeof(struct clientData), 1, fPtr);

    if (c.acctNum != 0) {
        printf("Account #%u already exists (%s %s).\n",
               acc, c.firstName, c.lastName);
        return;
    }

    c.acctNum = acc;
    printf("Enter last name  : "); scanf("%14s", c.lastName);
    printf("Enter first name : "); scanf("%9s",  c.firstName);
    printf("Enter balance    : "); scanf("%lf",  &c.balance);

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&c, sizeof(struct clientData), 1, fPtr);

    printf("Account #%u created for %s %s.\n", acc, c.firstName, c.lastName);

    char buf[120];
    sprintf(buf, "NEW     - Acct #%-3u  %-10s %-10s  Opening Balance: %.2f",
            acc, c.lastName, c.firstName, c.balance);
    logTransaction(buf);
}

/* ─────────────────────────────────────────
   CHOICE 4: Delete an account
   ───────────────────────────────────────── */
void deleteRecord(FILE *fPtr) {
    struct clientData c, blank = {0, "", "", 0.0};
    unsigned int acc;

    printf("Enter account number to delete: ");
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX_RECORDS) {
        printf("Account number must be 1-%d.\n", MAX_RECORDS);
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&c, sizeof(struct clientData), 1, fPtr);

    if (c.acctNum == 0) {
        printf("Account #%u is already empty.\n", acc);
        return;
    }

    printf("Deleting account #%u  -  %s %s  (Balance: %.2f)\n",
           c.acctNum, c.firstName, c.lastName, c.balance);

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&blank, sizeof(struct clientData), 1, fPtr);

    printf("Account #%u deleted.\n", acc);

    char buf[120];
    sprintf(buf, "DELETE  - Acct #%-3u  %-10s %-10s  Balance at close: %.2f",
            acc, c.lastName, c.firstName, c.balance);
    logTransaction(buf);
}

/* ═══════════════════════════════════════════════════════
   CHOICE 6 — ACCOUNT SUMMARY LOGIC
   Shows:
     • Total number of active accounts
     • Total funds held across all accounts
     • Average balance
     • Highest balance account
     • Lowest balance account
     • Count of accounts with negative balance (overdrawn)
   ═══════════════════════════════════════════════════════ */
void accountSummary(FILE *fPtr) {
    struct clientData c;
    int    count       = 0;
    int    overdrawn   = 0;
    double total       = 0.0;
    double highest     = -1e18;
    double lowest      =  1e18;
    char   highName[30] = "";
    char   lowName[30]  = "";
    unsigned int highAcct = 0, lowAcct = 0;

    rewind(fPtr);
    while (fread(&c, sizeof(struct clientData), 1, fPtr)) {
        if (c.acctNum != 0) {
            count++;
            total += c.balance;

            if (c.balance > highest) {
                highest  = c.balance;
                highAcct = c.acctNum;
                snprintf(highName, sizeof(highName), "%s %s", c.firstName, c.lastName);
            }
            if (c.balance < lowest) {
                lowest  = c.balance;
                lowAcct = c.acctNum;
                snprintf(lowName, sizeof(lowName), "%s %s", c.firstName, c.lastName);
            }
            if (c.balance < 0.0) overdrawn++;
        }
    }

    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║         ACCOUNT SUMMARY              ║\n");
    printf("╠══════════════════════════════════════╣\n");

    if (count == 0) {
        printf("║   No active accounts found.          ║\n");
        printf("╚══════════════════════════════════════╝\n");
        return;
    }

    printf("║  Total Active Accounts : %-4d         ║\n", count);
    printf("║  Total Funds on Deposit: $%-10.2f  ║\n", total);
    printf("║  Average Balance       : $%-10.2f  ║\n", total / count);
    printf("╠══════════════════════════════════════╣\n");
    printf("║  Highest Balance                     ║\n");
    printf("║    Acct #%-4u  %-14s         ║\n", highAcct, highName);
    printf("║    Balance: $%-10.2f              ║\n", highest);
    printf("╠══════════════════════════════════════╣\n");
    printf("║  Lowest Balance                      ║\n");
    printf("║    Acct #%-4u  %-14s         ║\n", lowAcct, lowName);
    printf("║    Balance: $%-10.2f              ║\n", lowest);
    printf("╠══════════════════════════════════════╣\n");
    printf("║  Overdrawn Accounts    : %-4d         ║\n", overdrawn);
    printf("╚══════════════════════════════════════╝\n");

    logTransaction("SUMMARY - Account summary viewed");
}

/* ═══════════════════════════════════════════════════════
   CHOICE 7 — VIEW TRANSACTION HISTORY LOGIC
   Reads transaction_history.txt and displays:
     • Each timestamped log line
     • A running count of entries
     • Separates sections (NEW / UPDATE / DELETE / EXPORT)
       with a label so output is easy to read
   ═══════════════════════════════════════════════════════ */
void viewHistory() {
    FILE *lPtr = fopen(LOG_FILE, "r");
    char  line[256];
    int   total = 0, adds = 0, updates = 0, deletes = 0, exports = 0;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  TRANSACTION HISTORY                        ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    if (!lPtr) {
        printf("║  No transaction history found.                               ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        return;
    }

    while (fgets(line, sizeof(line), lPtr)) {
        /* strip newline for clean printing */
        line[strcspn(line, "\n")] = '\0';

        /* count by type */
        if (strstr(line, "NEW"))    adds++;
        if (strstr(line, "UPDATE")) updates++;
        if (strstr(line, "DELETE")) deletes++;
        if (strstr(line, "EXPORT")) exports++;
        total++;

        printf("║  %s\n", line);
    }
    fclose(lPtr);

    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  TOTALS  |  Entries: %-3d  New: %-3d  Updates: %-3d           ║\n",
           total, adds, updates);
    printf("║          |  Deletes: %-3d  Exports: %-3d                      ║\n",
           deletes, exports);
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}