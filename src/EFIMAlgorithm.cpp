#include "EFIMAlgorithm.h"
#include <fstream>
#include <iostream>
#include <ctime>

#ifdef Counts_The_New_Number
int curNewCounts = 0;
int maxNewCounts = 0;
#endif

// some memory numbers
int curNewTransactionCounts = 0;
int maxNewTransactionCounts = 0;

int curNewItemsCounts = 0;
int maxNewItemsCounts = 0;

int curNewUtilitiesCounts = 0;
int maxNewUtilitiesCounts = 0;


int compareTwoTransaction(const void*c1, const void*c2);

EFIMAlgorithm::EFIMAlgorithm(void) : relevantCounts(0), absoluteValue(-1), relativeValue(-1), dataBaseName(""),
					priceFileName(""), candidatesCounts(0), huiCounts(0), initTimes(0.0), miningTimes(0.0), totalTimes(0.0)
{
//	temp = new int [500];
}


EFIMAlgorithm::~EFIMAlgorithm(void)
{
//	delete [] temp;
}

void EFIMAlgorithm::launch(int argc, char *argv[])
{
	/*
		1. handle parameter
		2. init some variable
		3. first read db
		4. second read db
		5. calculate the sub items through the SU strategy
	*/
	for (int index = 0; index < argc; ++ index)
	{
		std::cout << argv[index] << " ";
	}
	std::cout << std::endl;

	double proStart = clock();
	// 1. handle parameter
	if(!handleParameter(argc, argv))
	{
		// print some help msg
		return;
	}

	// 2. init some variable
	utilityBinArrayLU = new double[itemCounts + 1];

	newNameToOldName = new int[itemCounts + 1];
	oldNameToNewName = new int[itemCounts + 1];

	double *priceContent = new double[itemCounts + 1];

	curDatabase = new Dataset(transactionCounts);

	mergeTimes = 0.0;

#ifdef Counts_The_New_Number
	curNewCounts += 5;
	maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

	for (int index = 0; index < itemCounts + 1; ++ index)
	{
		utilityBinArrayLU[index] = 0.0;

		newNameToOldName[index] = -1;
		oldNameToNewName[index] = -1;
		priceContent[index] = 0.0;
	}
	
	double initStart = clock();

	// 3. first read db
	firstReadDB(priceContent);

	// the array of SU just need the size of relevantCounts + 1
	utilityBinArraySU = new double[relevantCounts + 1];

#ifdef Counts_The_New_Number
	curNewCounts ++;
	maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif
	
	for (int index = 0; index < relevantCounts + 1; ++ index)
	{
		utilityBinArraySU[index] = 0.0;
	}

	// 4. second read db
	secondReadDB(priceContent);

	double initEnd = clock();
	initTimes = (initEnd - initStart) / CLOCKS_PER_SEC;

	//the array of priceContent don't need , so delete
	delete [] priceContent;
	priceContent = NULL;

#ifdef Counts_The_New_Number
	curNewCounts --;
#endif

	// 5. calculate the sub items through the SU strategy
	std::vector<int> itemToExplore;
	// index is the new name of item
	for (int index = 0; index < relevantCounts; ++ index)
	{
		if (utilityBinArraySU[index] >= absoluteValue)
		{
			itemToExplore.push_back(index);
		}
	}

	// there, I need to give some traverse
	std::vector<Transaction *> initAllTransactions;
	initAllTransactions.resize(curDatabase->transactionCounts);
	for (int index = 0; index < curDatabase->transactionCounts; ++ index)
	{
		initAllTransactions[index] = curDatabase->allTransactions + index;
	}

	double mineStart = clock();

	backtrackingEFIM(initAllTransactions, itemToKeep, itemToExplore, 0, true);

	double mineEnd = clock();

	miningTimes = (mineEnd - mineStart) / CLOCKS_PER_SEC;
	double proEnd = clock();

	totalTimes = (proEnd - proStart) / CLOCKS_PER_SEC;

	printResult();

	std::cout << "minUtil: " << absoluteValue << "\ttotalNum: " << huiCounts << "\ttotal time: " << totalTimes << std::endl;
	delete [] utilityBinArrayLU;
	utilityBinArrayLU = NULL;

	delete [] utilityBinArraySU;
	utilityBinArraySU = NULL;

	delete [] newNameToOldName;
	newNameToOldName = NULL;

	delete [] oldNameToNewName;
	oldNameToNewName = NULL;

	delete curDatabase;
	curDatabase = NULL;

#ifdef Counts_The_New_Number
	curNewCounts -= 5;
	std::cout << "new counts: " << curNewCounts << std::endl;
#endif

}

