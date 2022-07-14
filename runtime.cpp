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
    // Возвращаем невладеющий shared_ptr (его deleter ничего не делает)
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
    // Сначала проверим саму ссылку
    if (!object)
    {
        return false;
    }

    // Проверяем Value-типы. У нас это наследники класса ValueObject<T>
    if (((object.TryAs<Number>() != nullptr) && (object.TryAs<Number>()->GetValue() != 0))    // если object это Number и не ноль
        || ((object.TryAs<Bool>() != nullptr) && (object.TryAs<Bool>()->GetValue() == true))    //  если object это Bool и == true
        || ((object.TryAs<String>() != nullptr) && (object.TryAs<String>()->GetValue().size() != 0)))   // если object - не пустая строка
    {
        return true;
    }

    // Во всех остальных случаях возвращаем false
    return false;
}

void ClassInstance::Print(std::ostream& os, Context& context)
{
    using namespace std::string_literals;

    // Заглушка, реализуйте метод самостоятельно
    if (this->HasMethod("__str__"s, 1))
    {
        //os << this->Call("__str__"s, {}, context);
    }
}

bool ClassInstance::HasMethod(const std::string& /*method*/, size_t /*argument_count*/) const
{
    // Заглушка, реализуйте метод самостоятельно
    return false;
}

Closure& ClassInstance::Fields()
{
    // Заглушка. Реализуйте метод самостоятельно
    throw std::logic_error("Not implemented"s);
}

const Closure& ClassInstance::Fields() const
{
    // Заглушка. Реализуйте метод самостоятельно
    throw std::logic_error("Not implemented"s);
}

ClassInstance::ClassInstance(const Class& /*cls*/)
{
    // Реализуйте метод самостоятельно
}

ObjectHolder ClassInstance::Call(const std::string& method,
                                 const std::vector<ObjectHolder>& actual_args,
                                 Context& context)
{
    // Заглушка. Реализуйте метод самостоятельно.
    throw std::runtime_error("Not implemented"s);
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
    : name_(std::move(name)), methods_(std::move(methods)), parent_(std::move(parent))
{
    // Реализуем механизм виртуальных функций для Mython классов

    // Сначала запомним в таблице виртуальных функций методы родителя, если он есть
    if (parent_ != nullptr)
    {
        for (const auto& parent_method : parent_->methods_)
        {
            vftable_[parent_method.name] = &parent_method;
        }
    }

    // Теперь поместим в таблицу виртуальных функций собственные методы класса.
    // Если таблица уже содержит методы родителя, методы с одинаковыми именами
    // будут перезаписаны адресами дочерних методов.
    for (const auto& method : methods_)
    {
        vftable_[method.name] = &method;
    }
}

const Method* Class::GetMethod(const std::string& name) const
{
    // Ищем метод в виртуальной таблице
    if (vftable_.count(name) != 0)
    {
        return vftable_.at(name);
    }

    // Такого метода в виртуальной таблице нет
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
// Функции сравнения объектов Mython программы

bool Equal(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context)
{
    // 1. Если lhs и rhs имеют значение None, функция возвращает true.
    // operator bool() проверяет внутренний shared_ptr на != nullptr
    // Значение None противоположно оператору bool()
    if (!lhs && !rhs)
    {
        return true;
    }

    // 2. Возвращает true, если lhs и rhs содержат одинаковые числа, строки или значения типа Bool.
    // Проверяем Value-типы Mython. У нас это наследники класса ValueObject<T>
    {
        auto lhs_ptr = lhs.TryAs<Number>();
        auto rhs_ptr = rhs.TryAs<Number>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() == rhs_ptr->GetValue();
        }
        //else разные типы, ничего не делаем, проверяем дальше
    }

    {
        auto lhs_ptr = lhs.TryAs<String>();
        auto rhs_ptr = rhs.TryAs<String>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() == rhs_ptr->GetValue();
        }
        //else разные типы, ничего не делаем, проверяем дальше
    }

    {
        auto lhs_ptr = lhs.TryAs<Bool>();
        auto rhs_ptr = rhs.TryAs<Bool>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() == rhs_ptr->GetValue();
        }
        //else разные типы, ничего не делаем, проверяем дальше
    }

    // 3. Если lhs - объект с методом __eq__, функция возвращает результат 
    //    вызова lhs.__eq__(rhs), приведённый к типу Bool.
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

    // 4. В остальных случаях функция выбрасывает исключение runtime_error.
    throw std::runtime_error("Cannot compare objects for equality"s);
}


bool Less(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context)
{
    // 1. Если lhs и rhs - числа, строки или значения bool, функция
    // возвращает результат их сравнения оператором <
    // Проверяем Value-типы Mython. У нас это наследники класса ValueObject<T>
    {
        auto lhs_ptr = lhs.TryAs<Number>();
        auto rhs_ptr = rhs.TryAs<Number>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() < rhs_ptr->GetValue();
        }
        //else разные типы, ничего не делаем, проверяем дальше
    }

    {
        auto lhs_ptr = lhs.TryAs<String>();
        auto rhs_ptr = rhs.TryAs<String>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() < rhs_ptr->GetValue();
        }
        //else разные типы, ничего не делаем, проверяем дальше
    }

    {
        auto lhs_ptr = lhs.TryAs<Bool>();
        auto rhs_ptr = rhs.TryAs<Bool>();
        if ((lhs_ptr != nullptr) && (rhs_ptr != nullptr))
        {
            return lhs_ptr->GetValue() < rhs_ptr->GetValue();
        }
        //else разные типы, ничего не делаем, проверяем дальше
    }

    // 2. Если lhs - объект с методом __lt__, возвращает результат
    //  вызова lhs.__lt__(rhs), приведённый к типу bool
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

    // 3. В остальных случаях функция выбрасывает исключение runtime_error.
    throw std::runtime_error("Cannot compare objects for less"s);
}

bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context)
{
    return !Equal(lhs, rhs, context);
}

bool Greater(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool LessOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool GreaterOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/)
{
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

}  // namespace runtime