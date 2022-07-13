#include "runtime.h"

#include <cassert>
#include <optional>
#include <sstream>

using namespace std;

namespace runtime
{

ObjectHolder::ObjectHolder(std::shared_ptr<Object> data)
    : data_(std::move(data))
{}

void ObjectHolder::AssertIsValid() const
{
    assert(data_ != nullptr);
}

ObjectHolder ObjectHolder::Share(Object& object)
{
    // ���������� ����������� shared_ptr (��� deleter ������ �� ������)
    return ObjectHolder(std::shared_ptr<Object>(&object, [](auto* /*p*/)
                                                { /* do nothing */
                                                }));
}

ObjectHolder ObjectHolder::None()
{
    return ObjectHolder();
}

Object& ObjectHolder::operator*() const
{
    AssertIsValid();
    return *Get();
}

Object* ObjectHolder::operator->() const
{
    AssertIsValid();
    return Get();
}

Object* ObjectHolder::Get() const
{
    return data_.get();
}

ObjectHolder::operator bool() const
{
    return Get() != nullptr;
}

bool IsTrue(const ObjectHolder& object)
{
    // ������� �������� ���� ������
    if (!object)
    {
        return false;
    }

    // ��������� Value-����. � ��� ��� ���������� ������ ValueObject<T>
    if (((object.TryAs<Number>() != nullptr) && (object.TryAs<Number>()->GetValue() != 0))    // ���� object ��� Number � �� ����
        || ((object.TryAs<Bool>() != nullptr) && (object.TryAs<Bool>()->GetValue() == true))    //  ���� object ��� Bool � == true
        || ((object.TryAs<String>() != nullptr) && (object.TryAs<String>()->GetValue().size() != 0)))   // ���� object - �� ������ ������
    {
        return true;
    }

    // �� ���� ��������� ������� ���������� false
    return false;
}

void ClassInstance::Print(std::ostream& os, Context& context)
{
    using namespace std::string_literals;

    // ��������, ���������� ����� ��������������
    if (this->HasMethod("__str__"s, 0))
    {
        //os << this->Call("__str__"s, {}, context);
    }
}

bool ClassInstance::HasMethod(const std::string& /*method*/, size_t /*argument_count*/) const
{
    // ��������, ���������� ����� ��������������
    return false;
}

Closure& ClassInstance::Fields()
{
    // ��������. ���������� ����� ��������������
    throw std::logic_error("Not implemented"s);
}

const Closure& ClassInstance::Fields() const
{
    // ��������. ���������� ����� ��������������
    throw std::logic_error("Not implemented"s);
}

ClassInstance::ClassInstance(const Class& /*cls*/)
{
    // ���������� ����� ��������������
}

ObjectHolder ClassInstance::Call(const std::string& method,
                                 const std::vector<ObjectHolder>& actual_args,
                                 Context& context)
{
    // ��������. ���������� ����� ��������������.
    throw std::runtime_error("Not implemented"s);
}

Class::Class(std::string /*name*/, std::vector<Method> /*methods*/, const Class* /*parent*/)
{
    // ���������� ����� ��������������
}

const Method* Class::GetMethod(const std::string& /*name*/) const
{
    // ��������. ���������� ����� ��������������
    return nullptr;
}

[[nodiscard]] inline const std::string& Class::GetName() const
{
    // ��������. ���������� ����� ��������������.
    throw std::runtime_error("Not implemented"s);
}

void Class::Print(ostream& /*os*/, Context& /*context*/)
{
    // ��������. ���������� ����� ��������������
}

void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context)
{
    os << (GetValue() ? "True"sv : "False"sv);
}

bool Equal(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // ��������. ���������� ������� ��������������
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool Less(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // ��������. ���������� ������� ��������������
    throw std::runtime_error("Cannot compare objects for less"s);
}

bool NotEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // ��������. ���������� ������� ��������������
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool Greater(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // ��������. ���������� ������� ��������������
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool LessOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // ��������. ���������� ������� ��������������
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool GreaterOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // ��������. ���������� ������� ��������������
    throw std::runtime_error("Cannot compare objects for equality"s);
}

}  // namespace runtime