int EFIMAlgorithm::handleParameter(int argc, char *argv[])
{
	char *tmp;
	for (int index = 1; index < argc; ++ index)
	{
		if (argv[index][0] == '-' && (argv[index][1] == 'D' || argv[index][1] == 'd'))
		{
			tmp = argv[index]; tmp += 2; dataBaseName = tmp;
		}else if(argv[index][0] == '-' && (argv[index][1] == 'E' || argv[index][1] == 'e'))
		{
			tmp = argv[index]; tmp += 2; priceFileName = tmp;
		}else if (argv[index][0] == '-' && (argv[index][1] == 'O' || argv[index][1] == 'o'))
		{
			tmp = argv[index]; tmp += 2; resultFileName = tmp;
		}else if (argv[index][0] == '-' && (argv[index][1] == 'N' || argv[index][1] == 'n'))
		{
			tmp = argv[index]; tmp += 2; itemCounts = atoi(tmp);
		}else if (argv[index][0] == '-' && (argv[index][1] == 'W' || argv[index][1] == 'w'))
		{
			tmp = argv[index]; tmp += 2; transactionWeight = atoi(tmp);
		}else if (argv[index][0] == '-' && argv[index][1] == '#')
		{
			tmp = argv[index]; tmp += 2; transactionCounts = atoi(tmp);
		}else if (argv[index][0] == '-' && (argv[index][1] == 'S' || argv[index][1] == 's'))
		{
			tmp = argv[index]; tmp += 2; absoluteValue = atof(tmp);
		}else if (argv[index][0] == '-' && (argv[index][1] == 'U' || argv[index][1] == 'u'))
		{
			tmp = argv[index]; tmp += 2; relativeValue = atof(tmp);
		}
	}

	if (absoluteValue == -1 && relativeValue == -1)
	{
		return 0;
	}else
	{
		return 1;
	}

}

int EFIMAlgorithm::firstReadDB(double *fPriceContent)
{
	/*
		1. read price
		2. calculate the TWU(LU), as the init localUtility
		3. get the itemToKeep and insert order
		4. rename the item, save two array, one is the newToOld, another is the oldToNew
	*/

	// 1. read price
	if(getPrice(fPriceContent) == -1)
	{
		return -1;
	}

	// 2. calculate the TWU(LU), as the init localUtility
	calculateTWU(fPriceContent);

	// 3. get the itemToKeep and insert order
	getItemToKeep();

	// 4. rename the item, save two array, one is the newToOld, another is the oldToNew
	reorderItems();

	return 0;
}

int EFIMAlgorithm::getPrice(double *priceContent)
{
	std::ifstream ifs(priceFileName.c_str());
	if (!ifs.good())
	{
		std::cout << "open price file failed!" << std::endl;
		return -1;
	}
	//read eu of item
	for (int index = 0; index < itemCounts + 1; ++ index)
	{
		ifs >> priceContent[index];
	}
	ifs.close();
	return 0;
}

