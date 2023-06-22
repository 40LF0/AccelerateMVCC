#include "epochList.h"
#include "common.h"

namespace acmvcc
{
	/**When insert epochList pointer to hashMap, we have to do CAS operation.
	* If CAS succeed(return : null value), new epochList is successfuly inserted to hashmap.
	* If CAS fail(return : not null value), epochList is already inserted.
	*  - Free useless epochList and try to insert undoLogEntry to existing epochList.
	*/
	EpochList::EpochList(uint64_t firstTrxId)
	{
		this->epochSize = epochSize;
		EpochNode* epochNode = new EpochNode(firstTrxId / EPOCH_SIZE);
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
