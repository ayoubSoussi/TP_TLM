#include "ensitlm.h"
#include "native_wrapper.h"

/*
 * The C compiler does a bit of magic on the main() function. Trick it
 * by changing the name to something else.
 */
#define main __start

/* extern "C" is needed since the software is compiled in C and
 * is linked against native_wrapper.cpp, which is compiled in C++.
 */
extern "C" int main();
extern "C" void interrupt_handler();

extern "C" void hal_write32(uint32_t addr, uint32_t data) {
	abort(); // TODO
}

extern "C" unsigned int hal_read32(uint32_t addr) {
	abort(); // TODO
}

extern "C" void hal_cpu_relax() {
	abort(); // TODO
}

extern "C" void hal_wait_for_irq() {
	abort(); // TODO
}

/* To keep it simple, the soft wrapper is a singleton, we can
 * call its methods in a simple manner, using
 * NativeWrapper::get_instance()->method_name()
 */
NativeWrapper * NativeWrapper::get_instance() {
	static NativeWrapper * instance = NULL;
	if (!instance)
		instance = new NativeWrapper("native_wrapper");
	return instance;
}

NativeWrapper::NativeWrapper(sc_core::sc_module_name name) : sc_module(name),
							     irq("irq")
{
	abort(); // TODO
}

void NativeWrapper::hal_write32(unsigned int addr, unsigned int data)
{
	abort(); // TODO
}

unsigned int NativeWrapper::hal_read32(unsigned int addr)
{
	abort(); // TODO
}

void NativeWrapper::hal_cpu_relax()
{
	abort(); // TODO
}

void NativeWrapper::hal_wait_for_irq()
{
	abort(); // TODO
}

void NativeWrapper::compute()
{
	abort(); // TODO
}

void NativeWrapper::interrupt_handler_internal()
{
	abort(); // TODO
}
