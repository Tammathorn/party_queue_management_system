#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

#define PASSWORD_MANAGER "partyowner"
#define PASSWORD_VIP "panpan"
#define NAME_LENGTH 50
#define FILE_NAME "queue_data.csv"
#define LOG_FILE "system_log.txt" 

typedef struct Customer {
    char name[NAME_LENGTH];
    int isVIP;
    int queueNumber;
    int waitTime;
    struct Customer *next;
} Customer;

Customer *head = NULL;
int queueOpen = 0;
int queueLimit = 5;
int customerCount = 0;
int timePerCustomer = 5;

typedef struct Stack {
    Customer *customer;
    struct Stack *next;
} Stack;

Stack *removedCustomers = NULL;

// ====== Main Systems =====
void managerPage();
void customerPage();
void pushRemovedCustomer(Customer *customer);
void popRemovedCustomer();
void setQueueLimit();
void openCloseQueue();
void viewCustomerDetails();
void assignTime();
void removeUser();
void inputCustomer();
void viewOwnDetails();
void addCustomer(char name[], int isVIP);
int nameExists(char name[]);
void countdownQueue();
void displayMenu();
// =========================


// CSV system
int getIntInput();
void saveQueueToCSV();
void loadQueueFromCSV();
void appendCustomerToCSV(Customer *c);
void rewriteCSV();
// =========================

// Log system
void logEvent(const char *message);
void logCustomerAction(const char *action, const char *name, int isVIP);
void logSystemAction(const char *action, const char *details);
void logManagerAction(const char *action);
void getCurrentTimeString(char *buffer, size_t bufferSize);
void viewLogs();
// =========================

// Helper
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Logging implementation
void getCurrentTimeString(char *buffer, size_t bufferSize) {
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", tm_info);
}

void logEvent(const char *message) {
    FILE *logFile = fopen(LOG_FILE, "a");
    if (logFile == NULL) return;
    
    char timestamp[20];
    getCurrentTimeString(timestamp, sizeof(timestamp));
    
    fprintf(logFile, "[%s] %s\n", timestamp, message);
    fclose(logFile);
}

void logCustomerAction(const char *action, const char *name, int isVIP) {
    char message[100];
    snprintf(message, sizeof(message), "CUSTOMER: %s - Name: %s, VIP: %s", 
             action, name, isVIP ? "Yes" : "No");
    logEvent(message);
}

void logSystemAction(const char *action, const char *details) {
    char message[150];
    snprintf(message, sizeof(message), "SYSTEM: %s - %s", action, details);
    logEvent(message);
}

void logManagerAction(const char *action) {
    char message[100];
    snprintf(message, sizeof(message), "MANAGER: %s", action);
    logEvent(message);
}

void viewLogs() {
    FILE *logFile = fopen(LOG_FILE, "r");
    if (logFile == NULL) {
        printf("Unable to open log file.\n");
        return;
    }

    char line[256];
    printf("\n=== SYSTEM LOGS ===\n");
    while (fgets(line, sizeof(line), logFile) != NULL) {
        printf("%s", line);
    }
    fclose(logFile);
}

void pushRemovedCustomer(Customer *customer) {
    Stack *newNode = (Stack *)malloc(sizeof(Stack));
    newNode->customer = customer;
    newNode->next = removedCustomers;
    removedCustomers = newNode;
    
    char message[100];
    snprintf(message, sizeof(message), "Customer removed (Name: %s, VIP: %d)", 
             customer->name, customer->isVIP);
    logSystemAction("Customer removed from queue", message);
}

void popRemovedCustomer() {
    if (removedCustomers != NULL) {
        Stack *temp = removedCustomers;
        removedCustomers = removedCustomers->next;
        free(temp->customer);
        free(temp);
        logSystemAction("Removed customer cleared from memory", "");
    }
}

