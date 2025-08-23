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
#include <random>


using namespace std;

string currentUserMobile;

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
void seatSelection(const string& busNo);
void passengerInfo(const vector<int>& seats, const string& boarding, const string& dropping, const string& busNo);
void modeOfPayment(const string& busNo, const vector<int>& seats, const string& boarding, const string& dropping, const string& userMobile, double amount);
void searchBus();
void bookings();
void upcomingTrips();
void pastTrips();
void cancellation(const string& busNo, const string& ticketId, const string& userMobile);
void freeSeatsAfterCancellation(const string& busNo, const vector<int>& seats);

void addBookingToCSV(const std::string& busNo, const std::vector<int>& seats,
                     const std::string& boarding, const std::string& dropping,
                     const std::string& userMobile, double amount,
                     const std::string& tripDate = "");


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
            cout << "Logging out...\n";
            currentUserMobile = "";
            loggedIn = false;
        }
    }
}

void payment(const string& busNo, const vector<int>& seats, const string& boarding, const string& dropping, const string& userMobile, double amount) {
    while (true) {
        cout << "Have a coupon? (Y/N): ";
        string input;
        cin >> input;
        if (input == "Y" || input == "y") {
            ifstream file("coupons.csv");
            if (!file.is_open()) {
                cout << "Coupon file not found.\n";
                continue;
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
                continue;
            }

            double discountedAmount = amount - discount;
            if (discountedAmount < 0) discountedAmount = 0;
            cout << "Discount applied. Amount to pay: " << discountedAmount << endl;

            while (true) {
                cout << "Ready to pay? (Y/N): ";
                cin >> input;
                if (input == "Y" || input == "y") {
                    modeOfPayment(busNo, seats, boarding, dropping, userMobile, discountedAmount);
                    return;
                } else if (input == "N" || input == "n") {
                    searchBus();
                    return;
                } else {
                    cout << "Invalid input. Please enter Y or N.\n";
                }
            }
        } else if (input == "N" || input == "n") {
            cout << "Amount to pay: " << amount << endl;
            while (true) {
                cout << "Ready to pay? (Y/N): ";
                cin >> input;
                if (input == "Y" || input == "y") {
                    modeOfPayment(busNo, seats, boarding, dropping, userMobile, amount);
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
}


void seatSelection(const string& busNo) {
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

    passengerInfo(selectedSeats, boarding, dropping, busNo);

}

 

void passengerInfo(const vector<int>& seats, const string& boarding, const string& dropping, const string& busNo) {
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

    for (int i = 0; i < numPassengers; ++i) {
        string name, gender;
        int age;

        cout << "\nPassenger " << (i + 1) << ":\n";
        cout << "Name: ";
        cin.ignore();
        getline(cin, name);
        cout << "Age: ";
        cin >> age;
        cout << "Gender: ";
        cin >> gender;

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

    payment(busNo, seats, boarding, dropping, currentUserMobile, totalFare);


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


void dashboard() {
    while (true) {
        cout << "\n---- Dashboard ----" << endl;
        cout << "1. Search Bus" << endl;
        cout << "2. Bookings" << endl;
        cout << "3. Edit Profile" << endl;
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
            default:
                cout << "Invalid choice. Try again.\n";
                continue;
        }

        if (!askYesOrNo("Do you want to continue to Dashboard?")) {
            break;
        }
    }
}


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
    cout << "BusNo | Name          | From     | To       | Date       | Time  | Fare\n";
    cout << "-------------------------------------------------------------\n";

    getline(file, line);

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
    seatSelection(selectedBusNo);


    
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

void modeOfPayment(const string& busNo, const vector<int>& seats, const string& boarding, const string& dropping, const string& userMobile, double amount) {
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
            addBookingToCSV(busNo, seats, boarding, dropping, userMobile, amount);
            cout << "Booking successful! Returning to dashboard...\n";
            break;
        } else {
            cout << "Payment failed. Returning to payment options...\n";
        }
    }
}

#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>

std::tm parseDate(const std::string& dateStr) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    return tm;
}

bool isUpcoming(const std::string& tripDate) {
    std::tm tripTm = parseDate(tripDate);
    std::time_t trip_time = std::mktime(&tripTm);
    std::time_t now = std::time(nullptr);
    return difftime(trip_time, now) > 0;
}

void addBookingToCSV(const std::string& busNo, const std::vector<int>& seats,
                     const std::string& boarding, const std::string& dropping,
                     const std::string& userMobile, double amount,
                     const std::string& tripDate)
{
    std::ofstream file("bookings.csv", std::ios::app);
    std::string status = isUpcoming(tripDate) ? "upcoming" : "past";

    for (int seat : seats) {
        file << userMobile << "," << busNo << "," << seat << "," << boarding << ","
             << dropping << "," << amount / seats.size() << "," << status << "\n";
    }
    file.close();
}






void editProfile() {
    cout << "Editing profile..\n";
}



void bookings() {
    while (true) {
        cout << "1. Upcoming Trips\n2. Past Bookings\nEnter choice: ";
        int choice; cin >> choice;
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

void upcomingTrips() {
    ifstream file("bookings.csv");
    if (!file.is_open()) {
        cout << "Unable to open bookings.csv\n";
        return;
    }

    vector<string> userBookings;
    string line;
    string header;
    getline(file, header);

    while (getline(file, line)) {
        stringstream ss(line);
        string ticketId, busNo, seatsStr, boarding, dropping, userMobile, status, date, rest;

        getline(ss, ticketId, ',');
        getline(ss, busNo, ',');
        getline(ss, seatsStr, ',');
        getline(ss, boarding, ',');
        getline(ss, dropping, ',');
        getline(ss, userMobile, ',');
        getline(ss, status, ',');
        getline(ss, date, ',');

        if (userMobile == currentUserMobile && status == "Upcoming") {
            userBookings.push_back(line);
        }
    }
    file.close();

    if (userBookings.empty()) {
        cout << "No upcoming trips found.\n";
        return;
    }

    cout << "\nUpcoming Trips:\n";
    for (const auto& booking : userBookings) {
        stringstream ss(booking);
        string ticketId, busNo, seatsStr, boarding, dropping, userMobile, status, date;

        getline(ss, ticketId, ',');
        getline(ss, busNo, ',');
        getline(ss, seatsStr, ',');
        getline(ss, boarding, ',');
        getline(ss, dropping, ',');
        getline(ss, userMobile, ',');
        getline(ss, status, ',');
        getline(ss, date, ',');

        cout << "Ticket ID: " << ticketId << ", Bus: " << busNo << ", Seats: " << seatsStr
             << ", Boarding: " << boarding << ", Dropping: " << dropping << ", Date: " << date << "\n";
    }

    cout << "\nEnter Ticket ID to view details or 0 to return: ";
    string ticketChoice; 
    getline(cin, ticketChoice);
    if (ticketChoice == "0") return;

    auto it = find_if(userBookings.begin(), userBookings.end(), [&](const string& bk){
        stringstream ss(bk);
        string tid;
        getline(ss, tid, ',');
        return tid == ticketChoice;
    });

    if (it == userBookings.end()) {
        cout << "Invalid Ticket ID.\n";
        return;
    }

    stringstream ss(*it);
    string ticketId, busNo, seatsStr, boarding, dropping, userMobile, status, date;

    getline(ss, ticketId, ',');
    getline(ss, busNo, ',');
    getline(ss, seatsStr, ',');
    getline(ss, boarding, ',');
    getline(ss, dropping, ',');
    getline(ss, userMobile, ',');
    getline(ss, status, ',');
    getline(ss, date, ',');

    cout << "\nTicket Details:\nTicket ID: " << ticketId << "\nBus No: " << busNo << "\nSeats: " << seatsStr
         << "\nBoarding: " << boarding << "\nDropping: " << dropping << "\nDate: " << date << "\nStatus: " << status << "\n";

    if (askYesOrNo("Do you want to cancel this ticket?")) {
        cancellation(busNo, ticketId, userMobile);
    }
}

void cancellation(const string& busNo, const string& ticketId, const string& userMobile) {
    ifstream file("bookings.csv");
    if (!file.is_open()) {
        cout << "Unable to open bookings.csv\n";
        return;
    }

    vector<string> allBookings;
    string line;
    string header;
    getline(file, header);

    vector<int> seatsToFree;
    string cancelledLine;
    bool found = false;

    while (getline(file, line)) {
        stringstream ss(line);
        string tid, bNo, seatsStr, boarding, dropping, uMobile, status, date;

        getline(ss, tid, ',');
        getline(ss, bNo, ',');
        getline(ss, seatsStr, ',');
        getline(ss, boarding, ',');
        getline(ss, dropping, ',');
        getline(ss, uMobile, ',');
        getline(ss, status, ',');
        getline(ss, date, ',');

        if (tid == ticketId && uMobile == userMobile && bNo == busNo) {
            found = true;
            cancelledLine = line;

            stringstream seatsSS(seatsStr);
            string seat;
            while (getline(seatsSS, seat, '|')) {
                seatsToFree.push_back(stoi(seat));
            }
            continue;
        }
        allBookings.push_back(line);
    }
    file.close();

    if (!found) {
        cout << "Ticket not found for cancellation.\n";
        return;
    }

    ofstream outFile("bookings.csv");
    outFile << header << "\n";
    for (const string& bk : allBookings) outFile << bk << "\n";
    outFile.close();

    freeSeatsAfterCancellation(busNo, seatsToFree);

    cout << "Ticket cancelled successfully. Amount will be refunded in 2 days.\n";
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
            vector<pair<int, string>> seatStatus;
            getline(ss, bNo, ',');
            string seatsData = line.substr(busNo.size() + 1);

            vector<string> parts;
            string tmp;
            stringstream sstr(seatsData);
            while (getline(sstr, tmp, ',')) parts.push_back(tmp);

            for (string& part : parts) {
                size_t colonPos = part.find(':');
                int seatNum = stoi(part.substr(0, colonPos));
                string stat = part.substr(colonPos + 1);
                for (int s : seats) {
                    if (seatNum == s && stat == "occupied") stat = "available";
                }
                part = to_string(seatNum) + ":" + stat;
            }

            string newLine = busNo + "," + parts[0];
            for (int i = 1; i < (int)parts.size(); i++) newLine += "," + parts[i];
            lines.push_back(newLine);
        } else {
            lines.push_back(line);
        }
    }
    file.close();

    ofstream outFile("seats.csv");
    for (const string& ln : lines) outFile << ln << "\n";
    outFile.close();
}

void pastTrips() {
    ifstream file("bookings.csv");
    if (!file.is_open()) {
        cout << "Unable to open bookings.csv\n";
        return;
    }

    string line;
    string header;
    getline(file, header);

    vector<string> pastBookings;
    while (getline(file, line)) {
        stringstream ss(line);
        string ticketId, busNo, seatsStr, boarding, dropping, userMobile, status, date;

        getline(ss, ticketId, ',');
        getline(ss, busNo, ',');
        getline(ss, seatsStr, ',');
        getline(ss, boarding, ',');
        getline(ss, dropping, ',');
        getline(ss, userMobile, ',');
        getline(ss, status, ',');
        getline(ss, date, ',');

        if (userMobile == currentUserMobile && status == "Completed") {
            pastBookings.push_back(line);
        }
    }
    file.close();

    if (pastBookings.empty()) {
        cout << "No past trips found.\n";
        return;
    }

    cout << "\nPast Trips:\n";
    for (const auto& booking : pastBookings) {
        stringstream ss(booking);
        string ticketId, busNo, seatsStr, boarding, dropping, userMobile, status, date;

        getline(ss, ticketId, ',');
        getline(ss, busNo, ',');
        getline(ss, seatsStr, ',');
        getline(ss, boarding, ',');
        getline(ss, dropping, ',');
        getline(ss, userMobile, ',');
        getline(ss, status, ',');
        getline(ss, date, ',');

        cout << "Ticket ID: " << ticketId << ", Bus: " << busNo << ", Seats: " << seatsStr
             << ", Boarding: " << boarding << ", Dropping: " << dropping << ", Date: " << date << "\n";
    }
}

