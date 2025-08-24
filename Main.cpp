#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cctype>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <limits>
#include <iomanip>

using namespace std;
std::string currentUserMobile;

struct BookingDetail {
    string busNo;
    string boarding;
    string dropping;
    string userMobile;
    string status;
    string date;
    set<int> seats;
};
bool isUpcoming(const std::string& tripDate);

bool loginOrRegister(bool isRegister);
bool validateMobileNumber(const string& mobile);
void storeMobileNumber(const string& mobile);
bool checkMobileExists(const string& mobile);
bool verifyOTP();
void dashboard();
void searchBus();
void bookings();
void editProfile();
bool askYesOrNo(const string& question);
void seatSelection(const string& busNo, const string& tripDate);
void passengerInfo(const vector<int>& seats, const string& boarding, const string& dropping, const string& busNo, const string& tripDate);
void modeOfPayment(const string& busNo,
                   const vector<int>& seats,
                   const string& boarding,
                   const string& dropping,
                   const string& userMobile,
                   double amount,
                   const vector<string>& names,
                   const vector<int>& ages,
                   const vector<string>& genders,
                   const string& tripDate);
void payment(const string& busNo,
             const vector<int>& seats,
             const string& boarding,
             const string& dropping,
             const string& userMobile,
             int totalFare,
             const vector<string>& names,
             const vector<int>& ages,
             const vector<string>& genders,
             const string& tripDate);


void upcomingTrips();
void pastTrips();
void cancellation(const string& busNo, const string& ticketId, const string& userMobile);
void freeSeatsAfterCancellation(const string& busNo, const vector<int>& seats);

void addBookingToCSV(const std::string& busNo, const std::vector<int>& seats,
                    const std::string& boarding, const std::string& dropping,
                    const std::string& userMobile, double amount,
                    const std::string& tripDate);


std::tm parseDate(const std::string& dateStr) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return tm;
}


void loadBookings(map<string, BookingDetail>& bookingsMap, const string& currentUserMobile) {
    ifstream file("bookings.csv");
    if (!file.is_open()) {
        cout << "Unable to open bookings.csv\n";
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string ticketId, busNo, seatStr, boarding, dropping, userMobile, status, date;

        getline(ss, ticketId, ','); getline(ss, busNo, ','); getline(ss, seatStr, ',');
        getline(ss, boarding, ','); getline(ss, dropping, ','); getline(ss, userMobile, ',');
        getline(ss, status, ','); getline(ss, date, ',');

        if (userMobile == currentUserMobile) {
            int seat = stoi(seatStr);
            auto& booking = bookingsMap[ticketId];
            booking.busNo = busNo;
            booking.boarding = boarding;
            booking.dropping = dropping;
            booking.userMobile = userMobile;
            booking.status = status;
            booking.date = date;
            booking.seats.insert(seat);
        }
    }
    file.close();
}

void saveBookings(const map<string, BookingDetail>& bookingsMap) {
    ofstream file("bookings.csv");
    for (const auto& [ticketId, booking] : bookingsMap) {
        for (int seat : booking.seats) {
            file << ticketId << "," << booking.busNo << "," << seat << "," << booking.boarding << ","
                 << booking.dropping << "," << booking.userMobile << "," << booking.status << "," << booking.date << "\n";
        }
    }
    file.close();
}


void freeSeatsAfterCancellation(const string& busNo, const vector<int>& seats) {
    fstream file("seats.csv", ios::in);
    if (!file.is_open()) return;

    vector<string> lines;
    string line;
    while (getline(file, line)) {
        if (line.substr(0, busNo.size()) == busNo) {
            stringstream ss(line);
            string bNo, seatNo, status;
            getline(ss, bNo, ',');
            string seatsData = line.substr(busNo.size() + 1);

            vector<string> parts;
            string tmp;
            stringstream sstr(seatsData);
            while (getline(sstr, tmp, ','))
                parts.push_back(tmp);

            for (string& part : parts) {
                size_t colonPos = part.find(':');
                int seatNum = stoi(part.substr(0, colonPos));
                string stat = part.substr(colonPos + 1);
                for (int s : seats) {
                    if (seatNum == s && stat == "occupied")
                        stat = "available";
                }
                part = to_string(seatNum) + ":" + stat;
            }

            string newLine = busNo + "," + parts[0];
            for (int i = 1; i < (int)parts.size(); i++)
                newLine += "," + parts[i];
            lines.push_back(newLine);
        } else {
            lines.push_back(line);
        }
    }
    file.close();

    ofstream outFile("seats.csv");
    for (const string& ln : lines)
        outFile << ln << "\n";
    outFile.close();
}


