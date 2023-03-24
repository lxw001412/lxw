#pragma once
#include <json/json.h>
#include <vector>
#include <map>
#include <string>


namespace star_protocol
{
class dataType;
class dataTypeStruct;
class dataTypeArray;
class dataTypeNumber;
class dataTypeString;
class dataTypeEnum;
class dataTypeBool;

class property
{
public:
    property();
    ~property();
    int init(const Json::Value &jsonRoot);

    inline const std::string& indentifer() const {
        return m_indentifer;
    }
    inline int index() const {
        return m_index;
    }
    inline bool required() const {
        return m_required;
    }
    inline const std::string& name() const {
        return m_name;
    }
    const dataType* getDataType() const {
        return m_dataType;
    }

private:
    std::string m_indentifer;
    int m_index;
    bool m_required;
    std::string m_name;
    dataType* m_dataType;
};

typedef enum
{
    pt_enum = 0,
    pt_string,
    pt_int8,
    pt_int16,
    pt_int32,
    pt_int64,
    pt_bool,
    pt_struct,
    pt_array,
    pt_bytes
}propertyType;

class dataType
{
public:
    static dataType* makeDataType(const Json::Value &jsonRoot);
    virtual ~dataType() {};
    virtual int init(const Json::Value &jsonRoot) = 0;
    virtual bool validate(const char* value) const = 0;
    virtual int length() const = 0;
    virtual std::string toString(const char* value) const = 0;
    virtual std::string toValue(const char* str) const = 0;
    virtual propertyType type() const = 0;
    virtual int b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const = 0;
    virtual int j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const = 0;
};

class dataTypeNumber : public dataType
{
public:
    dataTypeNumber(propertyType type);
    virtual ~dataTypeNumber();

    virtual int init(const Json::Value &jsonRoot);
    virtual int b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const;
    virtual int j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const;
    virtual bool validate(const char* value) const;
    virtual int length() const {
        return m_length;
    };
    virtual std::string toString(const char* value) const {
        return value;
    };
    virtual std::string toValue(const char* str) const {
        return str;
    };
    virtual propertyType type() const {
        return m_type;
    };

private:
    std::string m_unit;
    uint64_t m_min;
    uint64_t m_max;
    uint64_t m_step;
    int m_length;
    propertyType m_type;
};

class dataTypeString : public dataType
{
public:
    dataTypeString();
    virtual ~dataTypeString();

    virtual int init(const Json::Value &jsonRoot);
    virtual int b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const;
    virtual int j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const;
    virtual bool validate(const char* value) const;
    virtual int length() const {
        return m_length;
    };
    virtual std::string toString(const char* value) const {
        return value;
    };
    virtual std::string toValue(const char* str) const {
        return str;
    };
    virtual propertyType type() const {
        return pt_string;
    };
private:
    int m_length;
};

class dataTypeBytes : public dataType
{
public:
    dataTypeBytes();
    virtual ~dataTypeBytes();

    virtual int init(const Json::Value &jsonRoot);
    virtual int b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const;
    virtual int j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const;
    virtual bool validate(const char* value) const;
    virtual int length() const {
        return m_length;
    };
    virtual std::string toString(const char* value) const {
        return value;
    };
    virtual std::string toValue(const char* str) const {
        return str;
    };
    virtual propertyType type() const {
        return pt_bytes;
    };
private:
    int b2j_hexstring(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const;
    int j2b_hexstring(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const;
private:
    int m_length;
    enum codecType_t {
        hexstring = 0,
        base64
    };
    codecType_t m_codec;
};

class dataTypeBool : public dataType
{
public:
    dataTypeBool();
    virtual ~dataTypeBool();

    virtual int init(const Json::Value &jsonRoot);
    virtual int b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const;
    virtual int j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const;
    virtual bool validate(const char* value) const;
    virtual int length() const {
        return 1;
    };
    virtual std::string toString(const char* value) const;
    virtual std::string toValue(const char* str) const;
    virtual propertyType type() const {
        return pt_bool;
    };
private:
    std::string m_trueDesc;
    std::string m_falseDesc;
};

class dataTypeEnum : public dataType
{
public:
    dataTypeEnum();
    virtual ~dataTypeEnum();

    virtual int init(const Json::Value &jsonRoot);
    virtual int b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const;
    virtual int j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const;
    virtual bool validate(const char* value) const;
    virtual int length() const {
        return 1;
    };
    virtual std::string toString(const char* value) const;
    virtual std::string toValue(const char* str) const;
    virtual propertyType type() const {
        return pt_enum;
    };
private:
    std::map<int, std::string> m_valueNameMap;
};

class dataTypeArray : public dataType
{
public:
    dataTypeArray();
    virtual ~dataTypeArray();

    virtual int init(const Json::Value &jsonRoot);
    virtual int b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const;
    virtual int j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const;
    virtual bool validate(const char* value) const;
    virtual int length() const {
        return m_size;
    };
    virtual std::string toString(const char* value) const {
        return value;
    }
    virtual std::string toValue(const char* str) const {
        return str;
    }
    virtual propertyType type() const {
        return pt_array;
    };
    const property* getProperty() const {
        return m_property;
    }
private:
    int m_size;
    property* m_property;
};

class dataTypeStruct : public dataType
{
public:
    dataTypeStruct();
    virtual ~dataTypeStruct();

    virtual int init(const Json::Value &jsonRoot);
    virtual int b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const;
    virtual int j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const;
    virtual bool validate(const char* value) const {
        return true;
    };
    virtual int length() const {
        return 0;
    };
    virtual std::string toString(const char* value) const {
        return value;
    }
    virtual std::string toValue(const char* str) const {
        return str;
    }
    virtual propertyType type() const {
        return pt_struct;
    };
    const property* getProperty(int index) const;
    const property* getProperty(const char* name) const;
private:
    std::vector<property*> m_properties;
    std::map<int, property*> m_propertyIndexMap;
    std::map<std::string, property*> m_propertyNameMap;
};

};