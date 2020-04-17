// customer.cc
//	The implement of customer.h. Mainly about getter and setter
//	

#include <iostream>
#include "customer.h"


	// getter/ setter for itemNum
	void Customer::setitemNum(int n){
		this->itemNum = n;
	}
	int Customer::getitemNum(){
		return itemNum;
	}

	// getter/ setter for waitTimeBegin
	void Customer::setwaitTimeBegin(int n){
		this->waitTimeBegin = n;
	}
	int Customer::getwaitTimeBegin(){
		return waitTimeBegin;
	}

	// getter/ setter for totalWaitTime
	void Customer::setbeginCheckoutTime(int n){
		this->beginCheckoutTime = n;
	}
	int Customer::getbeginCheckoutTime(){
		return beginCheckoutTime;
	}

	// getter/ setter for serviceTime
	void Customer::setendCheckoutTime(int n){
		this->endCheckoutTime = n;
	}
	int Customer::getendCheckoutTime(){
		return endCheckoutTime;
	}

	// getter/ setter for ServiceTime
	void Customer::setserviceTime(int n){
		this->serviceTime = n;
	}
	int Customer::getserviceTime(){
		return serviceTime;
	}

	// check if two customer are the same
	bool Customer::operator==(Customer &A){
		if(this->getitemNum() == A.getitemNum() && this->getserviceTime() == A.getserviceTime() &&
			this->getendCheckoutTime() == A.getendCheckoutTime() &&  this->getbeginCheckoutTime() == A.getbeginCheckoutTime()
			 && this->getwaitTimeBegin() == A.getwaitTimeBegin() && this->UID == A.UID ){
			return true;
		} else {
			return false;
		}
	}

	bool Customer::operator!=(Customer &A){
		if(this->getitemNum() == A.getitemNum() && this->getserviceTime() == A.getserviceTime() &&
			this->getendCheckoutTime() == A.getendCheckoutTime() &&  this->getbeginCheckoutTime() == A.getbeginCheckoutTime()
			 && this->getwaitTimeBegin() == A.getwaitTimeBegin() && this->UID == A.UID ){
			return false;
		} else {
			return true;
		}

	}

	// getter/setter for users' ID
	void Customer::setUID(int n){
		this->UID = n;
	}

	int Customer::getUID(){
		return UID;
	}

