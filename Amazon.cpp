#include <iostream>
#include <cstdio>
#include <unistd.h>

using namespace std;

#include "Amazon.h"
#include "include/Mediator.h"
#include "include/Worker.h"

Amazon::Amazon(string branch_name) : branch_name(branch_name) 
{
	this->customer_db = new DBFile(10, 4);
    vector<string> customer_col_names = {
        "customer_id", 
        "name", 
        "phone_no", 
        "address"
    };
    this->customer_db_schema = new DBSchema(customer_db, customer_col_names);
    
    this->payment_db = new DBFile(10, 5);
    vector<string> payment_col_names = {
        "payment_id", 
        "customer_id", 
        "amount", 
        "mode_of_payment", 
        "payment_timestamp"
    };
    this->payment_db_schema = new DBSchema(payment_db, payment_col_names);

    this->shipping_db = new DBFile(10, 5);
    vector<string> shipping_col_names = {
        "shipping_id", 
        "payment_id", 
        "to_location", 
        "is_delivered", 
        "shipment_timestamp"
    };
    this->shipping_db_schema = new DBSchema(shipping_db, shipping_col_names);
    
    IMessageQueue *mq = new MessageQueue("Message system");
    Worker *w1 = new Worker(this->customer_db);
    Worker *w2 = new Worker(this->payment_db);
    Worker *w3 = new Worker(this->shipping_db);
    this->coord = new Coordinator(3, {w1, w2, w3}, mq);    
    
    this->coord->join(mq);
    w1->join(mq);
    w2->join(mq);
    w3->join(mq);
    
    userCount = 0;
    paymentCount = 0;
    shippingCount = 0;
}

Amazon::~Amazon() 
{
	free(customer_db);
	free(customer_db_schema);
		
	free(payment_db);
	free(payment_db_schema);
	
	free(shipping_db);
	free(shipping_db_schema);
	
	free(coord);
}

int Amazon::registerUser()
{
	char id[20];
	++userCount; 
  	sprintf(id, "c%d", userCount);  
  	string uid(id);	
  	string name, ph, addr;
  	
  	cout << "Enter user's name: ";
  	cin >> name;
  	
  	cout << "Enter user's phone number: ";
  	cin >> ph;
  	
  	cout << "Enter user's address: ";
  	cin >> addr;
  	
	vector <string> new_record = {uid, name, ph, addr};
    Log_t op = {1, 2, -1, -1, new_record};
    
    vector <Log_t *> opList = {&op, NULL, NULL};
    int result = coord->performTransaction(opList);
    
    if(result == 1)
    {
    	cout << "\nUser successfuly registered, user ID: " << id << "\n";  	
    	customer_db_schema->updateIdRowNum(op.row, uid, 0);
    	
    	return 1;  	
    }
    
    else cout << "\nRegistration failed, please try again\n";
    --userCount;
    return 0;
}	

vector <string> Amazon::getUserDetails(string id)
{	
    vector<string> record_values;
    pair<bool, int> success_row_num = customer_db_schema->getRowNumRecord(id);
    if (success_row_num.first && dynamic_cast<DBFile*>(customer_db)->acquire_lock(2,0) == true)
    {
        record_values = customer_db->readRecord(success_row_num.second);
        dynamic_cast<DBFile*>(customer_db)->release_lock();     
    }
    else
    {
        cout << "\nFailed to read record, invalid customer ID";
    }
    
    return record_values;
}	

vector <string> Amazon::getTransactionDetails(string id)
{
    vector<string> record_values;
    pair<bool, int> success_row_num = payment_db_schema->getRowNumRecord(id);
    if (success_row_num.first && dynamic_cast<DBFile*>(payment_db)->acquire_lock(2,0) == true)
    {
        record_values = payment_db->readRecord(success_row_num.second);
        dynamic_cast<DBFile*>(payment_db)->release_lock();    
    }
    else
    {
        cout << "\nFailed to read record, invalid Transction ID";
    }
    
    return record_values;
}	

