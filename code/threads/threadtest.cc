#include "kernel.h"
#include "main.h"
#include "thread.h"
#include "customer.h"
//#include "queue.h"
#include "list.h"
#include <stdlib.h>
#include <math.h>
using namespace std;

#define minItemNum 5          // each customer has least 5 items.
#define maxItemNum 40         // each customer has most 5 items.
#define minRegCusNum 0        // In regular time, the least customer number is 0
#define maxRegCusNum 5        // In regular time, the most customer number is 5
#define minPeakCusNum 5       // In peak time, the least customer number is 5
#define maxPeaklCusNum 10     // In peak time, the most customer number is 5
#define startupTime 10        // The average startup time for a check out is 10 seconds.
#define eachItemTime 5        // A casher takes an average of 5 seconds to scan a single item 
#define clostTime 90          // The average close time for a check out is 1.5 minute.
#define maxCashierLine 5      // Max caopacity of cashier line

// data structure to store the summary inforamtion
struct summary {
  int customerNum = 0;
  int minWaitTime = -1, maxWaitTime = -1, totalWaitTime = 0, minSerTime = -1, maxSerTime = -1, totalSerTime = 0;
  double avgWaitTime, avgSerTime;
  int maxOpenCashier = -1, totalOpenCashier = 0, cashierHalfFull = 0;
  int minCusNumInWaitLine = -1, maxCusNumInWaitLine = -1, totalCusNumInWaitLine = 0;
  double avgOpenCashier, avgCusNumInLine;
} hourSummary[5], totalSummary, testSummary;

void SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        kernel->currentThread->Yield();
    }
}

// return a random number range from min to max
int randomNum(int min, int max) {
    return (rand() % (max - min + 1)) + min;
  }

// check whether exists cashier is not full
bool cashierAvaliable(List<Customer> *cashierLine,int cashierNum) {
    //int n = cashierLine->size();
    int flag = false;
    for (int i = 0; i < cashierNum; i++) {
      flag = flag || (cashierLine[i].NumInList() != maxCashierLine);
    }
    return flag;
  }

// check whether exists empty cashier
bool cashierHasEmpty(List<Customer> *cashierLine, int cashierNum) {
    int flag = false;
    for (int i = 0; i < cashierNum; i++) {
      flag = flag || (cashierLine[i].IsEmpty());
    }
    return flag;
  }

// check whether each cashier has more than three people
// this function is written for summary
bool isCashierHalfFull(List<Customer> *cashierLine, int cashierNum) {
    int flag = true;
    for (int i = 0; i < cashierNum; i++) {
      flag = flag && (cashierLine[i].NumInList() > 3);
    }
    return flag;
  }

// close a cashier
int closeACashier(List<Customer> *cashierLine, int cashierNum) {
    if (!cashierLine[cashierNum - 1].IsEmpty()) {
      for (int i = 0; i < cashierNum - 1; i++) {
        if (cashierLine[i].IsEmpty()) {
          List<Customer> tmpQueue = cashierLine[cashierNum - 1];
          cashierLine[cashierNum - 1] = cashierLine[i];
          cashierLine[i] = tmpQueue;
        }
      }
    }
    cout << "Close a cashier line" << endl;
    return --cashierNum;
  }


// put a shopper from waiting line to an avaliable cashier
void waitLineToCashierLine(List<Customer> *cashierLine, Customer shopper, int currTime, int cashierNum) {
    //int n = cashierLine->size();
    for (int i = 0; i < cashierNum; i++) {
      if (cashierLine[i].NumInList() != 5) {
        if (cashierLine[i].IsEmpty()) {
          int endCheckoutTime = currTime + shopper.getserviceTime();
          shopper.setbeginCheckoutTime(currTime);
          shopper.setendCheckoutTime(endCheckoutTime);
          cout << "    Shopper UID: " << shopper.getUID()
            << " enqueue cashierLine" << i
            << " at the first palce, at the time of " << currTime 
            << ", should dequeue when " << shopper.getendCheckoutTime()
            << endl;
          cashierLine[i].Append(shopper);         
        }
        else {
          cout << "    Shopper UID: " << shopper.getUID()
            << " enqueue cashierLine" << i
            << " at the time of " << currTime             
            << endl;
          cashierLine[i].Append(shopper);
        }
        break;
      }

    }
  }

// every time shopper was dequeue, record shopper's information into sum
summary updateCusInfo(summary sum, Customer shopper) {
    sum.customerNum++;
    int waitTime = shopper.getbeginCheckoutTime() - shopper.getwaitTimeBegin();
    sum.totalWaitTime += waitTime;
    if (waitTime > sum.maxWaitTime)
      sum.maxWaitTime = waitTime;
    if (waitTime < sum.minWaitTime || sum.minWaitTime == -1)
      sum.minWaitTime = waitTime;
    int seriveTime = shopper.getendCheckoutTime() - shopper.getbeginCheckoutTime();
    sum.totalSerTime += seriveTime;
    if (seriveTime > sum.maxSerTime)
      sum.maxSerTime = seriveTime;
    if (seriveTime < sum.minSerTime || sum.minSerTime == -1 )
      sum.minSerTime = seriveTime;
    return sum;
  }

