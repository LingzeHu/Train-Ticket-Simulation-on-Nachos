// customer.h
// Data structure to store the customer information

class Customer
{
public:
	Customer() {};
	~Customer() {};
	// getter/ setter for itemNum
	void setitemNum(int n);
	int getitemNum();

	// getter/ setter for waitTimeBegin
	void setwaitTimeBegin(int n);
	int getwaitTimeBegin();

	// getter/ setter for totalWaitTime
	void setbeginCheckoutTime(int n);
	int getbeginCheckoutTime();

	// getter/ setter for serviceTime
	void setendCheckoutTime(int n);
	int getendCheckoutTime();

	// getter/ setter for leftServiceTime
	void setserviceTime(int n);
	int getserviceTime();

	bool operator==(Customer &A);
	bool operator!=(Customer &A);

	// getter/setter for users' ID
	void setUID(int n);
	int getUID();


private:
	int UID;					// users' ID
	int itemNum;				// value was set from 5-40
	int waitTimeBegin;			// the time that shopper begin to wait
	int beginCheckoutTime;		// the time that shopper begin checkout
	int endCheckoutTime;		// the time that shopper end checkout	
	int serviceTime;			// the service time left to be checkout. 
};
