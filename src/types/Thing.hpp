#pragma once

#include <string>
#include <utility>

class Thing
{
public:
    explicit Thing(std::string id) : m_id(std::move(id)) {}

    const std::string &id() const { return m_id; }

    std::string outTopic() const { return "/a/t/" + m_id + "/e/o"; }

private:
    std::string m_id;
};