int EFIMAlgorithm::calculateTWU(double *cPriceContent)
{
	int firstValue, secondValue, thirdValue;		// indicate the transactionCounts, itemCounts, avgWeight
	std::ifstream ifs(dataBaseName.c_str());
	if (!ifs.good())
	{
		std::cout << "open db error!" << std::endl;
		return -1;
	}
	ifs >> firstValue >> secondValue >> thirdValue;

	int eachTID, eachTCounts;
	double sumUtility = 0.0;
	double curLineUtility = 0.0;

	//20160802 change
	int *lineItems = new int[transactionWeight + 1];

#ifdef Counts_The_New_Number
	curNewCounts ++;
	maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

	for (int index = 0; index < transactionWeight + 1; ++ index)
	{
		lineItems[index] = 0;
	}

	while (ifs >> eachTID >> eachTCounts)
	{
		/*
			1. calculate the TU of current line
			2. save to utility-bin LU(local Utility)
		*/
		curLineUtility = 0;
		// 1. calculate the TU of current line
		int curItemCounts;
		int curItem, counts = 0;
		for (int index = 0; index < eachTCounts; ++ index)
		{
			ifs >> curItem >> curItemCounts;

			lineItems[counts ++] = curItem;
			curLineUtility += curItemCounts * cPriceContent[curItem];
		}
		sumUtility += curLineUtility;

		// 2. save to utility-bin LU(local Utility)
		for (int index = 0; index < counts; ++ index)
		{
			utilityBinArrayLU[lineItems[index]] += curLineUtility;
		}
		
	}

	delete [] lineItems;
	lineItems = NULL;

#ifdef Counts_The_New_Number
	curNewCounts --;
#endif

	//if absoluteValue not give, we can calculate to get
	if (absoluteValue == -1)
	{
		absoluteValue = sumUtility * relativeValue;
	}

	if (relativeValue < 0)
	{
		relativeValue = absoluteValue / sumUtility;
	}


	ifs.close();
	return 0;
}


void EFIMAlgorithm::getItemToKeep()
{
	/*
		1. get the itemToKeep
		2. insert order according to the ascending order of TWU
	*/
	int counts = 0;
	for (int index = 0; index < itemCounts + 1; ++ index)
	{
		if (utilityBinArrayLU[index] >= absoluteValue)
		{
			itemToKeep.push_back(index);
			counts ++;
		}
	}
	relevantCounts = counts;

	for (int index = 1; index < relevantCounts; ++ index)
	{
		int scan;
		int tmp = itemToKeep[index];
		for (scan = index - 1; scan >= 0 && utilityBinArrayLU[itemToKeep[scan]] >= utilityBinArrayLU[tmp]; -- scan)
		{
			itemToKeep[scan + 1] = itemToKeep[scan];
		}
		itemToKeep[scan + 1] = tmp;
	}
	
}

void EFIMAlgorithm::reorderItems()
{
	int currentName = 1;

	for (int index = 0; index != itemToKeep.size(); ++ index)
	{
		int item = itemToKeep[index];

		newNameToOldName[currentName] = item;
		oldNameToNewName[item] = currentName;
		itemToKeep[index] = currentName;

		currentName ++;
	}
	
}

