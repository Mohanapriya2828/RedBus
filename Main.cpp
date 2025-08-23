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
void modeOfPayment();
void searchBus();


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

void payment(int fare) {
    bool applyCoupon = askYesOrNo("Do you have a coupon?");
    int finalFare = fare;

    if (applyCoupon) {
        map<string, int> coupons;
        ifstream file("coupons.csv");

        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string code, discountStr;
                getline(ss, code, ',');
                getline(ss, discountStr, ',');
                int discount = stoi(discountStr);
                coupons[code] = discount;
                cout << "Code: " << code << " - Discount: ₹" << discount << endl;
            }
            file.close();
        } else {
            cout << "Could not open coupons.csv file!\n";
        }

        while (true) {
            string enteredCode;
            cout << "Enter coupon code: ";
            cin >> enteredCode;

            if (coupons.find(enteredCode) != coupons.end()) {
                int discount = coupons[enteredCode];
                finalFare -= discount;
                if (finalFare < 0) finalFare = 0;
                cout << "Coupon applied! ₹" << discount << " discount.\n";
                break;
            } else {
                cout << "Invalid coupon code.\n";
                if (!askYesOrNo("Do you want to try another coupon?")) {
                    break;
                }
            }
        }
    }

    cout << "Final Amount to Pay: ₹" << finalFare << endl;

    if (askYesOrNo("Ready to pay?")) {
        modeOfPayment();
    } else {
        cout << "Returning to bus search...\n";
        searchBus();
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

    payment(totalFare);
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


void bookings() {
    cout << "Viewing bookings...\n";
}

void editProfile() {
    cout << "Editing profile..\n";
}

void modeOfPayment() {
    cout << "Processing payment...\n";
}



