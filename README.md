Party Queue Management System
=============================

A C-based console application for managing customer queues in a party environment. It supports both VIP and regular customers, logging, and basic queue management features via a manager and customer interface.

Features
--------

### Manager Functions
- Set Queue Limit: Adjust the maximum number of people allowed in the queue.
- Open/Close Queue: Control customer access to the queue.
- View Customer Details: Display all customers currently in the queue.
- Assign Time for Queue: Set the estimated wait time per customer.
- Remove User: Remove a customer by name.
- View System Log: Review actions and events logged in `system_log.txt`.

### Customer Functions
- Enter Queue: Join the queue as VIP or normal customer.
- View Own Details: View individual queue details using name input.

### VIP Access
- Requires a separate VIP password (`panpan`) to enter the VIP queue.

### Logging System
All significant actions are logged into `system_log.txt` with timestamps:
- Customer joins or exits the queue
- Manager changes settings or removes customers
- System status changes (queue open/closed, invalid attempts)

### File Persistence
- Customer data is stored and loaded from `queue_data.csv`
- Removed customers are stored in a temporary stack (not persisted)

File Structure
--------------

- `main.c`: Main program source code.
- `queue_data.csv`: Stores current queue state.
- `system_log.txt`: Stores log events for manager/system monitoring.

Build and Run
-------------

### Compile
gcc -o party_queue main.c

### Run
./party_queue

Default Credentials
-------------------

- Manager Password: `partyowner`
- VIP Password: `panpan`

To-Do / Enhancements
--------------------

- Add persistent stack storage for removed customers.
- Add estimated waiting time prediction per user.
- Improve input validation and support for full names (with spaces).
