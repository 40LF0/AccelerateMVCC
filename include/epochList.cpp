#include "epochList.h"


namespace acmvcc
{
	EpochList::EpochList(uint64_t epochSize, uint64_t firstTrxId)
	{
		this->epochSize = epochSize;
		EpochNode* epochNode = new EpochNode(firstTrxId / epochSize);
		nextEpoch = epochNode;
	}

	bool EpochList::search(uint64_t trxId)
	{
		return false;
	}

	bool EpochList::insert(uint64_t trxId, bool information)
	{
		return false;
	}

} // namespace acmvcc