int EFIMAlgorithm::secondReadDB(double *sPriceContent)
{
	/*
		1. read a transaction, reorganization the current transaction
		2. insert order
		3. new a object of Transaction
		4. qsort the object of Dataset
		5. get the utility-bin su(sub-tree utility)
	*/

	std::ifstream ifs(dataBaseName.c_str());
	if (!ifs.good())
	{
		std::cout << "open db failed !" << std::endl;
		return -1;
	}

	int firstPara, secondPara, thirdPara;
	int currentTransactionID, currentTransactionCounts;
	int currentItem, currentQuantity;

	int allTransactionCounts = 0;

	int	*curLineItems = new int[transactionWeight + 1];
	double *curLineUtilities = new double[transactionWeight + 1];

#ifdef Counts_The_New_Number
	curNewCounts += 2;
	maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

	int curLineCounts;
	for (int index = 0; index < transactionWeight + 1; ++ index)
	{
		curLineItems[index] = 0;
		curLineUtilities[index] = 0.0;
	}
	ifs >> firstPara >> secondPara >> thirdPara;

	while (ifs >> currentTransactionID >> currentTransactionCounts)
	{
		double currentTransactionUtility = 0.0;
		curLineCounts = 0;
		// 1. read a transaction, reorganization the current transaction	
		// discard the irrelevant items, reorder the items and utilities according the ascending order of TWU
		for (int index = 0; index < currentTransactionCounts; ++ index)
		{
			ifs >> currentItem >> currentQuantity;

			if(oldNameToNewName[currentItem] == -1)
				continue;

			curLineItems[curLineCounts] = oldNameToNewName[currentItem];
			curLineUtilities[curLineCounts ++] = currentQuantity  * sPriceContent[currentItem];
		}

		//when the curLineCounts is 0, then indicates the items of current transaction are irrelevant items
		if(curLineCounts == 0)
			continue;

		//2. insert order
		insertOrder(curLineItems, curLineUtilities, curLineCounts);

		// 3. new a object of Transaction
		Transaction *newTraction = new Transaction(curLineItems, curLineUtilities, curLineCounts);
#ifdef Counts_The_New_Number
		curNewCounts ++;
		maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

		curNewTransactionCounts ++;
		maxNewTransactionCounts = (maxNewTransactionCounts > curNewTransactionCounts) ? maxNewTransactionCounts : curNewTransactionCounts;

		curDatabase->allTransactions[allTransactionCounts ++] = *newTraction;

	}
	curDatabase->curTransactionCounts = allTransactionCounts;
	// 4. qsort the object of Dataset
	qsort(curDatabase->allTransactions, allTransactionCounts, sizeof(Transaction), compareTwoTransaction);
	
	// 5. get the utility-bin su(sub-tree utility)
	useUtilityBinArrayToCalculateSubtreeUtilityFirstTime(curDatabase);


	delete [] curLineItems;
	curLineItems = NULL;
	delete [] curLineUtilities;
	curLineUtilities = NULL;

#ifdef Counts_The_New_Number
	curNewCounts -= 2;
#endif

	ifs.close();

	return 0;


}

void EFIMAlgorithm::insertOrder(int *items, double *utilities, int curCounts)
{
	for(int j=1; j< curCounts; j++)
	{
		int itemJ = items[j];
		double utilityJ = utilities[j];
		int i = j - 1;
		for(; i>=0 && (items[i] > itemJ); i--){
			items[i + 1] = items[i];
			utilities[i + 1] = utilities[i];
		}
		items[i + 1] = itemJ;
		utilities[i + 1] = utilityJ;
	}
}

int compareTwoTransaction(const void *c1, const void *c2)
{
	Transaction *t1 = (Transaction *)c1;
	Transaction *t2 = (Transaction *)c2;
	int pos1 = t1->itemsLength - 1;
	int pos2 = t2->itemsLength - 1;

	// if the first transaction is smaller than the second one
	if(t1->itemsLength < t2->itemsLength)
	{
		// while the current position in the first transaction is >0
		while(pos1 >=0)
		{
			int subtraction = t2->items[pos2]  - t1->items[pos1];
			if(subtraction !=0)
			{
				return subtraction;
			}
			pos1 --;
			pos2 --;
		}
		// if they ware the same, they we compare based on length
		return -1;
		// else if the second transaction is smaller than the first one
	}else if (t1->itemsLength > t2->itemsLength)
	{
		// while the current position in the second transaction is >0
		while(pos2 >=0)
		{
			int subtraction = t2->items[pos2]  - t1->items[pos1];
			if(subtraction !=0)
			{
				return subtraction;
			}
			pos1 --;
			pos2 --;
		}
		// if they are the same, they we compare based on length
		return 1;
	}else
	{
		// else if both transactions have the same size
		while(pos2 >=0)
		{
			int subtraction = t2->items[pos2]  - t1->items[pos1];
			if(subtraction !=0)
			{
				return subtraction;
			}
			pos1 --;
			pos2 --;
		}
		// if they ware the same, they we compare based on length
		return 0;
	}
}