void removeExtraCustomers() {
    while (customerCount > queueLimit) {
        printf("Queue limit exceeded! Removing last customer.\n");
        if (head == NULL) return;
        if (head->next == NULL) {
            pushRemovedCustomer(head);
            head = NULL;
            customerCount--;
            rewriteCSV();
            logSystemAction("Queue limit enforced", "Last customer removed");
            return;
        }
        Customer *temp = head;
        while (temp->next->next != NULL) {
            temp = temp->next;
        }
        pushRemovedCustomer(temp->next);
        free(temp->next);
        temp->next = NULL;
        customerCount--;
        rewriteCSV();
        logSystemAction("Queue limit enforced", "Customer removed from end of queue");
    }
}

void displayMenu() {
    printf("\n==== PARTY QUEUE SYSTEM ====\n");
    printf("1. Manager\n");
    printf("2. Customer\n");
    printf("Choose (1/2) or 'r' to return: ");
}

void managerPage() {
    char pass[50];
    printf("Enter Manager Password: ");
    scanf("%s", pass);
    clearInputBuffer();
    if (strcmp(pass, PASSWORD_MANAGER) != 0) {
        printf("Wrong password! Press 'r' to return.\n");
        logSystemAction("Manager login failed", "Incorrect password");
        return;
    }
    
    logManagerAction("Logged in successfully");
    int choice;
    while (1) {
        printf("\n=== Manager Page ===\n");
        printf("1. Set Queue Limit\n");
        printf("2. Open/Close Queue\n");
        printf("3. View Customer Details\n");
        printf("4. Assign Time for Queue\n");
        printf("5. Remove User\n");
        printf("6. Return\n");
        printf("7. View Logs\n");
        printf("Enter your choice: ");
        choice = getIntInput();
        switch(choice) {
            case 1: 
                logManagerAction("Selected Set Queue Limit");
                setQueueLimit(); 
                break;
            case 2: 
                logManagerAction("Selected Open/Close Queue");
                openCloseQueue(); 
                break;
            case 3: 
                logManagerAction("Viewed Customer Details");
                viewCustomerDetails(); 
                break;
            case 4: 
                logManagerAction("Selected Assign Time for Queue");
                assignTime(); 
                break;
            case 5: 
                logManagerAction("Selected Remove User");
                removeUser(); 
                break;
            case 6: 
                logManagerAction("Returned to main menu");
                return;
            case 7:
                logManagerAction("Viewed system logs");
                viewLogs();
                break;
            default: 
                printf("Invalid choice.\n");
                logSystemAction("Invalid manager menu choice", "");
        }
    }
}

void customerPage() {
    if (!queueOpen) {
        printf("Queue is closed. Press 'r' to return.\n");
        logSystemAction("Customer tried to access closed queue", "");
        return;
    }
    
    logSystemAction("Customer page accessed", "Queue is open");
    int choice;
    clearInputBuffer();
    while (1) {
        printf("\n=== Customer Page ===\n");
        printf("1. Enter Queue\n");
        printf("2. View My Details\n");
        printf("3. Return\n");
        printf("Enter your choice: ");
        choice = getIntInput();
        switch(choice) {
            case 1: 
                logSystemAction("Customer selected Enter Queue", "");
                inputCustomer(); 
                break;
            case 2: 
                logSystemAction("Customer selected View My Details", "");
                viewOwnDetails(); 
                break;
            case 3: 
                logSystemAction("Customer returned to main menu", "");
                return;
            default: 
                printf("Invalid choice.\n");
                logSystemAction("Invalid customer menu choice", "");
        }
    }
}

void setQueueLimit() {
    int newLimit;
    printf("Enter new queue limit: ");
    newLimit = getIntInput();
    if (newLimit <= 0) {
        printf("Error: Queue limit must be greater than 0. Try again.\n");
        logSystemAction("Invalid queue limit attempted", "Value <= 0");
        return;
    }
    
    char message[50];
    snprintf(message, sizeof(message), "Queue limit changed from %d to %d", queueLimit, newLimit);
    logManagerAction(message);
    
    queueLimit = newLimit;
    removeExtraCustomers();
    printf("Queue limit set to %d.\n", queueLimit);
    viewCustomerDetails();
}

