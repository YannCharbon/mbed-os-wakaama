#ifndef RESSOURCE_H
#define RESSOURCE_H

#include <stdlib.h>

#define RES_SUCCESS 0
#define BAD_EXPECTED_ACCESS 1

enum class AccessRights{RES_RD, RES_WR, RES_E, RES_RDWR};

typedef void (*write_callback_t)();
typedef void (*read_callback_t)();
typedef void (*exec_callback_t)();

template<typename T>
class Ressource{
public:
    Ressource(size_t index, AccessRights rights, T value);

    int read(T& ressource);
    int write(const T& value);
    int exec(void);

    int bind_on_write(write_callback_t action_on_write);
    int bind_on_read(read_callback_t action_on_read);
    int bind_on_exec(exec_callback_t action_on_exec);

private:
    const size_t index;
    const AccessRights rights;
    T value;

    write_callback_t action_on_write;
    read_callback_t action_on_read;
    exec_callback_t action_on_exec;
};



#endif