void cancelBooking(const string& ticketId, map<string, BookingDetail>& bookingsMap) {
    auto it = bookingsMap.find(ticketId);
    if (it == bookingsMap.end()) {
        cout << "Ticket ID not found.\n";
        return;
    }

    BookingDetail& booking = it->second;

    cout << "\nCanceling Ticket ID: " << ticketId << "\n";
    cout << "Bus: " << booking.busNo << "\nSeats: ";
    for (int s : booking.seats) cout << s << " ";
    cout << "\nBoarding: " << booking.boarding << ", Dropping: " << booking.dropping
         << "\nDate: " << booking.date << "\n";

    if (!askYesOrNo("Do you want to cancel this ticket?")) {
        cout << "Cancellation aborted.\n";
        return;
    }

    vector<int> seatsToFree(booking.seats.begin(), booking.seats.end());

    bookingsMap.erase(it);
    saveBookings(bookingsMap); 

    freeSeatsAfterCancellation(booking.busNo, seatsToFree);

    cout << "Cancellation successful! Amount will be refunded in 2 days.\n";
}

void upcomingTrips() {
    map<string, BookingDetail> bookingsMap;
    loadBookings(bookingsMap, currentUserMobile);

    
    vector<string> upcomingTicketIds;
    for (const auto& [ticketId, booking] : bookingsMap) {
        if (isUpcoming(booking.date)) {
            upcomingTicketIds.push_back(ticketId);
        }
    }

    if (upcomingTicketIds.empty()) {
        cout << "No upcoming trips found.\n";
        return;
    }

    bool keepViewing = true;
    while (keepViewing) {
        cout << "\nUpcoming Trips:\n";
        for (const string& tid : upcomingTicketIds) {
            const BookingDetail& booking = bookingsMap[tid];
            cout << "Ticket ID: " << tid << ", Bus: " << booking.busNo << ", Seats: ";
            bool first = true;
            for (int seat : booking.seats) {
                if (!first) cout << ", ";
                cout << seat;
                first = false;
            }
            cout << ", Boarding: " << booking.boarding << ", Dropping: " << booking.dropping
                 << ", Date: " << booking.date << "\n";
        }

        cout << "\nEnter Ticket ID to view details: ";
        string selectedTicketId;
        cin >> selectedTicketId;

        if (bookingsMap.find(selectedTicketId) == bookingsMap.end()) {
            cout << "Invalid Ticket ID.\n";
            continue;
        }

        const BookingDetail& booking = bookingsMap[selectedTicketId];
        cout << "\nTicket Details:\n";
        cout << "Bus No: " << booking.busNo << "\nBoarding: " << booking.boarding
             << "\nDropping: " << booking.dropping << "\nDate: " << booking.date << "\nSeats: ";
        for (int s : booking.seats) cout << s << " ";
        cout << "\n";

        if (askYesOrNo("Do you want to cancel this ticket?")) {
            cancelBooking(selectedTicketId, bookingsMap);
            upcomingTicketIds.clear();
            for (const auto& [tid, book] : bookingsMap) {
                if (isUpcoming(book.date)) {
                    upcomingTicketIds.push_back(tid);
                }
            }
            if (upcomingTicketIds.empty()) {
                cout << "No more upcoming trips.\n";
                return;
            }
        }

        keepViewing = askYesOrNo("Do you want to view another upcoming ticket?");
    }
}


