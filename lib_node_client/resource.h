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
    LX,
    KILOVAR_HOUR
};

/**
 * @brief Resource class implementing a resource contained in an object
 * described by the uCIFI standard.
 *
 */
class Resource
{
private:
    /**
     * @brief Parent Head structure containing information about the value stored on the
     * resource.
     *
     */
    struct Head
    {
        /**
         * @brief Destroy the Head object
         *
         */
        virtual ~Head() {}
        /**
         * @brief Copy value object
         *
         * @return void*
         */
        virtual void *Copy() = 0;
        /**
         * @brief Inform about the value type
         *
         */
        const std::type_info &type;
        /**
         * @brief Construct a new Head object by copy
         *
         * @param type
         */
        Head(const std::type_info &type) : type(type) {}
        /**
         * @brief Return pointer on the value object stored
         *
         * @return void*
         */
        void *Data() { return this + 1; }
    };

    /**
     * @brief THead srtucture inherits from Head structure, it simply specify type of the value object stored
     *
     * @tparam T
     */
    template <class T>
    struct THead : public Head
    {
        /**
         * @brief Construct a new THead object
         *
         */
        THead() : Head(typeid(T)) {}
        /**
         * @brief Destroy the THead object
         *
         */
        virtual ~THead() override { ((T *)Data())->~T(); }
        /**
         * @brief Specify copy function according to the type of value object stored
         *
         * @return void* pointer on the value object stored
         */
        virtual void *Copy() override
        {
            return new (new (malloc(sizeof(Head) + sizeof(T))) THead() + 1) T(*(const T *)Data());
        }
    };

    /**
     * @brief Return Head structure associated to the value object stored
     *
     * @return Head*
     */
    Head *_head() const { return (Head *)_value - 1; }
    /**
     * @brief Copy Head structure associated to the value object stored and value stored
     *
     * @return void* Pointer to the memory space where Head structure and value object are stored
     */
    void *_copy() const { return _value ? _head()->Copy() : nullptr; }

    void *_value;
    const ResourceOp _resourceOp;
    const std::string _name;
    const Units _unit;
    int _errorCode;
    const size_t _id;

    ResCallbackBase *_actionsOnWrite = nullptr;
    ResCallbackBase *_actionsOnRead = nullptr;
    ResCallbackBase *_actionsOnExec = nullptr;

public:
    /**
     * @brief Construct a new Resource object by default
     *
     */
    Resource() : _value(nullptr), _resourceOp(ResourceOp::RES_RD), _name(std::string("")), _unit(Units::NA), _errorCode(RES_SUCCESS), _id(0) {}

    /**
     * @brief Construct a new Resource object by copy
     *
     * @param src reference object instance
     */
    Resource(const Resource &src) : _value(src._copy()), _resourceOp(src._resourceOp), _name(src._name), _unit(src._unit), _errorCode(RES_SUCCESS), _id(src._id), _actionsOnWrite((src._actionsOnWrite ? src._actionsOnWrite->clone() : nullptr)), _actionsOnRead((src._actionsOnRead ? src._actionsOnRead->clone() : nullptr)), _actionsOnExec((src._actionsOnExec ? src._actionsOnExec->clone() : nullptr)) {}

    /**
     * @brief Construct a new Resource object by moving
     *
     * @param src reference object instance
     */
    Resource(Resource &&src) : _value(src._value), _resourceOp(src._resourceOp), _name(src._name), _unit(src._unit), _errorCode(RES_SUCCESS), _id(src._id), _actionsOnWrite((src._actionsOnWrite ? src._actionsOnWrite->move() : nullptr)), _actionsOnRead((src._actionsOnRead ? src._actionsOnRead->move() : nullptr)), _actionsOnExec((src._actionsOnExec ? src._actionsOnExec->move() : nullptr))
    {
        src._value = nullptr;
        src._actionsOnWrite = nullptr;
        src._actionsOnRead = nullptr;
        src._actionsOnExec = nullptr;
    }

    /**
     * @brief Construct a new Resource object by specifying attribute
     *
     * @tparam T type of value stored in the resource
     * @param src value object stored in the resource
     * @param rights operation allowed on the resource (R/W/E)
     * @param name resource name
     * @param unit resource unit
     * @param id resource id
     */
    template <class T>
    Resource(const T &src, ResourceOp rights = ResourceOp::RES_RD, const std::string &name = std::string("name"), Units unit = Units::NA, size_t id = 0) : _value(new(new(malloc(sizeof(Head) + sizeof(T))) THead<T>() + 1) T(src)), _resourceOp(rights), _name(name), _unit(unit), _errorCode(RES_SUCCESS), _id(id) {}

