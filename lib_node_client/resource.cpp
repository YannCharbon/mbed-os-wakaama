/**
 *  @file resource_impl.h
 *  @brief This header file contain the definition of resource non template methods.
 *
 *  @author Bastien Pillonel
 *
 *  @date 6/20/2024
 */

#include "resource.h"

Resource::~Resource()
{
    if (!_value)
        return;
    
    // Clean ResCallback instance
    delete _actionsOnWrite;
    delete _actionsOnRead;
    delete _actionsOnExec;

    // Clean value object and head structure
    Head *head = this->_head();
    head->~Head();
    free(head);
}

bool Resource::Empty() const
{
    return _value == nullptr;
}

const std::type_info &Resource::Type()
{
    if(!_value){
        _errorCode = VALUE_IS_EMPTY;
        return typeid(nullptr);
    }
    return ((Head *)_value - 1)->type;
}

const ResourceOp &Resource::GetOp() const
{
    return _resourceOp;
}

const std::string &Resource::GetName() const{
    return _name;
}

const Units &Resource::GetUnit() const {
    return _unit;
}

const size_t &Resource::GetId() const{
    return _id;
}

int Resource::GetErrorCode() {
    int errorCode = _errorCode;
    _errorCode = RES_SUCCESS;
    return errorCode;
}
