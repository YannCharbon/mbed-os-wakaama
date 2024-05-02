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

class ResCallbackBase
{
public:
    virtual ~ResCallbackBase() {}
};

template <class T>
class ResCallback : public ResCallbackBase
{
public:
    using Function = std::function<void(T)>;
    using FunctionPtr = std::shared_ptr<Function>;
    using Functions = std::vector<FunctionPtr>;

    ResCallback() {}
    ResCallback(const ResCallback &src) : _functions(src._functions) {}
    ResCallback(ResCallback &&src) : _functions(src._functions) { src._functions = nullptr; }
    ~ResCallback() override {}

    FunctionPtr AddListener(Function f)
    {
        FunctionPtr fp = std::make_shared<Function>(f);
        _functions.push_back(fp);
        return fp;
    }

    void RemoveListener(FunctionPtr fp)
    {
        _functions.erase(std::find(_functions.begin(), _functions.end(), fp));
    }

    void operator()(T value) const
    {
        for (auto &f : _functions)
            f.get()->operator()(value);
    }

private:
    Functions _functions;
};

#endif