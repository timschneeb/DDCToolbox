#ifndef ENUMTYPE_H
#define ENUMTYPE_H
template <typename TYPE>
class EnumClass {
  private:
    TYPE value_;
  public:
    explicit constexpr EnumClass(TYPE value) :
        value_(value){
    }
    constexpr EnumClass() = default;
    ~EnumClass() = default;
    constexpr explicit EnumClass(const EnumClass &) = default;
    constexpr EnumClass &operator=(const EnumClass &) = default;

    constexpr operator TYPE() const {return    value_;}
    constexpr TYPE value() const {return value_;}

};
#endif // ENUMTYPE_H
