#ifndef ENSITLM_H
#define ENSITLM_H

#include <systemc>
#include <tlm.h>
#include <stdint.h>
#include <sstream>

namespace ensitlm {
typedef uint32_t addr_t;
typedef uint32_t data_t;
}

#include "initiator_socket.h"
#include "target_socket.h"

#endif
