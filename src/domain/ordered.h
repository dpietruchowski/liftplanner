#pragma once

#include <vector>

namespace Ordered
{

template <typename T> void renumber(std::vector<T>& items)
{
    for (size_t i = 0; i < items.size(); ++i)
        items[i].setPosition(static_cast<int>(i));
}

template <typename T> void insert(std::vector<T>& items, const T& item, int atPosition)
{
    const int size = static_cast<int>(items.size());
    const int index = (atPosition < 0 || atPosition > size) ? size : atPosition;

    items.insert(items.begin() + index, item);
    renumber(items);
}

template <typename T> void remove(std::vector<T>& items, int index)
{
    if (index < 0 || index >= static_cast<int>(items.size()))
        return;

    items.erase(items.begin() + index);
    renumber(items);
}

template <typename T> void move(std::vector<T>& items, int from, int to)
{
    const int size = static_cast<int>(items.size());
    if (from < 0 || from >= size || to < 0 || to >= size || from == to)
        return;

    T moved = items[from];
    items.erase(items.begin() + from);
    items.insert(items.begin() + to, moved);
    renumber(items);
}

}
