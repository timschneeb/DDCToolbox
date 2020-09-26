#ifndef FILTERTYPE_H
#define FILTERTYPE_H

#include <map>
#include <string>
#include <QString>

class FilterType
{
public:
  enum Value : uint8_t
  {
      PEAKING = 0x00,
      LOW_PASS,
      HIGH_PASS,
      BAND_PASS1,
      BAND_PASS2,
      NOTCH,
      ALL_PASS,
      LOW_SHELF,
      HIGH_SHELF,
      UNITY_GAIN,
      ONEPOLE_LOWPASS,
      ONEPOLE_HIGHPASS,
      CUSTOM,
      INVALID = 0xFF
  };
  using StringMap = std::pair<Value, const char*>;

  FilterType() = default;
  FilterType(Value a) : value(a) { }
  FilterType(uint8_t a) : value(static_cast<Value>(a)) { }
  FilterType(const char* a) : value(findKey(a)) { }
  FilterType(const QString& a) : value(findKey(a.toLocal8Bit().constData())) { }

  operator Value() const { return value; }
  explicit operator bool() = delete;

  constexpr bool operator==(const FilterType& a) const { return value == a.value; }
  constexpr bool operator!=(const FilterType& a) const { return value != a.value; }
  constexpr bool operator==(const FilterType::Value& a) const { return value == a; }
  constexpr bool operator!=(const FilterType::Value& a) const { return value != a; }

  operator const char*() const {
      return findValue(value);
  }
  operator QString() const {
      return QString::fromStdString(findValue(value));
  }

  constexpr static StringMap string_map[] = {
        {PEAKING, "Peaking"},
        {LOW_PASS, "Low Pass"},
        {HIGH_PASS, "High Pass"},
        {BAND_PASS1, "Band Pass"},
        {BAND_PASS2, "Band Pass (peak gain = bw)"},
        {NOTCH, "Notch"},
        {ALL_PASS, "All Pass"},
        {LOW_SHELF, "Low Shelf"},
        {HIGH_SHELF, "High Shelf"},
        {UNITY_GAIN, "Unity Gain"},
        {ONEPOLE_LOWPASS, "One-Pole Low Pass"},
        {ONEPOLE_HIGHPASS, "One-Pole High Pass"},
        {CUSTOM, "Custom"},
        {INVALID, "Invalid"},
  };
  constexpr static auto string_map_size = sizeof string_map/sizeof string_map[0];

private:
  Value value;

  static constexpr const char* findValue(Value key) {
      return string_map[key].second;
  };

  static Value findKey(const char* value) {
      for(const auto& it : string_map){
          if(strcmp(it.second, value) == 0)
              return it.first;
      }
      return INVALID;
  };

};
#endif // FILTERTYPE_H
