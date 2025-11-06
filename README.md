# 🧠 Operating Systems – Exercise 1  
**Reichman University – April 2025**

## 👥 Authors
Yegor, Karaev  
Ram Eliyahu, Hamrani  

---

## 📦 Project Structure
This repository contains 6 main directories — one for each task:

TAS_spinlock/ → Task 1: TAS Semaphore
Ticket_lock/ → Task 2: Ticket Lock Semaphore
Condition_var/ → Task 3: Condition Variable
RW_lock/ → Task 4: Read–Write Lock
TLS/ → Task 5: Thread-Local Storage
Producer_consumer/ → Task 6: Producer–Consumer Pattern

Each directory includes:
- Source files (`.c`)
- Header files (`.h`)
- Makefile (if applicable)

All tasks are implemented in **C23**, compiled with **GCC 13** on **Ubuntu 24.04 LTS**.

---
### **Task 1 – TAS Semaphore**
Implement a **semaphore** using a **Test-And-Set spinlock**.  
Functions:  
- `semaphore_init()`
- `semaphore_wait()`
- `semaphore_signal()`

---

### **Task 2 – Ticket Lock Semaphore**
Implement a **semaphore** using a **ticket lock** mechanism.  
Functions:  
- `semaphore_init()`
- `semaphore_wait()`
- `semaphore_signal()`

---

### **Task 3 – Condition Variable**
Implement a **condition variable** to allow threads to wait for specific conditions.  
Functions:  
- `condition_variable_init()`
- `condition_variable_wait()`
- `condition_variable_signal()`
- `condition_variable_broadcast()`

---

### **Task 4 – Read–Write Lock**
Implement a **reader–writer lock** supporting multiple readers or one writer at a time.  
Functions:  
- `rwlock_init()`
- `rwlock_acquire_read()`
- `rwlock_release_read()`
- `rwlock_acquire_write()`
- `rwlock_release_write()`

---

### **Task 5 – Thread-Local Storage (TLS)**
Implement **thread-local storage** using a **static array** (no dynamic memory).  
Functions:  
- `init_storage()`
- `tls_thread_alloc()`
- `get_tls_data()`
- `set_tls_data()`
- `tls_thread_free()`

---

### **Task 6 – Producer–Consumer Pattern**
Implement a **Producer–Consumer** system that checks divisibility by 6.  
- Producers generate numbers and push them to a queue.  
- Consumers check divisibility and print synchronized results.  

Functions:  
- `start_consumers_producers(int consumers, int producers, int seed)`
- `stop_consumers()`
- `print_msg(const char* msg)`
- `wait_until_producers_produced_all_numbers()`
- `wait_consumers_queue_empty()`

---

## ⚙️ Compilation Instructions

Make sure you have **gcc-13** installed.  
Each task can be compiled separately, for example:
```bash
cd task1
gcc -std=c23 -Wall -Wextra -o tas_semaphore tas_semaphore.c


# 🧠 Operating Systems – Exercise 1  
**Reichman University – April 2025**  
**Final Score:** 🏆 **95 / 100**