    /**
     * @brief Destroy the Resource object
     *
     */
    ~Resource();

    /**
     * @brief Inform if resource instance has no value object
     *
     * @return true value object is empty
     * @return false value object is not empty
     */
    bool Empty() const;

    /**
     * @brief Inform on the value object type
     *
     * @return const std::type_info&
     */
    const std::type_info &Type();

    /**
     * @brief Get the operation allows on the resource
     *
     * @return const ResourceOp&
     */
    const ResourceOp &GetOp() const;

    /**
     * @brief Get the resource name
     *
     * @return const std::string&
     */
    const std::string &GetName() const;

    /**
     * @brief Get the resource unit
     *
     * @return const Units&
     */
    const Units &GetUnit() const;

    /**
     * @brief Get the resource id
     *
     * @return const size_t&
     */
    const size_t &GetId() const;

    /**
     * @brief Get the resource error code
     *
     * @return int
     */
    int GetErrorCode();

    /**
     * @brief Get the Value object
     *
     * @tparam T type of value object
     * @return T* pointer on the value object
     */
    template <class T>
    T *GetValue()
    {
        // Check value is not empty
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

        return (T *)_value;
    }

    /**
     * @brief Set the Value object
     *
     * @tparam T type of value object
     * @param writeValue new value object to store
     * @return int error code
     */
    template <class T>
    int SetValue(const T &writeValue)
    {
        // Create a new value object if empty
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

        return RES_SUCCESS;
    }

    /**
     * @brief Works as GetValue() function but call read callback functions registered
     *
     * @tparam T
     * @return T*
     */
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

        // Call of read callback functions
        if (_actionsOnRead)
            (*((ResCallback<T> *)_actionsOnRead))(*(T *)_value);

        return (T *)_value;
    }

    /**
     * @brief Works as SetValue() function but call write callback functions registered
     *
     * @tparam T
     * @param writeValue
     * @return int
     */
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

        // Call of write callback functions
        if (_actionsOnWrite)
            (*((ResCallback<T> *)_actionsOnWrite))(writeValue);

        return RES_SUCCESS;
    }

    /**
     * @brief Only call execute callback function registered
     *
     * @tparam T
     * @return int error code
     */
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

    /**
     * @brief Bind a callback function to the resource instance for write operation
     *
     * @tparam T type of parameter inside callback function
     * @param f callbcak function
     * @return std::shared_ptr<std::function<void(T)>> pointer on the callback function registered
     */
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

        // Create a callback object on first bind
        if (!_actionsOnWrite)
        {
            _actionsOnWrite = new ResCallback<T>();
        }

        return ((ResCallback<T> *)_actionsOnWrite)->AddListener(f);
    }

    /**
     * @brief Remove callback registered previously using pointer on that callback for write operation
     *
     * @tparam T type of parameter inside callback function
     * @param fp pointer on the callback function
     * @return int error code
     */
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

    /**
     * @brief Bind a callback function to the resource instance for read operation
     *
     * @tparam T type of parameter inside callback function
     * @param f callbcak function
     * @return std::shared_ptr<std::function<void(T)>> pointer on the callback function registered
     */
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

        // Create a callback object on first bind
        if (!_actionsOnRead)
        {
            _actionsOnRead = new ResCallback<T>();
        }

        return ((ResCallback<T> *)_actionsOnRead)->AddListener(f);
    }

    /**
     * @brief Remove callback registered previously using pointer on that callback for write operation
     *
     * @tparam T type of parameter inside callback function
     * @param fp pointer on the callback function
     * @return int error code
     */
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

    /**
     * @brief Bind a callback function to the resource instance for execute operation
     *
     * @tparam T type of parameter inside callback function
     * @param f callbcak function
     * @return std::shared_ptr<std::function<void(T)>> pointer on the callback function registered
     */
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

        // Create a callback object on first bind
        if (!_actionsOnExec)
        {
            _actionsOnExec = new ResCallback<T>();
        }refer

            return ((ResCallback<T> *)_actionsOnExec)->AddListener(f);
    }

    /**
     * @brief Remove callback registered previously using pointer on that callback for write operation
     *
     * @tparam T type of parameter inside callback function
     * @param fp pointer on the callback function
     * @return int error code
     */
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

#endif