void EFIMAlgorithm::useUtilityBinArrayToCalculateSubtreeUtilityFirstTime(Dataset *curDB)
{
	double sumSU = 0.0;

	for (int index = 0; index < curDB->curTransactionCounts; ++ index)
	{
		sumSU = 0.0;

		for (int scan = curDB->allTransactions[index].itemsLength - 1; scan >= 0; -- scan)
		{
			int item = curDB->allTransactions[index].items[scan];

			sumSU += curDB->allTransactions[index].utilities[scan];

			utilityBinArraySU[item] += sumSU;
		}
	}
}

void EFIMAlgorithm::backtrackingEFIM(std::vector<Transaction *> &curAllTransactionsOfP, std::vector<int> &bitemToKeep, std::vector<int> &bitemToExplore, int prefixLength, bool countsFlag)
{
	candidatesCounts += bitemToExplore.size();

	for (int j = 0; j != bitemToExplore.size(); ++ j)
	{
		int e = bitemToExplore[j];

		std::vector<Transaction *> transactionsPe;

		double utilityPe = 0.0;

		// this variable used to merge, just record the previous transaction
		Transaction *previousTransaction = NULL;
		int consecutiveMergeCount = 0;

		double ts1 = clock();

		for (int scan = 0; scan != curAllTransactionsOfP.size(); ++ scan)
		{
			Transaction *curTransaction = curAllTransactionsOfP[scan];

			int positionE = -1;

			int low = curTransaction->offset;
			int high = curTransaction->itemsLength - 1;

			// perform binary search to find e in the transaction
			while (high >= low ) 
			{
				int middle = (low + high) >> 1; // divide by 2
				if (curTransaction->items[middle] < e) 
				{
					low = middle + 1;
				}else if (curTransaction->items[middle] == e) 
				{
					positionE =  middle;
					break;
				}else
				{
					high = middle - 1;
				}
			}

			if (positionE > -1 ) 
			{ 

				// optimization: if the 'e' is the last one in this transaction,
				// we don't keep the transaction
				if(curTransaction->itemsLength - 1 == positionE)
				{
					// but we still update the sum of the utility of P U {e}
					utilityPe  += curTransaction->utilities[positionE] + curTransaction->prefixUtility;
				}else
				{
					// we cut the transaction starting from position 'e'
					Transaction *projectedTransaction = new Transaction(curTransaction, positionE);

#ifdef Counts_The_New_Number
					curNewCounts ++;
					maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

					curNewTransactionCounts ++;
					maxNewTransactionCounts = (maxNewTransactionCounts > curNewTransactionCounts) ? maxNewTransactionCounts : curNewTransactionCounts;

					utilityPe  += projectedTransaction->prefixUtility;

					// if it is the first transaction that we read
					if(previousTransaction == NULL)
					{
						// we keep the transaction in memory 
						previousTransaction = projectedTransaction;
					}else if (isEqualTo(projectedTransaction, previousTransaction))
					{
						// If it is not the first transaction of the database and 
						// if the transaction is equal to the previously read transaction,
						// we will merge the transaction with the previous one

						// if the first consecutive merge
						if(consecutiveMergeCount == 0)
						{
							// copy items and their profit from the previous transaction
							int itemsCount = previousTransaction->itemsLength - previousTransaction->offset;
							int *items = new int[itemsCount];

#ifdef Counts_The_New_Number
							curNewCounts ++;
							maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

							for (int index = 0; index < itemsCount; ++ index)
							{
								items[index] = previousTransaction->items[index + previousTransaction->offset];
							}
							//System.arraycopy(previousTransaction.items, previousTransaction.offset, items, 0, itemsCount);
							double *utilities = new double[itemsCount];
#ifdef Counts_The_New_Number
							curNewCounts ++;
							maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

							curNewItemsCounts += itemsCount;
							maxNewItemsCounts = (maxNewItemsCounts > curNewItemsCounts) ? maxNewItemsCounts : curNewItemsCounts;

							curNewUtilitiesCounts += itemsCount;
							maxNewUtilitiesCounts = (maxNewUtilitiesCounts > curNewUtilitiesCounts) ? maxNewUtilitiesCounts : curNewUtilitiesCounts;


							//System.arraycopy(previousTransaction.utilities, previousTransaction.offset, utilities, 0, itemsCount);
							for (int index = 0; index < itemsCount; ++ index)
							{
								utilities[index] = previousTransaction->utilities[index + previousTransaction->offset];
							}
							// make the sum of utilities from the previous transaction
							int positionPrevious = 0;
							int positionProjection = projectedTransaction->offset;
							while(positionPrevious < itemsCount)
							{
								utilities[positionPrevious] += projectedTransaction->utilities[positionProjection];
								positionPrevious ++;
								positionProjection ++;
							}

							// make the sum of prefix utilities
							double sumUtilities = previousTransaction->prefixUtility += projectedTransaction->prefixUtility;
							double tmpNum = previousTransaction->transactionUtility + projectedTransaction->transactionUtility;

							delete projectedTransaction;
							projectedTransaction = NULL;
							delete previousTransaction;
							previousTransaction = NULL;
							curNewTransactionCounts -= 2;

#ifdef Counts_The_New_Number
							curNewCounts -= 2;
#endif

							// create the new transaction replacing the two merged transactions
							previousTransaction = new Transaction(items, utilities, itemsCount, tmpNum);

							curNewTransactionCounts ++;
							maxNewTransactionCounts = (maxNewTransactionCounts > curNewTransactionCounts) ? maxNewTransactionCounts : curNewTransactionCounts;


#ifdef Counts_The_New_Number
							curNewCounts ++;
							maxNewCounts = (maxNewCounts > curNewCounts) ? maxNewCounts : curNewCounts;
#endif

							previousTransaction->prefixUtility = sumUtilities;	
							
						}else{
							// if not the first consecutive merge

							// add the utilities in the projected transaction to the previously
							// merged transaction
							int positionPrevious = 0;
							int positionProjected = projectedTransaction->offset;
							int itemsCount = previousTransaction->itemsLength;
							while(positionPrevious < itemsCount)
							{
								previousTransaction->utilities[positionPrevious] += projectedTransaction->utilities[positionProjected];
								positionPrevious ++;
								positionProjected ++;
							}

							// make also the sum of transaction utility and prefix utility
							previousTransaction->transactionUtility += projectedTransaction->transactionUtility;
							previousTransaction->prefixUtility += projectedTransaction->prefixUtility;	

#ifdef Delete_The_New_Object
							//20161113 add
							delete projectedTransaction;
							projectedTransaction = NULL;

							curNewTransactionCounts --;

#ifdef Counts_The_New_Number
							curNewCounts --;
#endif

#endif
						}
						// increment the number of consecutive transaction merged
						consecutiveMergeCount ++;
					}else
					{
						// if the transaction is not equal to the preceding transaction
						// we cannot merge it so we just add it to the database
						
						transactionsPe.push_back(previousTransaction);
						// the transaction becomes the previous transaction
						previousTransaction = projectedTransaction;
						consecutiveMergeCount = 0;
					}
				}
				// This is an optimization for binary search:
				// we remember the position of E so that for the next item, we will not search
				// before "e" in the transaction since items are visited in lexicographical order
				curTransaction->offset = positionE;   
			}else{
				// This is an optimization for binary search:
				// we remember the position of E so that for the next item, we will not search
				// before "e" in the transaction since items are visited in lexicographical order
				curTransaction->offset = low;
			}
		}

		double te1 = clock();

		mergeTimes += (te1 - ts1) / CLOCKS_PER_SEC;

		if (previousTransaction != NULL)
		{

			transactionsPe.push_back(previousTransaction);
		}

//		temp[prefixLength] = newNameToOldName[e];

		if (utilityPe >= absoluteValue)
		{
			huiCounts ++;
		}

		// calculate the local utility and sub-tree utility
		useUtilityBinArraysToCalculateUpperBounds(transactionsPe, j, bitemToKeep);

		// We will create the new list of secondary items
		std::vector<int> newItemsToKeep;
		// We will create the new list of primary items
		std::vector<int> newItemsToExplore;

		// for each item
		for (int k = j + 1; k != bitemToKeep.size(); k ++) 
		{
			int itemk =  bitemToKeep[k];

			// if the sub-tree utility is no less than min util
			if(utilityBinArraySU[itemk] >= absoluteValue) 
			{
				// and if sub-tree utility pruning is activated
				newItemsToExplore.push_back(itemk);

				// consider that item as a secondary item
				newItemsToKeep.push_back(itemk);
			}else if(utilityBinArrayLU[itemk] >= absoluteValue)
			{
				// otherwise, if local utility is no less than minutil,
				// consider this itemt to be a secondary item
				newItemsToKeep.push_back(itemk);
			}
		}

		//20161112 add
// 		if (newItemsToExplore.empty())
// 		{
// 			for (int index = 0; index != transactionsPe.size(); ++ index)
// 			{
// 				delete transactionsPe[index];
// 				transactionsPe[index] = NULL;
// 
// #ifdef Counts_The_New_Number
// 				curNewCounts --;
// #endif
// 
// 			}
// 			continue;
// 		}

		backtrackingEFIM(transactionsPe, newItemsToKeep, newItemsToExplore, prefixLength + 1);


	}

	if (countsFlag == false)
	{
		for (int index = 0; index != curAllTransactionsOfP.size(); ++ index)
		{
			//std::cout << typeid(curAllTransactionsOfP[index]).name() << std::endl;
			delete curAllTransactionsOfP[index];
			curAllTransactionsOfP[index] = NULL;

			curNewTransactionCounts --;

#ifdef Counts_The_New_Number
			curNewCounts --;
#endif

		}
	}
}

