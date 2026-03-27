#pragma once

#include <optional>
#include <vector>

class Set;
class SetQuery;

class SetRepository
{
public:
    virtual ~SetRepository() = default;

    virtual std::vector<Set> findAll(const SetQuery &query) const = 0;
    virtual std::optional<Set> findOne(const SetQuery &query) const = 0;
    virtual int save(const Set &set) = 0;
    virtual bool remove(const SetQuery &query) = 0;
    virtual int count(const SetQuery &query) const = 0;
    virtual bool exists(const SetQuery &query) const = 0;
};
