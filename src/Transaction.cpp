#include "Transaction.h"
#include "pub_def.h"
#include <iostream>

#ifdef Counts_The_New_Number
extern int curNewCounts;
extern int maxNewCounts;
#endif


extern int curNewItemsCounts;
extern int maxNewItemsCounts;

extern int curNewUtilitiesCounts;
extern int maxNewUtilitiesCounts;

Transaction::Transaction(void) : prefixUtility(0.0), transactionUtility(0.0), offset(0), items(NULL), utilities(NULL)
#ifdef Delete_The_New_Object
	, delFlag(true)
#endif
{
}


Transaction::Transaction(int *cItems, double *cUtilities, int cCounts) : prefixUtility(0.0), transactionUtility(0.0), offset(0)
#ifdef Delete_The_New_Object
	, delFlag(true)
#endif
{
	this->items = new int[cCounts];
	this->utilities = new double[cCounts];

#ifdef Counts_The_New_Number
	curNewCounts += 2;
	maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif


	curNewItemsCounts += cCounts;
	maxNewItemsCounts = (maxNewItemsCounts > curNewItemsCounts) ? maxNewItemsCounts : curNewItemsCounts;

	curNewUtilitiesCounts += cCounts;
	maxNewUtilitiesCounts = (maxNewUtilitiesCounts > curNewUtilitiesCounts) ? maxNewUtilitiesCounts : curNewUtilitiesCounts;


	for (int index = 0; index < cCounts; ++ index)
	{
		this->items[index] = cItems[index];
		this->utilities[index] = cUtilities[index];
		transactionUtility += cUtilities[index];
	}

	this->itemsLength = cCounts;
}


Transaction::Transaction(Transaction *cTran, int cPosition) : prefixUtility(0.0), transactionUtility(0.0), offset(0)
#ifdef Delete_The_New_Object
	, delFlag(false)
#endif
{
	this->items = cTran->items;
	this->utilities = cTran->utilities;
	this->itemsLength = cTran->itemsLength;

	// copy the utility of element e
	double utilityE = this->utilities[cPosition];

	// add the  utility of item e to the utility of the whole prefix used to project the transaction
	this->prefixUtility = cTran->prefixUtility + utilityE;

	// we will now calculate the remaining utility->
	// It is the transaction utility minus the profit of the element that was removed
	this->transactionUtility = cTran->transactionUtility - utilityE;
	// and we also need to subtract the utility of all items before e
	// but after the previous offset
	for(int i = cTran->offset; i < cPosition; i++){
		this->transactionUtility -= cTran->utilities[i];
	}
	// remember the offset for this projected transaction
	this->offset = cPosition + 1;

}

Transaction::Transaction(int *cItems, double *cUtilities, int cCounts, double transactionUtility)
{
	this->items = cItems;
	this->utilities = cUtilities;
	this->transactionUtility = transactionUtility;
	this->offset = 0;
	this->prefixUtility = 0;
	this->itemsLength = cCounts;
#ifdef Delete_The_New_Object
	this->delFlag = true;
#endif
}

Transaction::~Transaction(void)
{

#ifdef Delete_The_New_Object
	if (this->delFlag == true)
	{
		if (this->items != NULL)
		{
			delete [] this->items;
			this->items = NULL;

#ifdef Counts_The_New_Number
			curNewCounts --;
#endif

			curNewItemsCounts -= this->itemsLength;
		}
		
		if (this->utilities != NULL)
		{
			delete [] this->utilities;
			this->utilities = NULL;

#ifdef Counts_The_New_Number
			curNewCounts --;
#endif
			curNewUtilitiesCounts -= this->itemsLength;

		}
	}

#endif
	
}
