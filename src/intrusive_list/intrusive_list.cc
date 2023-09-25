#include "intrusive_list/intrusive_list.hh"

using namespace ljit;

void IntrusiveListNode::insertBefore(IntrusiveListNode &pos) noexcept
{
  setNext(&pos);

  auto *const newPrev = pos.getPrev();
  setPrev(newPrev);

  if (newPrev != nullptr)
    newPrev->setNext(this);

  pos.setPrev(this);
}

void IntrusiveListNode::insertAfter(IntrusiveListNode &pos) noexcept
{
  setPrev(&pos);

  auto *const newNext = pos.getNext();
  setNext(newNext);

  if (newNext != nullptr)
    newNext->setPrev(this);

  pos.setNext(this);
}
