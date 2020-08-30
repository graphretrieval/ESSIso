#include <cassert>

#include"FrequentPatternGroup.h"
#include"fptree.hpp"
#include<iostream>

void FrequentPatternGroup::frequentPatternGroup(std::vector<AdjacenceListsGRAPH> & queryGraphVector, std::vector<std::vector<int>> & similiarQueryGroups)
{

	std::map<string, vector<int>> TLSInvertedIndex;
	std::vector<Transaction> transactions;

	for (std::vector<AdjacenceListsGRAPH>::iterator queryIterator = queryGraphVector.begin(); queryIterator != queryGraphVector.end(); queryIterator++) {
		// each query is treated as a transaction
		transactions.push_back(std::vector<string>());
		Transaction & newTransaction = transactions[transactions.size() - 1];

		for (std::map<TLSequence, std::vector<std::vector<int>>>::iterator tlsSeqIterator = queryIterator->getTLSequenceMap()->begin(); tlsSeqIterator != queryIterator->getTLSequenceMap()->end(); tlsSeqIterator++) {
			string tlsQuanti = std::to_string(tlsSeqIterator->first.start);
			tlsQuanti += "_";
			tlsQuanti += std::to_string(tlsSeqIterator->first.pivot);
			tlsQuanti += "_";
			tlsQuanti += std::to_string(tlsSeqIterator->first.end);
			tlsQuanti += "_";
			for (size_t i = 0; i < tlsSeqIterator->second.size(); i++) {
				string tlsIndexKey = tlsQuanti + std::to_string(i);
				std::map<string, vector<int>>::iterator invertexIndexIter = TLSInvertedIndex.find(tlsIndexKey);
				if (invertexIndexIter == TLSInvertedIndex.end()) {
					vector<int> tlsIndexValue;
					tlsIndexValue.push_back(queryIterator->graphId);
					TLSInvertedIndex.insert(std::pair<string, vector<int>>(tlsIndexKey, tlsIndexValue));
				}
				else {
					invertexIndexIter->second.push_back(queryIterator->graphId);
				}

				// we use a format i.e. "start_pivot_end_1" to distinuish each tls, thus we can can consider the number of the same tls of each query graph
				newTransaction.push_back(tlsIndexKey);
			}
		}
	}

	const unsigned minimum_support_treshold = 2;

	const FPTree fptree{ transactions, minimum_support_treshold };

	std::set<Pattern> patterns = fptree_growth(fptree);


	cout << "Frequent TLS Size: " << patterns.size() << endl;
	for (std::set<Pattern>::iterator patternIterator = patterns.begin(); patternIterator != patterns.end(); patternIterator++) {
		cout << patternIterator->second << " ";
		for (std::set<Item>::iterator itemIterator = patternIterator->first.begin(); itemIterator != patternIterator->first.end(); itemIterator++) {
			cout<<(*itemIterator) << " ";
		}
		cout << endl;
	}
}