// every minute, record the cashier and waiting line's information
summary updateCashierInfo(summary sum, int cashierNum,
                int waitLineSize, List<Customer> *cashierLine) {
    if (sum.maxOpenCashier < cashierNum || sum.maxOpenCashier == -1)
      sum.maxOpenCashier = cashierNum;
    sum.totalOpenCashier += cashierNum;
    if (isCashierHalfFull(cashierLine, cashierNum))
      sum.cashierHalfFull++;
    if (waitLineSize < sum.minCusNumInWaitLine
      || sum.minCusNumInWaitLine == -1)
      sum.minCusNumInWaitLine = waitLineSize;
    if (waitLineSize > sum.maxCusNumInWaitLine
      || sum.maxCusNumInWaitLine == -1)
      sum.maxCusNumInWaitLine = waitLineSize;
    sum.totalCusNumInWaitLine += waitLineSize;

    return sum;
  }


// main function
void ThreadTest()
{

    int timeRecord = 0;   // Play the role as an clock 
    int cashierNum = 2;   // The number of cashier opened currently
    List<Customer> waitingLine = List<Customer>();
    List<Customer> *cashierLine = new List<Customer>[10];

    // inits each cashier line
    for (int i = 0; i < 300; i++) {
      cashierLine[i] = List<Customer>();
    }
    bool isPeak = false;
    int currSum = 0;
    
    // iterate 300 times means 300mins(2pm-7pm,5 hours)
    for (int i = 0; i < 300; i++) {
      //determine whether it in peak hour (4pm-6pm, 2 hours)
      if (i >= 0 && i < 60) {
        isPeak = false;
        currSum = 0;
      }
      else if (i > 59 && i < 120) {
        isPeak = false;
        currSum = 1;
      }
      else if(i > 119 && i < 180){
        isPeak = true;
        currSum = 2;
      }
      else if (i > 179 && i < 240) {
        isPeak = true;
        currSum = 3;
      }
      else if (i > 239 ) {
        isPeak = false;
        currSum = 4;
      }

      // cusNum: at thie period, the number of customers might to checkout
      int cusNum;
      if (isPeak) {
        cusNum = randomNum(minPeakCusNum, maxPeaklCusNum);
      }
      else
      {
        cusNum = randomNum(minRegCusNum, maxRegCusNum);
      }
      
      // inits each customer's item by
      for (int j = 0; j < cusNum; j++) {
        Customer shopper = Customer();
        int itemNum = randomNum(minItemNum, maxItemNum);
        shopper.setitemNum(itemNum);
        // caculate the service time accroding to the number of items
        int checkoutTime = round((startupTime + itemNum * eachItemTime + clostTime) / 60);
        shopper.setserviceTime(checkoutTime);
        shopper.setwaitTimeBegin(timeRecord);
        //cout << "The shopper " << j << "wait at the time of " << shopper.getwaitTimeBegin() << endl;
        shopper.setUID(rand());
        waitingLine.Append(shopper);
        // cout << "    The shopper created at the time of " << shopper.getwaitTimeBegin()
        // << " with UId " << shopper.getUID() << " begin wait in the waiting line. \n"; 
      } 

      // Check whether a new cashier needs to open
      while (waitingLine.NumInList() > 10 && !cashierAvaliable(cashierLine, cashierNum) && cashierNum < 10 ) {
        cashierNum++;
        cout << "Open a new cashier line " << cashierLine << endl;
        for (int i = 0; i < 5; i++) {
          Customer shopper = waitingLine.Front();
          waitLineToCashierLine(cashierLine, shopper, timeRecord, cashierNum);
          waitingLine.RemoveFront();
        }       
      }

      //Transfer the shopper in waiting line to cashier line.
      while (!waitingLine.IsEmpty() && cashierAvaliable(cashierLine, cashierNum)) {
        Customer shopper = waitingLine.Front();
        waitLineToCashierLine(cashierLine, shopper, timeRecord, cashierNum);
        waitingLine.RemoveFront();
      }

      // Check if each cashier's first shopper had done their checkout.
      for (int i = 0; i < cashierNum; i++){
        if (!cashierLine[i].IsEmpty()) {
          cout << "cashierLine" << i << "'s first shopper's end time would be "
           << cashierLine[i].Front().getendCheckoutTime() << endl;
          if (cashierLine[i].Front().getendCheckoutTime() == timeRecord) {
            // need code here to record the shopper;
            cout << "    Shopper UID: " << cashierLine[i].Front().getUID()
              << " dequeue cashierLine" << i
              << " at the time of " << timeRecord << endl;
            // Update customer's information
            hourSummary[currSum] = updateCusInfo(hourSummary[currSum], cashierLine[i].Front());
            totalSummary = updateCusInfo(totalSummary, cashierLine[i].Front());

            cashierLine[i].RemoveFront();
            // afther pop a shopper, if the cashier is not empty, then continue the program
            if (!cashierLine[i].IsEmpty()) {
              Customer shopper = cashierLine[i].Front();
              shopper.setbeginCheckoutTime(timeRecord);
              int endCheckoutTime = shopper.getserviceTime() + timeRecord;
              shopper.setendCheckoutTime(endCheckoutTime);
              cashierLine[i].Prepend(shopper);
              cout << "    Shopper UID: " << cashierLine[i].Front().getUID()
                << " begin to checkout at cashierLine" << i
                << " at the time of " << timeRecord
                << ", should dequeue when " << cashierLine[i].Front().getendCheckoutTime()
                << endl;
              // enqueue a shopper from waiting line
              if (!waitingLine.IsEmpty()) {
                Customer shopper = waitingLine.Front();
                waitLineToCashierLine(cashierLine, shopper, timeRecord, cashierNum);
                waitingLine.RemoveFront();
              }
            }
          }
        }     
        
      }

      // Check if there exists cashier need to be close.
      while (waitingLine.IsEmpty() && cashierHasEmpty(cashierLine, cashierNum) && (cashierNum > 2)) {
        cashierNum = closeACashier(cashierLine, cashierNum);
      }
      
      cout << "At the time of " << timeRecord
        << ", the size of waiting line is " << waitingLine.NumInList()
        << ", the # of opening cashier is " << cashierNum
        << endl;
      cout << " ============================================================" << endl;
      
      hourSummary[currSum] = updateCashierInfo(hourSummary[currSum], cashierNum, waitingLine.NumInList(), cashierLine);
      totalSummary = updateCashierInfo(totalSummary, cashierNum, waitingLine.NumInList(), cashierLine);

      timeRecord++;
    }

    //Summary of the code
    int time = 2;
    for (int i = 0; i < 5; i++) {
      cout << "\n=============================" << time << "pm-"<< time+1 << "pm Summary====================================" << endl;
      cout << "The number of customers is: " << hourSummary[i].customerNum << endl;
      cout << "The min wait time is " << hourSummary[i].minWaitTime
        << ", the max wait time is " << hourSummary[i].maxWaitTime
        << ", the average wait time is " << hourSummary[i].totalWaitTime / hourSummary[i].customerNum
        << endl;
      cout << "The min serivce time is " << hourSummary[i].minSerTime
        << ", the max serivce time is " << hourSummary[i].maxSerTime
        << ", the average serivce time is " << hourSummary[i].totalSerTime / hourSummary[i].customerNum
        << endl;
      cout << "The max number of opening cashier is " << hourSummary[i].maxOpenCashier
        << ", the average number of opening cashier is " << hourSummary[i].totalOpenCashier / 60
        << endl;
      cout << "The minutes each casher will have more than 3 customers standing in line is "
        << hourSummary[i].cashierHalfFull << endl;
      cout << "The min number of customers in waiting line is " << hourSummary[i].minCusNumInWaitLine
        << ", the max number of customers in waiting line is " << hourSummary[i].maxCusNumInWaitLine
        << ", the average number of customers in waiting line is " << hourSummary[i].totalCusNumInWaitLine / 60
        << endl;

      time++;
    }

    cout << "=============================Entire Summary====================================" << endl;
    cout << "The number of customers is: " << totalSummary.customerNum << endl;
    cout << "The min wait time is " << totalSummary.minWaitTime
      << ", the max wait time is " << totalSummary.maxWaitTime
      << ", the average wait time is " << totalSummary.totalWaitTime / totalSummary.customerNum
      << endl;
    cout << "The min serivce time is " << totalSummary.minSerTime
      << ", the max serivce time is " << totalSummary.maxSerTime
      << ", the average serivce time is " << totalSummary.totalSerTime / totalSummary.customerNum
      << endl;
    cout << "The max number of opening cashier is " << totalSummary.maxOpenCashier
      << ", the average number of opening cashier is " << totalSummary.totalOpenCashier / 300
      << endl;
    cout << "The minutes each casher will have more than 3 customers standing in line is "
      << totalSummary.cashierHalfFull << endl;
    cout << "The min number of customers in waiting line is " << totalSummary.minCusNumInWaitLine
      << ", the max number of customers in waiting line is " << totalSummary.maxCusNumInWaitLine
      << ", the average number of customers in waiting line is " << totalSummary.totalCusNumInWaitLine / 300
      << endl;

}