bool isUpcoming(const std::string& tripDate) {
    std::tm tripTm = parseDate(tripDate);
    std::time_t trip_time = std::mktime(&tripTm);
    std::time_t now = std::time(nullptr);
    std::tm* nowTm = std::localtime(&now);
    nowTm->tm_hour = 0;
    nowTm->tm_min = 0;
    nowTm->tm_sec = 0;
    std::time_t today = std::mktime(nowTm);
    return difftime(trip_time, today) > 0;
}

std::string generateTicketId() {
    return "T" + std::to_string(std::time(nullptr));
}

void addBookingToCSV(const std::string& busNo, const std::vector<int>& seats,
                    const std::string& boarding, const std::string& dropping,
                    const std::string& userMobile, double amount,
                    const std::string& tripDate)
{
    std::ofstream file("bookings.csv", std::ios::app);

    std::string status = isUpcoming(tripDate) ? "Upcoming" : "Completed";

    std::string ticketId = generateTicketId();

    for (int seat : seats) {
        file << ticketId << "," << busNo << "," << seat << "," << boarding << ","
             << dropping << "," << userMobile << "," << status << "," << tripDate << "\n";
    }
    file.close();
}

bool validateMobileNumber(const string& mobile) {
    if (mobile.length() != 10) return false;
    for (char c : mobile) {
        if (!isdigit(c)) return false;
    }
    return true;
}

void storeMobileNumber(const string& mobile) {
    ofstream file("users.csv", ios::app);
    file << mobile << endl;
    file.close();
}