void EFIMAlgorithm::useUtilityBinArraysToCalculateUpperBounds(std::vector<Transaction *> &transactionPe, int j, std::vector<int> uitemsToKeep)
{
	// For each promising item > e according to the total order
	for (int i = j + 1; i != uitemsToKeep.size(); i ++) 
	{
		int item = uitemsToKeep[i];
		// We reset the utility bins of that item for computing the sub-tree utility and
		// local utility
		utilityBinArraySU[item] = 0.0;
		utilityBinArrayLU[item] = 0.0;
	}

	double sumRemainingUtility;
	// for each transaction
	for (int index = 0; index  != transactionPe.size(); ++ index) 
	{
		Transaction *transaction = transactionPe[index];
		// count the number of transactions read
		//transactionReadingCount++;

		// We reset the sum of reamining utility to 0;
		sumRemainingUtility = 0;
		// we set high to the last promising item for doing the binary search
		int high = uitemsToKeep.size() - 1;

		// for each item in the transaction that is greater than i when reading the transaction backward
		// Note: >= is correct here-> It should not be >->
		for (int i = transaction->itemsLength - 1; i >= transaction->offset; i --) 
		{
			// get the item
			int item = transaction->items[i];

			// We will check if this item is promising using a binary search over promising items->

			// This variable will be used as a flag to indicate that we found the item or not using the binary search
			bool contains = false;
			// we set "low" for the binary search to the first promising item position
			int low = 0;

			// do the binary search
			while (high >= low) 
			{
				int middle = (low + high) >> 1; // divide by 2
				int itemMiddle = uitemsToKeep[middle];
				if (itemMiddle == item) 
				{
					// if we found the item, then we stop
					contains = true;
					break;
				} else if (itemMiddle < item) 
				{
					low = middle + 1;
				} else 
				{
					high = middle - 1;
				}  
			}
			// if the item is promising
			if (contains) 
			{
				// We add the utility of this item to the sum of remaining utility
				sumRemainingUtility += transaction->utilities[i];
				// We update the sub-tree utility of that item in its utility-bin
				utilityBinArraySU[item] += sumRemainingUtility + transaction->prefixUtility;
				// We update the local utility of that item in its utility-bin
				utilityBinArrayLU[item] += transaction->transactionUtility + transaction->prefixUtility;
			}
		}
	}
}

