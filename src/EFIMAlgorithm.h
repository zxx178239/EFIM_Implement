#pragma once

#include "Transaction.h"
#include "Dataset.h"
#include <string>
#include <string.h>
#include <vector>
#include "pub_def.h"


class EFIMAlgorithm
{
public:
	EFIMAlgorithm(void);
	~EFIMAlgorithm(void);

public:
	void launch(int argc, char *argv[]);

private:
	std::string		dataBaseName;
	std::string		priceFileName;
	std::string		resultFileName;
	int				itemCounts;			// the number of items
	int				transactionWeight;	// the weight of transaction
	int				transactionCounts;	// the number of transaction
	double			absoluteValue;		// the absolute value of threshold of user-specified
	double			relativeValue;		// the relative value of threshold of user-specified


	double			*utilityBinArrayLU;
	double			*utilityBinArraySU;
	int				*newNameToOldName;
	int				*oldNameToNewName;

	Dataset			*curDatabase;

	int				relevantCounts;

	std::vector<int> itemToKeep;
	
	//some counts for algorithm
	int				candidatesCounts;
	int				huiCounts;
	double			initTimes;
	double			mergeTimes;

	double			miningTimes;
	double			totalTimes;

//	int				*temp;

	

private:
	int handleParameter(int argc, char *argv[]);
	int firstReadDB(double *fPriceContent);

	int getPrice(double *priceContent);

	int calculateTWU(double *cPriceContent);

	void getItemToKeep();

	void reorderItems();

	int secondReadDB(double *sPriceContent);

	void useUtilityBinArrayToCalculateSubtreeUtilityFirstTime(Dataset *curDB);

	void useUtilityBinArraysToCalculateUpperBounds(std::vector<Transaction *> &transactionPe, int j, std::vector<int> itemsToKeep);

	void backtrackingEFIM(std::vector<Transaction *> &curAllTransactionsOfP, std::vector<int> &itemToKeep, std::vector<int> &itemToExplore, int prefixLength, bool countsFlag = false);
	bool isEqualTo(Transaction *t1, Transaction *t2) ;

	void printResult();

	void insertOrder(int *items, double *utilities, int curCounts);
};

