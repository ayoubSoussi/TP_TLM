#include "ensitlm.h"
#include "timer.h"
#include "offsets/timer.h"
#include "bit_manipulation.h"

#define DEBUG

TIMER::TIMER(sc_core::sc_module_name name, sc_core::sc_time p)
    : sc_core::sc_module(name), period(p) {
	refresh[0] = refresh[1] = false;
	SC_METHOD(timer0);
	dont_initialize();
	sensitive << csr_event[0];

	SC_METHOD(timer1);
	dont_initialize();
	sensitive << csr_event[1];

	SC_THREAD(interrupt);
	for (int timer = 0; timer < 2; ++timer) {
		csr[timer] = 0;
		tlr[timer] = 0;
		tcr[timer] = 0;
	}

#ifdef DEBUG
	std::cout << "Debug: " << sc_module::name()
	          << ": Xilinx AXI Timer/Counter (v1.00) TLM model\n";
#endif
}

void TIMER::interrupt() {
	while (true) {
		wait(irq_event);
#ifdef DEBUG
		std::cout << "Debug: " << name() << ": interrupt @ "
		          << sc_core::sc_time_stamp() << "\n";
#endif
		irq.write(1);
		wait(period);
		irq.write(0);
	}
}

void TIMER::timer(int timer) {
	if (refresh[timer]) {
		refresh[timer] = false;
	refresh:
		if (TEST_BIT(csr[timer], TIMER_UDT)) { // down
			csr_event[timer].notify(
			    period * (((unsigned long long)tcr[timer]) + 2));
		} else { // up
			csr_event[timer].notify(
			    period * (0xFFFFFFFF -
			              ((unsigned long long)tcr[timer]) + 2));
		}
		return;
	}
	SET_BIT(csr[timer], TIMER_TINT);        // interrupt
	if (TEST_BIT(csr[timer], TIMER_ENIT)) { // enable interrupt
		irq_event.notify();
	}
	if (TEST_BIT(csr[timer], TIMER_ARHT)) { // auto reload
		tcr[timer] = tlr[timer];
		goto refresh;
	} else {                                       // hold
		if (TEST_BIT(csr[timer], TIMER_UDT)) { // down
			tcr[timer] = -1;
		} else { // up
			tcr[timer] = 0;
		}
	}
}

void TIMER::timer0() {
	timer(0);
}

void TIMER::timer1() {
	timer(1);
}

tlm::tlm_response_status TIMER::read(ensitlm::addr_t a, ensitlm::data_t &d) {
	int timer = 0;
	switch (a) {
	case TIMER_0_CSR_OFFSET:
	CSR:
		d = csr[timer];
		break;
	case TIMER_0_TLR_OFFSET:
	TLR:
		d = tlr[timer];
		break;
	case TIMER_0_TCR_OFFSET:
	TCR:
		d = tcr[timer];
		break;
	case TIMER_1_CSR_OFFSET:
		timer = 1;
		goto CSR;
	case TIMER_1_TLR_OFFSET:
		timer = 1;
		goto TLR;
	case TIMER_1_TCR_OFFSET:
		timer = 1;
		goto TCR;
	default:
		SC_REPORT_ERROR(name(), "register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}

tlm::tlm_response_status TIMER::write(ensitlm::addr_t a, ensitlm::data_t d) {
	int timer = 0;
	switch (a) {
	case TIMER_0_CSR_OFFSET:
	CSR:
		if (TEST_BIT(d, TIMER_TINT)) { // Interrupt
			CLEAR_BIT(d, TIMER_TINT);
			CLEAR_BIT(csr[timer], TIMER_TINT);
#ifdef DEBUG
			std::cout << "Debug: " << name() << ": clean interrupt "
			          << timer << "\n";
#endif
		}
		if (TEST_BIT(d, TIMER_ENT)) { // Enable Timer
			CLEAR_BIT(d, TIMER_ENT);
			SET_BIT(csr[timer], TIMER_ENT);
			refresh[timer] = true;
			csr_event[timer].notify();
#ifdef DEBUG
			std::cout << "Debug: " << name() << ": enable " << timer
			          << "\n";
#endif
		} else {                                       // Disable Timer
			if (TEST_BIT(csr[timer], TIMER_ENT)) { // was enabled
				CLEAR_BIT(csr[timer], TIMER_ENT);
				csr_event[timer].cancel();
#ifdef DEBUG
				std::cout << "Debug: " << name() << ": disable "
				          << timer << "\n";
#endif
			}
		}
		if (TEST_BIT(d, TIMER_ENIT)) { // Enable Interrupt
			CLEAR_BIT(d, TIMER_ENIT);
			SET_BIT(csr[timer], TIMER_ENIT);
		} else { // Disable Interrupt
			CLEAR_BIT(csr[timer], TIMER_ENIT);
		}
		if (TEST_BIT(d, TIMER_LOAD)) { // Load
			CLEAR_BIT(d, TIMER_LOAD);
			if (TEST_BIT(csr[timer], TIMER_ENT)) { // enabled
				refresh[timer] = true;
				csr_event[timer].notify();
			}
#ifdef DEBUG
			std::cout << "Debug: " << name() << ": load " << timer
			          << "\n";
#endif
			tcr[timer] = tlr[timer];
		}
		if (TEST_BIT(d, TIMER_ARHT)) { // Auto Reload
			CLEAR_BIT(d, TIMER_ARHT);
			SET_BIT(csr[timer], TIMER_ARHT);
		} else { // Hold Timer
			CLEAR_BIT(csr[timer], TIMER_ARHT);
		}
		if (TEST_BIT(d, TIMER_UDT)) { // Up
			CLEAR_BIT(d, TIMER_UDT);
			SET_BIT(csr[timer], TIMER_UDT);
		} else { // Down
			CLEAR_BIT(csr[timer], TIMER_UDT);
		}
		if (d) {
			SC_REPORT_WARNING(name(),
			                  "invalid bits in write to CSR");
		}
		break;
	case TIMER_0_TLR_OFFSET:
	TLR:
		tlr[timer] = d;
		break;
	case TIMER_0_TCR_OFFSET:
	TCR:
		SC_REPORT_ERROR(name(), "TCR is read only");
		break;
	case TIMER_1_CSR_OFFSET:
		timer = 1;
		goto CSR;
	case TIMER_1_TLR_OFFSET:
		timer = 1;
		goto TLR;
	case TIMER_1_TCR_OFFSET:
		timer = 1;
		goto TCR;
	default:
		SC_REPORT_ERROR(name(), "register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}
