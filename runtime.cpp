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
    if (this->HasMethod("__str__"s, 1))
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

Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
    : name_(std::move(name)), methods_(std::move(methods)), parent_(std::move(parent))
{
    // ��������� �������� ����������� ������� ��� Mython �������

    // ������� �������� � ������� ����������� ������� ������ ��������, ���� �� ����
    if (parent_ != nullptr)
    {
        for (const auto& parent_method : parent_->methods_)
        {
            vftable_[parent_method.name] = &parent_method;
        }
    }

    // ������ �������� � ������� ����������� ������� ����������� ������ ������.
    // ���� ������� ��� �������� ������ ��������, ������ � ����������� �������
    // ����� ������������ �������� �������� �������.
    for (const auto& method : methods_)
    {
        vftable_[method.name] = &method;
    }
}

const Method* Class::GetMethod(const std::string& name) const
{
    // ���� ����� � ����������� �������
    if (vftable_.count(name) != 0)
    {
        return vftable_.at(name);
    }

    // ������ ������ � ����������� ������� ���
    return nullptr;
}

[[nodiscard]] inline const std::string& Class::GetName() const
{
    return name_;
}

void Class::Print(ostream& os, Context& context)
{
    os << "Class "sv << this->GetName();
}

void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context)
{
    os << (GetValue() ? "True"sv : "False"sv);
}


//////////////////////////////////////////////
// ������� ��������� �������� Mython ���������

bool Equal(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context)
{
    // 1. ���� lhs � rhs ����� �������� None, ������� ���������� true.
    // operator bool() ��������� ���������� shared_ptr �� != nullptr
    // �������� None �������������� ��������� bool()
    if (!lhs && !rhs)
    {
        return true;
    }

    // 2. ���������� true, ���� lhs � rhs �������� ���������� �����, ������ ��� �������� ���� Bool.
    // ��������� Value-���� Mython. � ��� ��� ���������� ������ ValueObject<T>
    {
        auto lhs_ptr = lhs.TryAs<Number>();
        auto rhs_ptr = rhs.TryAs<Number>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() == rhs_ptr->GetValue();
        }
        //else ������ ����, ������ �� ������, ��������� ������
    }

    {
        auto lhs_ptr = lhs.TryAs<String>();
        auto rhs_ptr = rhs.TryAs<String>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() == rhs_ptr->GetValue();
        }
        //else ������ ����, ������ �� ������, ��������� ������
    }

    {
        auto lhs_ptr = lhs.TryAs<Bool>();
        auto rhs_ptr = rhs.TryAs<Bool>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() == rhs_ptr->GetValue();
        }
        //else ������ ����, ������ �� ������, ��������� ������
    }

    // 3. ���� lhs - ������ � ������� __eq__, ������� ���������� ��������� 
    //    ������ lhs.__eq__(rhs), ���������� � ���� Bool.
    {
        auto lhs_ptr = lhs.TryAs<ClassInstance>();
        if (lhs_ptr != nullptr)
        {
            if (lhs_ptr->HasMethod("__eq__"s, 1))
            {
                ObjectHolder result = lhs_ptr->Call("__eq__"s, { rhs }, context);
                return result.TryAs<Bool>()->GetValue();
            }
        }
    }

    // 4. � ��������� ������� ������� ����������� ���������� runtime_error.
    throw std::runtime_error("Cannot compare objects for equality"s);
}


bool Less(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context)
{
    // 1. ���� lhs � rhs - �����, ������ ��� �������� bool, �������
    // ���������� ��������� �� ��������� ���������� <
    // ��������� Value-���� Mython. � ��� ��� ���������� ������ ValueObject<T>
    {
        auto lhs_ptr = lhs.TryAs<Number>();
        auto rhs_ptr = rhs.TryAs<Number>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() < rhs_ptr->GetValue();
        }
        //else ������ ����, ������ �� ������, ��������� ������
    }

    {
        auto lhs_ptr = lhs.TryAs<String>();
        auto rhs_ptr = rhs.TryAs<String>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() < rhs_ptr->GetValue();
        }
        //else ������ ����, ������ �� ������, ��������� ������
    }

    {
        auto lhs_ptr = lhs.TryAs<Bool>();
        auto rhs_ptr = rhs.TryAs<Bool>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() < rhs_ptr->GetValue();
        }
        //else ������ ����, ������ �� ������, ��������� ������
    }

    // 2. ���� lhs - ������ � ������� __lt__, ���������� ���������
    //  ������ lhs.__lt__(rhs), ���������� � ���� bool
    {
        auto lhs_ptr = lhs.TryAs<ClassInstance>();
        if (lhs_ptr != nullptr)
        {
            if (lhs_ptr->HasMethod("__lt__"s, 1))
            {
                ObjectHolder result = lhs_ptr->Call("__lt__"s, { rhs }, context);
                return result.TryAs<Bool>()->GetValue();
            }
        }
    }

    // 3. � ��������� ������� ������� ����������� ���������� runtime_error.
    throw std::runtime_error("Cannot compare objects for less"s);
}

bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context)
{
    return !Equal(lhs, rhs, context);
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