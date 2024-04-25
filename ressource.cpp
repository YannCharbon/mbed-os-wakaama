#include "ressource.h"

template <typename T>
Ressource<T>::Ressource(size_t index, AccessRights rights, T value) : index(index), rights(rights), value(value), bind_on_write(NULL), bind_on_read(NULL), bind_on_exec(NULL)
{
}

template <typename T>
int Ressource<T>::read(T &ressource)
{
    // Check access rights
    if (this->rights != RES_RD || this->rights != RES_RDWR)
    {
        return -BAD_EXPECTED_ACCESS;
    }

    ressource = this->value;

    // Check if read callback has been binded
    if (this->action_on_read)
    {
        // Read callback
        this->action_on_read();
    }

    return RES_SUCCESS;
}

template <typename T>
int Ressource<T>::write(const T &value)
{
    // Check access rights
    if (this->rights != RES_RW || this->rights != RES_RDWR)
    {
        return -BAD_EXPECTED_ACCESS;
    }

    this->value = value;

    if (this->action_on_write)
    {
        // Write callback
        this->action_on_write();
    }

    return RES_SUCCESS;
}

template <typename T>
int Ressource<T>::exec(void)
{
    // Check access rights
    if (this->rights != RES_E)
    {
        return -BAD_EXPECTED_ACCESS;
    }

    // Check if exec callback has been binded
    if (this->action_on_exec)
    {
        // Execute callback
        this->action_on_exec();
    }

    return RES_SUCCESS;
}

template <typename T>
int Ressource<T>::bind_on_read(read_callback_t action_on_read)
{
    // Check access rights
    if (this->rights != RES_RD || this->rights != RES_RDWR)
    {
        return -BAD_EXPECTED_ACCESS;
    }

    this->action_on_read = action_on_read;
}

template <typename T>
int Ressource<T>::bind_on_write(write_callback_t action_on_write)
{
    // Check access rights
    if (this->rights != RES_RW || this->rights != RES_RDWR)
    {
        return -BAD_EXPECTED_ACCESS;
    }

    this->action_on_write = action_on_write;
}

template <typename T>
int Ressource<T>::bind_on_exec(exec_callback_t action_on_exec)
{
    // Check access rights
    if (this->rights != RES_E)
    {
        return -BAD_EXPECTED_ACCESS;
    }

    this->action_on_exec = action_on_exec;
}