#include "node.h"

acmvcc::UndoLogEntryNode::UndoLogEntryNode(uint64_t trxId, uint64_t spaceId, uint64_t pageId, uint64_t offset)
{
	this->trxId = trxId;
	this->spaceId = spaceId;
	this->pageId = pageId;
	this->offset = offset;
}

acmvcc::EpochNode::EpochNode(uint64_t epochNumber)
{
	this->epochNumber = epochNumber;
}
