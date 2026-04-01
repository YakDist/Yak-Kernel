#include <limine-generic/request.h>
#include <limine.h>
#include <uacpi/uacpi.h>
#include <yak/arch-mm.h>
#include <yak/vm/address.h>

namespace yak {

extern "C" {
LIMINE_REQ static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
  auto addr = rsdp_request.response == NULL
                  ? 0
                  : arch::v2p((vaddr_t) rsdp_request.response->address);

  if (addr == 0) {
    return UACPI_STATUS_NOT_FOUND;
  }

  *out_rsdp_address = addr;
  return UACPI_STATUS_OK;
}
}

} // namespace yak
