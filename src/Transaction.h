#pragma once
#include "pub_def.h"

class Transaction
{
public:
	Transaction(void);
	Transaction(int *cItems, double *cUtilities, int cCounts);
	Transaction(int *cItems, double *cUtilities, int cCounts, double transactionUtility);
	Transaction(Transaction *cTran, int cPosition);
	~Transaction(void);



public:
	int offset;
	int	*items;
	double *utilities;

	double prefixUtility;
	double transactionUtility;

	int itemsLength;
#ifdef Delete_The_New_Object
	bool delFlag;
#endif
	


};

