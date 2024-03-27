#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <iomanip>
using namespace std;

const int FIELD_LENGTH = 25;

struct User
{
    uint32_t ID = 0;
    char name[FIELD_LENGTH] = {};
    char password[FIELD_LENGTH] = {};
    char phone[FIELD_LENGTH] = {};
    int64_t adress = -1;
    int64_t slave_adress = -1;
};

struct Order
{
    uint32_t UserID = 0;
    uint32_t OrderID = 0;
    char date[FIELD_LENGTH] = {};
    char status[FIELD_LENGTH] = {};

    int64_t next = -1;
};

struct Key
{
    int64_t UserID = 0;
    int64_t address = -1;
};

int64_t UserCounter = 0;
int64_t OrderCounter = 0;
int64_t LastUserID = 1;
int64_t LastOrderID = 1;
streampos RightMasterPos = 0;
streampos RightSlavePos = 0;
vector<int> rubbishMaster;
vector<int> rubbishSlave;
vector<Key> indexTable;

ostream& operator<<(ostream& os, const User& user) {
    os << "UserID: " << user.ID << ", "
        << "Name: " << user.name << ", "
        << "Password: " << user.password << ", "
        << "Phone: " << user.phone << ", ";
    return os;
}
ostream& operator<<(ostream& os, const Order& order)
{
    os << "+--------+---------+------------------+--------+\n";
    os << "| UserID | OrderID |       Date       | Status |\n";
    os << "+--------+---------+------------------+--------+\n";

    os << "| " << setw(6) << order.UserID << " | " << setw(7) << order.OrderID << " | " 
        << setw(16) << order.date << " | " << setw(5) << order.status << "  |\n";
    os << "+----------------------------------------------+\n";

    return os;

}

bool ReadUser(User& record, fstream& file, const streampos& pos)
{
    if (!file)
        return false;

    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&record), sizeof(User));

    return !file.fail();
}
bool ReadOrder(Order& record, fstream& file, const streampos& pos)
{
    if (!file)
        return false;

    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&record), sizeof(Order));

    return !file.fail();
}

void PrintNodesUser(fstream& file, const streampos& record_pos, bool isf)
{
    User tmp;
    streampos read_pos = record_pos;
    if (!ReadUser(tmp, file, read_pos))
    {
        cerr << "Unable to update next_ptr. Error: read failed" << "\n";
        return;
    }
    if (isf) cout << "+------+------------------+------------------+-------------------+---------+\n";
    if (isf) cout << "|  ID  |       Name       |     Password     |       Phone       | Address |\n";
    if (isf) cout << "+------+------------------+------------------+-------------------+---------+\n";
    cout << "| " << setw(4) << tmp.ID << " | " << setw(16) << tmp.name << " | " << setw(16) << tmp.password << " | " << setw(17) << tmp.phone << " | " << setw(8) << record_pos << "|\n";
                    cout << "+------+------------------+------------------+-------------------+---------+\n";
}
void PrintNodesOrder(fstream& file, const streampos& record_pos, bool isf)
{
    Order tmp;
    streampos read_pos = record_pos;
    if (isf) cout << "+--------+---------+------------------+--------+---------+--------------------+\n";
    if (isf) cout << "| UserID | OrderID |       Date       | Status | Address | Next Order Address |\n";
    if (isf) cout << "+--------+---------+------------------+--------+---------+--------------------+\n";
    while (read_pos != -1)
    {
        if (!ReadOrder(tmp, file, read_pos))
        {
            cerr << "Unable to update next_ptr. Error: read failed" << "\n";
            break;
        }
        cout << "| " << setw(6) << tmp.UserID << " | " << setw(7) << tmp.OrderID << " | " 
            << setw(16) << tmp.date << " | " << setw(6) << tmp.status<< " | " 
            << setw(7) << read_pos << " | " << setw(18) << tmp.next << " |\n";
        read_pos = tmp.next;
    }
                    cout << "+--------+---------+------------------+--------+---------+--------------------+\n";
}

vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool AddUserInf(User& user, string name, string password, string phone) {
    for (int i = 0; i < FIELD_LENGTH; ++i)
    {
        user.name[i] = 0;
        user.password[i] = 0;
    }
    if (name.size() > FIELD_LENGTH - 1 ||
        password.size() > FIELD_LENGTH - 1 ||
        phone.size() > FIELD_LENGTH - 1)
    {
        cout << "Error. One of the fields is too long." << "\n";
        return false;
    }

    for (int i = 0; i < name.size(); ++i) user.name[i] = name[i];
    for (int i = 0; i < password.size(); ++i) user.password[i] = password[i];
    for (int i = 0; i < phone.size(); ++i) user.phone[i] = phone[i];
    return true;
}
bool AddOrderInf(Order& order, string date, string status)
{
    for (int i = 0; i < FIELD_LENGTH; ++i)
    {
        order.date[i] = 0;
        order.status[i] = 0;
    }
    if (date.size() > FIELD_LENGTH - 1)
    {
        cout << "Error: the entered text is too long. Don't write more than 15 symbols" << "\n";
        return false;
    }
    if (status.size() > FIELD_LENGTH - 1)
    {
        cout << "Error: the entered text is too long. Don't write more than 15 symbols" << "\n";
        return false;
    }
    for (int i = 0; i < date.size(); ++i) order.date[i] = date[i];
    for (int i = 0; i < status.size(); ++i) order.status[i] = status[i];
    return true;
}

bool WriteUser(const User& record, fstream& file, const streampos& pos)
{
    if (!file)
        return false;

    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&record), sizeof(User));
    file.flush();

    return !file.fail();
}
bool WriteOrder(const Order& record, fstream& file, const streampos& pos)
{
    if (!file)
        return false;

    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&record), sizeof(Order));
    file.flush();

    return !file.fail();
}

int GetSlaveAdress(int master_index, int ID, fstream& masterfile, fstream& slavefile, streampos& prev_pos)
{
    streampos user_address = indexTable[master_index].address;
    User user;
    ReadUser(user, masterfile, user_address);
    if (user.slave_adress == -1) return -1;
    Order temp;
    streampos cur_pos = user.slave_adress;
    prev_pos = -1;
    while (cur_pos != -1)
    {
        if (!ReadOrder(temp, slavefile, cur_pos)) return -2;
        if (temp.OrderID == ID) return cur_pos;
        prev_pos = cur_pos;
        cur_pos = temp.next;
    }
    return -3;
}
int GetUserIndex(int UserID)
{
    for (int i = 0; i < indexTable.size(); ++i)
    {
        if (indexTable[i].UserID == UserID) return i;
    }
    return -1;
}
int GetMasterIndex(int ID, fstream& masterfile, fstream& slavefile, streampos& prev_pos)
{
    int adress;
    for (int i = 0; i < indexTable.size(); ++i)
    {
        adress = GetSlaveAdress(i, ID, masterfile, slavefile, prev_pos);
        if (adress >= 0) return i;
    }
    return -1;
}

void get_m(int ID, fstream& masterfile) {
    int userIndex = GetUserIndex(ID);
    if (userIndex == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }
    PrintNodesUser(masterfile, indexTable[userIndex].address, true);
}
void get_s(int ID, fstream& masterfile, fstream& slavefile) {
    streampos prevOrderPos = -1;
    int orderIndex = GetMasterIndex(ID, masterfile, slavefile, prevOrderPos);
    if (orderIndex == -1)
    {
        cout << "Error: the order with given ID doesn't exist" << "\n";
        return;
    }
    int64_t slave_adress = GetSlaveAdress(orderIndex, ID, masterfile, slavefile, prevOrderPos);
    Order tempOrder;
    if (slave_adress == -2)
    {
        cerr << "Unable to update next_ptr. Error: read failed" << "\n";
        return;
    }
    ReadOrder(tempOrder, slavefile, slave_adress);
    cout << tempOrder;
}

void del_m(int ID, fstream& masterfile, fstream& slavefile) {
    int userIndex = GetUserIndex(ID);
    if (userIndex == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }

    streampos user_address = indexTable[userIndex].address;
    rubbishMaster.push_back(user_address);
    
    User user;
    ReadUser(user, masterfile, user_address);

    indexTable.erase(indexTable.begin() + userIndex);
    
    --UserCounter;
    
    if (user.slave_adress == -1) return;
    
    streampos curPos = user.slave_adress;
    Order tempOrder;
    while (curPos != -1)
    {
        if (!ReadOrder(tempOrder, slavefile, curPos))
        {
            cerr << "Unable to update next_ptr. Error: read failed" << "\n";
            break;
        }
        rubbishSlave.push_back(curPos);
        --OrderCounter;
        curPos = tempOrder.next;
    }
}
void del_s(int ID, fstream& masterfile, fstream& slavefile) { 
    streampos prevOrderPos = -1;
    int orderIndex = GetMasterIndex(ID, masterfile, slavefile, prevOrderPos);
    if (orderIndex == -1)
    {
        cout << "Error: the order with given ID doesn't exist" << "\n";
        return;
    }
    streampos user_address = indexTable[orderIndex].address;
    streampos slave_adress = GetSlaveAdress(orderIndex, ID, masterfile, slavefile, prevOrderPos);
    if (slave_adress == -2)
    {
        cerr << "Unable to update next_ptr. Error: read failed" << "\n";
        return;
    }
    Order tempOrder;
    ReadOrder(tempOrder, slavefile, slave_adress);

    rubbishSlave.push_back(slave_adress);
    User user;
    if (prevOrderPos == -1)
    {
        ReadUser(user, masterfile, user_address);
        user.slave_adress = tempOrder.next;
        WriteUser(user, masterfile, user_address);
    }
    else
    {
        slave_adress = tempOrder.next;
        ReadOrder(tempOrder, slavefile, prevOrderPos);
        tempOrder.next = slave_adress;
        WriteOrder(tempOrder, slavefile, prevOrderPos);
    }
    --OrderCounter;
}

