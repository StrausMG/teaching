#include <iostream>
#include <utility>

class Person {
public:
    Person() = default;

    explicit Person(const std::string& name) : name_{name} {}

    Person(const Person& other) : name_{other.name_ + "_copy"} {
        std::cout << "constructed " << name_ << " from " << other.name_ << '\n';
    }

    Person(Person&& other) : name_{std::move(other.name_)} {
        std::cout << "Person(Person&& other)\n";
    }

    std::string GetName() const {
        return name_;
    }

private:
    const std::string name_;
};

Person MakePerson(const std::string& name) {
    return Person(name);
}

class Student : public Person {
public:
    Student(const Person& p) : name_{p.GetName()} {
        std::cout << "student from person " << name_ << '\n';
    }

    Student(Person&& other) : Person{std::move(other)} {
        std::cout << "Student(Person&& other)\n";
    }

    Student(
        const std::string& first_name,
        const std::string& last_name
    )
        : name_{first_name + " " + last_name}
    {}

private:
    const std::string name_;
};

// The goal is to write "MakeUnique<Student>(some_other_person);"

// template<typename T, typename A>
// std::unique_ptr<T> MakeUnique(A arg) {
//     return std::unique_ptr<T>(new T(arg));
// }

// template<typename T, typename A>
// std::unique_ptr<T> MakeUnique(A& arg) {
//     return std::unique_ptr<T>(new T(arg));
// }

// template<typename T, typename A>
// std::unique_ptr<T> MakeUnique(const A& arg) {
//     return std::unique_ptr<T>(new T(arg));
// }

// =================================================================================================
// UNIVERSAL REFERENCE
// (firstly let's learn how to distinguish between rvalue-references and universal references)
// Then after examples I'll explain what it is.
// If you see SomeType&& and "SomeType" is a template parameter
// (i.e. it is in triangle brackets after the word "template")
// then SomeType&& is a universal reference.

// Examples:

// "std::vector<int>" is a fully known type, so "std::vector<int>&&"" is just a rvalue-reference
void foo(std::vector<int>&& v) {}

// "std::vector<T>" is not in triangle brackets! Only "T" is in triangle brackets!
// So "std::vector<T>&&" is a usual rvalue-reference again.
template<typename T>
void foo(std::vector<T>&& v) {}

// "T" is a template parameter of the function. So "T&&" is a universal reference.
template<typename T>
void foo(T&& v) {}

// So what is a universal reference? It's easier to show an example:
template<typename T>
void bar(T&& arg) {}
// If you pass int-lvalue as the argument "arg",
// then "T&& arg" transforms to "int& arg", i.e. lvalue-reference.
// If you pass int-rvalue as the argument "arg",
// then "T&& arg" transforms to "int&& arg", i.e. rvalue-reference.
// So, that's how a universal reference works:
// it instantiates as lvalue-refence for lvalues and as rvalue-reference for rvalues.

// REFERENCE COLLAPSING RULES
// A&  &  ===> A&
// A&  && ===> A&
// A&& &  ===> A&
// A&& && ===> A&&
// In fact, you can explain universal references using these rules

template<typename T>
// Here in the return type REFERENCE COLLAPSING RULES are apllied
T&& forward(typename std::remove_reference<T>::type& a) noexcept {
    // Here in the triangle brackets of static_cast REFERENCE COLLAPSING RULES are apllied
    return static_cast<T&&>(a);
}

// =================================================================================================
// Now we can write a solution.

template<typename T, typename A>
std::unique_ptr<T> MakeUnique(A&& arg) {
    return std::unique_ptr<T>(new T(std::forward<A>(arg)));
}

// template<typename T, typename... Args>
// std::unique_ptr<T> MakeUnique(Args&&... args) {
//     return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
// }

int main() {
    Person a("Antosha");
    const auto a_ptr = MakeUnique<Student>(a);

    const auto v_ptr = MakeUnique<Student>(MakePerson("Vitalik"));

    // const auto d_ptr = MakeUnique<Student>("Donald", "Duck");

    return 0;
}
