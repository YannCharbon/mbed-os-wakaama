#ifndef RESOURCE_IMPL_H
#define RESOURCE_IMPL_H

#include "resource.h"

Resource::~Resource()
{
    if (!_value)
        return;
    delete _actionsOnWrite;
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

const ResourceOp &Resource::GetRights() const
{
    return _rights;
}

const std::string &Resource::GetName() const{
    return _name;
}

const Units &Resource::GetUnit() const {
    return _unit;
}

int Resource::GetErrorCode() {
    int errorCode = _errorCode;
    _errorCode = RES_SUCCESS;
    return errorCode;
}

#endif