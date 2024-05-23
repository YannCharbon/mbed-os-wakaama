/**
 *  @file resource.h
 *  @brief This header file contain the declaration of the resource class representing a generic resource contained in any object
 *  described by the uCIFI standard.
 *
 *  @author Bastien Pillonel
 *
 *  @date 4/25/2024
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdlib.h>
#include <typeinfo>
#include <string>
#include <iostream>

#include "res_callback.h"

#define RES_SUCCESS 0
#define BAD_EXPECTED_ACCESS 1
#define VALUE_IS_EMPTY 2
#define VALUE_TYPE_NOT_CORRESPONDING 3
#define NO_CALLBACK_OBJECT 4

enum class ResourceOp
{
    RES_RD,
    RES_WR,
    RES_E,
    RES_RDWR
};

enum class Units
{
    NA,
    DATE,
    HOURS,
    PERCENT,
    AMPER_HOUR,
    VOLT,
    DECIBEL_MILLIWATT,
    HERTZ,
    SECONDS,
    DECIBEL,
    MILLISECOND,
    BYTES,
    CELCIUS_DEGREES,
    WATT,
    KELVIN,
    AMPER,
    KILOWATT_HOUR,
    LUMEN,
    WATT_HOUR,
    VAR,
    VAR_HOUR,
    LX
};

/**
 * Resource class implementing a resource contained in an object described by the uCIFI standard.
 *
 */
class Resource
{
private:
    struct Head
    {
        virtual ~Head() {}
        virtual void *Copy() = 0;
        const std::type_info &type;
        Head(const std::type_info &type) : type(type) {}
        void *Data() { return this + 1; }
    };

    template <class T>
    struct THead : public Head
    {
        THead() : Head(typeid(T)) {}
        virtual ~THead() override { ((T *)Data())->~T(); }
        virtual void *Copy() override
        {
            return new (new (malloc(sizeof(Head) + sizeof(T))) THead() + 1) T(*(const T *)Data());
        }
    };

    Head *_head() const { return (Head *)_value - 1; }
    void *_copy() const { return _value ? _head()->Copy() : nullptr; }

    void *_value;
    const ResourceOp _resourceOp;
    const std::string _name;
    const Units _unit;
    int _errorCode;

    ResCallbackBase *_actionsOnWrite = nullptr;
    ResCallbackBase *_actionsOnRead = nullptr;
    ResCallbackBase *_actionsOnExec = nullptr;

public:
    Resource() : _value(nullptr), _resourceOp(ResourceOp::RES_RD), _name(std::string("")), _unit(Units::NA), _errorCode(RES_SUCCESS) {}
    Resource(const Resource &src) : _value(src._copy()), _resourceOp(src._resourceOp), _name(src._name), _unit(src._unit), _errorCode(RES_SUCCESS) {}
    Resource(Resource &&src) : _value(src._value), _resourceOp(src._resourceOp), _name(src._name), _unit(src._unit), _errorCode(RES_SUCCESS) { src._value = nullptr; }

    template <class T>
    Resource(const T &src, ResourceOp rights = ResourceOp::RES_RD, const std::string &name = std::string("name"), Units unit = Units::NA) : _value(new(new(malloc(sizeof(Head) + sizeof(T))) THead<T>() + 1) T(src)), _resourceOp(rights), _name(name), _unit(unit), _errorCode(RES_SUCCESS) {}
    ~Resource();

    bool Empty() const;
    const std::type_info &Type();
    const ResourceOp &GetOp() const;
    const std::string &GetName() const;
    const Units &GetUnit() const;
    int GetErrorCode();

    template <class T>
    T *Read()
    {
        if (!_value)
        {
            _errorCode = VALUE_IS_EMPTY;
            return nullptr;
        }

        // Check corresponding type between fct argument and _value
        if (Type() != typeid(T))
        {
            _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
            return nullptr;
        }

        // Check access rights
        if (_resourceOp != ResourceOp::RES_RD && _resourceOp != ResourceOp::RES_RDWR)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return nullptr;
        }

        if (_actionsOnRead)
            (*((ResCallback<T> *)_actionsOnWrite))(*(T *)_value);

