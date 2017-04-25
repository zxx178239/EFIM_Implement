#include "Dataset.h"
#include "pub_def.h"


extern int curNewCounts;
extern int maxNewCounts;

Dataset::Dataset(void)
{
}



Dataset::Dataset(int curTransactionCounts) : transactionCounts(curTransactionCounts), curTransactionCounts(0)
{
	allTransactions = new Transaction[transactionCounts];

#ifdef Counts_The_New_Number
	curNewCounts ++;
	maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

}

Dataset::~Dataset(void)
{
	delete [] allTransactions;
	allTransactions = NULL;
#ifdef Counts_The_New_Number
	curNewCounts -= transactionCounts + 1;
#endif
}

// void Dataset::appendTransaction(Transaction *curTransaction)
// {
// 	// through the order of the paper mentioned
// 	
// 	if (allTransactions.empty())
// 	{
// 		allTransactions.push_back(curTransaction);
// 		return;
// 	}
// 
// 	allTransactions.push_back(curTransaction);
// 
// 
// }
// 
// 
// inline int Dataset::compareTwoTransaction(Transaction *t1, Transaction *t2)
// {
// 	int pos1 = t1->itemsLength - 1;
// 	int pos2 = t2->itemsLength - 2;
// 
// 	// if the first transaction is smaller than the second one
// 	if(t1->itemsLength < t2->itemsLength)
// 	{
// 		// while the current position in the first transaction is >0
// 		while(pos1 >=0)
// 		{
// 			int subtraction = t2->items[pos2]  - t1->items[pos1];
// 			if(subtraction !=0)
// 			{
// 				return subtraction;
// 			}
// 			pos1 --;
// 			pos2 --;
// 		}
// 		// if they ware the same, they we compare based on length
// 		return -1;
// 		// else if the second transaction is smaller than the first one
// 	}else if (t1->itemsLength > t2->itemsLength)
// 	{
// 		// while the current position in the second transaction is >0
// 		while(pos2 >=0)
// 		{
// 			int subtraction = t2->items[pos2]  - t1->items[pos1];
// 			if(subtraction !=0)
// 			{
// 				return subtraction;
// 			}
// 			pos1 --;
// 			pos2 --;
// 		}
// 		// if they are the same, they we compare based on length
// 		return 1;
// 	}else
// 	{
// 		// else if both transactions have the same size
// 		while(pos2 >=0)
// 		{
// 			int subtraction = t2->items[pos2]  - t1->items[pos1];
// 			if(subtraction !=0)
// 			{
// 				return subtraction;
// 			}
// 			pos1 --;
// 			pos2 --;
// 		}
// 		// if they ware the same, they we compare based on length
// 		return 0;
// 	}
// }