void openCloseQueue() {
    int choice;
    while (1) {
        printf("1. Open Queue\n2. Close Queue\n");
        printf("Enter your choice (1 or 2): ");
        choice = getIntInput();
        if (choice == 1) {
            queueOpen = 1;
            printf("Queue is now Open.\n");
            logManagerAction("Queue opened");
            break;
        } else if (choice == 2) {
            queueOpen = 0;
            printf("Queue is now Closed.\n");
            logManagerAction("Queue closed");
            break;
        } else {
            printf("Invalid option! Please enter either 1 (Open) or 2 (Close).\n");
            logSystemAction("Invalid queue state change attempt", "");
        }
    }
}


void viewCustomerDetails() {
    Customer *temp = head;
    printf("\nQueue Number | VIP | Name | Wait Time\n");
    printf("--------------------------------------\n");
    while (temp != NULL) {
        printf("%12d | %3s | %-10s | %d\n", temp->queueNumber, temp->isVIP ? "Yes" : "No", temp->name, temp->waitTime);
        temp = temp->next;
    }
}

void assignTime() {
    printf("Enter wait time per customer: ");
    int newTime = getIntInput();
    if (newTime <= 0) {
        printf("Invalid time. Try again.\n");
        logSystemAction("Invalid wait time attempted", "Value <= 0");
        return;
    }
    
    char message[50];
    snprintf(message, sizeof(message), "Wait time changed from %d to %d", timePerCustomer, newTime);
    logManagerAction(message);
    
    timePerCustomer = newTime;
    Customer *temp = head;
    while (temp != NULL) {
        temp->waitTime = timePerCustomer;
        temp = temp->next;
    }
    rewriteCSV();
}

void removeUser() {
    char name[NAME_LENGTH];
    printf("Enter customer name to remove: ");
    scanf("%s", name);
    Customer *temp = head, *prev = NULL;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            if (prev == NULL) head = temp->next;
            else prev->next = temp->next;
            pushRemovedCustomer(temp);
            free(temp);
            customerCount--;
            rewriteCSV();
            
            char message[100];
            snprintf(message, sizeof(message), "Removed customer: %s", name);
            logManagerAction(message);
            
            printf("Customer removed.\n");
            viewCustomerDetails();
            return;
        }
        prev = temp;
        temp = temp->next;
    }
    printf("Customer not found.\n");
    logSystemAction("Customer removal failed", "Customer not found");
}

void inputCustomer() {
    char name[NAME_LENGTH];
    printf("Enter your name: ");
    scanf("%s", name);
    if (nameExists(name)) {
        printf("Name already exists. Please change your name.\n");
        logSystemAction("Duplicate name attempted", name);
        return;
    }
    int status;
    while (1) {
        printf("Enter status (1 for VIP, 0 for Normal): ");
        status = getIntInput();
        if (status == 1 || status == 0) break;
        else printf("Invalid input! Please enter either 1 (VIP) or 0 (Normal).\n");
    }
    if (status == 1) {
        char vipPass[50];
        printf("Enter VIP password: ");
        scanf("%s", vipPass);
        if (strcmp(vipPass, PASSWORD_VIP) != 0) {
            printf("Wrong VIP password. Returning to main menu.\n");
            logSystemAction("Failed VIP attempt", name);
            return;
        }
        logSystemAction("VIP customer authenticated", name);
    }
    addCustomer(name, status);
}

void addCustomer(char name[], int isVIP) {
    Customer *newCustomer = (Customer *)malloc(sizeof(Customer));
    strcpy(newCustomer->name, name);
    newCustomer->isVIP = isVIP;
    newCustomer->queueNumber = ++customerCount;
    newCustomer->waitTime = timePerCustomer;
    newCustomer->next = NULL;

    if (head == NULL || (isVIP && !head->isVIP)) {
        newCustomer->next = head;
        head = newCustomer;
    } else {
        Customer *temp = head;
        while (temp->next != NULL && (temp->next->isVIP || (!isVIP))) {
            temp = temp->next;
        }
        newCustomer->next = temp->next;
        temp->next = newCustomer;
    }

    logCustomerAction("Added to queue", name, isVIP);
    appendCustomerToCSV(newCustomer);
    removeExtraCustomers();
}