        return (T *)_value;
    }

    template <class T>
    int Write(const T &writeValue)
    {
        // Check access rights
        if (_resourceOp != ResourceOp::RES_WR && _resourceOp != ResourceOp::RES_RDWR)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return BAD_EXPECTED_ACCESS;
        }

        if (!_value)
            _value = new (new (malloc(sizeof(Head) + sizeof(T))) THead<T>() + 1) T(writeValue);
        else
        {
            // Check corresponding type between fct argument and _value
            if (Type() != typeid(T))
            {
                _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
                return VALUE_TYPE_NOT_CORRESPONDING;
            }

            *(T *)_value = writeValue;
        }

        if (_actionsOnWrite)
            (*((ResCallback<T> *)_actionsOnWrite))(writeValue);

        return RES_SUCCESS;
    }

    template <class T>
    int Exec()
    {
        if (!_value)
        {
            _errorCode = VALUE_IS_EMPTY;
            return VALUE_IS_EMPTY;
        }

        // Check corresponding type between fct argument and _value
        if (Type() != typeid(T))
        {
            _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
            return VALUE_TYPE_NOT_CORRESPONDING;
        }

        // Check access rights
        if (_resourceOp != ResourceOp::RES_E)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return BAD_EXPECTED_ACCESS;
        }

        if (_actionsOnExec)
            (*((ResCallback<T> *)_actionsOnExec))(*(T *)_value);

        return RES_SUCCESS;
    }

    template <class T>
    std::shared_ptr<std::function<void(T)>> BindOnWrite(std::function<void(T)> f)
    {
        // Check access rights
        if (this->_resourceOp != ResourceOp::RES_WR && this->_resourceOp != ResourceOp::RES_RDWR)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return nullptr;
        }

        // Check corresponding type between fct argument and _value
        if (Type() != typeid(T))
        {
            _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
            return nullptr;
        }

        // Creat a callback object on first bind
        if (!_actionsOnWrite)
        {
            _actionsOnWrite = new ResCallback<T>();
        }

        return ((ResCallback<T> *)_actionsOnWrite)->AddListener(f);
    }

    template <class T>
    int UnbindOnWrite(std::shared_ptr<std::function<void(T)>> fp)
    {
        // Check access rights
        if (this->_resourceOp != ResourceOp::RES_WR && this->_resourceOp != ResourceOp::RES_RDWR)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return BAD_EXPECTED_ACCESS;
        }

        // Check corresponding type between fct argument and _value
        if (Type() != typeid(T))
        {
            _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
            return VALUE_TYPE_NOT_CORRESPONDING;
        }

        // Can't unbind if no Callback object instanciate
        if (!_actionsOnWrite)
        {
            _errorCode = NO_CALLBACK_OBJECT;
            return NO_CALLBACK_OBJECT;
        }

        ((ResCallback<T> *)_actionsOnWrite)->RemoveListener(fp);

        return RES_SUCCESS;
    }

    template <class T>
    std::shared_ptr<std::function<void(T)>> BindOnRead(std::function<void(T)> f)
    {
        // Check access rights
        if (this->_resourceOp != ResourceOp::RES_RD && this->_resourceOp != ResourceOp::RES_RDWR)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return nullptr;
        }

        // Check corresponding type between fct argument and _value
        if (Type() != typeid(T))
        {
            _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
            return nullptr;
        }

        // Creat a callback object on first bind
        if (!_actionsOnRead)
        {
            _actionsOnRead = new ResCallback<T>();
        }

        return ((ResCallback<T> *)_actionsOnRead)->AddListener(f);
    }

    template <class T>
    int UnbindOnRead(std::shared_ptr<std::function<void(T)>> fp)
    {
        // Check access rights
        if (this->_resourceOp != ResourceOp::RES_RD && this->_resourceOp != ResourceOp::RES_RDWR)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return BAD_EXPECTED_ACCESS;
        }

        // Check corresponding type between fct argument and _value
        if (Type() != typeid(T))
        {
            _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
            return VALUE_TYPE_NOT_CORRESPONDING;
        }

        // Can't unbind if no Callback object instanciate
        if (!_actionsOnRead)
        {
            _errorCode = NO_CALLBACK_OBJECT;
            return NO_CALLBACK_OBJECT;
        }

        ((ResCallback<T> *)_actionsOnRead)->RemoveListener(fp);

        return RES_SUCCESS;
    }

    template <class T>
    std::shared_ptr<std::function<void(T)>> BindOnExec(std::function<void(T)> f)
    {
        // Check access rights
        if (this->_resourceOp != ResourceOp::RES_E)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return nullptr;
        }

        // Check corresponding type between fct argument and _value
        if (Type() != typeid(T))
        {
            _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
            return nullptr;
        }

        // Creat a callback object on first bind
        if (!_actionsOnExec)
        {
            _actionsOnExec = new ResCallback<T>();
        }

        return ((ResCallback<T> *)_actionsOnExec)->AddListener(f);
    }

    template <class T>
    int UnbindOnExec(std::shared_ptr<std::function<void(T)>> fp)
    {
        // Check access rights
        if (this->_resourceOp != ResourceOp::RES_E)
        {
            _errorCode = BAD_EXPECTED_ACCESS;
            return BAD_EXPECTED_ACCESS;
        }

        // Check corresponding type between fct argument and _value
        if (Type() != typeid(T))
        {
            _errorCode = VALUE_TYPE_NOT_CORRESPONDING;
            return VALUE_TYPE_NOT_CORRESPONDING;
        }

        // Can't unbind if no Callback object instanciate
        if (!_actionsOnExec)
        {
            _errorCode = NO_CALLBACK_OBJECT;
            return NO_CALLBACK_OBJECT;
        }

        ((ResCallback<T> *)_actionsOnExec)->RemoveListener(fp);

        return RES_SUCCESS;
    }
};

#include "resource_impl.h"

#endif
