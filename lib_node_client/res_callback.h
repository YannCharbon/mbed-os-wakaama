/**
 *  @file res_callback.h
 *  @brief This header file contain the declaration of the ResCallback class used in Ressource class. These callbacks let user defined functions to call when
 *  Read/Write/Exec operation are called on a ressource.
 *
 *  @author Bastien Pillonel
 *
 *  @date 4/28/2024
 */

#ifndef RES_CALLBACK_H
#define RES_CALLBACK_H

#include <functional>
#include <memory>
#include <algorithm>
#include <vector>

template <class T>
class ResCallback;

/**
 * @brief Parent class for generic child ResCallback class. 
 * 
 */
class ResCallbackBase
{
public:
    /**
     * @brief Destroy the Res Callback Base object
     * 
     */
    virtual ~ResCallbackBase() {}

    /**
     * @brief Copy instance of ResCallBackBase
     * 
     * @return ResCallbackBase*
     */
    virtual ResCallbackBase *clone() const = 0;

    /**
     * @brief Move ResCallbackBase attribute into another instance
     * 
     * @return ResCallbackBase* 
     */
    virtual ResCallbackBase *move() noexcept = 0;
};

/**
 * @brief Class registering callback functions. Used in Resource class
 * 
 * @tparam T type of argument passed to any callback function registered in ResCallback instance
 */
template <class T>
class ResCallback : public ResCallbackBase
{
public:
    using Function = std::function<void(T)>;
    using FunctionPtr = std::shared_ptr<Function>;
    using Functions = std::vector<FunctionPtr>;

    /**
     * @brief Construct a new ResCallback object
     * 
     */
    ResCallback() : _functions({}) {}

    /**
     * @brief Construct a new ResCallback object by copy
     * 
     * @param src 
     */
    ResCallback(const ResCallback &src) : _functions(src._functions) {}

    /**
     * @brief Construct a new ResCallback object by moving
     * 
     * @param src 
     */
    ResCallback(ResCallback &&src) : _functions(std::move(src._functions)) {}

    /**
     * @brief Destroy the ResCallback object
     * 
     */
    ~ResCallback() override {}

    /**
     * @brief Copy instance of ResCallBack
     * 
     * @return ResCallbackBase* 
     */
    ResCallbackBase *clone() const override
    {
        return new ResCallback<T>(*this);
    }

    /**
     * @brief Move ResCallback attribute into another instance
     * 
     * @return ResCallbackBase* 
     */
    ResCallbackBase *move() noexcept override
    {
        return new ResCallback<T>(std::move(*this));
    }

    /**
     * @brief Add a callback function to the vector of other callback functions registered
     * 
     * @param f function to register
     * @return FunctionPtr shared pointer on the function registered
     */
    FunctionPtr AddListener(Function f)
    {
        FunctionPtr fp = std::make_shared<Function>(f);
        _functions.push_back(fp);
        return fp;
    }

    /**
     * @brief Remove a callback function registered by searching it's address in the vector
     * 
     * @param fp shared pointer previously retrieved from returned value of AddListener function
     */
    void RemoveListener(FunctionPtr fp)
    {
        _functions.erase(std::find(_functions.begin(), _functions.end(), fp));
    }

    /**
     * @brief Overload of operator (), calling every function registered
     * 
     * @param value in the Resource class, it's the value of the resource that will be passed
     */
    void operator()(T value) const
    {
        for (auto &f : _functions)
            f.get()->operator()(value);
    }

private:
    Functions _functions;
};

#endif