bool EFIMAlgorithm::isEqualTo(Transaction *t1, Transaction *t2) 
{
	// we first compare the transaction lenghts
	int length1 = t1->itemsLength - t1->offset;
	int length2 = t2->itemsLength - t2->offset;
	// if not same length, then transactions are not identical
	if(length1 != length2)
	{
		return false;
	}
	// if same length, we need to compare each element position by position,
	// to see if they are the same
	int position1 = t1->offset;
	int position2 = t2->offset;

	// for each position in the first transaction
	while(position1 < t1->itemsLength)
	{
		// if different from corresponding position in transaction 2
		// return false because they are not identical
		if(t1->items[position1]  != t2->items[position2])
		{
			return false;
		}
		// if the same, then move to next position
		position1 ++;
		position2 ++;
	}
	// if all items are identical, then return to true
	return true;
}

void EFIMAlgorithm::printResult()
{
	time_t t = time(0); 
	char tmp[32]={NULL};
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&t));

	char machinename[ 1024 ];

#ifdef  Linux
	struct utsname u;
	uname(&u); // cout << u.sysname << u.release << u.machine << u.nodename << endl;
	strcpy(machinename, u.nodename);

#else
	size_t requiredSize;
	getenv_s( &requiredSize, machinename, 1024, "computername" );

