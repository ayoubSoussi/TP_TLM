#include "ensitlm.h"
#include "intc.h"
#include "offsets/intc.h"

Intc::Intc(sc_core::sc_module_name name) : sc_core::sc_module(name) {
	SC_THREAD(send_irq);
	sensitive << irq_to_send;

#define DECLARE_PROCESS_IRQ(n)                                                 \
	SC_METHOD(process_in_irq##n);                                          \
	sensitive << in##n.pos()

	DECLARE_PROCESS_IRQ(0);
	DECLARE_PROCESS_IRQ(1);
}

void Intc::send_irq() {
	while (true) {
		while (!irq_to_send_notified) {
			wait();
		}
		irq_to_send_notified = false;
		out.write(true);
		wait(20, sc_core::SC_NS);
		out.write(false);
		wait(20, sc_core::SC_NS);
	}
}

void Intc::process_in_irq(int N) {
	const ensitlm::data_t mask = (1 << N);
	m_active_it |= mask;
	send_irq_maybe(mask);
}

void Intc::send_irq_maybe(ensitlm::data_t mask) {
	if (m_enabled_it & m_active_it & mask) {
		irq_to_send_notified = true;
		irq_to_send.notify();
	}
}

tlm::tlm_response_status Intc::read(ensitlm::addr_t a, ensitlm::data_t &d) {
	switch (a) {
	case XIN_ISR_OFFSET: /* Interrupt Status Register */
		d = m_active_it;
		break;
	case XIN_IPR_OFFSET: /* Interrupt Pending Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IPR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_IER_OFFSET: /* Interrupt Enable Register */
		d = m_enabled_it;
		break;
	case XIN_IAR_OFFSET: /* Interrupt Acknowledge Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IAR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_SIE_OFFSET: /* Set Interrupt Enable Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_SIE_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_CIE_OFFSET: /* Clear Interrupt Enable Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_CIE_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_IVR_OFFSET: /* Interrupt Vector Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IVR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_MER_OFFSET: /* Master Enable Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_MER_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_IMR_OFFSET: /* Interrupt Mode Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IMR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_ILR_OFFSET: /* Interrupt level register */
		SC_REPORT_ERROR(name(),
		                "register XIN_ILR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_IVAR_OFFSET: /* Interrupt Vector Address Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IVAR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	default:
		SC_REPORT_ERROR(name(), "register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}

tlm::tlm_response_status Intc::write(ensitlm::addr_t a, ensitlm::data_t d) {
	switch (a) {
	case XIN_ISR_OFFSET: /* Interrupt Status Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_ISR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_IPR_OFFSET: /* Interrupt Pending Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IPR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_IER_OFFSET: /* Interrupt Enable Register */
	{
		const ensitlm::data_t old_d = d;
		m_enabled_it = d;
		// On n'envoie une IRQ que si l'une de celles qu'on
		// vient de démasquer est active
		send_irq_maybe(d & ~(old_d));
		break;
	}
	case XIN_IAR_OFFSET: /* Interrupt Acknowledge Register */
		m_active_it &= ~d;
		send_irq_maybe(d);
		break;
	case XIN_SIE_OFFSET: /* Set Interrupt Enable Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_SIE_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_CIE_OFFSET: /* Clear Interrupt Enable Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_CIE_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_IVR_OFFSET: /* Interrupt Vector Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IVR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_MER_OFFSET: /* Master Enable Register */
		/* Not implemented, we assume the Intc is active all
		   the time (TODO: unlike the real hardware). */
		break;
	case XIN_IMR_OFFSET: /* Interrupt Mode Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IMR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_ILR_OFFSET: /* Interrupt level register */
		SC_REPORT_ERROR(name(),
		                "register XIN_ILR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	case XIN_IVAR_OFFSET: /* Interrupt Vector Address Register */
		SC_REPORT_ERROR(name(),
		                "register XIN_IVAR_OFFSET not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
		break;
	default:
		SC_REPORT_ERROR(name(), "register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}
