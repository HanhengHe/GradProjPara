#include <list>
#include <iostream>
#include "doublelinkedlist.h"

int main(int argc, char *argv[])
{
    DoublyLinkedList<int> list;

    list.push_front(17);
    list.push_front(15);
    list.push_front(10);
    list.push_front(8);

    for (auto it = list.begin(); it != list.end(); ++it)
    {
        std::cout << *it << std::endl;
    }
}