#endif

	std::ofstream ofs(resultFileName.c_str(), std::ios::app);

	ofs << "Machine\t"
		<< "Algo\t"
		<< "Trans_Quan_File\t"
		<< "Item_Price_File\t"
		<< "absMinUtil\t"
		<< "relMinUtil\t"
		<< "candidates\t"
		<< "#UtilPatterns\t"
		<< "init time(s)\t"
		<< "merge time(s)\t"
		<< "mining time(s)\t"
		<< "time(s)\t"
		<< "dbsize\t"
		<< "maxWidth\t"
		<< "numItemsDistinct\t"
		<< "max items number\t"
		<< "sizeof item(Byte)\t"
		<< "max utility number\t"
		<< "sizeof utility(Byte)\t"
		<< "max transaction number\t"
		<< "sizeof transaction *(Byte)\t"
		<< "total memory number(MB)"
		<< "when_starting\n";


	ofs << machinename << "\t";
	ofs << "EFIM\t";
	ofs	<< dataBaseName << "\t"
		<< priceFileName << "\t"
		<< absoluteValue << "\t"
		<< relativeValue << "\t"
		<< candidatesCounts << "\t"
		<< huiCounts << "\t"
		<< initTimes << "\t"
		<< mergeTimes << "\t"
		<< miningTimes << "\t"
		<< totalTimes << "\t"
		<< transactionCounts << "\t"
		<< transactionWeight << "\t"
		<< itemCounts << "\t"
		<< maxNewItemsCounts << "\t"
		<< sizeof(int) << "\t"
		<< maxNewUtilitiesCounts << "\t"
		<< sizeof(double) << "\t"
		<< maxNewTransactionCounts << "\t"
		<< sizeof(Transaction *) << "\t"
		<< (maxNewItemsCounts * sizeof(int) + maxNewUtilitiesCounts * sizeof(double) + maxNewTransactionCounts * sizeof(Transaction *)) / 1024.0 / 1024.0 << "\t"
		<< tmp << "\n";
	ofs.close();
}