bool checkMobileExists(const string& mobile) {
    ifstream file("users.csv");
    string line;
    while (getline(file, line)) {
        if (line == mobile) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool verifyOTP() {
    srand(time(0));
    int otp = rand() % 9000 + 1000; 
    cout << "Your OTP is: " << otp << endl;

    int userOtp;
    cout << "Enter the OTP: ";
    cin >> userOtp;

    return userOtp == otp;
}

bool loginOrRegister(bool isRegister) {
    string mobile;
    while (true) {
        cout << "Enter your 10-digit mobile number: ";
        cin >> mobile;
        if (validateMobileNumber(mobile)) break;
        cout << "Invalid mobile number.\n";
    }

    bool exists = checkMobileExists(mobile);

    if (isRegister) {
        if (exists) {
            cout << "User already registered. Please login.\n";
            return false;
        } else {
            storeMobileNumber(mobile);
            cout << "Registration successful.\n";
        }
    } else {
        if (!exists) {
            cout << "User not found. Please register first.\n";
            return false;
        }
    }

    if (verifyOTP()) {
        cout << "OTP verified successfully.\n";
        currentUserMobile = mobile;
        return true;  
    } else {
        cout << "Incorrect OTP. Returning to main menu.\n";
        return false;
    }
}

bool askYesOrNo(const string& question) {
    string input;
    while (true) {
        cout << question << " (Y/N): ";
        cin >> input;
        if (input == "Y" || input == "y") return true;
        if (input == "N" || input == "n") return false;
        cout << "Invalid input. Please enter Y or N.\n";
    }
}

void editProfile() {
    string name, dob, mobile, email;
    mobile = currentUserMobile;

    cout << "---- Edit Profile ----\n";
    cin.ignore();

    cout << "Enter Name: ";
    getline(cin, name);
    cout << "Enter Date of Birth (YYYY-MM-DD): ";
    getline(cin, dob);
    cout << "Enter Mobile Number: ";
    getline(cin, mobile);
    cout << "Enter Email: ";
    getline(cin, email);

    vector<string> lines;
    ifstream inFile("passenger.csv");
    string line;
    bool found = false;
    while (getline(inFile, line)) {
        stringstream ss(line);
        string csvMobile;
        getline(ss, csvMobile, ',');
        if (csvMobile == mobile) {
            line = mobile + "," + name + "," + dob + "," + email;
            found = true;
        }
        lines.push_back(line);
    }
    inFile.close();

    if (!found) {
        lines.push_back(mobile + "," + name + "," + dob + "," + email);
    }

    ofstream outFile("passenger.csv");
    for (const string& l : lines) {
        outFile << l << "\n";
    }
    outFile.close();

    cout << "Profile updated successfully.\n";
}

void dashboard();

void searchBus() {
    string from, to, date;
    cout << "Enter source location: ";
    cin >> from;
    cout << "Enter destination location: ";
    cin >> to;
    cout << "Enter date of journey (YYYY-MM-DD): ";
    cin >> date;

    ifstream file("bus.csv");
    if (!file.is_open()) {
        cout << "Could not open bus.csv file!\n";
        return;
    }

    string line;
    bool busFound = false;

    cout << "\nAvailable Buses:\n";
    cout << "-------------------------------------------------------------\n";
    cout << "BusNo | Name           | From     | To       | Date       | Time  | Fare\n";
    cout << "-------------------------------------------------------------\n";

    getline(file, line); // skip header line

    vector<tuple<string,string,string,string,string,string,string>> foundBuses;  // store buses to pass date

    while (getline(file, line)) {
        stringstream ss(line);
        string bus_no, bus_name, from_csv, to_csv, date_csv, time, fare;

        getline(ss, bus_no, ',');
        getline(ss, bus_name, ',');
        getline(ss, from_csv, ',');
        getline(ss, to_csv, ',');
        getline(ss, date_csv, ',');
        getline(ss, time, ',');
        getline(ss, fare, ',');

        if ((from_csv == from || from_csv == from + "\r") &&
            (to_csv == to || to_csv == to + "\r") &&
            date_csv == date) {
            cout << bus_no << "     | "
                 << bus_name << " | "
                 << from_csv << " | "
                 << to_csv << " | "
                 << date_csv << " | "
                 << time << " | "
                 << fare << endl;

            foundBuses.push_back(make_tuple(bus_no, bus_name, from_csv, to_csv, date_csv, time, fare));
            busFound = true;
        }
    }
    file.close();

    if (!busFound) {
        cout << "No buses found for the given criteria.\n";
        return;
    }

    string selectedBusNo;
    cout << "\nEnter the Bus Number you want to book: ";
    cin >> selectedBusNo;

    // Find selected bus and get its date
    string selectedTripDate;
    for (auto& b : foundBuses) {
        if (get<0>(b) == selectedBusNo) {
            selectedTripDate = get<4>(b);
            break;
        }
    }

    if(selectedTripDate.empty()) {
        cout << "Invalid bus number selected.\n";
        return;
    }

    seatSelection(selectedBusNo, selectedTripDate);
}

void seatSelection(const string& busNo, const string& tripDate) {
    ifstream file("seats.csv");
    if (!file.is_open()) {
        cout << "Could not open seats.csv file!\n";
        return;
    }

    vector<pair<int, string>> seatStatus;
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string bus, seatStr, status;
        getline(ss, bus, ',');
        getline(ss, seatStr, ',');
        getline(ss, status, ',');

        if (bus == busNo) {
            seatStatus.push_back({stoi(seatStr), status});
        }
    }
    file.close();

    if (seatStatus.empty()) {
        cout << "No seats found for bus " << busNo << "\n";
        return;
    }

    cout << "Seats for Bus No " << busNo << ":\n";
    for (auto& seat : seatStatus) {
        cout << seat.first << " [" << seat.second << "]  ";
        if (seat.first % 5 == 0) cout << "\n";
    }
    cout << "\n";

    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    string inputSeats;
    vector<int> selectedSeats;

    while (true) {
        cout << "Enter seat numbers needed (comma separated) or press Enter to skip: ";
        getline(cin, inputSeats);

        if (inputSeats.empty()) {
            cout << "No seats selected. Moving to next step.\n";
            break;
        }

        stringstream ss(inputSeats);
        string seatNoStr;
        selectedSeats.clear();
        bool allAvailable = true;

        while (getline(ss, seatNoStr, ',')) {
            try {
                int seatNo = stoi(seatNoStr);
                selectedSeats.push_back(seatNo);
            } catch (...) {
                cout << "Invalid seat number: " << seatNoStr << "\n";
                allAvailable = false;
                break;
            }
        }

        if (!allAvailable) continue;

        bool anyUnavailable = false;
        for (int seat : selectedSeats) {
            auto it = find_if(seatStatus.begin(), seatStatus.end(),
                             [seat](const pair<int, string>& s) { return s.first == seat; });
            if (it == seatStatus.end()) {
                cout << "Seat number " << seat << " does not exist.\n";
                anyUnavailable = true;
            } else if (it->second == "booked") {
                cout << "Seat number " << seat << " is already booked.\n";
                anyUnavailable = true;
            }
        }

        if (anyUnavailable) {
            cout << "Please select seats again.\n";
            continue;
        }

        cout << "Selected seats: ";
        for (int seat : selectedSeats) cout << seat << " ";
        cout << "\n";
        break;
    }

    string boarding, dropping;
    cout << "Enter boarding point: ";
    getline(cin, boarding);
    cout << "Enter dropping point: ";
    getline(cin, dropping);

    cout << "Selected seats: ";
    for (int seat : selectedSeats) cout << seat << " ";
    cout << "\nBoarding point: " << boarding << "\nDropping point: " << dropping << "\n";

    passengerInfo(selectedSeats, boarding, dropping, busNo, tripDate);
}

void passengerInfo(const vector<int>& seats, const string& boarding, const string& dropping, const string& busNo, const string& tripDate) {
    int farePerSeat = 0;
    ifstream file("bus.csv");
    string line;
    getline(file, line); 

    while (getline(file, line)) {
        stringstream ss(line);
        string csvBusNo, bus_name, from, to, date, time, fareStr;

        getline(ss, csvBusNo, ',');
        getline(ss, bus_name, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');
        getline(ss, date, ',');
        getline(ss, time, ',');
        getline(ss, fareStr, ',');

        if (csvBusNo == busNo) {
            farePerSeat = stoi(fareStr);
            break;
        }
    }
    file.close();

    if (farePerSeat == 0) {
        cout << "Fare not found for this bus. Cannot proceed.\n";
        return;
    }

    int numPassengers;
    cout << "\nEnter number of passengers: ";
    cin >> numPassengers;

    if (numPassengers != seats.size()) {
        cout << "Number of passengers must match the number of selected seats (" << seats.size() << ").\n";
        return;
    }

    vector<string> names;
    vector<int> ages;
    vector<string> genders;

    cin.ignore();
    for (int i = 0; i < numPassengers; ++i) {
        string name, gender;
        int age;

        cout << "\nPassenger " << (i + 1) << ":\n";
        cout << "Name: ";
        getline(cin, name);
        cout << "Age: ";
        cin >> age;
        cout << "Gender: ";
        cin >> gender;
        cin.ignore();

        names.push_back(name);
        ages.push_back(age);
        genders.push_back(gender);
    }

    cout << "\nPassenger Details:\n";
    for (int i = 0; i < numPassengers; ++i) {
        cout << "Seat " << seats[i] << " - " << names[i] << ", Age: " << ages[i] << ", Gender: " << genders[i] << "\n";
    }

    cout << "Boarding: " << boarding << ", Dropping: " << dropping << "\n";

    int totalFare = farePerSeat * seats.size();
    cout << "\nTotal Fare: ₹" << totalFare << "\n";

    payment(busNo, seats, boarding, dropping, currentUserMobile, totalFare, names, ages, genders, tripDate);
}

bool upiPayment() {
    string upiId, pin;
    cout << "Enter UPI ID: ";
    cin >> upiId;
    cout << "Enter UPI PIN: ";
    cin >> pin;
    srand(time(0));
    bool verified = (rand() % 2 == 0);
    if (verified) {
        cout << "UPI Payment Verified Successfully.\n";
        return true;
    } else {
        cout << "UPI Payment Failed. Try again.\n";
        return false;
    }
}

bool cardPayment() {
    string cardNo;
    cout << "Enter Card Number: ";
    cin >> cardNo;
    srand(time(0));
    int otp = rand() % 9000 + 1000;
    cout << "OTP sent: " << otp << endl;
    int enteredOtp;
    cout << "Enter OTP: ";
    cin >> enteredOtp;
    if (enteredOtp == otp) {
        cout << "Card Payment Verified Successfully.\n";
        return true;
    } else {
        cout << "Incorrect OTP. Try again.\n";
        return false;
    }
}

bool netBankingPayment() {
    vector<string> banks = {"SBI", "HDFC", "ICICI", "Axis", "PNB"};
    cout << "Available Banks:\n";
    for (int i = 0; i < banks.size(); ++i) {
        cout << i+1 << ". " << banks[i] << endl;
    }
    int choice;
    cout << "Choose your bank (1-" << banks.size() << "): ";
    cin >> choice;
    if (choice < 1 || choice > banks.size()) {
        cout << "Invalid bank choice.\n";
        return false;
    }
    string username, password;
    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;
    srand(time(0));
    bool verified = (rand() % 2 == 0);
    if (verified) {
        cout << "Net Banking Verified Successfully.\n";
        return true;
    } else {
        cout << "Net Banking Authentication Failed. Try again.\n";
        return false;
    }
}

void payment(const string& busNo,
             const vector<int>& seats,
             const string& boarding,
             const string& dropping,
             const string& userMobile,
             int totalFare,
             const vector<string>& names,
             const vector<int>& ages,
             const vector<string>& genders,
             const string& tripDate)
{
    string input;
    double amount = static_cast<double>(totalFare);

    cout << "\nDo you have a coupon? (Y/N): ";
    cin >> input;

    if (input == "Y" || input == "y") {
        ifstream file("coupons.csv");
        if (!file.is_open()) {
            cout << "Coupon file not found.\n";
            return;
        }

        cout << "Available coupons:\n";
        string line;
        vector<string> coupons;
        while (getline(file, line)) {
            cout << line << endl;
            coupons.push_back(line);
        }
        file.close();

        cout << "Enter coupon code: ";
        string coupon;
        cin >> coupon;

        bool validCoupon = false;
        double discount = 0;

        for (const auto& c : coupons) {
            size_t commaPos = c.find(',');
            if (commaPos != string::npos) {
                string code = c.substr(0, commaPos);
                string discountStr = c.substr(commaPos + 1);
                if (code == coupon) {
                    discount = stod(discountStr);
                    validCoupon = true;
                    break;
                }
            }
        }

        if (!validCoupon) {
            cout << "Invalid coupon code.\n";
            return;
        }

        double discountedAmount = amount - discount;
        if (discountedAmount < 0) discountedAmount = 0;

        cout << "Discount applied. Amount to pay: ₹" << discountedAmount << endl;

        while (true) {
            cout << "Ready to pay? (Y/N): ";
            cin >> input;
            if (input == "Y" || input == "y") {
                modeOfPayment(busNo, seats, boarding, dropping, userMobile, discountedAmount, names, ages, genders, tripDate);
                return;
            } else if (input == "N" || input == "n") {
                searchBus();
                return;
            } else {
                cout << "Invalid input. Please enter Y or N.\n";
            }
        }
    } else if (input == "N" || input == "n") {
        cout << "Amount to pay: ₹" << amount << endl;
        while (true) {
            cout << "Ready to pay? (Y/N): ";
            cin >> input;
            if (input == "Y" || input == "y") {
                modeOfPayment(busNo, seats, boarding, dropping, userMobile, amount, names, ages, genders, tripDate);
                return;
            } else if (input == "N" || input == "n") {
                searchBus();
                return;
            } else {
                cout << "Invalid input. Please enter Y or N.\n";
            }
        }
    } else {
        cout << "Invalid input. Please enter Y or N.\n";
    }
}

void modeOfPayment(const string& busNo,
                   const vector<int>& seats,
                   const string& boarding,
                   const string& dropping,
                   const string& userMobile,
                   double amount,
                   const vector<string>& names,
                   const vector<int>& ages,
                   const vector<string>& genders,
                   const string& tripDate)
{
    while (true) {
        cout << "\nMode of Payment:\n1. UPI\n2. Credit/Debit Card\n3. Net Banking\nEnter your choice: ";
        int choice;
        cin >> choice;
        bool paymentSuccess = false;
        switch (choice) {
            case 1:
                paymentSuccess = upiPayment();
                break;
            case 2:
                paymentSuccess = cardPayment();
                break;
            case 3:
                paymentSuccess = netBankingPayment();
                break;
            default:
                cout << "Invalid choice. Try again.\n";
                continue;
        }
        if (paymentSuccess) {
            addBookingToCSV(busNo, seats, boarding, dropping, userMobile, amount, tripDate);
            cout << "Booking successful! Returning to dashboard...\n";
            break;
        } else {
            cout << "Payment failed. Returning to payment options...\n";
        }
    }
}

void bookings() {
    while(true) {
        cout << "1. Upcoming Trips\n2. Past Bookings\nEnter choice: ";
        int choice;
        cin >> choice;
        cin.ignore();
        if (choice == 1) {
            upcomingTrips();
        } else if (choice == 2) {
            pastTrips();
        } else {
            cout << "Invalid choice.\n";
            continue;
        }

        if (!askYesOrNo("Do you want to view bookings again?")) break;
    }
}


void pastTrips() {
    ifstream file("bookings.csv");
    if (!file.is_open()) {
        cout << "Unable to open bookings.csv\n";
        return;
    }

    string line;

    map<string, BookingDetail> bookingsMap;

    while (getline(file, line)) {
        stringstream ss(line);
        string ticketId, busNo, seatStr, boarding, dropping, userMobile, status, date;

        getline(ss, ticketId, ',');
        getline(ss, busNo, ',');
        getline(ss, seatStr, ',');
        getline(ss, boarding, ',');
        getline(ss, dropping, ',');
        getline(ss, userMobile, ',');
        getline(ss, status, ',');
        getline(ss, date, ',');

        if(userMobile == currentUserMobile && !isUpcoming(date)) { 
            int seat = stoi(seatStr);
            auto& booking = bookingsMap[ticketId];
            booking.busNo = busNo;
            booking.boarding = boarding;
            booking.dropping = dropping;
            booking.userMobile = userMobile;
            booking.status = "Completed";
            booking.date = date;
            booking.seats.insert(seat);
        }
    }
    file.close();

    if (bookingsMap.empty()) {
        cout << "No past trips found.\n";
        return;
    }

    cout << "\nPast Trips:\n";
    for (const auto& [ticketId, booking] : bookingsMap) {
        cout << "Ticket ID: " << ticketId << ", Bus: " << booking.busNo << ", Seats: ";
        bool first = true;
        for (int seat : booking.seats) {
            if (!first) cout << ", ";
            cout << seat;
            first = false;
        }
        cout << ", Boarding: " << booking.boarding << ", Dropping: " << booking.dropping
             << ", Date: " << booking.date << "\n";
    }
}

int main() {
    bool loggedIn = false;

    while (true) {
        if (!loggedIn) {
            cout << "---- RedBus System ----" << endl;
            cout << "1. Register" << endl;
            cout << "2. Login" << endl;
            cout << "3. Exit" << endl;
            cout << "Enter your choice: ";
            int choice;
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case 1:
                    loginOrRegister(true);
                    loggedIn = !currentUserMobile.empty();
                    break;
                case 2:
                    loginOrRegister(false);
                    loggedIn = !currentUserMobile.empty();
                    break;
                case 3:
                    cout << "Exiting...\n";
                    return 0;
                default:
                    cout << "Invalid choice. Try again.\n";
                    break;
            }
        } else {
            dashboard();
            loggedIn = false;
        }
    }
    return 0;
}

void dashboard() {
    while (true) {
        cout << "\n---- Dashboard ----" << endl;
        cout << "1. Search Bus" << endl;
        cout << "2. Bookings" << endl;
        cout << "3. Edit Profile" << endl;
        cout << "4. Logout" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                searchBus();
                break;
            case 2:
                bookings();
                break;
            case 3:
                editProfile();
                break;
            case 4:
                cout << "Logging out...\n";
                currentUserMobile = "";
                return;
            default:
                cout << "Invalid choice. Try again.\n";
                break;
        }
    }
}
