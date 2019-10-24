#ifndef BASIC_TARGET_SOCKET_H
#define BASIC_TARGET_SOCKET_H

#include "ensitlm.h"

namespace ensitlm {

typedef tlm::tlm_target_socket<CHAR_BIT * sizeof(data_t),
                               tlm::tlm_base_protocol_types> compatible_socket;

template <typename MODULE, bool MULTIPORT = false>
class target_socket
    : public tlm::tlm_target_socket<CHAR_BIT * sizeof(data_t),
                                    tlm::tlm_base_protocol_types,
                                    MULTIPORT ? 0 : 1>,
      public tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types> {
	typedef tlm::tlm_target_socket<CHAR_BIT * sizeof(data_t),
	                               tlm::tlm_base_protocol_types,
	                               MULTIPORT ? 0 : 1> base_type;
	typedef tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>
	    fw_if_type;

public:
	target_socket() : base_type(sc_core::sc_gen_unique_name(kind())) {
		// check_typing() is never actually called, but should be
		// statically reachable to force the compiler to do the
		// typechecking.
		if (false)
			check_typing();
		init();
	}

	explicit target_socket(const char *name) : base_type(name) {
		init();
	}

	virtual const char *kind() const {
		return "ensitlm::target_socket";
	}

	bool get_direct_mem_ptr(tlm::tlm_generic_payload &, tlm::tlm_dmi &) {
		std::cerr << "get_direct_mem_ptr not implemented" << std::endl;
		abort();
	}

	unsigned int transport_dbg(tlm::tlm_generic_payload &) {
		std::cerr << "transport_dbg not implemented" << std::endl;
		abort();
	}

	tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &,
	                                   tlm::tlm_phase &,
	                                   sc_core::sc_time &) {
		std::cerr << "nb_transport_fw not implemented" << std::endl;
		abort();
	}

private:
	void check_typing() {
		std::cerr << "You should never call this function, it is only "
		             "meant for typechecking"
		          << std::endl;
		abort();
		const ensitlm::data_t const_data = 12;
		const ensitlm::addr_t const_addr = 42;
		ensitlm::data_t data;
		// Check that MODULE inherits publicly from
		// sc_module. If You get an error on the following
		// line, check that the first template argument
		// is an sc_module.
		(void)static_cast<sc_core::sc_module *>(m_mod);

		// Check that the parent module declares read and
		// write methods properly. If you get an error on
		// one of the following lines, check that the
		// declaration of read and write match *exactly*
		// the ones in initiator_socket.h
		tlm::tlm_response_status s;
		s = m_mod->read(const_addr, data);
		s = m_mod->write(const_addr, const_data);
		(void)s;
	}

	void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &t) {
		(void)t;
		addr_t addr = static_cast<addr_t>(trans.get_address());
		data_t &data =
		    *(reinterpret_cast<data_t *>(trans.get_data_ptr()));

		switch (trans.get_command()) {
		case tlm::TLM_READ_COMMAND:
			trans.set_response_status(m_mod->read(addr, data));
			break;
		case tlm::TLM_WRITE_COMMAND:
			trans.set_response_status(m_mod->write(addr, data));
			break;
		case tlm::TLM_IGNORE_COMMAND:
			break;
		default:
			trans.set_response_status(
			    tlm::TLM_COMMAND_ERROR_RESPONSE);
		}
	}

	void init() {
		// we'll receive transactions ourselves ...
		this->bind(*(static_cast<fw_if_type *>(this)));
		// ... but we'll need to call read/write in the parent module.
		sc_core::sc_object *parent = this->get_parent_object();
		if (!parent) {
			std::cerr << this->name()
			          << ": target socket has no parent object."
			          << std::endl;
			abort();
		}
		m_mod = dynamic_cast<MODULE *>(parent);
		if (!m_mod) {
			std::cerr << this->name() << ": parent object "
			          << parent->name()
			          << " of socket has the wrong type."
			          << std::endl;
			abort();
		}
	}

	MODULE *m_mod;
};
}

#endif
