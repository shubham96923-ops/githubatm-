/* atm_simulation.c
   Simple ATM Simulation for 1st-year B.Tech students.
   Features:
   - PIN verification (3 attempts)
   - Check balance
   - Deposit
   - Withdraw (with insufficient funds check)
   - Mini-statement (last N transactions)
   - Change PIN
   - Save/load data to file: balance and pin persist between runs
   Compile: gcc atm_simulation.c -o atm_simulation
   Run: ./atm_simulation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "atm_data.txt"
#define MAX_TX 10
#define PIN_LENGTH 6

typedef struct {
    char type[10];    // "Deposit" or "Withdraw"
    double amount;
} Transaction;

typedef struct {
    double balance;
    char pin[PIN_LENGTH];
    Transaction txs[MAX_TX];
    int tx_count; // total transactions stored (circular)
} ATMData;

void init_default(ATMData *atm) {
    atm->balance = 1000.0; // default starting balance
    strcpy(atm->pin, "1234"); // default PIN
    atm->tx_count = 0;
}

int load_data(ATMData *atm) {
    FILE *f = fopen(DATA_FILE, "r");
    if (!f) return 0; // file doesn't exist
    if (fscanf(f, "%lf %s %d", &atm->balance, atm->pin, &atm->tx_count) != 3) {
        fclose(f);
        return 0;
    }
    // read transactions
    for (int i = 0; i < atm->tx_count && i < MAX_TX; ++i) {
        char type[10];
        double amount;
        if (fscanf(f, "%s %lf", type, &amount) == 2) {
            strcpy(atm->txs[i].type, type);
            atm->txs[i].amount = amount;
        } else {
            break;
        }
    }
    fclose(f);
    return 1;
}

void save_data(ATMData *atm) {
    FILE *f = fopen(DATA_FILE, "w");
    if (!f) {
        printf("Warning: Could not save data.\n");
        return;
    }
    fprintf(f, "%.2f %s %d\n", atm->balance, atm->pin, atm->tx_count);
    for (int i = 0; i < atm->tx_count && i < MAX_TX; ++i) {
        fprintf(f, "%s %.2f\n", atm->txs[i].type, atm->txs[i].amount);
    }
    fclose(f);
}

void add_transaction(ATMData *atm, const char *type, double amount) {
    if (atm->tx_count < MAX_TX) {
        strcpy(atm->txs[atm->tx_count].type, type);
        atm->txs[atm->tx_count].amount = amount;
        atm->tx_count++;
    } else {
        // shift left and append to end (keep last MAX_TX)
        for (int i = 1; i < MAX_TX; ++i) atm->txs[i - 1] = atm->txs[i];
        strcpy(atm->txs[MAX_TX - 1].type, type);
        atm->txs[MAX_TX - 1].amount = amount;
    }
}

int verify_pin(ATMData *atm) {
    char input[PIN_LENGTH];
    int tries = 3;
    while (tries--) {
        printf("Enter PIN: ");
        if (scanf("%s", input) != 1) return 0;
        if (strcmp(input, atm->pin) == 0) return 1;
        printf("Incorrect PIN. %d attempt(s) left.\n", tries);
    }
    return 0;
}

void check_balance(ATMData *atm) {
    printf("Your current balance: %.2f\n", atm->balance);
}

void deposit(ATMData *atm) {
    double amount;
    printf("Enter amount to deposit: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("Invalid amount.\n");
        while (getchar() != '\n'); // clear input
        return;
    }
    atm->balance += amount;
    add_transaction(atm, "Deposit", amount);
    printf("Deposited %.2f successfully.\n", amount);
    save_data(atm);
}

void withdraw(ATMData *atm) {
    double amount;
    printf("Enter amount to withdraw: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("Invalid amount.\n");
        while (getchar() != '\n');
        return;
    }
    if (amount > atm->balance) {
        printf("Insufficient funds. Current balance: %.2f\n", atm->balance);
        return;
    }
    atm->balance -= amount;
    add_transaction(atm, "Withdraw", amount);
    printf("Withdrawn %.2f successfully.\n", amount);
    save_data(atm);
}

void mini_statement(ATMData *atm) {
    printf("----- Mini Statement (last %d) -----\n", atm->tx_count);
    for (int i = 0; i < atm->tx_count; ++i) {
        printf("%d. %s : %.2f\n", i + 1, atm->txs[i].type, atm->txs[i].amount);
    }
    if (atm->tx_count == 0) printf("No transactions yet.\n");
}

void change_pin(ATMData *atm) {
    char oldpin[PIN_LENGTH], newpin[PIN_LENGTH], confirm[PIN_LENGTH];
    printf("Enter current PIN: ");
    if (scanf("%s", oldpin) != 1) return;
    if (strcmp(oldpin, atm->pin) != 0) {
        printf("PIN does not match.\n");
        return;
    }
    printf("Enter new PIN: ");
    if (scanf("%s", newpin) != 1) return;
    printf("Confirm new PIN: ");
    if (scanf("%s", confirm) != 1) return;
    if (strcmp(newpin, confirm) != 0) {
        printf("PINs do not match. Aborting.\n");
        return;
    }
    strcpy(atm->pin, newpin);
    save_data(atm);
    printf("PIN changed successfully.\n");
}

int main() {
    ATMData atm;
    if (!load_data(&atm)) {
        init_default(&atm);
        save_data(&atm); // create data file with default values
    }

    printf("Welcome to Simple ATM Simulation\n");
    if (!verify_pin(&atm)) {
        printf("Too many incorrect attempts. Exiting.\n");
        return 0;
    }

    int choice = 0;
    while (1) {
        printf("\n--- ATM Menu ---\n");
        printf("1. Check Balance\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Mini Statement\n");
        printf("5. Change PIN\n");
        printf("6. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1: check_balance(&atm); break;
            case 2: deposit(&atm); break;
            case 3: withdraw(&atm); break;
            case 4: mini_statement(&atm); break;
            case 5: change_pin(&atm); break;
            case 6:
                printf("Thank you. Goodbye.\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}