int nameExists(char name[]) {
    Customer *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) return 1;
        temp = temp->next;
    }
    return 0;
}

void viewOwnDetails() {
    char name[NAME_LENGTH];
    printf("Enter your name to view details: ");
    scanf("%s", name);
    Customer *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            printf("\nName: %s\nStatus: %s\nWait Time: %d\n", temp->name, temp->isVIP ? "VIP" : "Normal", temp->waitTime);
            logCustomerAction("Viewed details", name, temp->isVIP);
            return;
        }
        temp = temp->next;
    }
    printf("Customer not found.\n");
    logSystemAction("Customer details not found", name);
}

void countdownQueue() {
    if (head == NULL) return;
    Customer *temp = head;
    while (temp != NULL) {
        temp->waitTime--;
        temp = temp->next;
    }
    while (head != NULL && head->waitTime <= 0) {
        printf("\nCustomer %s can now enter the party!\n", head->name);
        logCustomerAction("Entered the party", head->name, head->isVIP);
        
        Customer *toRemove = head;
        head = head->next;
        free(toRemove);
        customerCount--;
    }
    rewriteCSV();
}

// File Functions
void appendCustomerToCSV(Customer *c) {
    FILE *fp = fopen(FILE_NAME, "a");
    if (!fp) {
        logSystemAction("File error", "Failed to open queue_data.csv for appending");
        return;
    }
    fprintf(fp, "%s,%d,%d,%d\n", c->name, c->isVIP, c->queueNumber, c->waitTime);
    fclose(fp);
}

void rewriteCSV() {
    FILE *fp = fopen(FILE_NAME, "w");
    if (!fp) {
        logSystemAction("File error", "Failed to open queue_data.csv for writing");
        return;
    }
    Customer *temp = head;
    while (temp != NULL) {
        fprintf(fp, "%s,%d,%d,%d\n", temp->name, temp->isVIP, temp->queueNumber, temp->waitTime);
        temp = temp->next;
    }
    fclose(fp);
}

void loadQueueFromCSV() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) {
        logSystemAction("File error", "Failed to open queue_data.csv for reading (might be first run)");
        return;
    }
    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        Customer *c = (Customer *)malloc(sizeof(Customer));
        sscanf(line, "%[^,],%d,%d,%d", c->name, &c->isVIP, &c->queueNumber, &c->waitTime);
        c->next = NULL;
        if (c->queueNumber > customerCount)
            customerCount = c->queueNumber;
        if (head == NULL)
            head = c;
        else {
            Customer *temp = head;
            while (temp->next != NULL) temp = temp->next;
            temp->next = c;
        }
    }
    fclose(fp);
    logSystemAction("System startup", "Queue loaded from file");
}

// Safe int input
int getIntInput() {
    char buffer[100];
    int num;
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin)) {
            if (sscanf(buffer, "%d", &num) == 1) {
                return num;
            } else {
                printf("Invalid input. Please enter a number: ");
                logSystemAction("Invalid integer input", buffer);
            }
        }
    }
}

int main() {
    logSystemAction("System started", "Application initialization");
    loadQueueFromCSV(); // CSV
    char choice[5];
    while (1) {
        displayMenu();
        scanf("%s", choice);
        if (strcmp(choice, "r") == 0) {
            logSystemAction("User returned to main menu", "");
            continue;
        }
        else if (strcmp(choice, "1") == 0) managerPage();
        else if (strcmp(choice, "2") == 0) customerPage();
        else {
            printf("Invalid input.\n");
            logSystemAction("Invalid main menu input", choice);
        }

        countdownQueue();
    }
    return 0;
}
