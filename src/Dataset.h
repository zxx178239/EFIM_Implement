#pragma once

#include "Transaction.h"
#include <vector>
class Dataset
{
public:
	Dataset(void);
	Dataset(int curTransactionCounts);
	~Dataset(void);


public:
//	std::vector<Transaction *> allTransactions;
	int			transactionCounts;
	Transaction  *allTransactions;
	int			curTransactionCounts;

public:

// 	void appendTransaction(Transaction *curTransaction);
// 
// 	int compareTwoTransaction(Transaction *t1, Transaction *t2);
};

