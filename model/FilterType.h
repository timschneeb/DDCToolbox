#ifndef FILTERTYPE_H
#define FILTERTYPE_H

#include <map>
#include <bitset>
#include <string>
#include <QString>

#include "utils/BitFlags.h"

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

    enum SpecFlag : uint8_t
    {
        SPEC_NO_FLAG        = 1 << 0,
        SPEC_REQUIRE_FREQ   = 1 << 1,
        SPEC_REQUIRE_BW     = 1 << 2,
        SPEC_REQUIRE_SLOPE  = 1 << 3,
        SPEC_REQUIRE_GAIN   = 1 << 4
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

    constexpr int ordinal() const {
        return value;
    }

    operator const char*() const {
        return findValue(value);
    }
    operator QString() const {
        return QString::fromStdString(findValue(value));
    }

    BitFlags<SpecFlag> getSpecs() const{
        BitFlags<SpecFlag> specs(SPEC_NO_FLAG);

        switch(value){
        case PEAKING:
            specs.set(SPEC_REQUIRE_FREQ | SPEC_REQUIRE_BW | SPEC_REQUIRE_GAIN);
            break;
        case LOW_SHELF:
        case HIGH_SHELF:
            specs.set(SPEC_REQUIRE_FREQ | SPEC_REQUIRE_SLOPE | SPEC_REQUIRE_GAIN);
            break;
        case UNITY_GAIN:
            specs.set(SPEC_REQUIRE_GAIN);
            break;
        case ONEPOLE_LOWPASS:
        case ONEPOLE_HIGHPASS:
            specs.set(SPEC_REQUIRE_FREQ);
            break;
        case LOW_PASS:
        case HIGH_PASS:
        case BAND_PASS1:
        case BAND_PASS2:
        case NOTCH:
        case ALL_PASS:
            specs.set(SPEC_REQUIRE_FREQ | SPEC_REQUIRE_BW);
            break;
        default:
            break;
        }

        return specs;
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