void update_m(int ID, string field, string value, fstream& masterfile) {
    int userIndex = GetUserIndex(ID);
    if (userIndex == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }

    streampos user_address = indexTable[userIndex].address;
    User user;
    ReadUser(user, masterfile, user_address);
    if (field != "name" && field != "password" && field != "phone")
    {
        cout << "Error: the User doesn't have a field named " << field << "\n";
        return;
    }

    bool filled;

    if (field == "name") filled = AddUserInf(user, value, user.password, user.phone);
    if (field == "password") filled = AddUserInf(user, user.name, value, user.phone);
    if (field == "phone") filled = AddUserInf(user, user.name, user.password, value);
    
    if (!filled) return;

    WriteUser(user, masterfile, user_address);
}
void update_s(int ID, string field, string val, fstream& masterfile, fstream& slavefile) {
    streampos prevOrderPos = -1;
    int orderIndex = GetMasterIndex(ID, masterfile, slavefile, prevOrderPos);

    if (orderIndex == -1)
    {
        cout << "Error: order with given ID doesn't exist" << "\n";
        return;
    }

    streampos slave_adress = GetSlaveAdress(orderIndex, ID, masterfile, slavefile, prevOrderPos);
    if (slave_adress == -2)
    {
        cerr << "Unable to update next_ptr. Error: read failed" << "\n";
        return;
    }

    Order tempOrder;
    ReadOrder(tempOrder, slavefile, slave_adress);
    if (field != "date" && field != "status")
    {
        cout << "Error: the order doesn't have a field named " << field << "\n";
        return;
    }

    bool filled;
    if (field == "date") filled = AddOrderInf(tempOrder, val, tempOrder.status);
    if (field == "status") filled = AddOrderInf(tempOrder, tempOrder.date, val);

    if (!filled) return;

    WriteOrder(tempOrder, slavefile, slave_adress);
}

void insert_m(string name, string password, string phone, fstream& masterfile) {
    User user;
    user.ID = LastUserID;

    bool filled = AddUserInf(user, name, password, phone);
    bool rightPosUsed = false;

    streampos insertPos;

    if (rubbishMaster.empty())
    {
        insertPos = RightMasterPos;
        rightPosUsed = true;
    }
    else
    {
        insertPos = rubbishMaster[rubbishMaster.size() - 1];
        rubbishMaster.pop_back();
    }

    if (!filled) return;

    WriteUser(user, masterfile, insertPos);

    Key k{ LastUserID,insertPos };
    indexTable.push_back(k);
    if (rightPosUsed) RightMasterPos = RightMasterPos + static_cast<streamoff>(sizeof(User));

    ++LastUserID;
    ++UserCounter;
}
void insert_s(int ID, string date, string status, fstream& masterfile, fstream& slavefile) {
    int orderIndex = GetUserIndex(ID);
    if (orderIndex == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }

    streampos user_address = indexTable[orderIndex].address;
    User user;
    ReadUser(user, masterfile, user_address);

    bool rightPosUsed = false;
    streampos prevOrderPos = -1;
    streampos insertPos, curPos;
    Order tempOrder;
    if (rubbishSlave.empty())
    {
        insertPos = RightSlavePos;
        rightPosUsed = true;
    }
    else
    {
        insertPos = rubbishSlave[0];
        rubbishSlave.erase(rubbishSlave.begin());
    }
    if (user.slave_adress == -1)
    {
        user.slave_adress = insertPos;
        WriteUser(user, masterfile, user_address);
    }
    else
    {
        curPos = user.slave_adress;
        while (curPos != -1)
        {
            if (!ReadOrder(tempOrder, slavefile, curPos))
            {
                cerr << "Unable to update next_ptr. Error: read failed" << "\n";
                break;
            }
            prevOrderPos = curPos;
            curPos = tempOrder.next;
        }
    }

    tempOrder.OrderID = LastOrderID;
    tempOrder.UserID = ID;
    tempOrder.next = -1;
    
    bool filled = AddOrderInf(tempOrder, date, status);
    
    if (!filled) return;
    
    WriteOrder(tempOrder, slavefile, insertPos);
    if (prevOrderPos != -1)
    {
        ReadOrder(tempOrder, slavefile, prevOrderPos);
        tempOrder.next = insertPos;
        WriteOrder(tempOrder, slavefile, prevOrderPos);
    }

    if (rightPosUsed) RightSlavePos = RightSlavePos + static_cast<streamoff>(sizeof(Order));
    ++LastOrderID;
    ++OrderCounter;
}