int Amazon::updateUserDetails()
{
	string uid, schema_col_name;
	int col_name;
	
	cout << "\nEnter user ID: ";
	cin >> uid;	
	
	cout << "Select the field whose value has to be changed: \n";
	cout << "1. Name  2. Phone no  3. Address\n";
	bool valid;
	do
	{
		valid = true;
		cin >> col_name;
		if(col_name == 1) schema_col_name = "name";
		else if(col_name == 2) schema_col_name = "phone_no";
		else if(col_name == 3) schema_col_name = "address";
		else 
		{
			cout << "Invalid choice\nPlease select a valid number: ";
			valid = false; 
		}		
	}while(!valid);
	
	string value;
	cout << "Enter the new value: ";
	cin >> value;	
	
    pair<bool, pair<int, int>> success_row_num = customer_db_schema->getRowColNumsCell(uid, schema_col_name);
    
    if (success_row_num.first)
    {
    	vector <string> new_record = {uid, "", "", ""};
    	new_record[success_row_num.second.second] = value;
    	
        Log_t op = {0, 0, success_row_num.second.first, success_row_num.second.second, new_record};
        
        vector <Log_t *> opList = {&op, NULL, NULL};
        int result = coord->performTransaction(opList);
        
        if(result == 1)
        {
        	cout << "\nDetails successfuly updated\n";
        	return 1;
        }
        else
        {
        	cout << "\nUpdation failed, please try again";
        	return 0;
        }
    }
    
    else
    {
        cout << "\nInvalid user ID";
        return 0;
    }
}	

int Amazon::makePayment()
{
	char id[20];
	
	++paymentCount; 	
  	sprintf(id, "p%d", paymentCount); 
  	string pid(id);; 
  	
	++shippingCount;
  	sprintf(id, "s%d", shippingCount);  
  	string sid(id);
  	
  	string uid, amount, mode, location, delivered, timestamp;
  	
  	cout << "\nEnter customer ID: ";
  	cin >> uid;
  	
  	cout << "Enter payment amount: ";
  	cin >> amount;
  	
  	cout << "Enter mode of payment: ";
  	cin >> mode;
  	
  	cout << "Enter delivery location: ";
  	cin >> location;
  	
  	delivered = "false";
  	timestamp = "now";
  	
  	vector <string> paymentRecord = {pid, uid, amount, mode, timestamp};
  	vector <string> shippingRecord = {sid, pid, location, delivered, timestamp};  	
  	
    Log_t payment_op = {1, 2, -1, -1, paymentRecord};
    Log_t shipping_op = {1, 2, -1, -1, shippingRecord};
    
    vector <Log_t *> opList = {NULL, &payment_op, &shipping_op}; 	
    int result = coord->performTransaction(opList);
    if(result == 1)
    {
    	cout << "\nDetails successfuly updated, transaction ID = " << pid << "\n";  
    	//cout << "[Amazon] Shipping row no: " << shipping_op.row << "  Payment row no: " << payment_op.row << '\n';
    	shipping_db_schema->updateIdRowNum(shipping_op.row, sid, 0);
    	payment_db_schema->updateIdRowNum(payment_op.row, pid, 0);
    	
    	return 1;
    }
    
    else cout << "\nUpdation failed, please try again\n";
    --shippingCount;
    --paymentCount;
    return 0;  	
}	

