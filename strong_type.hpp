#pragma once

template<typename UnderlingType, typename Tag>
class StrongType
{
public:
    explicit StrongType(const UnderlingType& val) : m_val{val} {}
    UnderlingType get() const { return m_val; }
    bool operator==(const StrongType<UnderlingType, Tag>& other) const { return this->m_val == other.m_val; }

private:
    UnderlingType m_val;
};