void calc_m() { 
    cout << "Number of users: " << UserCounter << "\n";
}
void calc_s(int ID, fstream& masterfile, fstream& slavefile) {
    int master_index = GetUserIndex(ID);
    if (master_index == -1) return;

    int OrderCounter = 0;
    streampos user_address = indexTable[master_index].address;

    User user;
    ReadUser(user, masterfile, user_address);

    if (user.slave_adress == -1) return;
    streampos curPos = user.slave_adress;

    Order tempOrder;
    while (curPos != -1)
    {
        if (!ReadOrder(tempOrder, slavefile, curPos))
        {
            cerr << "Unable to update next_ptr. Error: read failed" << "\n";
            break;
        }
        curPos = tempOrder.next;
        ++OrderCounter;
    }

    cout << "Number of orders: " << OrderCounter << "\n";
}

void ut_m(fstream& masterfile) 
{ 
    PrintNodesUser(masterfile, indexTable[0].address, true);
    for (int i = 1; i < indexTable.size(); ++i) {
        PrintNodesUser(masterfile, indexTable[i].address, false);
    }
}
void ut_s(fstream& masterfile, fstream& slavefile) {
    streampos user_address, curPos;
    User user;
    bool isf = true;
    for (int i = 0; i < indexTable.size(); ++i)
    {
        user_address = indexTable[i].address;
        ReadUser(user, masterfile, user_address);
        if (user.slave_adress != -1)
        {
            curPos = user.slave_adress;
            PrintNodesOrder(slavefile, curPos, isf);
            isf = false;
        }
    }
}

void help() {
    cout << "\nget-m [UserID]: shows information about a user with a given ID" << endl;
    cout << "get-s [OrderID]: shows information about an order with a given ID" << endl;

    cout << "del-m [UserID]: deletes information about a user with a given ID" << endl;
    cout << "del-s [OrderID]: deletes information about an order with a given ID" << endl;

    cout << "update-m [UserID] [field] [value]: changes the data of the selected field for a user with a given ID" << endl;
    cout << "update-s [OrderID] [field] [value]: changes the data of the selected field for an order with a given ID" << endl;

    cout << "insert-m [name] [password] [phone]: adds a new user" << endl;
    cout << "insert-s [UserID] [date] [status]: adds an order to the user with a given ID" << endl;

    cout << "calc-m: shows the number of users" << endl;
    cout << "calc-s [UserID]: shows the number of orders of the user with a given ID" << endl;

    cout << "ut-m: shows information about all users" << endl;
    cout << "ut-s: shows information about all orders" << endl;

    cout << "help: shows all commands" << endl;
    cout << "exit: turns off the program" << endl << endl;
}

