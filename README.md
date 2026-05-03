

## 📋 Project Overview

CineBook is a full-featured **Movie Ticket Booking System** built with:
- **Language:** C++ (C++17)
- **GUI Framework:** Qt 5/6 (Widgets)
- **Database:** SQLite (via Qt SQL module)
- **IDE:** Visual Studio / Qt Creator / Dev-C++ compatible

The system implements full **OOP principles**: classes, objects, inheritance, encapsulation, and abstraction.

---

## 🏗️ Project Structure

```
MovieTicketBookingSystem/
├── CMakeLists.txt          ← Build configuration
├── README.md               ← This file
├── include/                ← Header files (.h)
│   ├── models.h            ← Data models (User, Movie, Seat, Booking)
│   ├── database.h          ← Database singleton class
│   ├── loginwindow.h       ← Login/Register window
│   ├── mainwindow.h        ← User dashboard
│   ├── adminwindow.h       ← Admin panel
│   ├── seatselection.h     ← Seat picker dialog
│   └── receiptdialog.h     ← Booking receipt/print dialog
└── src/                    ← Implementation files (.cpp)
    ├── main.cpp            ← Entry point
    ├── database.cpp        ← SQLite database operations
    ├── loginwindow.cpp     ← Login UI & authentication
    ├── mainwindow.cpp      ← User interface
    ├── adminwindow.cpp     ← Admin interface
    ├── seatselection.cpp   ← Interactive seat map
    └── receiptdialog.cpp   ← Receipt generation & printing
```

---

## ✨ Features Implemented

### 🔐 User Authentication
- Secure SHA-256 password hashing
- Role-based access: **Admin** and **User**
- User registration with validation
- Show/hide password toggle

### 🎬 Movie Management (Admin)
- Add / Edit / Delete movies
- Set ticket price, show times, seat count
- Genre, duration, rating information
- Soft delete (deactivate movies)

### 🪑 Seat Selection
- Visual interactive 6×10 seat map (60 seats)
- Color coded: Available (dark), Selected (red), Booked (gray)
- Aisle gap between columns 4-5
- Maximum 8 seats per booking
- Real-time seat availability

### 🎟️ Ticket Booking
- Choose movie → select show time → pick seats → confirm
- Automatic total price calculation
- Atomic seat booking (prevents double-booking)
- Booking confirmation receipt

### 🖨️ Receipt / Print
- Detailed booking receipt with booking ID
- Print to physical printer via Qt Print Support
- Shows: movie, time, seats, tickets, total, date, status

### 📊 Admin Dashboard
- Live stats: Total movies, bookings, revenue, users
- View all bookings with status
- Manage user accounts
- Full booking report table

### 👤 User Dashboard
- Browse available movies with search
- Movie details: genre, rating, duration, description
- My Bookings history
- Cancel / view past bookings

---

## 🗄️ Database Schema

### users
```sql
id, username, password (SHA-256), role, full_name, email, created_at
```

### movies
```sql
id, title, genre, duration, description, ticket_price, 
show_times, total_seats, is_active, rating
```

### seats
```sql
id, movie_id, show_time, seat_code, is_booked
```

### bookings
```sql
id, user_id, movie_id, show_time, seats (CSV), 
num_tickets, total_amount, booked_at, status
```

---

## 🚀 How to Build & Run

### Prerequisites
- Qt 5.12+ or Qt 6.x (with Widgets, Sql, PrintSupport modules)
- CMake 3.16+
- C++17 compatible compiler (MSVC / GCC / Clang)

### Build Steps

```bash
# 1. Create build directory
mkdir build && cd build

# 2. Configure
cmake ..

# 3. Build
cmake --build . --config Release

# 4. Run
./MovieTicketBookingSystem
```

### Using Qt Creator
1. Open `CMakeLists.txt` in Qt Creator
2. Configure kit (Qt 5/6 + compiler)
3. Click Build → Run

---

## 🔑 Default Credentials

| Username | Password | Role |
|----------|----------|------|
| `admin` | `admin123` | Admin |
| `user1` | `user123` | User |

---

## 🎭 OOP Concepts Used

| Concept | Where Used |
|---------|-----------|
| **Classes** | User, Movie, Seat, Booking, Database, LoginWindow, MainWindow, AdminWindow, SeatButton, SeatSelection, ReceiptDialog |
| **Encapsulation** | Private members with public accessors in all classes |
| **Singleton Pattern** | `Database::instance()` — single DB connection |
| **Inheritance** | SeatButton extends QPushButton; all windows extend QMainWindow/QDialog |
| **Abstraction** | Database class hides SQL details from UI classes |
| **Polymorphism** | Qt signals/slots, virtual methods from Qt base classes |
| **Composition** | Windows contain and manage child widgets |

---

## 📅 Project Timeline

| Week | Milestone |
|------|-----------|
| Week 1 | Topic discussion, team formation |
| Week 2 | Proposal preparation & submission |
| Week 3 | Requirements analysis, class diagrams, flowcharts |
| Week 4 | Mid-semester examinations (no development) |
| Week 5 | Basic classes: Movie, User, Database, Login |
| Week 6 | Seat selection, booking, fee calculation |
| Week 7 | Testing & debugging |
| Week 8 | Documentation, presentation slides, final demo |

---

## ⚠️ Risks & Mitigations

| Risk | Mitigation |
|------|-----------|
| C++ knowledge gaps | Team study sessions, W3Schools, Qt documentation |
| Incorrect fee calculations | Unit testing of price logic |
| Team communication | Regular meetings, WhatsApp group |
| Technical skill gaps | Pair programming, code reviews |
| Time management | Weekly milestones tracking |

---

## 📚 Resources Used
- Qt Documentation: https://doc.qt.io
- W3Schools C++ Reference
- SQLite Documentation
- CMake Documentation

---

*University of Ruhuna — Faculty of Technology*
*Bachelor of Engineering Technology (Hons) — Level 2, Semester 1, 2026*
