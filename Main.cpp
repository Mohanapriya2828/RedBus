#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cctype>

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

int main() {
    while (true) {
        cout << "1. Register" << endl;
        cout << "2. Login" << endl;
        cout << "Enter your choice: ";
        int choice;
        cin >> choice;
        cin.ignore();

        bool success = false;

        switch (choice) {
            case 1:
                success = loginOrRegister(true);
                break;
            case 2:
                success = loginOrRegister(false);
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }

        if (success) {
            dashboard();  
        }
    }
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

    cout << "Seat selection for Bus No " << selectedBusNo << " (Feature coming soon)\n";
}


void bookings() {
    cout << "Viewing bookings...\n";
}

void editProfile() {
    cout << "Editing profile..\n";
}