void Amazon::testCase()
{  		

	cout << "\n----------------------------------------------------------\n";
	/* register new user */
	cout << "Registering new user 1";
	cout << "\n----------------------------------------------------------\n";
	
	vector <string> new_record1 = {"c1", "kavya", "8867845637", "india1"};
    Log_t op1 = {1, 2, -1, -1, new_record1};
    
    vector <Log_t *> opList1 = {&op1, NULL, NULL};
    int result1 = coord->performTransaction(opList1);
    customer_db_schema->updateIdRowNum(op1.row, "c1", 0);
    vector <string> user_details1 = getUserDetails("c1");
	cout << '\n';
	for (auto it : user_details1)
		cout << it << " ";
	cout << endl;    
    
	cout << "\n----------------------------------------------------------\n";
	/* register new user */
	cout << "Registering new user 2";
	cout << "\n----------------------------------------------------------\n";
	vector <string> new_record2 = {"c2", "ruchika", "456888", "india2"};
    Log_t op2 = {1, 2, -1, -1, new_record2};
    
    vector <Log_t *> opList2 = {&op2, NULL, NULL};
    int result2 = coord->performTransaction(opList2);
    customer_db_schema->updateIdRowNum(op2.row, "c2", 0);
    vector <string> user_details2 = getUserDetails("c2");
	cout << '\n';
	for (auto it : user_details2)
		cout << it << " ";
	cout << endl;
	
    
	/* register new user */
	#if 1
	cout << "\n----------------------------------------------------------\n";
	/* register new user */
	cout << "Adding payment record - 1";
	cout << "\n----------------------------------------------------------\n";
	vector <string> new_record3 = {"p1", "ruchika", "456888", "india2"};
    Log_t op3 = {1, 2, -1, -1, new_record3};
    
    vector <Log_t *> opList3 = {NULL, &op3, NULL};
    int result3 = coord->performTransaction(opList3);
    payment_db_schema->updateIdRowNum(op3.row, "p1", 0);
    
    vector <string> user_details3 = getTransactionDetails("p1");
	cout << '\n';
	for (auto it : user_details3)
		cout << it << " ";
	cout << endl;
	#endif
	
	cout << "\n----------------------------------------------------------\n";
    #if 1
    /* update user, add payment record */
    cout << "Test case: commit rollback (commit fails)";
	cout << "\n----------------------------------------------------------\n";
  	
  	vector <string> paymentRecord = {"p2", "c2", "9800", "cash", "bangalore"}; 	  	
    Log_t payment_op = {1, 2, -1, -1, paymentRecord}; 
    
	vector <string> update_record = {"c1", "balm", "", ""};		
    pair<bool, pair<int, int>> success_row_num = customer_db_schema->getRowColNumsCell("c1", "name");
    
    if (success_row_num.first)
    {
    	cout << "ID found\n";
    	Log_t op = {0, 0, success_row_num.second.first, success_row_num.second.second, update_record};
    
		vector <Log_t *> opList = {&op, &payment_op, NULL};
		int result = coord->performTransaction(opList);
		if(result == 1)
    		payment_db_schema->updateIdRowNum(payment_op.row, "p2", 0);
	}
	#endif
	cout << "\nCustomer DB: ";
    vector <string> user_details = getUserDetails("c1");
	cout << '\n';
	for (auto it : user_details)
		cout << it << " ";
	
    user_details = getUserDetails("c2");
	cout << '\n';
	for (auto it : user_details)
		cout << it << " ";
	
    user_details = getUserDetails("c3");
	cout << '\n';
	for (auto it : user_details)
		cout << it << " ";
	cout << endl;
	
	cout << "\nPayment DB: \n";
    vector <string> payment_details = getTransactionDetails("p1");
	cout << '\n';
	for (auto it : payment_details)
		cout << it << " ";
	
    payment_details = getTransactionDetails("p2");
	cout << '\n';
	for (auto it : payment_details)
		cout << it << " ";
	
    payment_details = getTransactionDetails("p3");
	cout << '\n';
	for (auto it : payment_details)
		cout << it << " ";
	
	cout << "\n----------------------------------------------------------\n";
    #if 1
    /* update user, add payment record */
    cout << "Test case: 2 phase works properly";
	cout << "\n----------------------------------------------------------\n";
  	
  	vector <string> paymentRecord4 = {"p3", "c2", "9800", "cash", "bangalore"}; 	  	
    Log_t payment_op4 = {1, 2, -1, -1, paymentRecord4}; 
    
	vector <string> new_record4 = {"c3", "balm34", "7685937", ""};
    Log_t op4 = {1, 2, -1, -1, new_record4};	

	vector <Log_t *> opList4 = {&op4, &payment_op4, NULL};
	int result = coord->performTransaction(opList4);
	if(result == 1)
		payment_db_schema->updateIdRowNum(payment_op4.row, "p3", 0);
		customer_db_schema->updateIdRowNum(op4.row, "c3", 0);
	#endif
	
	cout << "\nCustomer DB: ";
    user_details = getUserDetails("c1");
	cout << '\n';
	for (auto it : user_details)
		cout << it << " ";
	
    user_details = getUserDetails("c2");
	cout << '\n';
	for (auto it : user_details)
		cout << it << " ";
	
    user_details = getUserDetails("c3");
	cout << '\n';
	for (auto it : user_details)
		cout << it << " ";
	
	cout << "\n\nPayment DB: ";
    payment_details = getTransactionDetails("p1");
	cout << '\n';
	for (auto it : payment_details)
		cout << it << " ";
	
    payment_details = getTransactionDetails("p2");
	cout << '\n';
	for (auto it : payment_details)
		cout << it << " ";
	
    payment_details = getTransactionDetails("p3");
	cout << '\n';
	for (auto it : payment_details)
		cout << it << " ";
	cout << '\n';
}