int main()
{
    const string slavename = "db/Orders.fl";
    const string mastername = "db/Users.fl";
    const string tablename = "db/Users.ind";
    fstream slavefile(slavename, ios::binary | ios::in | ios::out);
    fstream masterfile(mastername, ios::binary | ios::in | ios::out);
    fstream indextable(tablename, ios::binary | ios::in | ios::out);

    auto err = errno;
    if (err == ENOENT)
    {
        slavefile = fstream(slavename, ios::binary | ios::in | ios::out | ios::trunc);
        masterfile = fstream(mastername, ios::binary | ios::in | ios::out | ios::trunc);
        indextable = fstream(tablename, ios::binary | ios::in | ios::out | ios::trunc);
    }
    if (!slavefile)
    {
        cerr << "Unable to open file=" << slavename << "\n";
        return -1;
    }
    if (!masterfile)
    {
        cerr << "Unable to open file=" << mastername << "\n";
        return -1;
    }
    if (!indextable)
    {
        cerr << "Unable to open file=" << tablename << "\n";
        return -1;
    }

    insert_m("Vlad", "123", "+380000000000", masterfile);
    insert_m("Oleg", "qwerty", "+380230943444", masterfile);
    insert_m("Max", "aboba", "+380507647474", masterfile);
    insert_m("Vasya", "optimus", "+381234567890", masterfile);
    insert_m("Olga", "pass word", "+387878787878", masterfile);

    insert_s(1, "54.13.4048", "1", masterfile, slavefile);
    insert_s(1, "12.12.1000", "3", masterfile, slavefile);
    insert_s(1, "01.00.-300", "2", masterfile, slavefile);

    insert_s(3, "05.05.2012", "1", masterfile, slavefile);
    insert_s(3, "31.02.2023", "3", masterfile, slavefile);
    insert_s(3, "10.10.2010", "2", masterfile, slavefile);
    insert_s(3, "12.12.2012", "2", masterfile, slavefile);
    insert_s(3, "03.01.2024", "3", masterfile, slavefile);

    insert_s(4, "16.11.2024", "1", masterfile, slavefile);
    insert_s(4, "19.05.2022", "3", masterfile, slavefile);
    insert_s(4, "05.05.2016", "2", masterfile, slavefile);
    insert_s(4, "03.03.2024", "1", masterfile, slavefile);


    cout << "Type 'help' to see all commands." << endl;
    cout << "> ";
    string input;
    while (getline(cin, input)) {
        vector<string> tokens = split(input, ' ');

        if (tokens.empty()) {
            cout << "> ";
            continue;
        }

        string command = tokens[0];

        if (command == "help") {
            help();
        }
        else if (command == "exit") {
            break;
        }
        else if (command == "get-m") {
            if (tokens.size() != 2) {
                cout << "Usage: get-m [UserID]" << endl;
            }
            else {
                int userID = stoi(tokens[1]);
                get_m(userID, masterfile);
            }
        }
        else if (command == "get-s") {
            if (tokens.size() != 2) {
                cout << "Usage: get-s [OrderID]" << endl;
            }
            else {
                int orderID = stoi(tokens[1]);
                get_s(orderID, masterfile, slavefile);
            }
        }
        else if (command == "del-m") {
            if (tokens.size() != 2) {
                cout << "Usage: del-m [UserID]" << endl;
            }
            else {
                int userID = stoi(tokens[1]);
                del_m(userID, masterfile, slavefile);
            }
        }
        else if (command == "del-s") {
            if (tokens.size() != 2) {
                cout << "Usage: del-s [OrderID]" << endl;
            }
            else {
                int orderID = stoi(tokens[1]);
                del_s(orderID, masterfile, slavefile);
            }
        }
        else if (command == "update-m") {
            if (tokens.size() != 4) {
                cout << "Usage: update-m [UserID] [field] [value]" << endl;
            }
            else {
                int userID = stoi(tokens[1]);
                string field = tokens[2];
                string value = tokens[3];

                update_m(userID, field, value, masterfile);
            }
        }
        else if (command == "update-s") {
            if (tokens.size() != 4) {
                cout << "Usage: update-s [OrderID] [field] [value]" << endl;
            }
            else {
                int orderID = stoi(tokens[1]);
                string field = tokens[2];
                string value = tokens[3];

                update_s(orderID, field, value, masterfile, slavefile);
            }
        }
        else if (command == "insert-m") {
            if (tokens.size() != 4) {
                cout << "Usage: insert-m [name] [password] [phone]" << endl;
            }
            else {
                string name = tokens[1];
                string password = tokens[2];
                string phone = tokens[3];

                insert_m(name, password, phone, masterfile);
            }
        }
        else if (command == "insert-s") {
            if (tokens.size() != 4) {
                cout << "Usage: insert-s [UserID] [date] [status]" << endl;
            }
            else {
                int userID = stoi(tokens[1]);
                string date = tokens[2];
                string status = tokens[3];

                insert_s(userID, date, status, masterfile, slavefile);
            }
        }
        else if (command == "calc-m") {
            calc_m();
        }
        else if (command == "calc-s") {
            if (tokens.size() != 2) {
                cout << "Usage: calc-s [UserID]" << endl;
            }
            else {
                int userID = stoi(tokens[1]);
                calc_s(userID, masterfile, slavefile);
            }
        }
        else if (command == "ut-m") {
            if (tokens.size() != 1) {
                cout << "Usage: ut-m" << endl;
            }
            else {
                ut_m(masterfile);
            }
        }
        else if (command == "ut-s") {
            ut_s(masterfile, slavefile);
        }
        else {
            cout << "Unknown command. Type 'help' to see all commands." << endl;
        }

        cout << "> ";
    }

    return 0;
}